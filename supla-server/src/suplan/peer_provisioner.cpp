// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include "suplan/peer_provisioner.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <utility>
#include <vector>
namespace supla_suplan {
static bool current_topology(Repository *repo, const Association &a,
                             const Identity &identity,
                             bool *read_failed = nullptr) {
  if (read_failed) *read_failed = false;
  for (const auto &entry : a.acl) {
    if (entry.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
        std::find(identity.channels.begin(), identity.channels.end(),
                  static_cast<uint32_t>(entry.ResourceId)) ==
            identity.channels.end())
      return false;
  }
  bool owned = false;
  if (!repo->check_acl_ownership(a.source, a.destination, a.acl, &owned)) {
    if (read_failed) *read_failed = true;
    return false;
  }
  return owned;
}
void wipe_key(void *buffer, size_t size) {
  volatile unsigned char *p = static_cast<volatile unsigned char *>(buffer);
  while (size--) *p++ = 0;
}
PeerProvisioner::PeerProvisioner(PeerService::Factory factory,
                                 PeerTransport *transport, Clock clock)
    : factory(factory),
      transport(transport),
      // NOLINTNEXTLINE(whitespace/indent_namespace)
      service(factory, [this](uint64_t id) { reconcile(id); }),
      clock(clock) {}
uint64_t PeerProvisioner::time_left_usec(int device) {
  std::lock_guard<std::mutex> lock(flows_mutex);
  auto next = Time::max();
  auto found = deadlines.find(device);
  if (found != deadlines.end())
    for (const auto &entry : found->second) next = std::min(next, entry.second);
  auto queued = waiting.find(device);
  auto active = slots.find(device);
  if (queued != waiting.end() && !queued->second.empty() &&
      (active == slots.end() || active->second.size() < window))
    return 1;
  for (const auto &entry : reconnect_due)
    if (entry.second.owner == device) next = std::min(next, entry.second.due);
  if (next == Time::max()) return 0;
  return next <= clock()
             ? 1
             : std::max<int64_t>(
                   1, std::chrono::duration_cast<std::chrono::microseconds>(
                          next - clock())
                          .count());
}
void PeerProvisioner::schedule(uint64_t id) {
  // Association gate held. Never acquire a Device/SRPC lock under flows_mutex.
  auto f = flow(id, false);
  std::map<int, Time> next;
  if (f) {
    for (bool source : {true, false}) {
      const auto &side = source ? f->source : f->destination;
      if (side.pending)
        next[source ? f->desired.source : f->desired.destination] =
            side.deadline;
    }
    if (f->refresh_at != Time()) {
      int device = transport->ready(f->desired.source) ? f->desired.source
                   : transport->ready(f->desired.destination)
                       ? f->desired.destination
                       : transport->recovery_device();
      if (device) {
        auto found = next.find(device);
        next[device] = found == next.end()
                           ? f->refresh_at
                           : std::min(found->second, f->refresh_at);
      }
    }
  }
  std::vector<int> notify;
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    if (f) {
      for (bool source : {true, false}) {
        auto &side = source ? f->source : f->destination;
        int device = source ? f->desired.source : f->desired.destination;
        auto incident = exhausted.find(device);
        if (!side.acknowledged && terminated_sessions.count(device) &&
            incident != exhausted.end() && incident->second.count(id))
          side.reconnect_spent = true;
      }
    }
    for (auto *index : {&slots, &waiting, &exhausted}) {
      for (auto it = index->begin(); it != index->end();) {
        if (it->second.erase(id)) notify.push_back(it->first);
        if (it->second.empty())
          it = index->erase(it);
        else
          ++it;
      }
    }
    if (f) {
      for (bool source : {true, false}) {
        const auto &side = source ? f->source : f->destination;
        int device = source ? f->desired.source : f->desired.destination;
        if (side.pending) slots[device].insert(id);
        if (side.waiting) waiting[device].insert(id);
        if (side.reconnect_spent) exhausted[device].insert(id);
      }
    }
    for (auto it = deadlines.begin(); it != deadlines.end();) {
      it->second.erase(id);
      if (it->second.empty())
        it = deadlines.erase(it);
      else
        ++it;
    }
    for (const auto &entry : next) {
      deadlines[entry.first][id] = entry.second;
      notify.push_back(entry.first);
    }
    if (!f) {
      for (auto it = pairs.begin(); it != pairs.end();) {
        if (it->second == id) it = pairs.erase(it);
        else ++it;
      }
    }
    // No completed incidents accumulate permanent per-device bookkeeping.
    for (auto it = terminated_sessions.begin();
         it != terminated_sessions.end();) {
      if (!exhausted.count(it->first))
        it = terminated_sessions.erase(it);
      else
        ++it;
    }
  }
  for (int device : notify) transport->wake(device, transport->session(device));
}
void PeerProvisioner::defer_reconnect(int device) {
  int owner = transport->ready(device) ? device : transport->recovery_device();
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    reconnect_due[device] = {owner, clock() + std::chrono::seconds(5)};
  }
  if (owner) transport->wake(owner, transport->session(owner));
}
void PeerProvisioner::iterate(int device) {
  std::vector<uint64_t> due;
  std::vector<int> recovery;
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    for (const auto &target : deadlines) {
      if (device && target.first != device) continue;
      for (const auto &entry : target.second)
        if (entry.second <= clock()) due.push_back(entry.first);
    }
    for (const auto &target : waiting) {
      if (device && target.first != device) continue;
      auto active = slots.find(target.first);
      if (active == slots.end() || active->second.size() < window)
        due.insert(due.end(), target.second.begin(), target.second.end());
    }
    for (auto it = reconnect_due.begin(); it != reconnect_due.end();) {
      if ((!device || it->second.owner == device) &&
          it->second.due <= clock()) {
        recovery.push_back(it->first);
        it = reconnect_due.erase(it);
      } else {
        ++it;
      }
    }
  }
  for (int target : recovery) reconnect(target);
  std::sort(due.begin(), due.end());
  due.erase(std::unique(due.begin(), due.end()), due.end());
  unsigned work = 0;
  for (uint64_t id : due) {
    auto gate = association_gate(id);
    std::unique_lock<std::mutex> lock(*gate, std::try_to_lock);
    if (!lock) continue;
    Schedule update{this, id};
    auto f = flow(id, false);
    if (!f) continue;
    bool db = f->refresh_at != Time() && clock() >= f->refresh_at;
    if (db) {
      f->refresh_at = {};
      reconcile_locked(id, false);
    } else {
      if (f->source.waiting || f->destination.waiting)
        reconcile_locked(id, false);
      for (bool source : {true, false}) {
        auto &side = source ? f->source : f->destination;
        if (!side.pending || clock() < side.deadline) continue;
        auto repo = factory();
        Association a;
        auto state = current(repo.get(), *f, &a);
        if (state == Current::DatabaseError) {
          db_failure(f.get(), source);
        } else if (state == Current::Stale) {
          side.pending = false;
          reconcile_locked(id, false);
          break;
        } else {
          no_response(f.get(), source);
        }
      }
    }
    if (++work == 16) break;
  }
}
std::shared_ptr<PeerProvisioner::Flow> PeerProvisioner::flow(uint64_t id,
                                                             bool create) {
  std::lock_guard<std::mutex> lock(flows_mutex);
  auto it = flows.find(id);
  if (it != flows.end()) return it->second;
  if (!create) return nullptr;
  // Bounded volatile diagnostic/pending bookkeeping. No payload/key queue.
  if (flows.size() >= 4096) {
    transport->diagnostic(id, 0, SUPLA_SUPLAN_RESULT_CAPACITY_EXCEEDED);
    return nullptr;
  }
  auto f = std::make_shared<Flow>();
  flows[id] = f;
  return f;
}
void PeerProvisioner::retire_completed(uint64_t id,
                                      const std::shared_ptr<Flow> &f) {
  if (f->refresh_at != Time()) return;
  // Caller holds the association gate. Finish phase transitions/forwarding
  // before this check: their pending replies still need the duplicate fences.
  for (const Side *side : {&f->source, &f->destination}) {
    if (side->waiting || side->pending || side->blocked || side->committing ||
        (side->sent && !side->acknowledged))
      return;
  }
  // Offline/unsent sides need no resident work: reconnect reconstructs them
  // from DB. Keep unfinished failed/unknown work's existing retry policy.
  std::lock_guard<std::mutex> lock(flows_mutex);
  auto it = flows.find(id);
  if (it != flows.end() && it->second == f) flows.erase(it);
}
bool PeerProvisioner::accepted(int device, const Identity &identity) {
  auto repo = factory();
  return repo->accept_identity(device, identity);
}
void PeerProvisioner::reconnect(int device) {
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    reconnect_due.erase(device);
    slots.erase(device);
  }
  std::vector<ApprovalPair> failed;
  if (!service.reconcile_approvals(0, device, &failed)) {
    transport->diagnostic(0, device, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
    defer_reconnect(device);
    if (failed.empty()) return;  // Unknown scope: fail closed.
  }
  auto repo = factory();
  std::vector<uint64_t> ids;
  if (!repo->for_device(device, false, &ids)) {
    transport->diagnostic(0, device, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
    defer_reconnect(device);
    return;
  }
  for (auto id : ids) {
    Association a;
    int owner = 0;
    if (!repo->load(id, &a)) {
      defer_reconnect(device);
      continue;
    }
    if (!a.id) continue;
    // Do not publish stale authorization for a failed pair, but keep
    // independent associations converging (including revocations).
    if (std::any_of(failed.begin(), failed.end(), [&](const ApprovalPair &p) {
          return p.source == a.source && p.destination == a.destination;
        })) continue;
    // Recover missed deletion notifications after Server downtime. Query
    // failure is never interpreted as authoritative endpoint/resource removal.
    if (!repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, a.source, &owner)) {
      defer_reconnect(device);
      continue;
    }
    if (!owner && !a.source_removed) service.remove_device(a.source);
    if (!repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, a.destination,
                     &owner)) {
      defer_reconnect(device);
      continue;
    }
    if (!owner && !a.destination_removed) service.remove_device(a.destination);
    for (const auto &entry : a.acl) {
      if (repo->owner(entry.ResourceType, entry.ResourceId, &owner) &&
          owner != a.source)
        service.delete_resource(id, entry.ResourceType, entry.ResourceId);
    }
    service.observe_root(id);
    reconcile(id, true);
  }
}
PeerProvisioner::Current PeerProvisioner::current(Repository *repo,
                                                  const Flow &f,
                                                  Association *a) {
  Identity identity;
  if (!repo->load(f.desired.id, a)) return Current::DatabaseError;
  if (!a->id || !same_state(*a, f.desired)) return Current::Stale;
  if (!repo->identity(a->source, &identity)) return Current::DatabaseError;
  if (a->lifecycle == Lifecycle::Draining) return Current::Yes;
  if (identity.root != f.wire_root ||
      !transport->owner_ready(a->source, identity.root))
    return Current::Stale;
  bool failed = false;
  if (!current_topology(repo, *a, identity, &failed))
    return failed ? Current::DatabaseError : Current::Stale;
  return Current::Yes;
}
void PeerProvisioner::db_failure(Flow *f, bool source) {
  auto &side = source ? f->source : f->destination;
  bool spent = side.reconnect_spent;
  if (side.key_requested) {
    f->force_key = true;
    if (!source && !f->source.pending && !f->source.blocked) {
      bool source_spent = f->source.reconnect_spent;
      f->source = {};
      f->source.reconnect_spent = source_spent;
    }
  }
  side = {};
  side.reconnect_spent = spent;
  f->refresh_at = clock() + std::chrono::seconds(5);
  transport->diagnostic(f->desired.id, 0,
                        SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
}
void PeerProvisioner::no_response(Flow *f, bool source, bool timeout) {
  auto &side = source ? f->source : f->destination;
  int device = source ? f->desired.source : f->desired.destination;
  side.pending = false;
  side.blocked = true;
  transport->transient_diagnostic(f->desired.id, device, timeout);
  if (!side.session) return;
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    if (side.reconnect_spent && terminated_sessions.count(device)) return;
    side.reconnect_spent = false;
    auto incidents = exhausted.find(device);
    if (incidents != exhausted.end() &&
        terminated_sessions.count(device) &&
        (incidents->second.size() > 1 ||
         !incidents->second.count(f->desired.id))) {
      side.reconnect_spent = true;
      return;
    }
    auto found = terminated_sessions.find(device);
    if (found != terminated_sessions.end() && found->second == side.session) {
      side.reconnect_spent = true;
      return;
    }
    // Reserve while holding only the map lock: concurrent association failures
    // cannot both terminate the same connection. No Device lock under this
    // lock.
    // All requests already queued on this connection share its reconnect.
    auto active = slots.find(device);
    if (active != slots.end())
      exhausted[device].insert(active->second.begin(), active->second.end());
    exhausted[device].insert(f->desired.id);
    terminated_sessions[device] = side.session;
  }
  side.reconnect_spent = transport->reconnect_session(device, side.session);
  if (!side.reconnect_spent) {
    std::lock_guard<std::mutex> lock(flows_mutex);
    auto found = terminated_sessions.find(device);
    if (found != terminated_sessions.end() && found->second == side.session)
      terminated_sessions.erase(found);
    exhausted.erase(device);
  }
}
void PeerProvisioner::sent(Flow *f, bool source,
                           PeerTransport::SendResult result) {
  auto &side = source ? f->source : f->destination;
  side.session = result.session;
  side.pending = result.code == PeerTransport::SendCode::Queued;
  if (side.pending) {
    side.deadline = clock() + std::chrono::seconds(15);
  } else if (result.code == PeerTransport::SendCode::EnqueueFailed) {
    no_response(f, source, false);
  } else {
    side.sent = false;
    // A root/bootstrap race waits for its authoritative registration event.
    side.blocked = result.code == PeerTransport::SendCode::ContextChanged;
  }
}
bool PeerProvisioner::reserve(Flow *f, bool source) {
  auto &side = source ? f->source : f->destination;
  int device = source ? f->desired.source : f->desired.destination;
  std::lock_guard<std::mutex> lock(flows_mutex);
  auto &active = slots[device];
  auto queued = waiting.find(device);
  bool older = queued != waiting.end() && !queued->second.empty() &&
               *queued->second.begin() < f->desired.id;
  side.waiting = !active.count(f->desired.id) &&
                 (active.size() >= window || older);
  if (side.waiting) return false;
  active.insert(f->desired.id);
  return true;
}
void PeerProvisioner::send_source(Flow *f, bool key) {
  if (f->source.sent || f->source.blocked ||
      !transport->ready(f->desired.source) || !reserve(f, true))
    return;
  TSDS_SuplaSetSuplanSourceAssociation q = {};
  q.PeerContext = context(f->desired, f->wire_root);
  q.AclRevision = f->desired.revision;
  q.Flags = key ? SUPLA_SUPLAN_SOURCE_FLAG_RETURN_PEER_KEY : 0;
  q.AclEntryCount = f->desired.acl.size();
  std::copy(f->desired.acl.begin(), f->desired.acl.end(), q.Acl);
  f->source.sent = true;
  f->source.key_requested = key;
  if (!key) f->source_no_key_sent = true;
  sent(f, true, transport->send(f->desired.source, &q));
}
void PeerProvisioner::send_destination(Flow *f, const unsigned char *key) {
  if (f->destination.sent || f->destination.blocked ||
      !transport->ready(f->desired.destination) || !reserve(f, false))
    return;
  TSDS_SuplaSetSuplanDestinationAssociation q = {};
  q.PeerContext = context(f->desired, f->wire_root);
  q.AclRevision = f->desired.revision;
  q.PeerKeySize = key ? SUPLA_SUPLAN_PEER_KEY_SIZE : 0;
  if (key) memcpy(q.PeerKey, key, sizeof(q.PeerKey));
  q.ResourceCount = f->desired.acl.size();
  std::copy(f->desired.acl.begin(), f->desired.acl.end(), q.Resources);
  f->destination.sent = true;
  f->destination.key_requested = key != nullptr;
  auto outcome = transport->send(f->desired.destination, &q);
  // SRPC copies/serializes synchronously into the existing connection buffer.
  // No key is retained for retries: reconnect requests deterministic rederive.
  wipe_key(q.PeerKey, sizeof(q.PeerKey));
  sent(f, false, outcome);
}
void PeerProvisioner::reconcile(uint64_t id, bool meaningful_event) {
  auto gate = association_gate(id);
  std::lock_guard<std::mutex> lock(*gate);
  reconcile_locked(id, meaningful_event);
}
void PeerProvisioner::reconcile_locked(uint64_t id, bool meaningful_event) {
  Schedule update{this, id};
  auto repo = factory();
  Association a;
  Identity identity;
  auto defer = [&] {
    auto f = flow(id);
    if (f) {
      if (!f->desired.id) f->desired = a;
      f->source.waiting = f->destination.waiting = false;
      f->refresh_at = clock() + std::chrono::seconds(5);
    }
  };
  if (!repo->load(id, &a)) {
    defer();
    transport->diagnostic(id, 0, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
    return;
  }
  if (!a.id || a.lifecycle == Lifecycle::Dormant) {
    std::lock_guard<std::mutex> map_lock(flows_mutex);
    flows.erase(id);
    return;
  }
  if (!repo->identity(a.source, &identity)) {
    defer();
    return;
  }
  uint32_t root =
      a.lifecycle == Lifecycle::Draining && a.root ? a.root : identity.root;
  auto park = [&] {
    auto existing = flow(id, false);
    if (existing) {
      existing->source.waiting = existing->destination.waiting = false;
      retire_completed(id, existing);
    }
  };
  if (!root || root == UINT32_MAX) {
    park();
    return;
  }
  if (a.lifecycle == Lifecycle::Active) {
    if (!identity.root || !transport->owner_ready(a.source, identity.root)) {
      park();
      return;
    }
    // Both current DB ownership and accepted owner identity must agree. Never
    // replay a newly allocated/stale ChannelId merely because it exists in DB.
    bool read_failed = false;
    if (!current_topology(repo.get(), a, identity, &read_failed)) {
      if (read_failed) defer();
      else
        park();
      transport->diagnostic(id, a.source,
                            read_failed ? SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR
                                        : SUPLA_SUPLAN_RESULT_NOT_FOUND);
      return;
    }
  }
  auto f = flow(id);
  if (!f) return;
  bool unchanged = same_state(f->desired, a) && f->wire_root == root;
  bool same_revision = f->wire_root == root &&
                       f->desired.source == a.source &&
                       f->desired.destination == a.destination &&
                       f->desired.generation == a.generation &&
                       f->desired.revision == a.revision;
  if (!same_revision) {
    std::lock_guard<std::mutex> lock(flows_mutex);
    for (int device : {a.source, a.destination}) {
      auto found = exhausted.find(device);
      if (found != exhausted.end()) found->second.erase(id);
    }
  }
  if (!unchanged || meaningful_event) {
    // Destination reconnect must not overlap a still-live Source request
    // with a different RETURN_PEER_KEY flag: the frozen reply has no flag echo.
    Side pending_source;
    Side conflict_destination;
    // Neither reconnect nor provisioned-root bookkeeping repairs a conflict
    // at the same wire context/revision. A changed payload alone is not repair.
    if (same_revision && f->source.conflict) pending_source = f->source;
    if (same_revision && f->destination.conflict)
      conflict_destination = f->destination;
    bool force_key = false;
    bool source_no_key_sent = unchanged && f->source_no_key_sent;
    if (unchanged && f->source.pending &&
        f->source.session == transport->session(a.source) &&
        clock() < f->source.deadline) {
      pending_source = f->source;
      force_key = f->force_key;
    }
    auto spent = [&](const Side &side, int device) {
      if (!same_revision) return false;
      std::lock_guard<std::mutex> lock(flows_mutex);
      auto it = terminated_sessions.find(device);
      return it != terminated_sessions.end() &&
             (side.reconnect_spent ||
              (side.session && it->second == side.session));
    };
    bool source_spent = spent(f->source, a.source);
    bool destination_spent = spent(f->destination, a.destination);
    *f = {};
    f->desired = a;
    f->wire_root = root;
    f->source = pending_source;
    f->destination = conflict_destination;
    f->source.reconnect_spent = source_spent;
    f->destination.reconnect_spent = destination_spent;
    f->force_key = force_key;
    f->source_no_key_sent = source_no_key_sent;
  }
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    pairs[{a.source, a.destination}] = id;
  }
  if (a.lifecycle == Lifecycle::Draining) {
    if (a.source_acked || a.source_removed || identity.root != root)
      f->source = {};
    if (a.destination_acked || a.destination_removed) f->destination = {};
    if (!a.source_acked && !a.source_removed && identity.root == root)
      send_source(f.get(), false);
    if (!a.destination_acked && !a.destination_removed)
      send_destination(f.get(), nullptr);
    retire_completed(id, f);
    return;
  }
  bool new_context = a.root != root ||
                     a.provisioned_generation != a.generation || f->force_key;
  if (new_context) {
    bool consumer = transport->ready(a.destination);
    if (consumer && f->source.pending && !f->source.key_requested)
      f->force_key = true;
    // Source can sync while Destination is offline, without requesting a key.
    if (consumer && f->source.sent && !f->source.key_requested &&
        f->source.acknowledged && !f->source.pending)
      f->source = {};
    send_source(f.get(), consumer);
  } else {
    send_source(f.get(), false);
    send_destination(f.get(), nullptr);
  }
  retire_completed(id, f);
}
void PeerProvisioner::result(Flow *f, bool source, uint8_t code) {
  Side &side = source ? f->source : f->destination;
  side.pending = false;
  side.acknowledged = code == SUPLA_SUPLAN_RESULT_OK;
  if (code == SUPLA_SUPLAN_RESULT_OK) {
    side.reconnect_spent = false;
    side.session = 0;
    return;
  }
  if (!source && code == SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED &&
      f->desired.lifecycle == Lifecycle::Active && !f->key_recovery_started &&
      !f->source.blocked) {
    f->key_recovery_started = true;
    f->force_key = true;
    bool destination_spent = f->destination.reconnect_spent;
    f->destination = {};
    f->destination.reconnect_spent = destination_spent;
    if (!f->source.pending) {
      f->source = {};
      send_source(f, transport->ready(f->desired.destination));
    }
    return;
  }
  side.blocked = true;
  side.conflict = code == SUPLA_SUPLAN_RESULT_REVISION_CONFLICT;
  // Permanent errors need a meaningful lifecycle event; conflicts additionally
  // require a changed authoritative snapshot, not an identical reconnect.
  transport->diagnostic(
      f->desired.id, source ? f->desired.source : f->desired.destination, code);
  if (source && code == SUPLA_SUPLAN_RESULT_ROOT_EPOCH_MISMATCH)
    transport->rebootstrap(f->desired.source);
}
uint64_t PeerProvisioner::correlate(int device,
                                    const TSuplaSuplanPeerContext &ctx,
                                    uint32_t revision, bool source,
                                    uint64_t session) {
  uint64_t id = 0;
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    auto found = pairs.find({static_cast<int>(ctx.SourceNodeId),
                             static_cast<int>(ctx.DestinationNodeId)});
    if (found == pairs.end()) return 0;
    id = found->second;
  }
  auto gate = association_gate(id);
  std::lock_guard<std::mutex> lock(*gate);
  auto f = flow(id, false);
  if (!f) return 0;
  const auto &side = source ? f->source : f->destination;
  if (!side.pending || side.committing || side.session != session ||
      device != (source ? f->desired.source : f->desired.destination)) return 0;
  auto wire = f->desired;
  wire.root = f->wire_root;
  return matches(ctx, wire) && revision == wire.revision ? id : 0;
}

