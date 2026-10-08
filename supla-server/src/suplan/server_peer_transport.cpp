// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include "suplan/server_peer_transport.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "device/device.h"
#include "device/device_dao.h"
#include "jsonconfig/channel/hvac_config.h"
#include "log.h"
#include "srpc/srpc.h"
#include "suplan/peer_dao.h"
#include "user.h"
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

bool supla_suplan_server_peers::reconcile_dependencies(int user_id,
                                                       int source_channel_id) {
  supla_suplan::PeerDao repo(user_id);
  std::vector<uint64_t> associations;
  if (!repo.for_resource(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, source_channel_id,
                         &associations))
    return false;
  std::vector<uint64_t> destinations;
  if (!repo.hvac_dependents(source_channel_id, &destinations)) return false;
  for (auto association : associations) {
    std::vector<supla_suplan::Grant> grants;
    if (!repo.grants(association, &grants)) return false;
    for (const auto &grant : grants) {
      if (grant.origin_type != supla_suplan::CHANNEL_CONFIG_ORIGIN ||
          (grant.origin_id & 255) != 1)
        continue;
      uint32_t channel_id = grant.origin_id >> 8;
      destinations.push_back(channel_id);
    }
  }
  std::sort(destinations.begin(), destinations.end());
  destinations.erase(std::unique(destinations.begin(), destinations.end()),
                     destinations.end());
  for (auto channel : destinations)
    if (!reconcile_hvac(user_id, channel)) return false;
  return true;
}

bool supla_suplan_server_peers::reconcile_hvac(
    int user_id, int channel_id, std::unique_ptr<supla_json_config> *current) {
  supla_user *user = supla_user::find(user_id, false);
  supla_suplan::PeerService service(
      [user_id]() { return std::make_unique<supla_suplan::PeerDao>(user_id); },
      [user](uint64_t id) {
        if (user) user->get_suplan_peers()->peers()->reconcile(id);
      });
  std::unique_ptr<supla_json_config> config;
  bool ok = service.reconcile_main_thermometer(
      channel_id, [&](uint32_t *source) {
    supla_mariadb_access_provider db;
    supla_device_dao dao(&db);
    config.reset(dao.get_channel_config(channel_id, nullptr, nullptr));
    if (!config) {
      supla_suplan::PeerDao repo(user_id);
      supla_suplan::ChannelInfo info;
      if (!repo.channel(channel_id, &info) || info.device) return false;
    }
    *source = hvac_config(config.get()).reference(0);
    return true;
  });
  if (ok && current) *current = std::move(config);
  return ok;
}

bool supla_suplan_server_peers::channel_deleted(int user_id, int channel_id) {
  supla_suplan::PeerService service(
      [user_id]() { return std::make_unique<supla_suplan::PeerDao>(user_id); },
      [user_id](uint64_t id) {
        auto user = supla_user::find(user_id, false);
        if (user) user->get_suplan_peers()->peers()->reconcile(id);
      });
  if (!(service.delete_origin(
            supla_suplan::CHANNEL_CONFIG_ORIGIN,
            supla_suplan::main_thermometer_origin(channel_id)) &&
        service.delete_resource(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL,
                                channel_id)))
    return false;
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
  return true;
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
