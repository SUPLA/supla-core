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

#include <call_handler/register_device_d.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include "log.h"
#include "proto.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"

using std::shared_ptr;

supla_ch_register_device_d::supla_ch_register_device_d(void)
    : supla_abstract_srpc_call_handler() {}

supla_ch_register_device_d::~supla_ch_register_device_d() {}

bool supla_ch_register_device_d::handle_call(
    std::shared_ptr<supla_abstract_connection_object> object, void* srpc,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_DS_CALL_REGISTER_DEVICE_D) {
    return false;
  }

  supla_log(LOG_DEBUG, "SUPLA_DS_CALL_REGISTER_DEVICE_D");

  if (rd->data.ds_register_device_d != NULL) {
    TDS_SuplaRegisterDevice_E* register_device_e =
        (TDS_SuplaRegisterDevice_E*)malloc(sizeof(TDS_SuplaRegisterDevice_E));
    if (register_device_e != NULL) {
      memset(register_device_e, 0, sizeof(TDS_SuplaRegisterDevice_E));

      memcpy(register_device_e->Email, rd->data.ds_register_device_d->Email,
             SUPLA_EMAIL_MAXSIZE);
      memcpy(register_device_e->AuthKey, rd->data.ds_register_device_d->AuthKey,
             SUPLA_AUTHKEY_SIZE);

      memcpy(register_device_e->GUID, rd->data.ds_register_device_d->GUID,
             SUPLA_GUID_SIZE);
      memcpy(register_device_e->Name, rd->data.ds_register_device_d->Name,
             SUPLA_DEVICE_NAME_MAXSIZE);
      memcpy(register_device_e->SoftVer, rd->data.ds_register_device_d->SoftVer,
             SUPLA_SOFTVER_MAXSIZE);
      memcpy(register_device_e->ServerName,
             rd->data.ds_register_device_d->ServerName,
             SUPLA_SERVER_NAME_MAXSIZE);

      register_device_e->channel_count =
          rd->data.ds_register_device_d->channel_count;

      for (int c = 0; c < register_device_e->channel_count; c++) {
        memset(&register_device_e->channels[c], 0,
               sizeof(TDS_SuplaDeviceChannel_C));
        register_device_e->channels[c].Number =
            rd->data.ds_register_device_d->channels[c].Number;
        register_device_e->channels[c].Type =
            rd->data.ds_register_device_d->channels[c].Type;
        register_device_e->channels[c].FuncList =
            rd->data.ds_register_device_d->channels[c].FuncList;
        register_device_e->channels[c].Default =
            rd->data.ds_register_device_d->channels[c].Default;
        memcpy(register_device_e->channels[c].value,
               rd->data.ds_register_device_d->channels[c].value,
               SUPLA_CHANNELVALUE_SIZE);
      }
    }

    free(rd->data.ds_register_device_d);
    rd->data.ds_register_device_e = register_device_e;

    return object->get_srpc_call_handler_collection()->handle_call(
        object, srpc, rd, SUPLA_DS_CALL_REGISTER_DEVICE_E, proto_version);
  }

  return true;
}
