// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "suplan/peer_service.h"

#include <algorithm>
#include <limits>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace supla_suplan {

bool Repository::owns_acl(int source, int destination, const Acl &acl) {
  if (!device_exists(source) || !device_exists(destination)) return false;
  for (const auto &entry : acl) {
    int current_owner = 0;
    if (!owner(entry.ResourceType, entry.ResourceId, &current_owner) ||
        current_owner != source)
      return false;
  }
  return true;
}

bool valid_permissions(uint8_t p) { return p && !(p & ~7); }
uint8_t normalize(uint8_t p) { return p & 2 ? p & ~1 : p; }
bool canonicalize(const std::vector<Grant> &grants, Acl *acl) {
  std::map<std::pair<uint8_t, uint32_t>, uint8_t> entries;
  for (const auto &g : grants) {
    if (!valid_permissions(g.permissions) || !g.resource_id ||
        (g.resource_type != SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
         g.resource_type != SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE))
      return false;
    entries[{g.resource_type, g.resource_id}] |= g.permissions;
  }
  if (entries.size() > SUPLA_SUPLAN_MAX_ACL_ENTRIES) return false;
  acl->clear();
  for (const auto &e : entries) {
    acl->push_back({e.first.first, e.first.second, normalize(e.second)});
  }
  return true;
}
bool equal_acl(const Acl &a, const Acl &b) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i) {
    if (a[i].ResourceType != b[i].ResourceType ||
        a[i].ResourceId != b[i].ResourceId ||
        a[i].Permissions != b[i].Permissions)
      return false;
  }
  return true;
}
bool same_state(const Association &a, const Association &b) {
  return a.id == b.id && a.source == b.source &&
         a.destination == b.destination && a.root == b.root &&
         a.provisioned_generation == b.provisioned_generation &&
         a.generation == b.generation && a.revision == b.revision &&
         a.lifecycle == b.lifecycle && a.source_removed == b.source_removed &&
         a.destination_removed == b.destination_removed &&
         equal_acl(a.acl, b.acl);
}
TSuplaSuplanPeerContext context(const Association &a, uint32_t root) {
  return {SUPLA_SUPLAN_AUTHORITY_TYPE_SERVER,
          0,
          SUPLA_SUPLAN_NODE_ID_NAMESPACE_DEVICE_ID,
          static_cast<uint32_t>(a.source),
          SUPLA_SUPLAN_NODE_ID_NAMESPACE_DEVICE_ID,
          static_cast<uint32_t>(a.destination),
          root,
          a.generation};
}
bool matches(const TSuplaSuplanPeerContext &p, const Association &a) {
  return p.AuthorityType == SUPLA_SUPLAN_AUTHORITY_TYPE_SERVER &&
         p.AuthorityId == 0 &&
         p.SourceNodeIdNamespace == SUPLA_SUPLAN_NODE_ID_NAMESPACE_DEVICE_ID &&
         p.DestinationNodeIdNamespace ==
             SUPLA_SUPLAN_NODE_ID_NAMESPACE_DEVICE_ID &&
         p.SourceNodeId == static_cast<uint32_t>(a.source) &&
         p.DestinationNodeId == static_cast<uint32_t>(a.destination) &&
         p.RootEpoch == a.root && p.PeerGeneration == a.generation;
}
std::vector<unsigned char> encode_acl(const Acl &acl) {
  std::vector<unsigned char> out;
  for (const auto &e : acl) {
    out.push_back(e.ResourceType);
    for (int shift = 24; shift >= 0; shift -= 8)
      out.push_back(e.ResourceId >> shift);
    out.push_back(e.Permissions);
  }
  return out;
}
bool decode_acl(const unsigned char *data, size_t size, Acl *acl) {
  if (size % 6 || size > SUPLA_SUPLAN_MAX_ACL_ENTRIES * 6) return false;
  std::vector<Grant> grants;
  for (size_t i = 0; i < size; i += 6) {
    uint32_t id = 0;
    for (size_t j = 1; j < 5; ++j) id = (id << 8) | data[i + j];
    grants.push_back({data[i], id, 0, 0, data[i + 5]});
  }
  if (!canonicalize(grants, acl)) return false;
  auto encoded = encode_acl(*acl);
  return encoded.size() == size &&
         std::equal(encoded.begin(), encoded.end(), data);
}
std::shared_ptr<std::mutex> association_gate(uint64_t id) {
  static std::mutex registry_mutex;
  static std::map<uint64_t, std::weak_ptr<std::mutex>> registry;
  std::lock_guard<std::mutex> lock(registry_mutex);
  auto gate = registry[id].lock();
  if (!gate) {
    gate = std::shared_ptr<std::mutex>(new std::mutex(), [id](std::mutex *m) {
      delete m;
      std::lock_guard<std::mutex> cleanup(registry_mutex);
      auto entry = registry.find(id);
      if (entry != registry.end() && entry->second.expired())
        registry.erase(entry);
    });
    registry[id] = gate;
  }
  return gate;
}
PeerService::PeerService(Factory factory, Changed changed)
    : factory(factory), changed(changed) {}

