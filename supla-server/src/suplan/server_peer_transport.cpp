// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include "suplan/server_peer_transport.h"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "device/device.h"
#include "device/device_dao.h"
#include "jsonconfig/channel/container_config.h"
#include "jsonconfig/channel/hvac_config.h"
#include "jsonconfig/channel/valve_config.h"
#include "log.h"
#include "srpc/srpc.h"
#include "suplan/peer_dao.h"
#include "user.h"
#include "svrcfg.h"
#include "user/user_dao.h"
supla_suplan_server_peers::supla_suplan_server_peers(supla_user *user)
    : user(user),
      provisioner(
          [user]() {
            return std::make_unique<supla_suplan::PeerDao>(user->getUserID());
          },
          this) {}
bool supla_suplan_server_peers::ready(int id) {
  auto d = user->get_devices()->get(id);
  return d && d->get_connection() && d->is_suplan_peer_ready();
}
bool supla_suplan_server_peers::owner_ready(int id, uint32_t root) {
  auto d = user->get_devices()->get(id);
  return !d ||
         (d->is_suplan_peer_ready() && d->get_suplan_root_epoch() == root);
}
bool supla_suplan_server_peers::send(int id,
                                     TSDS_SuplaSetSuplanSourceAssociation *q) {
  auto d = user->get_devices()->get(id);
  if (!d || !d->get_connection() || !d->is_suplan_peer_ready() ||
      d->get_suplan_root_epoch() != q->PeerContext.RootEpoch)
    return false;
  auto srpc = d->get_connection()->get_srpc_adapter();
  srpc->lock();
  bool ok =
      srpc_sd_async_set_suplan_source_association(srpc->get_srpc(), q) > 0;
  srpc->unlock();
  return ok;
}
bool supla_suplan_server_peers::send(
    int id, TSDS_SuplaSetSuplanDestinationAssociation *q) {
  auto d = user->get_devices()->get(id);
  if (!d || !d->get_connection() || !d->is_suplan_peer_ready()) return false;
  auto srpc = d->get_connection()->get_srpc_adapter();
  srpc->lock();
  bool ok =
      srpc_sd_async_set_suplan_destination_association(srpc->get_srpc(), q) > 0;
  srpc->unlock();
  return ok;
}
void supla_suplan_server_peers::rebootstrap(int source) {
  auto d = user->get_devices()->get(source);
  if (d) {
    d->reset_suplan_identity_bootstrap();
    d->terminate();
  }
}
void supla_suplan_server_peers::diagnostic(uint64_t id, int device,
                                           uint8_t result) {
  // Metadata only: no provisioning structure or credential bytes.
  supla_log(LOG_WARNING,
            "SupLAN provisioning: association=%llu device=%d result=%u",
            static_cast<unsigned long long>(id), device, result);
}
void supla_suplan_server_peers::transient_diagnostic(uint64_t id, int device,
                                                   bool timeout) {
  supla_log(LOG_WARNING,
            "SupLAN provisioning: association=%llu device=%d transport=%s",
            static_cast<unsigned long long>(id), device,
            timeout ? "timeout" : "send-failed");
}

bool supla_suplan_server_peers::reconcile_manual(int user_id,
                                                 uint32_t source_channel) {
  supla_suplan::PeerService service(
      [user_id]() { return std::make_unique<supla_suplan::PeerDao>(user_id); },
      [user_id](uint64_t id) {
        auto user = supla_user::find(user_id, false);
        if (user) user->get_suplan_peers()->peers()->reconcile(id);
      });
  return service.reconcile_approvals(source_channel);
}
bool supla_suplan_server_peers::reconcile_dependencies(int user_id,
                                                       int source_channel_id) {
  supla_suplan::PeerDao repo(user_id);
  std::vector<uint64_t> associations;
  if (!repo.for_resource(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, source_channel_id,
                         &associations))
    return false;
  // Pump/HOCS grants name HVAC, not the changed relay target. Include the
  // endpoint's existing origins even when Cloud already cleared the old key.
  supla_suplan::ChannelInfo metadata;
  if (!repo.channel(source_channel_id, &metadata)) {
    return false;
  }
  if (metadata.device) {
    std::vector<uint64_t> endpoint_associations;
    if (!repo.for_device(metadata.device, false, &endpoint_associations)) {
      return false;
    }
    associations.insert(associations.end(), endpoint_associations.begin(),
                        endpoint_associations.end());
    std::sort(associations.begin(), associations.end());
    associations.erase(std::unique(associations.begin(), associations.end()),
                       associations.end());
  }
  std::vector<uint64_t> destinations;
  if (!repo.hvac_dependents(source_channel_id, &destinations)) return false;
  for (auto association : associations) {
    std::vector<supla_suplan::Grant> grants;
    if (!repo.grants(association, &grants)) return false;
    for (const auto &grant : grants) {
      if (grant.origin_type != supla_suplan::CHANNEL_CONFIG_ORIGIN) {
        continue;
      }
      uint32_t channel_id = grant.origin_id >> 8;
      destinations.push_back(channel_id);
    }
  }
  std::sort(destinations.begin(), destinations.end());
  destinations.erase(std::unique(destinations.begin(), destinations.end()),
                     destinations.end());
  bool reconciled = true;
  for (auto channel : destinations) {
    // A failed desired expansion must not skip revocation in later consumers.
    if (!reconcile_hvac(user_id, channel)) reconciled = false;
  }
  return reconciled;
}

