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
static std::shared_ptr<std::mutex> origin_gate(uint16_t type, uint64_t origin) {
  using Key = std::pair<uint16_t, uint64_t>;
  static std::mutex registry_mutex;
  static std::map<Key, std::weak_ptr<std::mutex>> registry;
  Key key{type, origin};
  std::lock_guard<std::mutex> lock(registry_mutex);
  auto gate = registry[key].lock();
  if (!gate) {
    gate = std::shared_ptr<std::mutex>(new std::mutex(), [key](std::mutex *m) {
      delete m;
      std::lock_guard<std::mutex> cleanup(registry_mutex);
      auto entry = registry.find(key);
      if (entry != registry.end() && entry->second.expired())
        registry.erase(entry);
    });
    registry[key] = gate;
  }
  return gate;
}

bool PeerService::reconcile_origin(uint16_t type, uint64_t origin, int source,
                                   int destination, const Grant *desired) {
  // Serialize discovery of the origin's affected pairs. Association send gates
  // are still acquired in ascending order, before any row transaction.
  auto origin_mutex = origin_gate(type, origin);
  std::unique_lock<std::mutex> origin_lock(*origin_mutex);
  return reconcile_origin_locked(type, origin, source, destination, desired,
                                 origin_lock);
}

bool PeerService::reconcile_origin_locked(
    uint16_t type, uint64_t origin, int source, int destination,
    const Grant *desired, std::unique_lock<std::mutex> &origin_lock) {
  auto repo = factory();
  if (!repo->supports_batch() || !type || !origin) return false;
  uint64_t target = 0;
  if (desired) {
    int owner = 0;
    if (desired->origin_type != type || desired->origin_id != origin ||
        source <= 0 || destination <= 0 || source == destination ||
        !valid_permissions(desired->permissions) ||
        !repo->owner(desired->resource_type, desired->resource_id, &owner) ||
        owner != source || !repo->device_exists(destination))
      return false;
    Association a;
    if (!repo->find(source, destination, &a)) return false;
    if (!a.id && (!repo->begin(source, destination, &a) || !repo->commit()))
      return false;
    target = a.id;
  }
  std::vector<uint64_t> ids;
  if (!repo->for_origin(type, origin, &ids)) return false;
  if (target) ids.push_back(target);
  std::sort(ids.begin(), ids.end());
  ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
  if (ids.empty()) return true;
  std::vector<std::shared_ptr<std::mutex>> gates;
  std::vector<std::unique_lock<std::mutex>> locks;
  for (auto id : ids) {
    gates.push_back(association_gate(id));
    locks.emplace_back(*gates.back());
  }
  std::vector<uint64_t> notifications;
  for (auto id : ids) {
    Association a;
    std::vector<Grant> grants;
    Identity identity;
    if (!repo->begin(id, &a) || !repo->grants(id, &grants) ||
        !repo->identity(a.source, &identity)) {
      repo->rollback();
      return false;
    }
    size_t matches = 0;
    bool exact = false;
    for (const auto &g : grants) {
      if (g.origin_type != type || g.origin_id != origin) continue;
      ++matches;
      exact = desired && id == target &&
              g.resource_type == desired->resource_type &&
              g.resource_id == desired->resource_id &&
              g.permissions == desired->permissions;
    }
    if ((id == target && matches == 1 && exact) ||
        (id != target && matches == 0))
      continue;
    Association before = a;
    int current_owner = 0;
    if (id == target &&
        (!repo->owner(desired->resource_type, desired->resource_id,
                      &current_owner) ||
         current_owner != source || !repo->device_exists(destination))) {
      repo->rollback();
      return false;
    }
    if (!repo->erase_origin(id, type, origin) ||
        (id == target && (a.source_removed || a.destination_removed ||
                          !repo->put(id, *desired))) ||
        !recompute(repo.get(), &a, identity.root)) {
      repo->rollback();
      return false;
    }
    if (!same_state(before, a)) notifications.push_back(id);
  }
  if (!repo->commit()) {
    repo->rollback();
    return false;
  }
  locks.clear();
  origin_lock.unlock();
  for (auto id : notifications)
    if (changed) changed(id);
  return true;
}

bool PeerService::reconcile_main_thermometer(uint32_t destination_channel,
                                             uint32_t source_channel) {
  return reconcile_main_thermometer(
      destination_channel, [source_channel](uint32_t *current) {
        *current = source_channel;
        return true;
      });
}

bool PeerService::reconcile_main_thermometer(
    uint32_t destination_channel,
    const std::function<bool(uint32_t *)> &read_current_source) {
  return reconcile_reference(destination_channel, 1, read_current_source);
}

