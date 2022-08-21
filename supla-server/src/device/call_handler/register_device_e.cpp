/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <call_handler/register_device_e.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include "log.h"
#include "proto.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"

using std::shared_ptr;

supla_ch_register_device_e::supla_ch_register_device_e(void)
    : supla_abstract_srpc_call_handler() {}

supla_ch_register_device_e::~supla_ch_register_device_e() {}

bool supla_ch_register_device_e::handle_call(
    std::shared_ptr<supla_abstract_connection_object> object,
    supla_abstract_srpc_adapter* srpc_adapter, TsrpcReceivedData* rd,
    unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_DS_CALL_REGISTER_DEVICE_E) {
    return false;
  }

  supla_log(LOG_DEBUG, "SUPLA_DS_CALL_REGISTER_DEVICE_E");

  if (rd->data.ds_register_device_e != NULL) {
    rd->data.ds_register_device_e->Email[SUPLA_EMAIL_MAXSIZE - 1] = 0;
    rd->data.ds_register_device_e->Name[SUPLA_DEVICE_NAME_MAXSIZE - 1] = 0;
    rd->data.ds_register_device_e->SoftVer[SUPLA_SOFTVER_MAXSIZE - 1] = 0;
    rd->data.ds_register_device_e->ServerName[SUPLA_SERVER_NAME_MAXSIZE - 1] =
        0;

    // TODO(przemyslawzygmunt): Replace the old implementation with the new one
    // if (device->register_device(NULL, rd->data.ds_register_device_e,
    //                             proto_version) == 1) {
    //   set_registered(REG_DEVICE);
    // }
  }

  return true;
}