bool supla_suplan_server_peers::reconcile_access_dependencies(
    int user_id, int resource_channel_id, int destination_device_id) {
  supla_suplan::PeerDao repo(user_id);
  using Origin = std::pair<uint32_t, uint8_t>;
  std::vector<Origin> origins;
  auto reserved = [](uint8_t field) {
    return (field >= 1 && field <= 6) || (field >= 32 && field < 42) ||
           (field >= 64 && field < 84);
  };
  std::vector<uint64_t> associations;
  if (!repo.for_resource(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL,
                         resource_channel_id, &associations))
    return false;
  for (auto id : associations) {
    supla_suplan::Association association;
    if (!repo.load(id, &association)) return false;
    if (association.destination != destination_device_id) continue;
    std::vector<supla_suplan::Grant> grants;
    if (!repo.grants(association.id, &grants)) return false;
    for (const auto &grant : grants) {
      uint8_t field = grant.origin_id & 255;
      if (grant.resource_type == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL &&
          grant.resource_id == static_cast<uint32_t>(resource_channel_id) &&
          grant.origin_type == supla_suplan::CHANNEL_CONFIG_ORIGIN &&
          reserved(field)) {
        // Include stale origins whose authoritative reference has changed.
        origins.emplace_back(grant.origin_id >> 8, field);
      }
    }
  }
  std::vector<uint64_t> consumers;
  if (!repo.hvac_dependents(resource_channel_id, &consumers)) return false;
  for (auto consumer : consumers) {
    supla_suplan::ChannelInfo info;
    if (!repo.channel(consumer, &info)) return false;
    if (info.device != destination_device_id) continue;
    for (unsigned field = 1; field < 84; ++field) {
      if (!reserved(field) || field == 5 || field == 6) continue;
      uint32_t reference = 0;
      if (!repo.reference(consumer, field, &reference)) return false;
      if (reference == static_cast<uint32_t>(resource_channel_id))
        origins.emplace_back(consumer, field);
    }
  }
  // Pump/HOCS invert the roles: the requested resource is the configuring
  // HVAC, while the requesting Device owns its referenced relay endpoint.
  for (uint8_t field : {5, 6}) {
    uint32_t reference = 0;
    supla_suplan::ChannelInfo target;
    if (!repo.reference(resource_channel_id, field, &reference)) return false;
    if (!reference) continue;
    if (!repo.channel(reference, &target)) return false;
    if (target.device == destination_device_id)
      origins.emplace_back(resource_channel_id, field);
  }
  std::sort(origins.begin(), origins.end());
  origins.erase(std::unique(origins.begin(), origins.end()), origins.end());
  supla_user *user = supla_user::find(user_id, false);
  supla_suplan::PeerService service(
      [user_id]() { return std::make_unique<supla_suplan::PeerDao>(user_id); },
      [user](uint64_t id) {
        if (user) user->get_suplan_peers()->peers()->reconcile(id);
      });
  bool reconciled = true;
  for (const auto &origin : origins) {
    // Each origin commits independently. A failed expansion must not starve
    // later stale-origin revocation; ENSURE still fails closed overall.
    if (!service.reconcile_access_reference(
            origin.first, origin.second, resource_channel_id,
            destination_device_id, [&](uint32_t *reference) {
              return repo.reference(origin.first, origin.second, reference);
            }))
      reconciled = false;
  }
  return reconciled;
}

