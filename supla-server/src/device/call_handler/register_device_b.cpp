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

#include "device/call_handler/register_device_b.h"

#include <stdlib.h>
#include <string.h>

#include <memory>

#include "log.h"
#include "proto.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"

using std::shared_ptr;

supla_ch_register_device_b::supla_ch_register_device_b(void)
    : supla_abstract_srpc_call_handler() {}

supla_ch_register_device_b::~supla_ch_register_device_b() {}

bool supla_ch_register_device_b::handle_call(
    shared_ptr<supla_abstract_connection_object> object,
    supla_abstract_srpc_adapter* srpc_adapter, TsrpcReceivedData* rd,
    unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_DS_CALL_REGISTER_DEVICE_B) {
    return false;
  }

  supla_log(LOG_DEBUG, "SUPLA_DS_CALL_REGISTER_DEVICE_B");

  if (rd->data.ds_register_device_b != NULL) {
    TDS_SuplaRegisterDevice_C* register_device_c =
        (TDS_SuplaRegisterDevice_C*)malloc(sizeof(TDS_SuplaRegisterDevice_C));

    if (register_device_c != NULL) {
      memset(register_device_c, 0, sizeof(TDS_SuplaRegisterDevice_C));

      register_device_c->LocationID = rd->data.ds_register_device_b->LocationID;
      memcpy(register_device_c->LocationPWD,
             rd->data.ds_register_device_b->LocationPWD,
             SUPLA_LOCATION_PWD_MAXSIZE);
      memcpy(register_device_c->GUID, rd->data.ds_register_device_b->GUID,
             SUPLA_GUID_SIZE);
      memcpy(register_device_c->Name, rd->data.ds_register_device_b->Name,
             SUPLA_DEVICE_NAME_MAXSIZE);
      memcpy(register_device_c->SoftVer, rd->data.ds_register_device_b->SoftVer,
             SUPLA_SOFTVER_MAXSIZE);

      register_device_c->channel_count =
          rd->data.ds_register_device_b->channel_count;

      for (int c = 0; c < register_device_c->channel_count; c++) {
        memcpy(&register_device_c->channels[c],
               &rd->data.ds_register_device_b->channels[c],
               sizeof(TDS_SuplaDeviceChannel_B));
      }
    }

    free(rd->data.ds_register_device_b);
    rd->data.ds_register_device_c = register_device_c;

    return object->get_srpc_call_handler_collection()->handle_call(
        object, srpc_adapter, rd, SUPLA_DS_CALL_REGISTER_DEVICE_C,
        proto_version);
  }

  return true;
}
