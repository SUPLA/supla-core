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

#include <call_handler/register_device_a.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include "log.h"
#include "proto.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"

using std::shared_ptr;

supla_ch_register_device_a::supla_ch_register_device_a(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_register_device_a::~supla_ch_register_device_a() {}

bool supla_ch_register_device_a::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_DS_CALL_REGISTER_DEVICE) {
    return CH_UNHANDLED;
  }

  supla_log(LOG_DEBUG, "SUPLA_DS_CALL_REGISTER_DEVICE");

  if (rd->data.ds_register_device != nullptr) {
    TDS_SuplaRegisterDevice_B* register_device_b =
        (TDS_SuplaRegisterDevice_B*)malloc(sizeof(TDS_SuplaRegisterDevice_B));

    if (register_device_b != nullptr) {
      memset(register_device_b, 0, sizeof(TDS_SuplaRegisterDevice_B));

      register_device_b->LocationID = rd->data.ds_register_device->LocationID;
      memcpy(register_device_b->LocationPWD,
             rd->data.ds_register_device->LocationPWD,
             SUPLA_LOCATION_PWD_MAXSIZE);
      memcpy(register_device_b->GUID, rd->data.ds_register_device->GUID,
             SUPLA_GUID_SIZE);
      memcpy(register_device_b->Name, rd->data.ds_register_device->Name,
             SUPLA_DEVICE_NAME_MAXSIZE);
      memcpy(register_device_b->SoftVer, rd->data.ds_register_device->SoftVer,
             SUPLA_SOFTVER_MAXSIZE);

      register_device_b->channel_count =
          rd->data.ds_register_device->channel_count;

      for (int c = 0; c < register_device_b->channel_count; c++) {
        register_device_b->channels[c].Number =
            rd->data.ds_register_device->channels[c].Number;
        register_device_b->channels[c].Type =
            rd->data.ds_register_device->channels[c].Type;
        memcpy(register_device_b->channels[c].value,
               rd->data.ds_register_device->channels[c].value,
               SUPLA_CHANNELVALUE_SIZE);
      }

      free(rd->data.ds_register_device);
      rd->data.ds_register_device_b = register_device_b;

      return device->get_srpc_call_handler_collection()->handle_call(
          device, srpc_adapter, rd, SUPLA_DS_CALL_REGISTER_DEVICE_B,
          proto_version);
    }
  }

  return CH_HANDLED;
}