bool supla_suplan_server_peers::reconcile_hvac(
    int user_id, int channel_id, std::unique_ptr<supla_json_config> *current,
    int *selected_function) {
  supla_user *user = supla_user::find(user_id, false);
  supla_suplan::PeerService service(
      [user_id]() { return std::make_unique<supla_suplan::PeerDao>(user_id); },
      [user](uint64_t id) {
        if (user) user->get_suplan_peers()->peers()->reconcile(id);
      });
  std::unique_ptr<supla_json_config> config;
  int function = 0;
  supla_suplan::PeerDao repo(user_id);
  supla_suplan::ChannelInfo info;
  if (!repo.channel(channel_id, &info)) {
    return false;
  }
  std::vector<uint64_t> existing_fields;
  if (!repo.config_origin_fields(channel_id, &existing_fields)) return false;
  std::vector<uint8_t> fields;
  // Revisit every reserved config origin for reference-bearing Channels.
  // This also revokes the old family's origins after a Type change.
  if (!info.device || info.type == SUPLA_CHANNELTYPE_HVAC ||
      info.type == SUPLA_CHANNELTYPE_CONTAINER ||
      info.type == SUPLA_CHANNELTYPE_VALVE_OPENCLOSE ||
      info.type == SUPLA_CHANNELTYPE_VALVE_PERCENTAGE) {
    for (uint8_t f = 1; f <= 6; ++f) {
      fields.push_back(f);
    }
    for (uint8_t f = 32; f < 42; ++f) {
      fields.push_back(f);
    }
    for (uint8_t f = 64; f < 84; ++f) {
      fields.push_back(f);
    }
  }
  if (fields.empty()) {
    for (auto field : existing_fields) {
      if ((field >= 1 && field <= 6) || (field >= 32 && field < 42) ||
          (field >= 64 && field < 84)) {
        fields.push_back(field);
      }
    }
  }
  auto reload = [&]() {
    supla_mariadb_access_provider db;
    supla_device_dao dao(&db);
    config.reset(dao.get_channel_config(channel_id, nullptr, nullptr,
                                        &function));
    if (!config) {
      supla_suplan::ChannelInfo exists;
      return repo.channel(channel_id, &exists) && !exists.device;
    }
    return true;
  };
  if (fields.empty()) {
    bool ok = reload();
    if (ok && selected_function) *selected_function = function;
    if (ok && current) {
      *current = std::move(config);
    }
    return ok;
  }
  // Audit persisted origins before any desired ACL expansion. Each read is
  // protected by its origin gate; no new Grant is created by this sweep.
  // Continue after a DB fault so other stale origins still get a cleanup try.
  bool revoked = true;
  for (auto field : existing_fields) {
    if (!((field >= 1 && field <= 6) || (field >= 32 && field < 42) ||
          (field >= 64 && field < 84))) continue;
    if (!service.revoke_stale_reference(channel_id, field,
                                       [&](uint32_t *reference) {
                                         return repo.reference(channel_id,
                                                               field,
                                                               reference);
                                       })) revoked = false;
  }
  if (!revoked) return false;
  // A full payload must correspond to one coherent authoritative root. A
  // writer may change another field while its predecessor's origin is being
  // reconciled. Retry the bounded field pass, never deliver that mixed root.
  for (unsigned attempt = 0; attempt < 3; ++attempt) {
    std::string expected_user, expected_properties;
    bool captured = false, changed = false, ok = true;
    int expected_function = 0;
    for (auto field : fields) {
      ok =
          service.reconcile_reference(channel_id, field, [&](uint32_t *source) {
            if (!reload()) {
              return false;
            }
            char *user_json = config ? config->get_user_config() : nullptr;
            char *properties_json = config ? config->get_properties() : nullptr;
            std::string user_root = user_json ? user_json : "";
            std::string properties_root =
                properties_json ? properties_json : "";
            free(user_json);
            free(properties_json);
            if (!captured) {
              expected_user = user_root;
              expected_properties = properties_root;
              expected_function = function;
              captured = true;
            } else if (expected_user != user_root ||
                       expected_properties != properties_root ||
                       expected_function != function) {
              changed = true;
              return false;
            }
            *source = 0;
            if (field <= 6) {
              *source = hvac_config(config.get()).reference(field - 1);
            } else if (field < 42) {
              TChannelConfig_Container raw = {};
              container_config(config.get()).get_config(&raw);
              *source = raw.SensorInfo[field - 32].ChannelId;
            } else {
              TChannelConfig_Valve raw = {};
              valve_config(config.get()).get_config(&raw);
              *source = raw.SensorInfo[field - 64].ChannelId;
            }
            return true;
          });
      if (!ok) {
        break;
      }
    }
    if (ok) {
      if (selected_function) *selected_function = function;
      if (current) {
        *current = std::move(config);
      }
      return true;
    }
    if (!changed) {
      return false;  // real DB/authorization fault, no retry storm
    }
  }
  return false;  // bounded churn: terminate/reconnect/replay at the sender
}

