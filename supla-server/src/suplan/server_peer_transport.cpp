// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include "suplan/server_peer_transport.h"

#include <memory>

#include "device/device.h"
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

bool supla_suplan_server_peers::channel_deleted(int user_id, int channel_id) {
  supla_suplan::PeerService service(
      [user_id]() { return std::make_unique<supla_suplan::PeerDao>(user_id); },
      nullptr);
  return service.delete_resource(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL,
                                 channel_id);
}
bool supla_suplan_server_peers::device_deleted(int user_id, int device_id) {
  supla_suplan::PeerService service(
      [user_id]() { return std::make_unique<supla_suplan::PeerDao>(user_id); },
      nullptr);
  return service.remove_device(device_id);
}
