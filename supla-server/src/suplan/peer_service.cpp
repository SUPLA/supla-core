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
  bool owned = false;
  return check_acl_ownership(source, destination, acl, &owned) && owned;
}
bool Repository::check_acl_ownership(int source, int destination,
                                      const Acl &acl, bool *owned) {
  *owned = false;
  for (int endpoint : {source, destination}) {
    int current_owner = 0;
    if (!owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, endpoint, &current_owner))
      return false;
    if (current_owner != endpoint) return true;
  }
  for (const auto &entry : acl) {
    int current_owner = 0;
    if (!owner(entry.ResourceType, entry.ResourceId, &current_owner))
      return false;
    if (current_owner != source) return true;
  }
  *owned = true;
  return true;
}

bool valid_permissions(uint8_t p) { return p && !(p & ~7); }
uint8_t normalize(uint8_t p) { return p & 2 ? p & ~1 : p; }
uint8_t missing_permissions(uint8_t requested, uint8_t effective) {
  if (effective & SUPLA_SUPLAN_PERMISSION_CONTROL)
    effective |= SUPLA_SUPLAN_PERMISSION_READ;
  return normalize(requested) & ~effective;
}
static bool active_binary_source(const ChannelInfo &info);
static bool manual_permissions_supported(const ChannelInfo &c, uint8_t p) {
  if (!c.type || !c.function || !valid_permissions(p))
    return false;
  if (c.type == SUPLA_CHANNELTYPE_ACTIONTRIGGER ||
      c.function == SUPLA_CHANNELFNC_ACTIONTRIGGER)
    return c.type == SUPLA_CHANNELTYPE_ACTIONTRIGGER &&
           c.function == SUPLA_CHANNELFNC_ACTIONTRIGGER &&
           c.action_capabilities && p == SUPLA_SUPLAN_PERMISSION_ACTION;
  if (p & SUPLA_SUPLAN_PERMISSION_ACTION) return false;
  if (!(p & SUPLA_SUPLAN_PERMISSION_CONTROL)) {
    switch (c.type) {
      case SUPLA_CHANNELTYPE_BINARYSENSOR: return active_binary_source(c);
      case SUPLA_CHANNELTYPE_THERMOMETER:
        return c.function == SUPLA_CHANNELFNC_THERMOMETER;
      case SUPLA_CHANNELTYPE_HUMIDITYSENSOR:
        return c.function == SUPLA_CHANNELFNC_HUMIDITY;
      case SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR:
        return c.function == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE;
      case SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT:
        return c.function == SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT;
      case SUPLA_CHANNELTYPE_GENERAL_PURPOSE_METER:
        return c.function == SUPLA_CHANNELFNC_GENERAL_PURPOSE_METER;
      case SUPLA_CHANNELTYPE_ELECTRICITY_METER:
        return c.function == SUPLA_CHANNELFNC_ELECTRICITY_METER;
      case SUPLA_CHANNELTYPE_IMPULSE_COUNTER:
        return c.function == SUPLA_CHANNELFNC_IC_ELECTRICITY_METER ||
               c.function == SUPLA_CHANNELFNC_IC_GAS_METER ||
               c.function == SUPLA_CHANNELFNC_IC_WATER_METER ||
               c.function == SUPLA_CHANNELFNC_IC_HEAT_METER ||
               c.function == SUPLA_CHANNELFNC_IC_EVENTS ||
               c.function == SUPLA_CHANNELFNC_IC_SECONDS;
      case SUPLA_CHANNELTYPE_CONTAINER:
        return c.function == SUPLA_CHANNELFNC_CONTAINER ||
               c.function == SUPLA_CHANNELFNC_SEPTIC_TANK ||
               c.function == SUPLA_CHANNELFNC_WATER_TANK;
      default:
        return manual_permissions_supported(c, SUPLA_SUPLAN_PERMISSION_CONTROL);
    }
  }
  switch (c.type) {
    case SUPLA_CHANNELTYPE_RELAY:
      switch (c.function) {
        case SUPLA_CHANNELFNC_POWERSWITCH:
        case SUPLA_CHANNELFNC_LIGHTSWITCH:
        case SUPLA_CHANNELFNC_STAIRCASETIMER:
        case SUPLA_CHANNELFNC_PUMPSWITCH:
        case SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND:
        case SUPLA_CHANNELFNC_TERRACE_AWNING:
        case SUPLA_CHANNELFNC_PROJECTOR_SCREEN:
        case SUPLA_CHANNELFNC_CURTAIN:
        case SUPLA_CHANNELFNC_VERTICAL_BLIND:
        case SUPLA_CHANNELFNC_ROLLER_GARAGE_DOOR: return true;
      }
      return false;
    case SUPLA_CHANNELTYPE_HVAC:
      return c.function == SUPLA_CHANNELFNC_HVAC_THERMOSTAT ||
             c.function == SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL ||
             c.function == SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL ||
             c.function == SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER;
    case SUPLA_CHANNELTYPE_VALVE_OPENCLOSE:
      return c.function == SUPLA_CHANNELFNC_VALVE_OPENCLOSE;
    case SUPLA_CHANNELTYPE_VALVE_PERCENTAGE:
      return c.function == SUPLA_CHANNELFNC_VALVE_PERCENTAGE;
    case SUPLA_CHANNELTYPE_DIMMER:
      return c.function == SUPLA_CHANNELFNC_DIMMER ||
             c.function == SUPLA_CHANNELFNC_DIMMER_CCT;
    case SUPLA_CHANNELTYPE_RGBLEDCONTROLLER:
      return c.function == SUPLA_CHANNELFNC_RGBLIGHTING;
    case SUPLA_CHANNELTYPE_DIMMERANDRGBLED:
      return c.function == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING ||
             c.function == SUPLA_CHANNELFNC_DIMMER_CCT_AND_RGB;
  }
  return false;
}
bool stream_read_supported(const ChannelInfo &channel) {
  if (!channel.type || !channel.function ||
      channel.type == SUPLA_CHANNELTYPE_ACTIONTRIGGER ||
      channel.function == SUPLA_CHANNELFNC_ACTIONTRIGGER) return false;
  auto c = channel;
  // Local compatibility families remain readable over SRPC only. This does
  // not restore deprecated HVAC reference or SUPLAN_PEER eligibility.
  switch (c.type) {
    case SUPLA_CHANNELTYPE_THERMOMETERDS18B20:
      return c.function == SUPLA_CHANNELFNC_THERMOMETER;
    case SUPLA_CHANNELTYPE_RELAYHFD4:
      return c.function == SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK ||
             c.function == SUPLA_CHANNELFNC_CONTROLLINGTHEGATE ||
             c.function == SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR ||
             c.function == SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK;
    case SUPLA_CHANNELTYPE_RELAYG5LA1A:
      if (c.function != SUPLA_CHANNELFNC_POWERSWITCH &&
          c.function != SUPLA_CHANNELFNC_LIGHTSWITCH &&
          c.function != SUPLA_CHANNELFNC_STAIRCASETIMER)
        return stream_read_supported(ChannelInfo{c.device,
            SUPLA_CHANNELTYPE_RELAYHFD4, c.function});
      return true;
    case SUPLA_CHANNELTYPE_2XRELAYG5LA1A:
      c.type = SUPLA_CHANNELTYPE_RELAY;
      break;
    case SUPLA_CHANNELTYPE_SENSORNC: c.type = SUPLA_CHANNELTYPE_BINARYSENSOR;
      break;
    case SUPLA_CHANNELTYPE_DHT11:
    case SUPLA_CHANNELTYPE_DHT21:
    case SUPLA_CHANNELTYPE_DHT22:
    case SUPLA_CHANNELTYPE_AM2301:
    case SUPLA_CHANNELTYPE_AM2302:
      return c.function == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE;
    case SUPLA_CHANNELTYPE_DISTANCESENSOR:
      return c.function == SUPLA_CHANNELFNC_DEPTHSENSOR ||
             c.function == SUPLA_CHANNELFNC_DISTANCESENSOR;
    case SUPLA_CHANNELTYPE_WINDSENSOR:
      return c.function == SUPLA_CHANNELFNC_WINDSENSOR;
    case SUPLA_CHANNELTYPE_PRESSURESENSOR:
      return c.function == SUPLA_CHANNELFNC_PRESSURESENSOR;
    case SUPLA_CHANNELTYPE_RAINSENSOR:
      return c.function == SUPLA_CHANNELFNC_RAINSENSOR;
    case SUPLA_CHANNELTYPE_WEIGHTSENSOR:
      return c.function == SUPLA_CHANNELFNC_WEIGHTSENSOR;
    case SUPLA_CHANNELTYPE_WEATHER_STATION:
      return c.function == SUPLA_CHANNELFNC_WEATHER_STATION;
    case SUPLA_CHANNELTYPE_THERMOSTAT:
      return c.function ==
             400;  // Historical thermostat function, no v29 macro.
    case SUPLA_CHANNELTYPE_THERMOSTAT_HEATPOL_HOMEPLUS:
      return c.function == SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS;
    case SUPLA_CHANNELTYPE_DIGIGLASS:
      return c.function == SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL ||
             c.function == SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL;
    case SUPLA_CHANNELTYPE_HVAC:
      if (c.function == SUPLA_CHANNELFNC_HVAC_DRYER ||
          c.function == SUPLA_CHANNELFNC_HVAC_FAN) return true;
      break;
  }
  return manual_permissions_supported(c, SUPLA_SUPLAN_PERMISSION_READ);
}

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
std::shared_ptr<std::mutex> origin_gate(uint16_t type, uint64_t origin) {
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
  return reconcile_origin(type, origin, 0, 0, nullptr);
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

bool PeerService::reconcile_approvals(uint32_t resource, int device,
                                      std::vector<ApprovalPair> *failed,
                                      bool project_stream) {
  if (failed) failed->clear();
  auto repo = factory();
  bool available = false;
  if (!repo->approval_schema(&available)) return false;
  if (!available) return true;  // M2/M3/M4-only deployments have no M5 state.
  std::vector<ApprovalPair> pairs;
  if (!repo->approval_pairs(resource, device, &pairs)) return false;
  bool success = true;
  for (const auto &pair : pairs) {
    if (!reconcile_approval_pair(pair, 0, false, nullptr, project_stream)) {
      success = false;
      if (failed) failed->push_back(pair);
    }
  }
  return success;
}
bool PeerService::reconcile_approval_pair(
    ApprovalPair pair, uint8_t requested, bool allow_approval,
    TSD_SuplaEnsureResourceAccessResult *result, bool project_stream) {
  // If a just-accepted pair has no M2 association, create its empty stable
  // identity outside the pair transaction. Never wait for a gate while holding
  // an association row lock. Two bounded attempts cover first creation.
  for (int attempt = 0; attempt < 2; ++attempt) {
    auto repo = factory();
    Association existing;
    if (!repo->find(pair.source, pair.destination, &existing) ||
        !repo->approval_lock(&pair, allow_approval)) return false;
    ChannelInfo channel;
    DeviceInfo source, destination;
    Identity identity;
    std::vector<ApprovalDecision> decisions;
    if (!repo->approval_context(pair, &channel, &source, &destination,
                                &identity) ||
        !repo->approval_decisions(pair.id, &decisions)) return false;
    bool valid = !channel.is_virtual && channel.device == pair.source &&
                 pair.source != pair.destination && identity.root &&
                 source.protocol >= 29 && destination.protocol >= 29 &&
                 (source.flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) &&
                 (destination.flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) &&
                 std::find(identity.channels.begin(), identity.channels.end(),
                           pair.resource) != identity.channels.end();
    if ((!valid || !manual_permissions_supported(channel, requested)) &&
        decisions.empty() && allow_approval) {
      repo->rollback();
      if (result) {
        result->Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
        result->AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_INVALID;
      }
      return true;
    }
    bool needs_association = false;
    for (const auto &d : decisions)
      needs_association |= (project_stream || d.delivery_mode !=
                           SUPLA_RESOURCE_DELIVERY_SERVER_STREAM) &&
                           d.status == ApprovalStatus::Accepted && valid &&
                           manual_permissions_supported(channel, d.permissions);
    if (needs_association && !existing.id) {
      repo->rollback();
      if (!repo->begin(pair.source, pair.destination, &existing) ||
          !repo->commit()) return false;
      continue;
    }
    std::shared_ptr<std::mutex> gate;
    std::unique_lock<std::mutex> lock;
    Association current;
    bool notify = false;
    if (existing.id) {
      gate = association_gate(existing.id);
      lock = std::unique_lock<std::mutex>(*gate);
      if (!repo->begin(existing.id, &current)) return false;
    }
    Association before = current;
    std::vector<Grant> grants;
    if (current.id && !repo->grants(current.id, &grants)) return false;
    bool grant_change = false;
    for (const auto &d : decisions) {
      bool accepted = d.status == ApprovalStatus::Accepted && valid &&
                      manual_permissions_supported(channel, d.permissions) &&
                      !current.source_removed && !current.destination_removed;
      const Grant *old = nullptr;
      for (const auto &g : grants)
        if (g.origin_type == USER_APPROVAL_ORIGIN && g.origin_id == d.id)
          old = &g;
      accepted = accepted && (project_stream || old ||
          d.delivery_mode != SUPLA_RESOURCE_DELIVERY_SERVER_STREAM);
      if (accepted) {
        const Grant desired{SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, pair.resource,
                            USER_APPROVAL_ORIGIN, d.id,
                            normalize(d.permissions)};
        if (!old || old->permissions != desired.permissions ||
            old->resource_id != desired.resource_id) {
          if (!repo->erase_origin(current.id, USER_APPROVAL_ORIGIN, d.id) ||
              !repo->put(current.id, desired)) return false;
          grant_change = true;
        }
      } else {
        if (old) {
          if (!repo->erase_origin(current.id, USER_APPROVAL_ORIGIN, d.id))
            return false;
          grant_change = true;
        }
        // Validation revokes projection, not the Cloud business decision.
        // Only an explicit Cloud Remove changes Accepted to Removed.
      }
    }
    if (grant_change && !recompute(repo.get(), &current, identity.root))
      return false;
    notify = !same_state(before, current);
    if (result) {
      uint8_t effective = 0;
      for (const auto &entry : current.acl)
        if ((entry.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
             entry.ResourceId == pair.resource) ||
            (entry.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE &&
             entry.ResourceId == static_cast<uint32_t>(pair.source)))
          effective |= entry.Permissions;
      uint8_t missing = missing_permissions(requested, effective);
      const ApprovalDecision *open = nullptr;
      for (const auto &d : decisions)
        if (d.status == ApprovalStatus::Pending ||
            d.status == ApprovalStatus::Rejected) open = &d;
      if (!valid || !manual_permissions_supported(channel, requested)) {
        result->Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
        result->AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_INVALID;
      } else if (!missing) {
        result->Result = SUPLA_SUPLAN_RESULT_OK;
        result->AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_GRANTED;
      } else if (open && open->status == ApprovalStatus::Rejected) {
        result->Result = SUPLA_SUPLAN_RESULT_OK;
        result->AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_REJECTED;
      } else {
        uint8_t pending = normalize(missing | (open ? open->permissions : 0));
        if (!repo->approval_pending(pair.id, pending, open)) return false;
        result->Result = SUPLA_SUPLAN_RESULT_OK;
        result->AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL;
      }
    }
    if (!repo->commit()) return false;
    if (lock.owns_lock()) lock.unlock();
    if (notify && changed) changed(current.id);
    return true;
  }
  return false;
}
TSD_SuplaEnsureResourceAccessResult PeerService::ensure_access(
    int destination, const TDS_SuplaEnsureResourceAccess &q,
    const std::function<bool()> &reconcile) {
  TSD_SuplaEnsureResourceAccessResult result = {
      SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT, SUPLA_SUPLAN_ACCESS_STATUS_INVALID,
      SUPLA_RESOURCE_DELIVERY_INVALID};
  if ((q.Flags & ~SUPLA_SUPLAN_ENSURE_ACCESS_FLAG_ALLOW_APPROVAL) ||
      (q.DeliveryMode != SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER &&
       q.DeliveryMode != SUPLA_RESOURCE_DELIVERY_SERVER_STREAM) ||
      destination <= 0 || !q.Resource.ResourceId ||
      q.Resource.ResourceId > INT32_MAX || !valid_permissions(q.Permissions))
    return result;
  result.DeliveryMode = q.DeliveryMode;
  if (q.DeliveryMode == SUPLA_RESOURCE_DELIVERY_SERVER_STREAM) {
    if (q.Resource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL ||
        q.Permissions != SUPLA_SUPLAN_PERMISSION_READ) {
      result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
      return result;
    }
    return ensure_stream(destination, q.Resource.ResourceId, q.Flags != 0,
                         reconcile, true);
  }
  if ((q.Resource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
       q.Resource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE) ||
      (q.Flags && q.Resource.ResourceType !=
                      SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL)) {
    result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
    return result;
  }
  auto repo = factory();
  int requester = 0;
  if (!repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, destination,
                   &requester)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  if (requester != destination) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    return result;
  }
  int source = 0;
  if (!repo->owner(q.Resource.ResourceType, q.Resource.ResourceId, &source)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  if (!source) {
    // A missing resource still requires authoritative cleanup of existing
    // origins. The authenticated requester and wire shape are checked first.
    if ((reconcile && !reconcile()) ||
        (q.Resource.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
         !reconcile_approvals(q.Resource.ResourceId, destination, nullptr,
                              true))) {
      result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
      return result;
    }
    result.Result = SUPLA_SUPLAN_RESULT_NOT_FOUND;
    return result;
  }
  int owner = 0;
  if (!repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, source, &owner)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  if (requester != destination || owner != source || source == destination) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    return result;
  }
  // ACTION is event permission, not CONTROL_ACTION. Even an old broad ACL
  // cannot make a non-AT channel a supported event resource.
  if (q.Resource.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
      (q.Flags || (q.Permissions & SUPLA_SUPLAN_PERMISSION_ACTION))) {
    ChannelInfo info;
    if (!repo->channel(q.Resource.ResourceId, &info)) {
      result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
      return result;
    }
    if (info.is_virtual) {
      result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
      return result;
    }
    if (info.device != source) {
      result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
      return result;
    }
    if ((q.Flags && !manual_permissions_supported(info, q.Permissions)) ||
        !info.function ||
        ((q.Permissions & SUPLA_SUPLAN_PERMISSION_ACTION) &&
         (info.type != SUPLA_CHANNELTYPE_ACTIONTRIGGER ||
          info.function != SUPLA_CHANNELFNC_ACTIONTRIGGER ||
          !info.action_capabilities))) {
      result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
      return result;
    }
  }
  if (q.Flags) {
    DeviceInfo from, to;
    Identity identity;
    if (!repo->device(source, &from) || !repo->device(destination, &to) ||
        !repo->identity(source, &identity)) {
      result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
      return result;
    }
    if (from.protocol < 29 || to.protocol < 29 ||
        !(from.flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) ||
        !(to.flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED)) {
      result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
      return result;
    }
    if (!identity.root ||
        std::find(identity.channels.begin(), identity.channels.end(),
                  static_cast<uint32_t>(q.Resource.ResourceId)) ==
            identity.channels.end()) {
      result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
      return result;
    }
  }
  // Validate before a hint can touch config-derived authorization. Transport
  // remains outside any approval transaction (the Cloud schema is required).
  if (reconcile && !reconcile()) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  bool approval_available = false;
  if (!repo->approval_schema(&approval_available) ||
      (approval_available &&
       q.Resource.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
       !reconcile_approvals(q.Resource.ResourceId, destination, nullptr,
                            true))) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
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
  if (!missing_permissions(q.Permissions, allowed)) {
    Identity current;
    if (!repo->identity(source, &current)) {
      result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
      return result;
    }
    result.Result = SUPLA_SUPLAN_RESULT_OK;
    result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_GRANTED;
    // Mutations and reconnect already reconcile ACL-only/offline work. A
    // status ENSURE must not recreate a retired, fully acknowledged flow.
    if (changed && (a.root != current.root ||
                    a.provisioned_generation != a.generation))
      changed(a.id);
  } else {
    if (q.Flags && approval_available) {
      if (!reconcile_approval_pair({0, source, destination,
                                    q.Resource.ResourceId},
                                  q.Permissions, true, &result, true)) {
        result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
        result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_INVALID;
      }
      return result;
    }
    // The Cloud migration is required for a durable pending decision.
    // Fail closed; never pretend an in-memory request is PENDING_APPROVAL.
    // Existing partial grants remain untouched.
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
  }
  return result;
}

