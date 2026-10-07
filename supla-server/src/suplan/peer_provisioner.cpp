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
                             const Identity &identity) {
  for (const auto &entry : a.acl) {
    if (entry.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
        std::find(identity.channels.begin(), identity.channels.end(),
                  entry.ResourceId) == identity.channels.end())
      return false;
  }
  return repo->owns_acl(a.source, a.destination, a.acl);
}
void wipe_key(void *buffer, size_t size) {
  volatile unsigned char *p = static_cast<volatile unsigned char *>(buffer);
  while (size--) *p++ = 0;
}
PeerProvisioner::PeerProvisioner(PeerService::Factory factory,
                                 PeerTransport *transport)
    : factory(factory),
      transport(transport),
      // NOLINTNEXTLINE(whitespace/indent_namespace)
      service(factory, [this](uint64_t id) { reconcile(id); }) {}
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
bool PeerProvisioner::accepted(int device, const Identity &identity) {
  auto repo = factory();
  return repo->accept_identity(device, identity);
}
void PeerProvisioner::reconnect(int device) {
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
  f->source.pending = transport->send(f->desired.source, &q);
  f->source.deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(15);
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
  f->destination.deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(15);
}
void PeerProvisioner::reconcile(uint64_t id, bool meaningful_event) {
  auto gate = association_gate(id);
  std::lock_guard<std::mutex> lock(*gate);
  auto repo = factory();
  Association a;
  Identity identity;
  if (!repo->load(id, &a)) {
    transport->diagnostic(id, 0, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
    return;
  }
  if (!a.id || a.lifecycle == Lifecycle::Dormant) {
    std::lock_guard<std::mutex> map_lock(flows_mutex);
    flows.erase(id);
    return;
  }
  if (!repo->identity(a.source, &identity)) return;
  uint32_t root =
      a.lifecycle == Lifecycle::Draining && a.root ? a.root : identity.root;
  if (!root || root == UINT32_MAX) return;
  if (a.lifecycle == Lifecycle::Active) {
    if (!identity.root || !transport->owner_ready(a.source, identity.root))
      return;
    // Both current DB ownership and accepted owner identity must agree. Never
    // replay a newly allocated/stale ChannelId merely because it exists in DB.
    if (!current_topology(repo.get(), a, identity)) {
      transport->diagnostic(id, a.source, SUPLA_SUPLAN_RESULT_NOT_FOUND);
      return;
    }
  }
  auto f = flow(id);
  if (!f) return;
  bool unchanged = same_state(f->desired, a) && f->wire_root == root;
  if (!unchanged || meaningful_event) {
    // Destination reconnect must not overlap a still-live Source request
    // with a different RETURN_PEER_KEY flag: the frozen reply has no flag echo.
    Side pending_source;
    bool force_key = false;
    bool source_no_key_acked = unchanged && f->source_no_key_acked;
    if (unchanged && f->source.pending &&
        std::chrono::steady_clock::now() <= f->source.deadline) {
      pending_source = f->source;
      force_key = f->force_key;
    }
    *f = {};
    f->desired = a;
    f->wire_root = root;
    f->source = pending_source;
    f->force_key = force_key;
    f->source_no_key_acked = source_no_key_acked;
  }
  if (a.lifecycle == Lifecycle::Draining) {
    if (!a.source_acked && !a.source_removed && identity.root == root)
      send_source(f.get(), false);
    if (!a.destination_acked && !a.destination_removed)
      send_destination(f.get(), nullptr);
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
        !f->source.pending)
      f->source = {};
    send_source(f.get(), consumer);
  } else {
    send_source(f.get(), false);
    send_destination(f.get(), nullptr);
  }
}
void PeerProvisioner::result(Flow *f, bool source, uint8_t code) {
  Side &side = source ? f->source : f->destination;
  side.pending = false;
  if (code == SUPLA_SUPLAN_RESULT_OK) return;
  if (!source && code == SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED &&
      f->desired.lifecycle == Lifecycle::Active && !f->key_recovery_started) {
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
  // Zero immediate retries is a bounded policy: unknown/failed durable writes
  // are reconstructed on reconnect or a changed desired state, never assumed
  // OK.
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
    if (!f || !f->source.pending) return;
    if (std::chrono::steady_clock::now() > f->source.deadline) {
      f->source.pending = false;
      return;
    }
    if (!current(repo.get(), *f, &a)) return;
    Association wire = f->desired;
    wire.root = f->wire_root;
    if (!matches(q->PeerContext, wire) || q->AclRevision != a.revision) return;
    if (q->Result == SUPLA_SUPLAN_RESULT_OK &&
        q->PeerKeySize !=
            (f->source.key_requested ? SUPLA_SUPLAN_PEER_KEY_SIZE : 0)) {
      // RETURN_PEER_KEY is not echoed in the frozen result. An already
      // accepted no-key OK can be duplicated while rederive is pending for
      // this same context/revision; it must not consume the pending key reply.
      if (f->source.key_requested && !q->PeerKeySize &&
          f->source_no_key_acked)
        return;
      result(f.get(), true, SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT);
      return;
    }
    result(f.get(), true, q->Result);
    if (q->Result != SUPLA_SUPLAN_RESULT_OK) return;
    if (a.lifecycle == Lifecycle::Draining) {
      cleanup = a;
      ack = true;
    } else {
      // Commit the accepted Source credential context, then drop the DB lock
      // before forwarding. Recheck the exact state while holding the send gate.
      if (!repo->begin(a.id, &pair) || !same_state(pair, a)) {
        repo->rollback();
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
      if (!f->source.key_requested) f->source_no_key_acked = true;
      if (f->source.key_requested && current(repo.get(), *f, &a)) {
        f->force_key = false;
        send_destination(f.get(), q->PeerKey);
      } else if (f->force_key && transport->ready(pair.destination) &&
                 current(repo.get(), *f, &a)) {
        f->source = {};
        send_source(f.get(), true);
      }
    }
  }
  if (ack) service.cleanup_ack(cleanup, true);
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
    if (!f || !f->destination.pending) return;
    if (std::chrono::steady_clock::now() > f->destination.deadline) {
      f->destination.pending = false;
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
    result(f.get(), false, q->Result);
    if (q->Result == SUPLA_SUPLAN_RESULT_OK &&
        a.lifecycle == Lifecycle::Draining) {
      cleanup = a;
      ack = true;
    }
  }
  if (ack) service.cleanup_ack(cleanup, false);
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
