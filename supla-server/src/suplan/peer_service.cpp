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

bool repository::observe_root(uint64_t id) {
  association_t a;
  identity_t identity;
  if (!begin(id, &a) || !this->identity(a.source, &identity)) return false;
  if (a.lifecycle == lifecycle_t::draining && !a.source_acked &&
      (a.source_removed ||
       (a.root && identity.root && a.root != identity.root))) {
    a.source_acked = true;
    if (a.destination_acked) a.lifecycle = lifecycle_t::dormant;
    if (!save(a)) return false;
  }
  return commit();
}

bool repository::owns_acl(int source, int destination, const acl_t &acl) {
  bool owned = false;
  return check_acl_ownership(source, destination, acl, &owned) && owned;
}
bool repository::check_acl_ownership(int source, int destination,
                                     const acl_t &acl, bool *owned) {
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
static bool active_binary_source(const channel_info_t &info);
static bool manual_permissions_supported(const channel_info_t &c, uint8_t p) {
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
bool stream_read_supported(const channel_info_t &channel) {
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
        return stream_read_supported(
            channel_info_t{c.device, SUPLA_CHANNELTYPE_RELAYHFD4, c.function});
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

static bool active_binary_source(const channel_info_t &info) {
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

bool canonicalize(const acl_t &input, acl_t *acl) {
  std::map<std::pair<uint8_t, uint32_t>, uint8_t> entries;
  for (const auto &g : input) {
    if (!valid_permissions(g.Permissions) || !g.ResourceId ||
        (g.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
         g.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE))
      return false;
    entries[{g.ResourceType, g.ResourceId}] |= g.Permissions;
  }
  if (entries.size() > SUPLA_SUPLAN_MAX_ACL_ENTRIES) return false;
  acl->clear();
  for (const auto &e : entries) {
    acl->push_back({e.first.first, e.first.second, normalize(e.second)});
  }
  return true;
}
bool equal_acl(const acl_t &a, const acl_t &b) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i) {
    if (a[i].ResourceType != b[i].ResourceType ||
        a[i].ResourceId != b[i].ResourceId ||
        a[i].Permissions != b[i].Permissions)
      return false;
  }
  return true;
}
bool same_state(const association_t &a, const association_t &b) {
  return a.id == b.id && a.source == b.source &&
         a.destination == b.destination && a.root == b.root &&
         a.provisioned_generation == b.provisioned_generation &&
         a.generation == b.generation && a.revision == b.revision &&
         a.lifecycle == b.lifecycle && a.source_removed == b.source_removed &&
         a.destination_removed == b.destination_removed &&
         equal_acl(a.acl, b.acl);
}
TSuplaSuplanPeerContext context(const association_t &a, uint32_t root) {
  return {SUPLA_SUPLAN_AUTHORITY_TYPE_SERVER,
          0,
          SUPLA_SUPLAN_NODE_ID_NAMESPACE_DEVICE_ID,
          static_cast<uint32_t>(a.source),
          SUPLA_SUPLAN_NODE_ID_NAMESPACE_DEVICE_ID,
          static_cast<uint32_t>(a.destination),
          root,
          a.generation};
}
bool matches(const TSuplaSuplanPeerContext &p, const association_t &a) {
  return p.AuthorityType == SUPLA_SUPLAN_AUTHORITY_TYPE_SERVER &&
         p.AuthorityId == 0 &&
         p.SourceNodeIdNamespace == SUPLA_SUPLAN_NODE_ID_NAMESPACE_DEVICE_ID &&
         p.DestinationNodeIdNamespace ==
             SUPLA_SUPLAN_NODE_ID_NAMESPACE_DEVICE_ID &&
         p.SourceNodeId == static_cast<uint32_t>(a.source) &&
         p.DestinationNodeId == static_cast<uint32_t>(a.destination) &&
         p.RootEpoch == a.root && p.PeerGeneration == a.generation;
}
std::vector<unsigned char> encode_acl(const acl_t &acl) {
  std::vector<unsigned char> out;
  for (const auto &e : acl) {
    out.push_back(e.ResourceType);
    for (int shift = 24; shift >= 0; shift -= 8)
      out.push_back(e.ResourceId >> shift);
    out.push_back(e.Permissions);
  }
  return out;
}
bool decode_acl(const unsigned char *data, size_t size, acl_t *acl) {
  if (size % 6 || size > SUPLA_SUPLAN_MAX_ACL_ENTRIES * 6) return false;
  acl_t entries;
  for (size_t i = 0; i < size; i += 6) {
    uint32_t id = 0;
    for (size_t j = 1; j < 5; ++j) id = (id << 8) | data[i + j];
    entries.push_back({data[i], id, data[i + 5]});
  }
  if (!canonicalize(entries, acl)) return false;
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
peer_service::peer_service(factory_t factory, changed_callback_t changed)
    : factory(factory), changed(changed) {}

bool peer_service::mutate(
    uint64_t id, const std::function<bool(repository *, association_t *)> &fn) {
  bool notify = false;
  {
    auto gate = association_gate(id);
    std::lock_guard<std::mutex> lock(*gate);
    auto repo = factory();
    association_t a;
    if (!repo->begin(id, &a)) return false;
    association_t before = a;
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
bool peer_service::cleanup_ack(const association_t &sent, bool source) {
  return mutate(sent.id, [&](repository *r, association_t *a) {
    if (!same_state(sent, *a) || a->lifecycle != lifecycle_t::draining)
      return true;
    bool &acked = source ? a->source_acked : a->destination_acked;
    if (acked) return true;
    acked = true;
    if (a->source_acked && a->destination_acked) {
      a->lifecycle = lifecycle_t::dormant;
    }
    return r->save(*a);
  });
}
bool peer_service::observe_root(uint64_t id) {
  auto gate = association_gate(id);
  std::lock_guard<std::mutex> lock(*gate);
  return factory()->observe_root(id);
}
bool peer_service::provisioned(const association_t &sent, uint32_t root) {
  return mutate(sent.id, [&](repository *r, association_t *a) {
    identity_t identity;
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

TSD_SuplaEnsureResourceAccessResult peer_service::ensure_access(
    int destination, const TDS_SuplaEnsureResourceAccess &q,
    const std::function<bool()> &) {
  TSD_SuplaEnsureResourceAccessResult result = {
      SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT, SUPLA_SUPLAN_ACCESS_STATUS_INVALID,
      SUPLA_RESOURCE_DELIVERY_INVALID};
  if (destination <= 0 || !q.Resource.ResourceId ||
      q.Resource.ResourceId > INT32_MAX || !valid_permissions(q.Permissions) ||
      (q.Flags & ~SUPLA_SUPLAN_ENSURE_ACCESS_FLAG_ALLOW_APPROVAL) ||
      (q.DeliveryMode != SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER &&
       q.DeliveryMode != SUPLA_RESOURCE_DELIVERY_SERVER_STREAM)) return result;
  result.DeliveryMode = q.DeliveryMode;
  const bool stream = q.DeliveryMode == SUPLA_RESOURCE_DELIVERY_SERVER_STREAM;
  if ((!stream && q.DeliveryMode != SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER) ||
      (stream &&
       (q.Resource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL ||
        q.Permissions != SUPLA_SUPLAN_PERMISSION_READ)) ||
      (q.Resource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
       q.Resource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE) ||
      (q.Flags &&
       q.Resource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL)) {
    result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
    return result;
  }
  if (stream) return ensure_stream(destination, q.Resource.ResourceId,
                                    q.Flags != 0, nullptr, true);
  auto repo = factory();
  device_info_t to, from;
  identity_t accepted;
  int source = 0, requester = 0;
  auto fail = [&]() {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_INVALID;
    return result;
  };
  if (!repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, destination, &requester))
    return fail();
  if (requester != destination) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    return result;
  }
  if (!repo->device(destination, &to) ||
      !repo->identity(destination, &accepted)) return fail();
  if (to.protocol < 29 || !(to.flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED)) {
    result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
    return result;
  }
  if (to.is_virtual || !accepted.root ||
      accepted.root == UINT32_MAX) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    return result;
  }
  if (!repo->owner(q.Resource.ResourceType, q.Resource.ResourceId, &source))
    return fail();
  if (!source) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_FOUND;
    return result;
  }
  if (source == destination) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    return result;
  }
  if (q.Resource.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL) {
    channel_info_t channel;
    if (!repo->channel(q.Resource.ResourceId, &channel)) return fail();
    if (channel.device != source) {
      result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
      return result;
    }
    if ((stream && !stream_read_supported(channel)) ||
        (!stream && (channel.is_virtual ||
          !manual_permissions_supported(channel, q.Permissions)))) {
      result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
      return result;
    }
  }
  if (!stream) {
    if (!repo->device(source, &from) || !repo->identity(source, &accepted))
      return fail();
    if (from.is_virtual || from.protocol < 29 ||
        !(from.flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED)) {
      result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
      return result;
    }
    if (!accepted.root || accepted.root == UINT32_MAX ||
        (q.Resource.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
         std::find(accepted.channels.begin(), accepted.channels.end(),
                   q.Resource.ResourceId) == accepted.channels.end())) {
      result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
      return result;
    }
  }
  uint8_t allowed = 0;
  if (!repo->permissions(destination, q.Resource.ResourceType,
                          q.Resource.ResourceId, !stream, &allowed))
    return fail();
  if (!missing_permissions(q.Permissions, allowed)) {
    if (!stream) {
      association_t association;
      if (!repo->find(source, destination, &association)) return fail();
      auto coverage = [&](const association_t &a) {
        uint8_t mask = 0;
        for (const auto &entry : a.acl)
          if ((entry.ResourceType == q.Resource.ResourceType &&
               entry.ResourceId == q.Resource.ResourceId) ||
              (entry.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE &&
               entry.ResourceId == static_cast<uint32_t>(source)))
            mask |= entry.Permissions;
        return a.id && a.lifecycle == lifecycle_t::active &&
               !a.source_removed && !a.destination_removed &&
               !missing_permissions(q.Permissions, mask);
      };
      // A request may project committed authority, never manufacture an origin.
      if (!coverage(association)) {
        if (q.Resource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL)
          return fail();
        if (!repo->project_peer(destination, q.Resource.ResourceId)) {
          if (!repo->permissions(destination, q.Resource.ResourceType,
                                  q.Resource.ResourceId, true, &allowed))
            return fail();
          if (!missing_permissions(q.Permissions, allowed)) return fail();
          result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
          return result;
        }
        if (!repo->find(source, destination, &association)) return fail();
        if (!association.id || !coverage(association)) {
          result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
          return result;
        }
        if (changed) changed(association.id);
      } else if (changed && (association.root != accepted.root ||
                             association.provisioned_generation !=
                                 association.generation)) {
        changed(association.id);
      }
    }
    result.Result = SUPLA_SUPLAN_RESULT_OK;
    result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_GRANTED;
    return result;
  }
  if (q.Resource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    return result;
  }
  std::vector<approval_pair_t> pairs;
  if (!repo->approval_pairs(q.Resource.ResourceId, destination, &pairs))
    return fail();
  result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
  bool pending_covers = false;
  for (const auto &pair : pairs) {
    std::vector<approval_decision_t> decisions;
    if (!repo->approval_decisions(pair.id, &decisions)) return fail();
    for (const auto &decision : decisions) {
      if (decision.status != approval_status::pending &&
          decision.status != approval_status::rejected)
        continue;
      result.Result = SUPLA_SUPLAN_RESULT_OK;
      result.AccessStatus = decision.status == approval_status::rejected
                                ? SUPLA_SUPLAN_ACCESS_STATUS_REJECTED
                                : SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL;
      if (decision.status == approval_status::rejected) return result;
      pending_covers = decision.delivery_mode == q.DeliveryMode &&
          !missing_permissions(missing_permissions(q.Permissions, allowed),
                                decision.permissions);
    }
  }
  if (q.Flags && !pending_covers) {
    if (!repo->request_pending(destination, q.Resource.ResourceId,
                               q.Permissions, q.DeliveryMode)) return fail();
    auto current = q;
    current.Flags = 0;
    return ensure_access(destination, current);
  }
  return result;
}
TSD_SuplaEnsureResourceAccessResult peer_service::ensure_stream(
    int destination, uint32_t resource, bool allow_approval,
    const std::function<bool()> &, bool record_request) {
  TSD_SuplaEnsureResourceAccessResult result = {
      SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT, SUPLA_SUPLAN_ACCESS_STATUS_INVALID,
      SUPLA_RESOURCE_DELIVERY_SERVER_STREAM};
  if (destination <= 0 || !resource || resource > INT32_MAX) return result;
  auto repo = factory();
  channel_info_t channel;
  uint8_t allowed = 0;
  bool ready = false;
  if (!repo->stream_access(destination, resource, &channel, &allowed, &ready)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  if (!ready || !channel.device || channel.device == destination) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    // Preserve ENSURE's unsupported endpoint result without extra hot-path
    // lookups for authorized value publication or quiet revalidation.
    if (record_request) {
      int owner = 0;
      device_info_t info;
      if (!repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, destination,
                       &owner) ||
          !repo->device(destination, &info)) {
        result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
      } else if (owner == destination &&
                 (info.protocol < 29 ||
                  !(info.flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED))) {
        result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
      }
    }
    return result;
  }
  if (!stream_read_supported(channel)) {
    result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
    return result;
  }
  if (!missing_permissions(SUPLA_SUPLAN_PERMISSION_READ, allowed)) {
    if (record_request) {
      std::vector<approval_pair_t> pairs;
      if (!repo->approval_pairs(resource, destination, &pairs)) {
        result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
        return result;
      }
      for (const auto &pair : pairs) {
        std::vector<approval_decision_t> decisions;
        if (!repo->approval_decisions(pair.id, &decisions)) {
          result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
          return result;
        }
        for (const auto &decision : decisions) {
          if (decision.status != approval_status::accepted ||
              decision.delivery_mode == SUPLA_RESOURCE_DELIVERY_SERVER_STREAM ||
              missing_permissions(SUPLA_SUPLAN_PERMISSION_READ,
                                  decision.permissions))
            continue;
          // Existing consent provenance only. A revoke racing this call must
          // never turn Flags=0 into a new approval request.
          if (!repo->request_pending(destination, resource, 1, 2, false)) {
            result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
            return result;
          }
          return ensure_stream(destination, resource, false);
        }
      }
    }
    result.Result = SUPLA_SUPLAN_RESULT_OK;
    result.AccessStatus = SUPLA_SUPLAN_ACCESS_STATUS_GRANTED;
    return result;
  }
  std::vector<approval_pair_t> pairs;
  if (!repo->approval_pairs(resource, destination, &pairs)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
  bool pending = false, stream_mode = false;
  for (const auto &pair : pairs) {
    std::vector<approval_decision_t> decisions;
    if (!repo->approval_decisions(pair.id, &decisions)) {
      result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
      return result;
    }
    for (const auto &decision : decisions) {
      if (decision.status != approval_status::pending &&
          decision.status != approval_status::rejected)
        continue;
      result.Result = SUPLA_SUPLAN_RESULT_OK;
      result.AccessStatus = decision.status == approval_status::rejected
                                ? SUPLA_SUPLAN_ACCESS_STATUS_REJECTED
                                : SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL;
      if (decision.status == approval_status::rejected) return result;
      pending = true;
      stream_mode = decision.delivery_mode ==
                    SUPLA_RESOURCE_DELIVERY_SERVER_STREAM;
    }
  }
  if (allow_approval && (!pending || !stream_mode)) {
    if (!repo->request_pending(destination, resource,
                              SUPLA_SUPLAN_PERMISSION_READ,
                              SUPLA_RESOURCE_DELIVERY_SERVER_STREAM)) {
      result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
      return result;
    }
    return ensure_stream(destination, resource, false);
  }
  return result;
}
TSD_SuplaEnsureResourceShareResult peer_service::ensure_share(
    int source, const TDS_SuplaEnsureResourceShare &request) {
  TSD_SuplaEnsureResourceShareResult result = {
      SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT, 0};
  if (!valid_permissions(request.Permissions) ||
      !request.SourceResource.ResourceId ||
      !request.DestinationResource.ResourceId ||
      request.SourceResource.ResourceId > INT32_MAX ||
      request.DestinationResource.ResourceId > INT32_MAX) return result;
  if (request.SourceResource.ResourceType !=
          SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL ||
      request.DestinationResource.ResourceType !=
          SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL) {
    result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
    return result;
  }
  auto repo = factory();
  int owner = 0, destination = 0;
  if (!repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL,
                   request.SourceResource.ResourceId, &owner) ||
      !repo->owner(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL,
                   request.DestinationResource.ResourceId, &destination)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  if (!owner || !destination) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_FOUND;
    return result;
  }
  if (owner != source || destination == source ||
      request.Permissions != SUPLA_SUPLAN_PERMISSION_READ) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    return result;
  }
  bool authorized = false;
  if (!repo->binding_granted(source, destination,
                            request.SourceResource.ResourceId,
                            request.DestinationResource.ResourceId,
                            &authorized)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  if (!authorized) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    return result;
  }
  if (!repo->project_share(destination, request.SourceResource.ResourceId,
                            request.DestinationResource.ResourceId)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    if (repo->binding_granted(source, destination,
                             request.SourceResource.ResourceId,
                             request.DestinationResource.ResourceId,
                             &authorized) && !authorized)
      result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    return result;
  }
  association_t association;
  if (!repo->find(source, destination, &association)) {
    result.Result = SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR;
    return result;
  }
  if (changed && association.id) changed(association.id);
  result.Result = SUPLA_SUPLAN_RESULT_OK;
  result.DestinationDeviceId = destination;
  return result;
}
}  // namespace supla_suplan