// Match Cloud ChannelType::functions()[SENSORNO], not a per-consumer label.
static bool active_binary_source(const ChannelInfo &info) {
  if (info.type != SUPLA_CHANNELTYPE_BINARYSENSOR) return false;
  switch (info.function) {
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
    case SUPLA_CHANNELFNC_HOTELCARDSENSOR:
    case SUPLA_CHANNELFNC_ALARMARMAMENTSENSOR:
    case SUPLA_CHANNELFNC_MAILSENSOR:
    case SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR:
    case SUPLA_CHANNELFNC_FLOOD_SENSOR:
    case SUPLA_CHANNELFNC_MOTION_SENSOR:
    case SUPLA_CHANNELFNC_BINARY_SENSOR:
      return true;
    default:
      return false;
  }
}

static bool hvac_function(const ChannelInfo &info) {
  return info.type == SUPLA_CHANNELTYPE_HVAC &&
         (info.function == SUPLA_CHANNELFNC_HVAC_THERMOSTAT ||
          info.function == SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL ||
          info.function == SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL ||
          info.function == SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER);
}

bool PeerService::reconcile_reference(
    uint32_t consumer, uint8_t field,
    const std::function<bool(uint32_t *)> &read_current) {
  return reconcile_reference(consumer, field, read_current,
                             ReferenceReconciliation::Desired);
}

bool PeerService::reconcile_access_reference(
    uint32_t consumer, uint8_t field, uint32_t resource, int destination,
    const std::function<bool(uint32_t *)> &read_current) {
  return reconcile_reference(consumer, field, read_current,
                             ReferenceReconciliation::RequestedAccess,
                             resource, destination);
}

bool PeerService::revoke_stale_reference(
    uint32_t consumer, uint8_t field,
    const std::function<bool(uint32_t *)> &read_current) {
  return reconcile_reference(consumer, field, read_current,
                             ReferenceReconciliation::RevokeStale);
}