bool supla_suplan_server_peers::channel_deleted(int user_id, int channel_id) {
  supla_suplan::PeerService service(
      [user_id]() { return std::make_unique<supla_suplan::PeerDao>(user_id); },
      [user_id](uint64_t id) {
        auto user = supla_user::find(user_id, false);
        if (user) user->get_suplan_peers()->peers()->reconcile(id);
      });
  // Consumer deletion must clear every stable field/slot origin, including
  // source-first HVAC grants whose resource may otherwise survive elsewhere.
  for (uint8_t field = 1; field < 84; ++field) {
    if ((field > 6 && field < 32) || (field >= 42 && field < 64)) {
      continue;
    }
    if (!service.delete_origin(
            supla_suplan::CHANNEL_CONFIG_ORIGIN,
            supla_suplan::channel_config_origin(channel_id, field))) {
      return false;
    }
  }
  // Revoke the deleted READ resource before auditing binding targets.
  if (!service.delete_resource(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL,
                               channel_id)) {
    return false;
  }
  // Binding grants name HVAC, not the relay. Cloud may already have cleared
  // the old target key, so audit persisted origins against current authority.
  // Revoke only: a different pending relation must not expand an ACL here.
  supla_suplan::PeerDao repo(user_id);
  std::vector<uint64_t> binding_consumers;
  if (!repo.binding_consumers(&binding_consumers)) {
    return false;
  }
  bool cleaned = true;
  for (auto consumer : binding_consumers) {
    for (uint8_t field : {5, 6}) {
      if (!service.revoke_stale_reference(
              consumer, field, [&](uint32_t *target) {
                if (!repo.reference(consumer, field, target)) return false;
                // IPC can arrive before the channel row is physically gone.
                if (*target == static_cast<uint32_t>(channel_id)) *target = 0;
                return true;
              })) {
        cleaned = false;  // keep cleaning other origins after a real DB fault
      }
    }
  }
  supla_mariadb_access_provider db;
  if (!db.connect()) return false;
  for (const char *key : hvac_config::reference_keys) {
    std::string sql =
        "UPDATE supla_dev_channel SET user_config=JSON_SET("
        "user_config,'$." +
        std::string(key) + "',NULL) WHERE user_id=" + std::to_string(user_id) +
        " AND type=" + std::to_string(SUPLA_CHANNELTYPE_HVAC) +
        " AND JSON_VALUE(user_config,'$." + key +
        "')=" + std::to_string(channel_id);
    if (db.query(sql.c_str(), true)) return false;
  }
  return cleaned;
}
bool supla_suplan_server_peers::device_deleted(int user_id, int device_id) {
  supla_suplan::PeerService service(
      [user_id]() { return std::make_unique<supla_suplan::PeerDao>(user_id); },
      [user_id](uint64_t id) {
        auto user = supla_user::find(user_id, false);
        if (user) user->get_suplan_peers()->peers()->reconcile(id);
      });
  if (!service.remove_device(device_id)) return false;
  // Cloud may already have deleted the Source's Channels. Clear any orphan
  // reference as well as references to an endpoint still awaiting DB deletion.
  supla_mariadb_access_provider db;
  if (!db.connect()) return false;
  for (const char *key : hvac_config::reference_keys) {
    std::string path = "$." + std::string(key);
    std::string sql =
        "UPDATE supla_dev_channel c LEFT JOIN supla_dev_channel s "
        "ON s.id=JSON_VALUE(c.user_config,'" +
        path +
        "') "
        "AND s.user_id=c.user_id SET c.user_config=JSON_SET(c.user_config,'" +
        path + "',NULL) WHERE c.user_id=" + std::to_string(user_id) +
        " AND c.type=" + std::to_string(SUPLA_CHANNELTYPE_HVAC) +
        " AND JSON_VALUE(c.user_config,'" + path +
        "')>0 "
        "AND (s.id IS NULL OR s.iodevice_id=" +
        std::to_string(device_id) + ")";
    if (db.query(sql.c_str(), true)) return false;
  }
  return true;
}