void PeerProvisioner::on_source(int device,
                                TDS_SuplaSetSuplanSourceAssociationResult *q,
                                uint64_t session) {
  if (!q) return;
  struct KeyWiper {
    unsigned char *key;
    ~KeyWiper() { wipe_key(key, SUPLA_SUPLAN_PEER_KEY_SIZE); }
  } wiper{q->PeerKey};
  if (!session) session = transport->session(device);
  auto id = correlate(device, q->PeerContext, q->AclRevision, true, session);
  if (!id) return;
  auto repo = factory();
  Association pair;
  Association cleanup;
  bool ack = false;
  {
    auto gate = association_gate(id);
    std::lock_guard<std::mutex> lock(*gate);
    Schedule update{this, id};
    auto f = flow(id, false);
    Association a;
    if (!f || !f->source.pending || f->source.committing ||
        f->source.session != session)
      return;
    auto wire = f->desired;
    wire.root = f->wire_root;
    if (!matches(q->PeerContext, wire) || q->AclRevision != wire.revision)
      return;
    if (q->Result == SUPLA_SUPLAN_RESULT_OK &&
        q->PeerKeySize !=
            (f->source.key_requested ? SUPLA_SUPLAN_PEER_KEY_SIZE : 0)) {
      // RETURN_PEER_KEY is not echoed in the frozen result. A no-key OK from
      // an earlier send (also after timeout) may arrive while rederive is
      // pending at this context/revision. It cannot consume the key reply.
      if (f->source.key_requested && !q->PeerKeySize &&
          f->source_no_key_sent)
        return;
      result(f.get(), true, SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT);
      return;
    }
    bool found = repo->find(q->PeerContext.SourceNodeId,
                            q->PeerContext.DestinationNodeId, &pair);
    auto state = found ? current(repo.get(), *f, &a) : Current::DatabaseError;
    if (state == Current::DatabaseError) {
      db_failure(f.get(), true);
      return;
    }
    if (!pair.id || pair.id != id || state == Current::Stale) {
      f->source.pending = false;
      reconcile_locked(id, false);
      return;
    }
    if (q->Result != SUPLA_SUPLAN_RESULT_OK) {
      result(f.get(), true, q->Result);
      return;
    }
    if (a.lifecycle == Lifecycle::Draining) {
      cleanup = a;
      ack = true;
      f->source.pending = false;
      f->source.committing = true;
    } else {
      // Commit the accepted Source credential context, then drop the DB lock
      // before forwarding. Recheck the exact state while holding the send gate.
      if (!repo->begin(a.id, &pair) || !same_state(pair, a)) {
        repo->rollback();
        db_failure(f.get(), true);
        return;
      }
      bool changed = pair.root != f->wire_root ||
                     pair.provisioned_generation != pair.generation;
      pair.root = f->wire_root;
      pair.provisioned_generation = pair.generation;
      if ((changed && !repo->save(pair)) || !repo->commit()) {
        repo->rollback();
        db_failure(f.get(), true);
        return;
      }
      f->desired = pair;
      bool forward = f->source.key_requested;
      bool rederive = f->force_key && transport->ready(pair.destination);
      if ((forward || rederive) &&
          current(repo.get(), *f, &a) != Current::Yes) {
        // The post-commit fence can fail transiently too. Keep recovery work,
        // not the reply's credential, and rederive after a fresh desired read.
        f->force_key = true;
        db_failure(f.get(), true);
        return;
      }
      result(f.get(), true, q->Result);
      if (forward) {
        f->force_key = false;
        send_destination(f.get(), q->PeerKey);
      } else if (rederive) {
        f->source = {};
        send_source(f.get(), true);
      }
    }
    retire_completed(pair.id, f);
  }
  if (ack) finish_cleanup(cleanup, true, flow(cleanup.id, false));
}
void PeerProvisioner::on_destination(
    int device, const TDS_SuplaSetSuplanDestinationAssociationResult *q,
    uint64_t session) {
  if (!q) return;
  if (!session) session = transport->session(device);
  auto id = correlate(device, q->PeerContext, q->AclRevision, false, session);
  if (!id) return;
  auto repo = factory();
  Association pair, cleanup;
  bool ack = false;
  {
    auto gate = association_gate(id);
    std::lock_guard<std::mutex> lock(*gate);
    Schedule update{this, id};
    auto f = flow(id, false);
    Association a;
    if (!f || !f->destination.pending || f->destination.committing ||
        f->destination.session != session)
      return;
    auto wire = f->desired;
    wire.root = f->wire_root;
    if (!matches(q->PeerContext, wire) || q->AclRevision != wire.revision)
      return;
    bool found = repo->find(q->PeerContext.SourceNodeId,
                            q->PeerContext.DestinationNodeId, &pair);
    auto state = found ? current(repo.get(), *f, &a) : Current::DatabaseError;
    if (state == Current::DatabaseError) {
      db_failure(f.get(), false);
      return;
    }
    if (!pair.id || pair.id != id || state == Current::Stale) {
      f->destination.pending = false;
      reconcile_locked(id, false);
      return;
    }
    // A duplicate response to the preceding keyless Expected replacement
    // cannot cancel the key-bearing replacement now pending at Destination.
    if (q->Result == SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED &&
        f->key_recovery_started && f->destination.key_requested)
      return;
    if (q->Result == SUPLA_SUPLAN_RESULT_OK &&
        a.lifecycle == Lifecycle::Draining) {
      cleanup = a;
      ack = true;
      f->destination.pending = false;
      f->destination.committing = true;
    } else {
      result(f.get(), false, q->Result);
    }
    retire_completed(pair.id, f);
  }
  if (ack) finish_cleanup(cleanup, false, flow(cleanup.id, false));
}
void PeerProvisioner::finish_cleanup(const Association &sent, bool source,
                                     const std::shared_ptr<Flow> &f) {
  // PeerService acquires the same gate and may invoke reconcile after commit.
  // Never call it while holding the gate. Keep the Flow until that commit.
  bool ok = service.cleanup_ack(sent, source);
  auto gate = association_gate(sent.id);
  std::lock_guard<std::mutex> lock(*gate);
  if (!f || flow(sent.id, false) != f || !same_state(f->desired, sent)) return;
  Schedule update{this, sent.id};
  Side &side = source ? f->source : f->destination;
  side.committing = false;
  if (ok)
    result(f.get(), source, SUPLA_SUPLAN_RESULT_OK);
  else
    db_failure(f.get(), source);
  retire_completed(sent.id, f);
}
void PeerProvisioner::disconnected(int device, uint64_t session) {
  // Move error-only inventory recovery off a closing connection if another
  // eligible connection exists. A later natural registration also reloads it.
  int replacement = transport->recovery_device();
  bool signal = false;
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    for (auto &entry : reconnect_due) {
      if (entry.second.owner == device) {
        entry.second.owner = replacement;
        signal = replacement != 0;
      }
    }
  }
  if (signal) transport->wake(replacement, transport->session(replacement));
  std::vector<std::pair<uint64_t, std::shared_ptr<Flow>>> snapshots;
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    for (const auto &item : flows) snapshots.push_back(item);
  }
  for (auto &item : snapshots) {
    auto gate = association_gate(item.first);
    std::lock_guard<std::mutex> lock(*gate);
    if (flow(item.first, false) != item.second) continue;
    Schedule update{this, item.first};
    auto &f = *item.second;
    if (f.desired.source == device && (!session || session == f.source.session))
      f.source.pending = false;
    if (f.desired.destination == device &&
        (!session || session == f.destination.session))
      f.destination.pending = false;
  }
}
}  // namespace supla_suplan