bool PeerService::reconcile_reference(
    uint32_t consumer, uint8_t field,
    const std::function<bool(uint32_t *)> &read_current,
    ReferenceReconciliation mode, uint32_t requested_channel,
    int requested_destination) {
  if (!((field >= 1 && field <= 6) || (field >= 32 && field < 42) ||
        (field >= 64 && field < 84))) {
    return false;
  }
  const uint64_t origin = channel_config_origin(consumer, field);
  auto gate = origin_gate(CHANNEL_CONFIG_ORIGIN, origin);
  std::unique_lock<std::mutex> origin_lock(*gate);
  uint32_t referenced = 0;
  if (!read_current(&referenced)) {
    return false;
  }
  if (mode == ReferenceReconciliation::RequestedTarget) {
    mode = referenced == requested_channel
               ? ReferenceReconciliation::Desired
               : ReferenceReconciliation::RevokeStale;
  }
  auto repo = factory();
  ChannelInfo owner, target;
  if (!repo->channel(consumer, &owner) ||
      (referenced && !repo->channel(referenced, &target))) {
    return false;
  }
  bool eligible = false;
  if (field <= 6 && hvac_function(owner)) {
    if (field <= 2) {
      eligible = (target.type == SUPLA_CHANNELTYPE_THERMOMETER ||
                  target.type == SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR) &&
                 (target.function == SUPLA_CHANNELFNC_THERMOMETER ||
                  target.function == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE);
    } else if (field == 3) {
      eligible = active_binary_source(target);
    } else if (field == 4) {
      uint32_t master = 0;
      bool has_slaves = false;
      eligible = hvac_function(target);
      if (eligible) {
        if (!repo->reference(referenced, 4, &master) ||
            !repo->has_master_dependents(consumer, &has_slaves)) {
          return false;
        }
        // Preserve the existing no-master-chain contract.
        eligible = master == 0 && !has_slaves;
      }
    } else {
      eligible = target.type == SUPLA_CHANNELTYPE_RELAY &&
                 target.function ==
                     (field == 5 ? SUPLA_CHANNELFNC_PUMPSWITCH
                                 : SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH);
    }
  } else if (field >= 32 && field < 42) {
    eligible = owner.type == SUPLA_CHANNELTYPE_CONTAINER &&
               (owner.function == SUPLA_CHANNELFNC_CONTAINER ||
                owner.function == SUPLA_CHANNELFNC_SEPTIC_TANK ||
                owner.function == SUPLA_CHANNELFNC_WATER_TANK) &&
               active_binary_source(target);
  } else if (field >= 64) {
    eligible = (owner.type == SUPLA_CHANNELTYPE_VALVE_OPENCLOSE ||
                owner.type == SUPLA_CHANNELTYPE_VALVE_PERCENTAGE) &&
               (owner.function == SUPLA_CHANNELFNC_VALVE_OPENCLOSE ||
                owner.function == SUPLA_CHANNELFNC_VALVE_PERCENTAGE) &&
               active_binary_source(target);
  }
  bool remote = owner.device && target.device &&
                owner.device != target.device && consumer != referenced &&
                eligible &&
                (owner.device_flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) &&
                (target.device_flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED);
  const bool binding = field == 5 || field == 6;
  // Pump/HOCS: HVAC owns the resource; referenced relay owns Destination.
  uint32_t resource = binding ? consumer : referenced;
  int source = binding ? owner.device : target.device;
  int destination = binding ? target.device : owner.device;
  if (mode == ReferenceReconciliation::RequestedAccess) {
    // The resource alone is insufficient for Source-first bindings: the
    // current relay must still belong to this requesting Destination Device.
    mode = resource == requested_channel && destination == requested_destination
               ? ReferenceReconciliation::Desired
               : ReferenceReconciliation::RevokeStale;
  }
  Grant grant{SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, resource,
              CHANNEL_CONFIG_ORIGIN, origin, SUPLA_SUPLAN_PERMISSION_READ};
  if (remote && mode == ReferenceReconciliation::RevokeStale) {
    Association current;
    std::vector<Grant> grants;
    if (!repo->find(source, destination, &current) ||
        (current.id && !repo->grants(current.id, &grants))) {
      return false;
    }
    remote = false;
    for (const auto &existing : grants) {
      if (existing.origin_type == grant.origin_type &&
          existing.origin_id == grant.origin_id &&
          existing.resource_type == grant.resource_type &&
          existing.resource_id == grant.resource_id &&
          existing.permissions == grant.permissions &&
          !current.source_removed && !current.destination_removed) {
        remote = true;
        break;
      }
    }
  }
  return reconcile_origin_locked(CHANNEL_CONFIG_ORIGIN, origin, source,
                                 destination, remote ? &grant : nullptr,
                                 origin_lock);
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
    // The SRPC adapter reconciles current config origins first.
    // Neither the mode nor ALLOW_APPROVAL can manufacture a manual grant.
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
                   q.DestinationResource.ResourceId, &destination)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
  } else if (!owner || !destination) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_FOUND;
  } else if (owner != source || destination == source) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
  } else if (q.Permissions != SUPLA_SUPLAN_PERMISSION_READ) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
  } else {
    // Neither an arbitrary existing ACL nor ENSURE can authorize topology.
    // Re-read the config under the same field origin gate as trusted updates.
    Association previous;
    std::vector<Grant> previous_grants;
    if (!repo->find(source, destination, &previous) ||
        (previous.id && !repo->grants(previous.id, &previous_grants))) {
      result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
      return result;
    }
    bool matched = false;
    for (uint8_t field : {5, 6}) {
      uint32_t target = 0;
      if (!repo->reference(q.SourceResource.ResourceId, field, &target)) {
        result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
        return result;
      }
      bool relevant = target == q.DestinationResource.ResourceId;
      for (const auto &grant : previous_grants) {
        relevant |= grant.origin_type == CHANNEL_CONFIG_ORIGIN &&
                    grant.origin_id ==
                        channel_config_origin(q.SourceResource.ResourceId,
                                              field) &&
                    grant.resource_type == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
                    grant.resource_id == q.SourceResource.ResourceId;
      }
      // Include stale origins for this pair, but do not project another
      // relay's pending configuration just to answer this request.
      if (!relevant) continue;
      // Decide the policy from a fresh read under the origin gate. An origin
      // left on this pair after moving elsewhere needs only revocation here.
      auto read_current = [&](uint32_t *current) {
        return repo->reference(q.SourceResource.ResourceId, field, current);
      };
      if (!reconcile_reference(
              q.SourceResource.ResourceId, field, read_current,
              ReferenceReconciliation::RequestedTarget,
              q.DestinationResource.ResourceId)) {
        result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
        return result;
      }
      // Check both current target and exact config origin, not a manual grant.
      uint32_t latest = 0;
      if (!repo->reference(q.SourceResource.ResourceId, field, &latest)) {
        result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
        return result;
      }
      Association a;
      if (!repo->find(source, destination, &a)) {
        result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
        return result;
      }
      std::vector<Grant> grants;
      if (a.id && !repo->grants(a.id, &grants)) {
        result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
        return result;
      }
      if (latest == q.DestinationResource.ResourceId &&
          a.lifecycle == Lifecycle::Active) {
        for (const auto &g : grants) {
          if (g.origin_type == CHANNEL_CONFIG_ORIGIN &&
              g.origin_id ==
                  channel_config_origin(q.SourceResource.ResourceId, field) &&
              g.resource_type == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
              g.resource_id == q.SourceResource.ResourceId &&
              g.permissions == SUPLA_SUPLAN_PERMISSION_READ) {
            matched = true;
          }
        }
      }
    }
    result.Result =
        matched ? SUPLA_SUPLAN_RESULT_OK : SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    if (matched) {
      result.DestinationDeviceId = destination;
    }
  }
  return result;
}
}  // namespace supla_suplan
