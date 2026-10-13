// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include "suplan/peer_provisioner.h"

#include <algorithm>
#include <cstring>
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
void PeerProvisioner::iterate() {
  // Connection threads supply the clock trigger. Only resident unfinished
  // flows are examined; no DB inventory scan or periodic full ACL replay.
  std::vector<std::pair<uint64_t, std::shared_ptr<Flow>>> snapshots;
  struct IterationGuard {
    PeerProvisioner *peers;
    ~IterationGuard() {
      std::lock_guard<std::mutex> lock(peers->flows_mutex);
      peers->iterating = false;
    }
  };
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    if (iterating || clock() < next_iteration) return;
    iterating = true;
    next_iteration = clock() + std::chrono::seconds(1);
    auto start = flows.upper_bound(iteration_cursor);
    for (auto it = start; it != flows.end(); ++it) snapshots.push_back(*it);
    for (auto it = flows.begin(); it != start; ++it) snapshots.push_back(*it);
  }
  IterationGuard guard{this};
  unsigned work = 0;
  for (const auto &item : snapshots) {
    auto gate = association_gate(item.first);
    // Never wait on a busy publisher in the connection timer. No map mutex
    // is held while acquiring an association gate or doing DB/SRPC work.
    std::unique_lock<std::mutex> lock(*gate, std::try_to_lock);
    if (!lock || flow(item.first, false) != item.second) continue;
    auto &f = *item.second;
    bool due = f.refresh_at != std::chrono::steady_clock::time_point() &&
               clock() >= f.refresh_at;
    for (bool source : {true, false}) {
      Side &side = source ? f.source : f.destination;
      if (side.blocked || side.committing) continue;
      if (side.pending && clock() >= side.deadline) {
        transport->transient_diagnostic(
            item.first, source ? f.desired.source : f.desired.destination,
            true);
        transient(&f, source);
      } else if (side.sent && !side.acknowledged && !side.pending &&
                 clock() >= side.retry_at) {
        side.sent = false;
        // No key cache: a failed/lost key-bearing Destination send must
        // obtain a fresh Source reply for the current authoritative context.
        if (!source && side.key_requested) {
          f.force_key = true;
          if (!f.source.pending && !f.source.blocked) {
            f.source.sent = false;
            f.source.acknowledged = false;
          }
        }
        due = true;
      }
    }
    if (!due) continue;
    f.refresh_at = {};
    reconcile_locked(item.first, false);
    {
      std::lock_guard<std::mutex> map_lock(flows_mutex);
      iteration_cursor = item.first;
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
  // Caller holds the association gate. Finish phase transitions/forwarding
  // before this check: their pending replies still need the duplicate fences.
  for (const Side *side : {&f->source, &f->destination}) {
    if (side->pending || side->blocked || side->committing ||
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
  std::vector<ApprovalPair> failed;
  if (!service.reconcile_approvals(0, device, &failed)) {
    transport->diagnostic(0, device, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
    if (failed.empty()) return;  // Unknown scope: fail closed.
  }
  auto repo = factory();
  std::vector<uint64_t> ids;
  if (!repo->for_device(device, false, &ids)) {
    transport->diagnostic(0, device, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
    return;
  }
  for (auto id : ids) {
    Association a;
    int owner = 0;
    if (!repo->load(id, &a) || !a.id) continue;
    // Do not publish stale authorization for a failed pair, but keep
    // independent associations converging (including revocations).
    if (std::any_of(failed.begin(), failed.end(), [&](const ApprovalPair &p) {
          return p.source == a.source && p.destination == a.destination;
        })) continue;
    // Recover missed deletion notifications after Server downtime. Query
    // failure is never interpreted as authoritative endpoint/resource removal.
    if (!repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, a.source, &owner))
      continue;
    if (!owner && !a.source_removed) service.remove_device(a.source);
    if (!repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, a.destination, &owner))
      continue;
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
bool PeerProvisioner::current(Repository *repo, const Flow &f, Association *a) {
  Identity identity;
  return repo->load(f.desired.id, a) && a->id && same_state(*a, f.desired) &&
         repo->identity(a->source, &identity) &&
         (a->lifecycle == Lifecycle::Draining ||
          (identity.root == f.wire_root &&
           transport->owner_ready(a->source, identity.root) &&
           current_topology(repo, *a, identity)));
}
void PeerProvisioner::send_source(Flow *f, bool key) {
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
  f->source.pending = transport->send(f->desired.source, &q);
  f->source.deadline = clock() + std::chrono::seconds(15);
  if (!f->source.pending) {
    transport->transient_diagnostic(f->desired.id, f->desired.source, false);
    transient(f, true);
  }
}
void PeerProvisioner::send_destination(Flow *f, const unsigned char *key) {
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
  f->destination.pending = transport->send(f->desired.destination, &q);
  // SRPC copies/serializes synchronously into the existing connection buffer.
  // No key is retained for retries: reconnect requests deterministic rederive.
  wipe_key(q.PeerKey, sizeof(q.PeerKey));
  f->destination.deadline = clock() + std::chrono::seconds(15);
  if (!f->destination.pending) {
    transport->transient_diagnostic(f->desired.id, f->desired.destination,
                                    false);
    transient(f, false);
  }
}
void PeerProvisioner::reconcile(uint64_t id, bool meaningful_event) {
  auto gate = association_gate(id);
  std::lock_guard<std::mutex> lock(*gate);
  reconcile_locked(id, meaningful_event);
}
void PeerProvisioner::reconcile_locked(uint64_t id, bool meaningful_event) {
  auto repo = factory();
  Association a;
  Identity identity;
  auto defer = [&] {
    auto f = flow(id);
    if (f) f->refresh_at = clock() + std::chrono::seconds(15) +
                           std::chrono::milliseconds(id % 1000);
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
  if (!root || root == UINT32_MAX) return;
  if (a.lifecycle == Lifecycle::Active) {
    if (!identity.root || !transport->owner_ready(a.source, identity.root))
      return;
    // Both current DB ownership and accepted owner identity must agree. Never
    // replay a newly allocated/stale ChannelId merely because it exists in DB.
    bool read_failed = false;
    if (!current_topology(repo.get(), a, identity, &read_failed)) {
      if (read_failed) defer();
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
        clock() < f->source.deadline) {
      pending_source = f->source;
      force_key = f->force_key;
    }
    *f = {};
    f->desired = a;
    f->wire_root = root;
    f->source = pending_source;
    f->destination = conflict_destination;
    f->force_key = force_key;
    f->source_no_key_sent = source_no_key_sent;
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
void PeerProvisioner::transient(Flow *f, bool source) {
  Side &side = source ? f->source : f->destination;
  side.pending = false;
  side.acknowledged = false;
  // Bounded rate, not a command queue. Transport loss remains recoverable
  // while connected; repeated device persistence failures block explicitly.
  static const unsigned delays[] = {5, 15, 60, 300};
  unsigned index = std::min(side.failures, 3u);
  if (side.failures < 4) ++side.failures;
  side.retry_at = clock() + std::chrono::seconds(delays[index]) +
                  std::chrono::milliseconds((f->desired.id * 37 +
                                              (source ? 0 : 503)) % 1000);
}
void PeerProvisioner::result(Flow *f, bool source, uint8_t code) {
  Side &side = source ? f->source : f->destination;
  side.pending = false;
  side.acknowledged = code == SUPLA_SUPLAN_RESULT_OK;
  if (code == SUPLA_SUPLAN_RESULT_OK) {
    side.failures = side.persistence_errors = 0;
    return;
  }
  if (code == SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR &&
      ++side.persistence_errors < 3) {
    transient(f, source);
    transport->diagnostic(f->desired.id,
                          source ? f->desired.source : f->desired.destination,
                          code);
    return;
  }
  if (!source && code == SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED &&
      f->desired.lifecycle == Lifecycle::Active && !f->key_recovery_started &&
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
void PeerProvisioner::on_source(int device,
                                TDS_SuplaSetSuplanSourceAssociationResult *q) {
  if (!q) return;
  struct KeyWiper {
    unsigned char *key;
    ~KeyWiper() { wipe_key(key, SUPLA_SUPLAN_PEER_KEY_SIZE); }
  } wiper{q->PeerKey};
  auto repo = factory();
  Association pair;
  if (!repo->find(q->PeerContext.SourceNodeId, q->PeerContext.DestinationNodeId,
                  &pair) ||
      !pair.id || device != pair.source)
    return;
  Association cleanup;
  bool ack = false;
  {
    auto gate = association_gate(pair.id);
    std::lock_guard<std::mutex> lock(*gate);
    auto f = flow(pair.id, false);
    Association a;
    if (!f || !f->source.pending || f->source.committing) return;
    if (clock() >= f->source.deadline) {
      transient(f.get(), true);
      return;
    }
    if (!current(repo.get(), *f, &a)) return;
    Association wire = f->desired;
    wire.root = f->wire_root;
    if (!matches(q->PeerContext, wire) || q->AclRevision != a.revision) return;
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
        result(f.get(), true, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
        return;
      }
      bool changed = pair.root != f->wire_root ||
                     pair.provisioned_generation != pair.generation;
      pair.root = f->wire_root;
      pair.provisioned_generation = pair.generation;
      if ((changed && !repo->save(pair)) || !repo->commit()) {
        repo->rollback();
        result(f.get(), true, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
        return;
      }
      f->desired = pair;
      bool forward = f->source.key_requested;
      bool rederive = f->force_key && transport->ready(pair.destination);
      if ((forward || rederive) && !current(repo.get(), *f, &a)) {
        // The post-commit fence can fail transiently too. Keep recovery work,
        // not the reply's credential, and rederive after a fresh desired read.
        f->force_key = true;
        result(f.get(), true, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
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
    int device, const TDS_SuplaSetSuplanDestinationAssociationResult *q) {
  if (!q) return;
  auto repo = factory();
  Association pair, cleanup;
  if (!repo->find(q->PeerContext.SourceNodeId, q->PeerContext.DestinationNodeId,
                  &pair) ||
      !pair.id || device != pair.destination)
    return;
  bool ack = false;
  {
    auto gate = association_gate(pair.id);
    std::lock_guard<std::mutex> lock(*gate);
    auto f = flow(pair.id, false);
    Association a;
    if (!f || !f->destination.pending || f->destination.committing) return;
    if (clock() >= f->destination.deadline) {
      transient(f.get(), false);
      return;
    }
    if (!current(repo.get(), *f, &a)) return;
    Association wire = f->desired;
    wire.root = f->wire_root;
    if (!matches(q->PeerContext, wire) || q->AclRevision != a.revision) return;
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
  Side &side = source ? f->source : f->destination;
  side.committing = false;
  result(f.get(), source, ok ? SUPLA_SUPLAN_RESULT_OK
                             : SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
  retire_completed(sent.id, f);
}
void PeerProvisioner::disconnected(int device) {
  std::vector<std::pair<uint64_t, std::shared_ptr<Flow>>> snapshots;
  {
    std::lock_guard<std::mutex> lock(flows_mutex);
    for (const auto &item : flows) snapshots.push_back(item);
  }
  for (auto &item : snapshots) {
    auto gate = association_gate(item.first);
    std::lock_guard<std::mutex> lock(*gate);
    auto &f = *item.second;
    if (f.desired.source == device) f.source.pending = false;
    if (f.desired.destination == device) f.destination.pending = false;
  }
}
}  // namespace supla_suplan
