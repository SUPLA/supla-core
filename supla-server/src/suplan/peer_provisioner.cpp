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
static bool current_topology(repository *repo, const association_t &a,
                             const identity_t &identity,
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
peer_provisioner::peer_provisioner(peer_service::factory_t factory,
                                   peer_transport *transport, clock_t clock)
    : factory(factory),
      transport(transport),
      // NOLINTNEXTLINE(whitespace/indent_namespace)
      service(factory, [this](uint64_t id) { reconcile(id); }),
      clock(clock) {}
uint64_t peer_provisioner::time_left_usec(int device) {
  std::lock_guard<std::mutex> lock(flows_mutex);
  auto next = time_point_t::max();
  auto found = deadlines.find(device);
  if (found != deadlines.end())
    for (const auto &entry : found->second)
      next = std::min(next, entry.second.due);
  for (const auto &entry : reconnect_due)
    if (entry.second.owner == device) next = std::min(next, entry.second.due);
  if (next == time_point_t::max()) return 0;
  return next <= clock()
             ? 1
             : std::max<int64_t>(
                   1, std::chrono::duration_cast<std::chrono::microseconds>(
                          next - clock())
                          .count());
}
void peer_provisioner::schedule(uint64_t id) {
  // Association gate held. Only this flow's endpoints/DB timer owner change.
  auto f = flow(id, false);
  if (!f) return;
  int owner = 0;
  if (f->refresh_at != time_point_t()) {
    owner = transport->ready(f->desired.source) ? f->desired.source
            : transport->ready(f->desired.destination) ? f->desired.destination
                                                      : 0;
    if (!owner) owner = transport->recovery_device();
  }
  std::vector<int> notify;
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    for (int device : {f->desired.source, f->desired.destination,
                       f->recovery_owner, owner}) {
      if (!device) continue;
      auto due = time_point_t::max();
      uint64_t session = 0;
      const auto &side =
          device == f->desired.source ? f->source : f->destination;
      if ((device == f->desired.source || device == f->desired.destination) &&
          side.pending) {
        due = side.deadline;
        session = side.session;
      }
      if (device == owner) due = std::min(due, f->refresh_at);
      auto &entries = deadlines[device];
      if (due == time_point_t::max()) {
        entries.erase(id);
        if (entries.empty()) deadlines.erase(device);
      } else {
        if (!entries.count(id) || due < entries.at(id).due)
          notify.push_back(device);
        entries[id] = {due, session};
      }
    }
    f->recovery_owner = owner;
  }
  for (int device : notify) transport->wake(device, transport->session(device));
}
void peer_provisioner::resolved(int device, uint64_t id) {
  std::lock_guard<std::mutex> lock(flows_mutex);
  auto found = incidents.find(device);
  if (found == incidents.end()) return;
  found->second.associations.erase(id);
  if (found->second.associations.empty()) incidents.erase(found);
}
void peer_provisioner::defer_reconnect(int device) {
  int owner = transport->ready(device) ? device : transport->recovery_device();
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    reconnect_due[device] = {owner, clock() + std::chrono::seconds(5)};
  }
  if (owner) transport->wake(owner, transport->session(owner));
}
void peer_provisioner::iterate(int device) {
  std::vector<uint64_t> due;
  std::vector<int> recovery;
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    for (const auto &target : deadlines) {
      if (device && target.first != device) continue;
      for (const auto &entry : target.second)
        if (entry.second.due <= clock()) due.push_back(entry.first);
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
    // A due request is real work. Join the existing per-association critical
    // section instead of repeatedly rearming an expired timer every 100 ms.
    // No map/Device/SRPC lock is held while waiting.
    std::lock_guard<std::mutex> lock(*gate);
    schedule_guard update{this, id};
    auto f = flow(id, false);
    if (!f) continue;
    bool db = f->refresh_at != time_point_t() && clock() >= f->refresh_at;
    if (db) {
      f->refresh_at = {};
      reconcile_locked(id, false);
    } else {
      for (bool source : {true, false}) {
        auto &side = source ? f->source : f->destination;
        if (!side.pending || clock() < side.deadline) continue;
        auto repo = factory();
        association_t a;
        auto state = current(repo.get(), *f, &a);
        if (state == current_state::database_error) {
          db_failure(f.get(), source);
        } else if (state == current_state::stale) {
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
std::shared_ptr<peer_provisioner::flow_t> peer_provisioner::flow(uint64_t id,
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
  auto f = std::make_shared<flow_t>();
  flows[id] = f;
  return f;
}
void peer_provisioner::retire_completed(uint64_t id,
                                        const std::shared_ptr<flow_t> &f) {
  if (f->refresh_at != time_point_t()) return;
  // Caller holds the association gate. Finish phase transitions/forwarding
  // before this check: their pending replies still need the duplicate fences.
  for (const side_t *side : {&f->source, &f->destination}) {
    if (side->pending || side->blocked || side->committing ||
        (side->sent && !side->acknowledged))
      return;
  }
  // Offline/unsent sides are reconstructed from DB on registration.
  schedule(id);  // Clear only this flow's deadlines before retiring it.
  std::lock_guard<std::mutex> lock(flows_mutex);
  // Keep the exact incident context even when both endpoints are offline.
  // Registration may replay it, but must not renew its reconnect budget.
  for (int device : {f->desired.source, f->desired.destination}) {
    auto incident = incidents.find(device);
    if (incident != incidents.end() &&
        incident->second.associations.count(id)) return;
  }
  auto it = flows.find(id);
  if (it != flows.end() && it->second == f) {
    pairs.erase({f->desired.source, f->desired.destination});
    flows.erase(it);
  }
}
bool peer_provisioner::accepted(int device, const identity_t &identity) {
  auto repo = factory();
  return repo->accept_identity(device, identity);
}
void peer_provisioner::reconnect(int device) {
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    reconnect_due.erase(device);
  }
  auto repo = factory();
  std::vector<uint64_t> ids;
  if (!repo->for_device(device, false, &ids)) {
    transport->diagnostic(0, device, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
    defer_reconnect(device);
    return;
  }
  for (auto id : ids) {
    // A failed cleanup must not leave a transaction open for the next peer.
    repo->rollback();
    association_t a;
    int owner = 0;
    if (!repo->load(id, &a)) {
      defer_reconnect(device);
      continue;
    }
    if (!a.id) continue;
    // No business interpretation on reconnect. Recover technical endpoint
    // removal only from authoritative absence; logical ACL belongs to Cloud.
    if (!repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, a.source, &owner)) {
      defer_reconnect(device);
      continue;
    }
    if (!owner && !a.source_removed && !repo->cleanup_device(a.source)) {
      defer_reconnect(device);
      continue;
    }
    if (!repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, a.destination,
                     &owner)) {
      defer_reconnect(device);
      continue;
    }
    if (!owner && !a.destination_removed &&
        !repo->cleanup_device(a.destination)) {
      defer_reconnect(device);
      continue;
    }
    auto gate = association_gate(id);
    std::lock_guard<std::mutex> lock(*gate);
    repo->observe_root(id);
    // Keep the existing per-association transaction boundary, including a
    // failed observe_root; share the connection, never the transaction.
    repo->rollback();
    reconcile_locked(id, true, repo.get());
  }
}
peer_provisioner::current_state peer_provisioner::current(repository *repo,
                                                          const flow_t &f,
                                                          association_t *a) {
  identity_t identity;
  if (!repo->load(f.desired.id, a)) return current_state::database_error;
  if (!a->id || !same_state(*a, f.desired)) return current_state::stale;
  if (!repo->identity(a->source, &identity))
    return current_state::database_error;
  if (a->lifecycle == lifecycle_t::draining) return current_state::yes;
  if (identity.root != f.wire_root ||
      !transport->owner_ready(a->source, identity.root))
    return current_state::stale;
  bool failed = false;
  if (!current_topology(repo, *a, identity, &failed))
    return failed ? current_state::database_error : current_state::stale;
  return current_state::yes;
}
void peer_provisioner::db_failure(flow_t *f, bool source) {
  auto &side = source ? f->source : f->destination;
  if (side.key_requested) {
    f->force_key = true;
    if (!source && !f->source.pending && !f->source.blocked) {
      f->source = {};
    }
  }
  side = {};
  f->refresh_at = clock() + std::chrono::seconds(5);
  transport->diagnostic(f->desired.id, 0,
                        SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
}
void peer_provisioner::no_response(flow_t *f, bool source, bool timeout) {
  auto &side = source ? f->source : f->destination;
  int device = source ? f->desired.source : f->desired.destination;
  side.pending = false;
  side.blocked = true;
  transport->transient_diagnostic(f->desired.id, device, timeout);
  if (!side.session) return;
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    auto found = incidents.find(device);
    if (found != incidents.end()) {
      found->second.associations.insert(f->desired.id);
      return;
    }
    // One shared incident survives Flow reset and registration. Include all
    // requests sent on the terminated session; unrelated ACKs cannot clear it.
    auto &incident = incidents[device];
    incident.session = side.session;
    auto pending = deadlines.find(device);
    if (pending != deadlines.end())
      for (const auto &entry : pending->second)
        if (entry.second.session == side.session)
          incident.associations.insert(entry.first);
    incident.associations.insert(f->desired.id);
  }
  if (!transport->reconnect_session(device, side.session)) {
    std::lock_guard<std::mutex> lock(flows_mutex);
    auto found = incidents.find(device);
    if (found != incidents.end() && found->second.session == side.session)
      incidents.erase(found);  // No eligible reconnect: no budget spent.
  }
}
void peer_provisioner::sent(flow_t *f, bool source,
                            peer_transport::send_result_t result) {
  auto &side = source ? f->source : f->destination;
  side.session = result.session;
  side.pending = result.code == peer_transport::send_code::queued;
  if (side.pending) {
    side.deadline = clock() + std::chrono::seconds(15);
  } else if (result.code == peer_transport::send_code::enqueue_failed) {
    no_response(f, source, false);
  } else {
    side.sent = false;
    // A root/bootstrap race waits for its authoritative registration event.
    side.blocked = result.code == peer_transport::send_code::context_changed;
  }
}
void peer_provisioner::send_source(flow_t *f, bool key) {
  if (f->source.sent || f->source.blocked ||
      !transport->ready(f->desired.source))
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
void peer_provisioner::send_destination(flow_t *f, const unsigned char *key) {
  if (f->destination.sent || f->destination.blocked ||
      !transport->ready(f->desired.destination))
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
bool peer_provisioner::reconcile(uint64_t id, bool meaningful_event) {
  auto gate = association_gate(id);
  std::lock_guard<std::mutex> lock(*gate);
  return reconcile_locked(id, meaningful_event);
}
bool peer_provisioner::reconcile_locked(uint64_t id, bool meaningful_event,
                                       repository *repo) {
  schedule_guard update{this, id};
  auto local = repo ? nullptr : factory();
  if (!repo) repo = local.get();
  association_t a;
  identity_t identity;
  auto defer = [&] {
    auto f = flow(id);
    if (f) {
      if (!f->desired.id) f->desired = a;
      f->refresh_at = clock() + std::chrono::seconds(5);
    }
  };
  if (!repo->load(id, &a)) {
    defer();
    transport->diagnostic(id, 0, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
    return false;
  }
  if (!a.id || a.lifecycle == lifecycle_t::dormant) {
    auto old = flow(id, false);
    if (old) {
      old->source = old->destination = {};
      old->refresh_at = {};
      resolved(old->desired.source, id);
      resolved(old->desired.destination, id);
      retire_completed(id, old);
    }
    return true;
  }
  if (!repo->identity(a.source, &identity)) {
    defer();
    return false;
  }
  uint32_t root =
      a.lifecycle == lifecycle_t::draining && a.root ? a.root : identity.root;
  if (!root || root == UINT32_MAX) {
    return true;
  }
  if (a.lifecycle == lifecycle_t::active) {
    if (!identity.root || !transport->owner_ready(a.source, identity.root)) {
      return true;
    }
    // Both current DB ownership and accepted owner identity must agree. Never
    // replay a newly allocated/stale ChannelId merely because it exists in DB.
    bool read_failed = false;
    if (!current_topology(repo, a, identity, &read_failed)) {
      if (read_failed) defer();
      transport->diagnostic(id, a.source,
                            read_failed ? SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR
                                        : SUPLA_SUPLAN_RESULT_NOT_FOUND);
      return false;
    }
  }
  auto f = flow(id);
  if (!f) return false;
  bool unchanged = same_state(f->desired, a) && f->wire_root == root;
  bool same_revision = f->wire_root == root &&
                       f->desired.source == a.source &&
                       f->desired.destination == a.destination &&
                       f->desired.generation == a.generation &&
                       f->desired.revision == a.revision;
  if (!same_revision) {
    resolved(a.source, id);
    resolved(a.destination, id);
  }
  if (!unchanged || meaningful_event) {
    // Destination reconnect must not overlap a still-live Source request
    // with a different RETURN_PEER_KEY flag: the frozen reply has no flag echo.
    side_t pending_source;
    side_t conflict_destination;
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
    int recovery_owner = f->recovery_owner;
    *f = {};
    f->recovery_owner = recovery_owner;
    f->desired = a;
    f->wire_root = root;
    f->source = pending_source;
    f->destination = conflict_destination;
    f->force_key = force_key;
    f->source_no_key_sent = source_no_key_sent;
  }
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    pairs[{a.source, a.destination}] = id;
  }
  if (a.lifecycle == lifecycle_t::draining) {
    if (a.source_acked || a.source_removed || identity.root != root)
      f->source = {};
    if (a.destination_acked || a.destination_removed) f->destination = {};
    if (!a.source_acked && !a.source_removed && identity.root == root)
      send_source(f.get(), false);
    if (!a.destination_acked && !a.destination_removed)
      send_destination(f.get(), nullptr);
    retire_completed(id, f);
    return true;
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
  return true;
}
void peer_provisioner::result(flow_t *f, bool source, uint8_t code) {
  side_t &side = source ? f->source : f->destination;
  side.pending = false;
  side.acknowledged = code == SUPLA_SUPLAN_RESULT_OK;
  if (code == SUPLA_SUPLAN_RESULT_OK) {
    resolved(source ? f->desired.source : f->desired.destination,
             f->desired.id);
    side.session = 0;
    return;
  }
  if (!source && code == SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED &&
      f->desired.lifecycle == lifecycle_t::active && !f->key_recovery_started &&
      !f->source.blocked) {
    f->key_recovery_started = true;
    f->force_key = true;
    f->destination = {};
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
uint64_t peer_provisioner::correlate(const TSuplaSuplanPeerContext &ctx) {
  std::lock_guard<std::mutex> lock(flows_mutex);
  auto found = pairs.find({static_cast<int>(ctx.SourceNodeId),
                           static_cast<int>(ctx.DestinationNodeId)});
  return found == pairs.end() ? 0 : found->second;
}

void peer_provisioner::on_source(int device,
                                 TDS_SuplaSetSuplanSourceAssociationResult *q,
                                 uint64_t session) {
  if (!q) return;
  struct key_wiper {
    unsigned char *key;
    ~key_wiper() { wipe_key(key, SUPLA_SUPLAN_PEER_KEY_SIZE); }
  } wiper{q->PeerKey};
  if (!session) session = transport->session(device);
  auto id = correlate(q->PeerContext);
  if (!id) return;
  auto repo = factory();
  association_t pair;
  association_t cleanup;
  bool ack = false;
  {
    auto gate = association_gate(id);
    std::lock_guard<std::mutex> lock(*gate);
    schedule_guard update{this, id};
    auto f = flow(id, false);
    association_t a;
    if (!f || device != f->desired.source || !f->source.pending ||
        f->source.committing ||
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
    auto state =
        found ? current(repo.get(), *f, &a) : current_state::database_error;
    if (state == current_state::database_error) {
      db_failure(f.get(), true);
      return;
    }
    if (!pair.id || pair.id != id || state == current_state::stale) {
      f->source.pending = false;
      reconcile_locked(id, false);
      return;
    }
    if (q->Result != SUPLA_SUPLAN_RESULT_OK) {
      result(f.get(), true, q->Result);
      return;
    }
    if (a.lifecycle == lifecycle_t::draining) {
      cleanup = a;
      ack = true;
      f->source.pending = false;
      f->source.committing = true;
    } else {
      // Commit the accepted Source credential context, then drop the DB lock
      // before forwarding. Recheck the exact state while holding the send gate.
      if (!repo->begin(a.id, &pair)) {
        repo->rollback();
        db_failure(f.get(), true);
        return;
      }
      if (!same_state(pair, a)) {
        repo->rollback();
        f->source.pending = false;
        reconcile_locked(id, false);
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
      if (forward || rederive) {
        auto state = current(repo.get(), *f, &a);
        if (state != current_state::yes) {
          if (state == current_state::database_error) {
            f->force_key = true;
            db_failure(f.get(), true);
          } else {
            f->source.pending = false;
            reconcile_locked(id, false);
          }
          return;
        }
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
void peer_provisioner::on_destination(
    int device, const TDS_SuplaSetSuplanDestinationAssociationResult *q,
    uint64_t session) {
  if (!q) return;
  if (!session) session = transport->session(device);
  auto id = correlate(q->PeerContext);
  if (!id) return;
  auto repo = factory();
  association_t pair, cleanup;
  bool ack = false;
  {
    auto gate = association_gate(id);
    std::lock_guard<std::mutex> lock(*gate);
    schedule_guard update{this, id};
    auto f = flow(id, false);
    association_t a;
    if (!f || device != f->desired.destination || !f->destination.pending ||
        f->destination.committing ||
        f->destination.session != session)
      return;
    auto wire = f->desired;
    wire.root = f->wire_root;
    if (!matches(q->PeerContext, wire) || q->AclRevision != wire.revision)
      return;
    bool found = repo->find(q->PeerContext.SourceNodeId,
                            q->PeerContext.DestinationNodeId, &pair);
    auto state =
        found ? current(repo.get(), *f, &a) : current_state::database_error;
    if (state == current_state::database_error) {
      db_failure(f.get(), false);
      return;
    }
    if (!pair.id || pair.id != id || state == current_state::stale) {
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
        a.lifecycle == lifecycle_t::draining) {
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
void peer_provisioner::finish_cleanup(const association_t &sent, bool source,
                                      const std::shared_ptr<flow_t> &f) {
  // PeerService acquires the same gate and may invoke reconcile after commit.
  // Never call it while holding the gate. Keep the Flow until that commit.
  bool ok = service.cleanup_ack(sent, source);
  auto gate = association_gate(sent.id);
  std::lock_guard<std::mutex> lock(*gate);
  if (!f || flow(sent.id, false) != f || !same_state(f->desired, sent)) return;
  schedule_guard update{this, sent.id};
  side_t &side = source ? f->source : f->destination;
  side.committing = false;
  if (ok)
    result(f.get(), source, SUPLA_SUPLAN_RESULT_OK);
  else
    db_failure(f.get(), source);
  retire_completed(sent.id, f);
}
void peer_provisioner::disconnected(int device, uint64_t session) {
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
  std::vector<uint64_t> ids;
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    auto timed = deadlines.find(device);
    if (timed != deadlines.end())
      for (const auto &entry : timed->second) ids.push_back(entry.first);
  }
  for (auto id : ids) {
    auto gate = association_gate(id);
    std::lock_guard<std::mutex> lock(*gate);
    auto f = flow(id, false);
    if (!f) continue;
    schedule_guard update{this, id};
    for (bool source : {true, false}) {
      auto &side = source ? f->source : f->destination;
      if (device == (source ? f->desired.source : f->desired.destination) &&
          (!session || session == side.session)) side.pending = false;
    }
  }
}
}  // namespace supla_suplan
