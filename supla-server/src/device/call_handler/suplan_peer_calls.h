// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SUPLA_CH_SUPLAN_PEER_CALLS_H_
#define SUPLA_CH_SUPLAN_PEER_CALLS_H_
#include "conn/connection.h"
#include "device/call_handler/abstract_device_srpc_call_handler.h"
#include "srpc/srpc.h"
#include "suplan/server_peer_transport.h"
#include "user.h"
class supla_ch_suplan_peer_calls
    : public supla_abstract_device_srpc_call_handler {
 public:
  bool can_handle_call(unsigned int id) override {
    return id == SUPLA_DS_CALL_SET_SUPLAN_SOURCE_ASSOCIATION_RESULT ||
           id == SUPLA_DS_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION_RESULT ||
           id == SUPLA_DS_CALL_ENSURE_SUPLAN_RESOURCE_ACCESS ||
           id == SUPLA_DS_CALL_ENSURE_SUPLAN_RESOURCE_SHARE;
  }

 protected:
  void handle_call(std::shared_ptr<supla_device> d,
                   supla_abstract_srpc_adapter *srpc, TsrpcReceivedData *rd,
                   unsigned int id, unsigned char version) override {
    if (id == SUPLA_DS_CALL_ENSURE_SUPLAN_RESOURCE_ACCESS &&
        rd->data.ds_ensure_suplan_resource_access &&
        rd->data.ds_ensure_suplan_resource_access->DeliveryMode ==
            SUPLA_RESOURCE_DELIVERY_SERVER_STREAM) {
      auto reply = [srpc,
                    d](const TSD_SuplaEnsureResourceAccessResult &result) {
        srpc->lock();
        auto copy = result;
        bool ok = srpc_sd_async_ensure_suplan_resource_access_result(
                      srpc->get_srpc(), &copy) > 0;
        srpc->unlock();
        if (!ok) d->get_connection()->terminate();
        return ok;
      };
      if (!d->get_user() || version < 29 ||
          !(d->get_flags() & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) ||
          !d->is_server_stream_ready()) {
        reply({SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
               SUPLA_SUPLAN_ACCESS_STATUS_INVALID,
               SUPLA_RESOURCE_DELIVERY_SERVER_STREAM});
      } else {
        d->get_stream(true)->ensure(*rd->data.ds_ensure_suplan_resource_access,
                                   reply);
      }
      return;
    }
    auto source_result = rd->data.ds_set_suplan_source_association_result;
    if (!d->get_user() || version < 29 || !d->get_suplan_root_epoch()) {
      if (id == SUPLA_DS_CALL_SET_SUPLAN_SOURCE_ASSOCIATION_RESULT &&
          source_result)
        supla_suplan::wipe_key(source_result->PeerKey,
                               sizeof(source_result->PeerKey));
      return;
    }
    auto p = d->get_user()->get_suplan_peers()->peers();
    if (id == SUPLA_DS_CALL_SET_SUPLAN_SOURCE_ASSOCIATION_RESULT) {
      p->on_source(d->get_id(), source_result,
                   d->get_connection()->get_session_id());
    } else if (id == SUPLA_DS_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION_RESULT) {
      p->on_destination(d->get_id(),
                        rd->data.ds_set_suplan_destination_association_result,
                        d->get_connection()->get_session_id());
    } else if (id == SUPLA_DS_CALL_ENSURE_SUPLAN_RESOURCE_ACCESS &&
               rd->data.ds_ensure_suplan_resource_access) {
      auto request = rd->data.ds_ensure_suplan_resource_access;
      auto result = p->grants()->ensure_access(d->get_id(), *request);
      srpc->lock();
      srpc_sd_async_ensure_suplan_resource_access_result(srpc->get_srpc(),
                                                         &result);
      srpc->unlock();
    } else if (id == SUPLA_DS_CALL_ENSURE_SUPLAN_RESOURCE_SHARE &&
               rd->data.ds_ensure_suplan_resource_share) {
      auto result = p->grants()->ensure_share(
          d->get_id(), *rd->data.ds_ensure_suplan_resource_share);
      srpc->lock();
      srpc_sd_async_ensure_suplan_resource_share_result(srpc->get_srpc(),
                                                        &result);
      srpc->unlock();
    }
  }
};
#endif  // SUPLA_CH_SUPLAN_PEER_CALLS_H_
