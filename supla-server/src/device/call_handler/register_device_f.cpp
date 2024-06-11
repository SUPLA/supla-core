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

#include "register_device_f.h"

#include <stdlib.h>
#include <string.h>

#include <memory>

#include "device/call_handler/register_device.h"
#include "log.h"
#include "proto.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"

using std::shared_ptr;

supla_ch_register_device_f::supla_ch_register_device_f(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_register_device_f::~supla_ch_register_device_f() {}

bool supla_ch_register_device_f::is_registration_required(void) {
  return false;
}

bool supla_ch_register_device_f::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_DS_CALL_REGISTER_DEVICE_F;
}

void supla_ch_register_device_f::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  supla_log(LOG_DEBUG, "SUPLA_DS_CALL_REGISTER_DEVICE_F");

  if (rd->data.ds_register_device_f != nullptr) {
    TDS_SuplaRegisterDevice_G* register_device_g =
        (TDS_SuplaRegisterDevice_G*)malloc(sizeof(TDS_SuplaRegisterDevice_G));
    if (register_device_g != nullptr) {
      memset(register_device_g, 0, sizeof(TDS_SuplaRegisterDevice_G));

      memcpy(register_device_g->Email, rd->data.ds_register_device_g->Email,
             SUPLA_EMAIL_MAXSIZE);
      memcpy(register_device_g->AuthKey, rd->data.ds_register_device_g->AuthKey,
             SUPLA_AUTHKEY_SIZE);

      memcpy(register_device_g->GUID, rd->data.ds_register_device_g->GUID,
             SUPLA_GUID_SIZE);
      memcpy(register_device_g->Name, rd->data.ds_register_device_g->Name,
             SUPLA_DEVICE_NAME_MAXSIZE);
      memcpy(register_device_g->SoftVer, rd->data.ds_register_device_g->SoftVer,
             SUPLA_SOFTVER_MAXSIZE);
      memcpy(register_device_g->ServerName,
             rd->data.ds_register_device_g->ServerName,
             SUPLA_SERVER_NAME_MAXSIZE);

      register_device_g->Flags = rd->data.ds_register_device_g->Flags;
      register_device_g->ManufacturerID =
          rd->data.ds_register_device_g->ManufacturerID;
      register_device_g->ProductID = rd->data.ds_register_device_g->ProductID;

      register_device_g->channel_count =
          rd->data.ds_register_device_g->channel_count;

      for (int c = 0; c < register_device_g->channel_count; c++) {
        memset(&register_device_g->channels[c], 0,
               sizeof(TDS_SuplaDeviceChannel_C));
        register_device_g->channels[c].Number =
            rd->data.ds_register_device_g->channels[c].Number;
        register_device_g->channels[c].Type =
            rd->data.ds_register_device_g->channels[c].Type;
        register_device_g->channels[c].FuncList =
            rd->data.ds_register_device_g->channels[c].FuncList;
        register_device_g->channels[c].Default =
            rd->data.ds_register_device_g->channels[c].Default;
        register_device_g->channels[c].Flags =
            rd->data.ds_register_device_g->channels[c].Flags;
        register_device_g->channels[c].Offline =
            rd->data.ds_register_device_g->channels[c].Offline;
        register_device_g->channels[c].ValueValidityTimeSec =
            rd->data.ds_register_device_g->channels[c].ValueValidityTimeSec;
        memcpy(register_device_g->channels[c].value,
               rd->data.ds_register_device_g->channels[c].value,
               SUPLA_CHANNELVALUE_SIZE);
      }

      free(rd->data.ds_register_device_f);
      rd->data.ds_register_device_g = register_device_g;

      device->get_srpc_call_handler_collection()->handle_call(
          device, srpc_adapter, rd, SUPLA_DS_CALL_REGISTER_DEVICE_G,
          proto_version);
    }
  }
}
