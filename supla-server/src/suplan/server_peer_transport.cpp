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
#include "log.h"
#include "srpc/srpc.h"
#include "suplan/peer_dao.h"
#include "suplan/grant_changes_cyclictask.h"
#include "user.h"
#include "svrcfg.h"
#include "user/user_dao.h"
void supla_suplan_grant_changes_cyclictask::run(
    const std::vector<supla_user *> *users,
    supla_abstract_db_access_provider *) {
  for (auto user : *users) {
    bool active = false;
    user->get_devices()->for_each(
        [&](std::shared_ptr<supla_device> device, bool *more) {
          if (device->is_suplan_peer_ready()) {
            active = true;
            *more = false;
          }
        });
    if (active) user->get_suplan_peers()->drain_changes();
  }
}

supla_suplan_server_peers::supla_suplan_server_peers(supla_user *user)
    : user(user),
      provisioner(
          [user]() {
            return std::make_unique<supla_suplan::peer_dao>(user->getUserID());
          },
          this) {}
bool supla_suplan_server_peers::ready(int id) {
  auto d = user->get_devices()->get(id);
  return d && d->get_connection() && d->get_connection()->is_secure() &&
         d->is_suplan_peer_ready();
}
bool supla_suplan_server_peers::owner_ready(int id, uint32_t root) {
  auto d = user->get_devices()->get(id);
  return !d ||
         (d->is_suplan_peer_ready() && d->get_suplan_root_epoch() == root);
}
int supla_suplan_server_peers::recovery_device() {
  int result = 0;
  user->get_devices()->for_each(
      [&](std::shared_ptr<supla_device> d, bool *more) {
        if (d->is_suplan_peer_ready()) {
          result = d->get_id();
          *more = false;
        }
      });
  return result;
}
uint64_t supla_suplan_server_peers::session(int id) {
  auto d = user->get_devices()->get(id);
  return d && d->get_connection() ? d->get_connection()->get_session_id() : 0;
}
void supla_suplan_server_peers::wake(int id, uint64_t expected) {
  auto d = user->get_devices()->get(id);
  if (d && d->get_connection() &&
      d->get_connection()->get_session_id() == expected)
    d->get_connection()->raise_event();
}
bool supla_suplan_server_peers::reconnect_session(int id, uint64_t expected) {
  auto d = user->get_devices()->get(id);
  return d && d->get_connection() && d->is_suplan_peer_ready() &&
         d->get_connection()->get_session_id() == expected && d->reconnect();
}
supla_suplan::peer_transport::send_result_t supla_suplan_server_peers::send(
    int id, TSDS_SuplaSetSuplanSourceAssociation *q) {
  auto d = user->get_devices()->get(id);
  if (!d || !d->get_connection() || !d->get_connection()->is_secure() ||
      !d->is_suplan_peer_ready())
    return {send_code::not_ready, 0};
  auto current = d->get_connection()->get_session_id();
  if (d->get_suplan_root_epoch() != q->PeerContext.RootEpoch)
    return {send_code::context_changed, current};
  auto srpc = d->get_connection()->get_srpc_adapter();
  srpc->lock();
  bool ok =
      srpc_sd_async_set_suplan_source_association(srpc->get_srpc(), q) > 0;
  srpc->unlock();
  return {ok ? send_code::queued : send_code::enqueue_failed, current};
}
supla_suplan::peer_transport::send_result_t supla_suplan_server_peers::send(
    int id, TSDS_SuplaSetSuplanDestinationAssociation *q) {
  auto d = user->get_devices()->get(id);
  if (!d || !d->get_connection() || !d->get_connection()->is_secure() ||
      !d->is_suplan_peer_ready())
    return {send_code::not_ready, 0};
  auto current = d->get_connection()->get_session_id();
  auto srpc = d->get_connection()->get_srpc_adapter();
  srpc->lock();
  bool ok =
      srpc_sd_async_set_suplan_destination_association(srpc->get_srpc(), q) > 0;
  srpc->unlock();
  return {ok ? send_code::queued : send_code::enqueue_failed, current};
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

bool supla_suplan_server_peers::drain_changes() {
  // IPC is a latency hint. The existing cyclic task recovers lost hints.
  std::unique_lock<std::mutex> lock(changes_mutex, std::try_to_lock);
  if (!lock.owns_lock()) return true;
  supla_suplan::peer_dao repo(user->getUserID());
  std::vector<std::pair<uint64_t, uint64_t>> changed;
  if (!repo.changed_pairs(changes_cursor, 64, &changed)) return false;
  for (const auto &entry : changed) {
    changes_cursor = entry.first;
    supla_suplan::association_t association;
    if (!repo.load(entry.first, &association)) return false;
    if (association.id && (ready(association.source) ||
                           ready(association.destination))) {
      if (!provisioner.reconcile(entry.first) ||
          !repo.ack_changed(entry.first, entry.second)) return false;
    }
  }
  if (changed.size() < 64) changes_cursor = 0;
  return true;
}
bool supla_suplan_server_peers::channel_deleted(int user_id, int channel) {
  if (channel) {
    supla_suplan::peer_dao repo(user_id);
    int owner = 0;
    if (!repo.owner(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, channel, &owner))
      return false;
    if (!owner && !repo.cleanup_resource(channel)) return false;
  }
  auto current = supla_user::find(user_id, false);
  return !current || current->get_suplan_peers()->drain_changes();
}
bool supla_suplan_server_peers::device_deleted(int user_id, int device) {
  supla_suplan::peer_dao repo(user_id);
  int owner = 0;
  if (!repo.owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, device, &owner))
    return false;
  // Cloud owns deletion and logical grants; never delete a still-owned Device
  // or its references on the strength of an IPC payload alone.
  if (!owner && !repo.cleanup_device(device)) return false;
  return channel_deleted(user_id, 0);
}

std::shared_ptr<supla_suplan::stream_connection>
supla_suplan_server_peers::new_stream(supla_device *destination) {
  using namespace supla_suplan;  // NOLINT
  int device = destination->get_id();
  int account = user->getUserID();
  auto stream = std::make_shared<stream_connection>(
      std::max(0, scfg_int(CFG_SERVER_STREAM_MAX_CHANNELS)),
      [this, account, device](uint32_t channel, bool approval, bool request) {
        return provisioner.grants()->ensure_stream(device, channel, approval,
                                                   nullptr, request);
      },
      [this, account](uint32_t channel,
                      stream_connection::snapshot_t *snapshot) {
        peer_dao dao(account);
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
      },
      std::chrono::steady_clock::now,
      destination->get_connection()->event_notifier());
  std::lock_guard<std::mutex> lock(stream_mutex);
  streams.erase(std::remove_if(streams.begin(), streams.end(),
                               [](const std::weak_ptr<stream_connection> &s) {
                                 return s.expired();
                               }),
                streams.end());
  streams.push_back(stream);
  return stream;
}
void supla_suplan_server_peers::stream_dirty(uint32_t channel) {
  std::vector<std::shared_ptr<supla_suplan::stream_connection>> current;
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
  std::vector<std::shared_ptr<supla_suplan::stream_connection>> current;
  {
    std::lock_guard<std::mutex> lock(stream_mutex);
    for (auto &weak : streams) if (auto stream = weak.lock())
      current.push_back(stream);
    streams.clear();
  }
  for (const auto &stream : current) stream->close();
}