std::shared_ptr<supla_suplan::StreamConnection>
supla_suplan_server_peers::new_stream(supla_device *destination) {
  using namespace supla_suplan;  // NOLINT
  int device = destination->get_id();
  int account = user->getUserID();
  auto stream = std::make_shared<StreamConnection>(
      std::max(0, scfg_int(CFG_SERVER_STREAM_MAX_CHANNELS)),
      [this, account, device](uint32_t channel, bool approval, bool request) {
        return provisioner.grants()->ensure_stream(device, channel, approval,
            [account, device, channel] {
              return reconcile_access_dependencies(account, channel, device);
            }, request);
      },
      [this, account](uint32_t channel, StreamConnection::Snapshot *snapshot) {
        PeerDao dao(account);
        if (!dao.stream_metadata(channel, &snapshot->state.Channel))
          return false;
        snapshot->state.Channel.Offline = SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE;
        auto physical = user->get_devices()->get(0, channel);
        bool has_expiry = false;
        int64_t remaining = 0;
        if (physical) {
          physical->get_channels()->access_channel(channel,
              [&](supla_device_channel *source) {
                source->get_stream_value(&snapshot->state.Channel, &has_expiry,
                                         &remaining);
              });
        } else {
          supla_mariadb_access_provider db;
          supla_user_dao virtuals(&db);
          // Read one current persisted value: virtual or disconnected
          // physical cache. The legacy incremental cache is second-granular.
          auto channels = virtuals.get_virtual_channels(user, 0, channel);
          if (!channels.empty()) {
            has_expiry = true;
            channels.front().get_stream_value(&snapshot->state.Channel,
                                               &remaining);
          } else {
            auto source = user->get_devices()->get_ocpp_channel(channel);
            if (source.get_channel_id()) {
              source.get_value(snapshot->state.Channel.value);
              snapshot->state.Channel.Offline =
                  source.get_availability_status().get_proto_offline();
              auto ttl = source.get_value_validity_time_sec();
              has_expiry = ttl > 0;
              remaining = static_cast<int64_t>(ttl) * 1000000;
              snapshot->state.Channel.ValueValidityTimeSec = ttl;
            }
          }
        }
        if (has_expiry) {
          // SQL DATETIME can exceed both the wire TTL and steady_clock range.
          remaining = std::max<int64_t>(0, std::min<int64_t>(
              remaining, static_cast<int64_t>(UINT32_MAX) * 1000000));
          snapshot->expires = std::chrono::steady_clock::now() +
                              std::chrono::microseconds(remaining);
        }
        return true;
      },
      [destination](const TSD_SuplaRemoteChannelState &snapshot) {
        auto adapter = destination->get_connection()->get_srpc_adapter();
        adapter->lock();
        auto copy = snapshot;
        bool ok = srpc_sd_async_remote_channel_state(adapter->get_srpc(),
                                                     &copy) > 0;
        adapter->unlock();
        if (!ok) destination->get_connection()->terminate();
        return ok;
      },
      [destination] {
        return destination->get_connection() &&
               destination->get_protocol_version() >= 29 &&
               (destination->get_flags() &
                SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) &&
               destination->is_server_stream_ready();
      });
  std::lock_guard<std::mutex> lock(stream_mutex);
  streams.erase(std::remove_if(streams.begin(), streams.end(),
      [](const std::weak_ptr<StreamConnection> &s) { return s.expired(); }),
      streams.end());
  streams.push_back(stream);
  return stream;
}
void supla_suplan_server_peers::stream_dirty(uint32_t channel) {
  std::vector<std::shared_ptr<supla_suplan::StreamConnection>> current;
  {
    std::lock_guard<std::mutex> lock(stream_mutex);
    for (auto it = streams.begin(); it != streams.end();) {
      auto stream = it->lock();
      if (stream) {
        current.push_back(stream);
        ++it;
      } else {
        it = streams.erase(it);
      }
    }
  }
  for (const auto &stream : current) stream->dirty(channel);
}

supla_suplan_server_peers::~supla_suplan_server_peers() {
  // Runtime shutdown ends connection demands before callbacks lose their
  // account/service owner. Do not hold the registry lock while closing.
  std::vector<std::shared_ptr<supla_suplan::StreamConnection>> current;
  {
    std::lock_guard<std::mutex> lock(stream_mutex);
    for (auto &weak : streams) if (auto stream = weak.lock())
      current.push_back(stream);
    streams.clear();
  }
  for (const auto &stream : current) stream->close();
}