bool PeerService::recompute(Repository *repo, Association *a, uint32_t root) {
  std::vector<Grant> grants;
  Acl acl;
  if (!repo->grants(a->id, &grants) || !canonicalize(grants, &acl))
    return false;
  if (equal_acl(acl, a->acl)) return true;
  if (!acl.empty() && a->lifecycle != Lifecycle::Active) {
    if (root && root == a->root) {
      if (a->generation == UINT32_MAX) return false;
      ++a->generation;
    }
    a->revision = 1;
    a->lifecycle = Lifecycle::Active;
  } else if (a->revision == UINT32_MAX) {
    if (a->generation == UINT32_MAX) return false;
    ++a->generation;
    a->revision = 1;
  } else {
    ++a->revision;
  }
  if (acl.empty()) {
    a->lifecycle = Lifecycle::Draining;
    // Source may have installed the current root's replacement before its ACK
    // arrives, even when an older provisioned root is still recorded here.
    // Select and retain the current namespace for empty cleanup on both sides;
    // Destination accepts empty replacement without requiring a peer key.
    // This is a lifecycle transition, not a mass update on root observation.
    if (root) a->root = root;
  }
  a->source_acked = a->destination_acked = false;
  a->acl = std::move(acl);
  return repo->save(*a);
}
bool PeerService::mutate(
    uint64_t id, const std::function<bool(Repository *, Association *)> &fn) {
  bool notify = false;
  {
    auto gate = association_gate(id);
    std::lock_guard<std::mutex> lock(*gate);
    auto repo = factory();
    Association a;
    if (!repo->begin(id, &a)) return false;
    Association before = a;
    if (!fn(repo.get(), &a)) {
      repo->rollback();
      return false;
    }
    notify = !same_state(before, a);
    if (!repo->commit()) {
      repo->rollback();
      return false;
    }
  }
  if (notify && changed) changed(id);
  return true;
}
bool PeerService::upsert(int source, int destination, const Grant &grant) {
  if (source <= 0 || destination <= 0 || source == destination ||
      !grant.origin_type || !grant.origin_id ||
      !valid_permissions(grant.permissions))
    return false;
  auto repo = factory();
  int owner = 0;
  if (!repo->owner(grant.resource_type, grant.resource_id, &owner) ||
      owner != source || !repo->device_exists(destination))
    return false;
  Association a;
  // Resolve/create the stable pair first, avoiding gate/DB lock inversion.
  // Existing pairs never execute a no-op UPDATE during a Grant upsert.
  if (!repo->find(source, destination, &a)) return false;
  if (!a.id && (!repo->begin(source, destination, &a) || !repo->commit()))
    return false;
  return mutate(a.id, [&](Repository *r, Association *state) {
    Identity identity;
    int current_owner = 0;
    return !state->source_removed && !state->destination_removed &&
           r->owner(grant.resource_type, grant.resource_id, &current_owner) &&
           current_owner == source && r->device_exists(destination) &&
           r->identity(source, &identity) && r->put(a.id, grant) &&
           recompute(r, state, identity.root);
  });
}
bool PeerService::delete_origin(uint16_t type, uint64_t origin) {
  auto repo = factory();
  std::vector<uint64_t> ids;
  if (!repo->for_origin(type, origin, &ids)) return false;
  for (auto id : ids) {
    if (!mutate(id, [&](Repository *r, Association *a) {
          Identity identity;
          return r->identity(a->source, &identity) &&
                 r->erase_origin(id, type, origin) &&
                 recompute(r, a, identity.root);
        }))
      return false;
  }
  return true;
}
bool PeerService::delete_resource(uint8_t type, uint32_t resource) {
  auto repo = factory();
  std::vector<uint64_t> ids;
  if (!repo->for_resource(type, resource, &ids)) return false;
  for (auto id : ids) {
    if (!delete_resource(id, type, resource)) return false;
  }
  return true;
}
bool PeerService::delete_resource(uint64_t id, uint8_t type,
                                  uint32_t resource) {
  return mutate(id, [&](Repository *r, Association *a) {
    Identity identity;
    return r->identity(a->source, &identity) &&
           r->erase_resource(id, type, resource) &&
           recompute(r, a, identity.root);
  });
}
bool PeerService::remove_device(int device) {
  auto repo = factory();
  std::vector<uint64_t> ids;
  if (!repo->for_device(device, true, &ids)) return false;
  for (auto id : ids) {
    if (!mutate(id, [&](Repository *r, Association *a) {
          Identity identity;
          if (a->lifecycle == Lifecycle::Dormant) return r->erase(id);
          if (!r->identity(a->source, &identity) || !r->erase_grants(id) ||
              !recompute(r, a, identity.root))
            return false;
          if (a->source == device) a->source_removed = a->source_acked = true;
          if (a->destination == device)
            a->destination_removed = a->destination_acked = true;
          if (a->source_acked && a->destination_acked) return r->erase(id);
          // No Source OK context was ever recorded, so no Destination key
          // could have been forwarded. Removing Source cannot strand it.
          if (a->source_removed && !a->root && !a->provisioned_generation)
            return r->erase(id);
          return r->save(*a);
        }))
      return false;
  }
  return true;
}
bool PeerService::distrust(uint64_t id) {
  return mutate(id, [](Repository *r, Association *a) {
    if (a->generation == UINT32_MAX) return false;
    ++a->generation;
    a->revision = 1;
    a->source_acked = a->destination_acked = false;
    return r->save(*a);
  });
}
bool PeerService::cleanup_ack(const Association &sent, bool source) {
  return mutate(sent.id, [&](Repository *r, Association *a) {
    if (!same_state(sent, *a) || a->lifecycle != Lifecycle::Draining)
      return true;
    bool &acked = source ? a->source_acked : a->destination_acked;
    if (acked) return true;
    acked = true;
    if (a->source_acked && a->destination_acked) {
      if (a->source_removed || a->destination_removed) return r->erase(a->id);
      a->lifecycle = Lifecycle::Dormant;
    }
    return r->save(*a);
  });
}
bool PeerService::observe_root(uint64_t id) {
  return mutate(id, [](Repository *r, Association *a) {
    if (a->lifecycle != Lifecycle::Draining || a->source_acked) return true;
    Identity identity;
    if (!r->identity(a->source, &identity)) return false;
    if ((a->root && identity.root && identity.root != a->root) ||
        a->source_removed) {
      a->source_acked = true;
      if (a->destination_acked) {
        if (a->source_removed || a->destination_removed) return r->erase(a->id);
        a->lifecycle = Lifecycle::Dormant;
      }
      return r->save(*a);
    }
    return true;
  });
}
bool PeerService::provisioned(const Association &sent, uint32_t root) {
  return mutate(sent.id, [&](Repository *r, Association *a) {
    Identity identity;
    if (!same_state(sent, *a)) return true;
    if (!r->identity(a->source, &identity) || identity.root != root)
      return false;
    if (a->root == root && a->provisioned_generation == a->generation)
      return true;
    a->root = root;
    a->provisioned_generation = a->generation;
    return r->save(*a);
  });
}
TSD_SuplaEnsureResourceAccessResult PeerService::ensure_access(
    int destination, const TDS_SuplaEnsureResourceAccess &q) {
  TSD_SuplaEnsureResourceAccessResult result = {
      SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT, SUPLA_SUPLAN_ACCESS_STATUS_INVALID,
      SUPLA_RESOURCE_DELIVERY_INVALID};
  if ((q.Flags & ~SUPLA_SUPLAN_ENSURE_ACCESS_FLAG_ALLOW_APPROVAL) ||
      (q.DeliveryMode != SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER &&
       q.DeliveryMode != SUPLA_RESOURCE_DELIVERY_SERVER_STREAM) ||
      !q.Resource.ResourceId || !valid_permissions(q.Permissions))
    return result;
  result.DeliveryMode = q.DeliveryMode;
  if (q.DeliveryMode == SUPLA_RESOURCE_DELIVERY_SERVER_STREAM ||
      (q.Resource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
       q.Resource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE)) {
    result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
    return result;
  }
  auto repo = factory();
  int source = 0;
  if (!repo->owner(q.Resource.ResourceType, q.Resource.ResourceId, &source)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  if (!source) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_FOUND;
    return result;
  }
  Association a;
  if (!repo->find(source, destination, &a)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  uint8_t allowed = 0;
  if (a.id && a.lifecycle == Lifecycle::Active && !a.source_removed &&
      !a.destination_removed) {
    for (const auto &e : a.acl) {
      if ((e.ResourceType == q.Resource.ResourceType &&
           e.ResourceId == q.Resource.ResourceId) ||
          (e.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE &&
           e.ResourceId == static_cast<uint32_t>(source)))
        allowed |= e.Permissions;
    }
  }
  if (allowed & SUPLA_SUPLAN_PERMISSION_CONTROL)
    allowed |= SUPLA_SUPLAN_PERMISSION_READ;
  if ((allowed & q.Permissions) == q.Permissions) {
    result.Result = SUPLA_SUPLAN_RESULT_OK;
    result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_GRANTED;
    if (changed) changed(a.id);
  } else {
    // M3 authoritative configuration origin and M5 approval are future
    // adapters. Neither the mode nor ALLOW_APPROVAL can manufacture a grant.
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
  }
  return result;
}
TSD_SuplaEnsureResourceShareResult PeerService::ensure_share(
    int source, const TDS_SuplaEnsureResourceShare &q) {
  TSD_SuplaEnsureResourceShareResult result = {
      SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT, 0};
  if (!valid_permissions(q.Permissions) || !q.SourceResource.ResourceId ||
      !q.DestinationResource.ResourceId)
    return result;
  if (q.SourceResource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL ||
      q.DestinationResource.ResourceType !=
          SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL) {
    result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
    return result;
  }
  auto repo = factory();
  int owner = 0, destination = 0;
  if (!repo->owner(q.SourceResource.ResourceType, q.SourceResource.ResourceId,
                   &owner) ||
      !repo->owner(q.DestinationResource.ResourceType,
                   q.DestinationResource.ResourceId, &destination))
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
  else if (!owner || !destination)
    result.Result = SUPLA_SUPLAN_RESULT_NOT_FOUND;
  else if (owner != source || destination == source)
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
  else
    // An unrelated grant is not evidence of a source-first binding relation.
    // Only M3/M4 can supply the authoritative resource-to-resource origin.
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
  return result;
}
}  // namespace supla_suplan