TSD_SuplaEnsureResourceAccessResult PeerService::ensure_stream(
    int destination, uint32_t resource, bool allow_approval,
    const std::function<bool()> &reconcile, bool record_request) {
  TSD_SuplaEnsureResourceAccessResult result = {
      SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, SUPLA_SUPLAN_ACCESS_STATUS_INVALID,
      SUPLA_RESOURCE_DELIVERY_SERVER_STREAM};
  auto repo = factory();
  int owner = 0;
  DeviceInfo device;
  Identity identity;
  ChannelInfo channel;
  if (!repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, destination, &owner) ||
      !repo->device(destination, &device) ||
      !repo->identity(destination, &identity) ||
      !repo->channel(resource, &channel)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  if (owner != destination || !identity.root || identity.root == UINT32_MAX)
    return result;
  if (device.is_virtual || device.protocol < 29 ||
      !(device.flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED)) {
    result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
    return result;
  }
  if (!channel.device || !channel.type) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_FOUND;
    return result;
  }
  if (channel.device == destination) return result;
  if (!stream_read_supported(channel)) {
    result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
    return result;
  }
  if (reconcile && !reconcile()) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  uint8_t allowed = 0;
  if (!repo->stream_permissions(destination, resource, &allowed)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  const bool covered =
      !missing_permissions(SUPLA_SUPLAN_PERMISSION_READ, allowed);
  if (covered && !record_request) {
    result.Result = SUPLA_SUPLAN_RESULT_OK;
    result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_GRANTED;
    return result;
  }
  bool available = false;
  if (!repo->stream_schema(&available)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  if (!available) {
    if (covered) {
      result.Result = SUPLA_SUPLAN_RESULT_OK;
      result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_GRANTED;
    }
    return result;
  }
  ApprovalPair pair{0, channel.device, destination, resource};
  std::vector<ApprovalPair> pairs;
  if (!repo->approval_pairs(resource, destination, &pairs)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  if (pairs.empty() && (covered || !allow_approval)) {
    if (covered) {
      result.Result = SUPLA_SUPLAN_RESULT_OK;
      result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_GRANTED;
    }
    return result;
  }
  if (!repo->approval_lock(&pair, allow_approval)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  std::vector<ApprovalDecision> decisions;
  ChannelInfo current;
  DeviceInfo from, to;
  Identity accepted;
  if (!repo->approval_context(pair, &current, &from, &to, &accepted) ||
      !repo->approval_decisions(pair.id, &decisions) ||
      !repo->identity(destination, &identity)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  if (pair.source != current.device || current.device != channel.device ||
      !stream_read_supported(current) || !identity.root ||
      identity.root == UINT32_MAX || to.is_virtual || to.protocol < 29 ||
      !(to.flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED)) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    return result;
  }
  if (covered) {
    // Recheck after the shared pair fence; a concurrent removal cannot be
    // answered from the earlier permission snapshot.
    if (!repo->stream_permissions(destination, resource, &allowed)) {
      result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
      return result;
    }
    if (!missing_permissions(SUPLA_SUPLAN_PERMISSION_READ, allowed)) {
      result.Result = SUPLA_SUPLAN_RESULT_OK;
      result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_GRANTED;
    }
  }
  const ApprovalDecision *open = nullptr;
  for (const auto &d : decisions) {
    if (record_request && d.delivery_mode != 2 &&
        ((d.status == ApprovalStatus::Accepted && (d.permissions & 3)) ||
         ((d.status == ApprovalStatus::Pending ||
           d.status == ApprovalStatus::Rejected) && d.permissions == 1)) &&
        !repo->approval_stream_mode(d)) {
      result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
      return result;
    }
    if (d.status == ApprovalStatus::Pending ||
        d.status == ApprovalStatus::Rejected) open = &d;
    if (d.status == ApprovalStatus::Accepted &&
        !missing_permissions(SUPLA_SUPLAN_PERMISSION_READ, d.permissions)) {
      result.Result = SUPLA_SUPLAN_RESULT_OK;
      result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_GRANTED;
    }
  }
  if (result.AccessStatus != SUPLA_SUPLAN_ACCESS_STATUS_GRANTED) {
    if (open && open->status == ApprovalStatus::Rejected) {
      result.Result = SUPLA_SUPLAN_RESULT_OK;
      result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_REJECTED;
    } else if (open || allow_approval) {
      if (!open && !repo->approval_stream_pending(pair.id, nullptr)) {
        result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
        return result;
      }
      result.Result = SUPLA_SUPLAN_RESULT_OK;
      result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL;
    }
  }
  if (!repo->commit()) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_INVALID;
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
