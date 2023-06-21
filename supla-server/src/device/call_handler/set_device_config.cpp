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

#include "device/call_handler/set_device_config.h"

#include <memory>

#include "db/db_access_provider.h"
#include "device.h"
#include "device/device_dao.h"
#include "jsonconfig/device/device_json_config.h"

using std::shared_ptr;

supla_ch_set_device_config::supla_ch_set_device_config(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_set_device_config::~supla_ch_set_device_config() {}

bool supla_ch_set_device_config::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_DS_CALL_SET_DEVICE_CONFIG;
}

void supla_ch_set_device_config::handle_multipart_call(
    shared_ptr<supla_device> device, supla_set_device_multipart_call* mp_call) {
  supla_db_access_provider dba;
  supla_device_dao dao(&dba);

  TSDS_SetDeviceConfigResult result = {};
  result.Result = SUPLA_CONFIG_RESULT_FALSE;

  {
    device_json_config* config =
        dao.get_device_config(device->get_id(), nullptr);
    if (config) {
      if (config->is_local_config_disabled()) {
        result.Result = SUPLA_CONFIG_RESULT_LOCAL_CONFIG_DISABLED;
      }
      delete config;
    }
  }

  if (result.Result != SUPLA_CONFIG_RESULT_LOCAL_CONFIG_DISABLED) {
    TSDS_SetDeviceConfig config = {};
    device_json_config json_config;
    unsigned _supla_int16_t fields = 0;
    unsigned _supla_int16_t available_fields = 0;

    while (mp_call->part_pop(&config)) {
      json_config.set_config(&config);
      fields |= config.Fields;
      available_fields |= config.AvailableFields;
      if (config.EndOfDataFlag) {
        json_config.leave_only_thise_fields(fields);
        if (dao.set_device_config(device->get_user_id(), device->get_id(),
                                  &json_config, available_fields)) {
          result.Result = SUPLA_CONFIG_RESULT_TRUE;
        }
        break;
      }
    }
  }

  delete mp_call;

  device->get_connection()
      ->get_srpc_adapter()
      ->sd_async_set_device_config_result(&result);

  device->send_config_to_device();  // Regardless of the result, call
                                           // this method.
}

void supla_ch_set_device_config::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.sds_set_device_config_request != nullptr) {
    supla_set_device_multipart_call* _mp_call = nullptr;

    device->access_multipart_call_store(
        [&rd, &_mp_call, this](supla_multipart_call_store* store) -> void {
          supla_set_device_multipart_call* mp_call =
              dynamic_cast<supla_set_device_multipart_call*>(
                  store->get(SUPLA_DS_CALL_SET_DEVICE_CONFIG));

          if (!mp_call) {
            mp_call = new supla_set_device_multipart_call();
            store->add(mp_call);
          }

          mp_call->part_push(rd->data.sds_set_device_config_request);

          if (rd->data.sds_set_device_config_request->EndOfDataFlag) {
            store->detach(mp_call);
            _mp_call = mp_call;
          }
        });

    if (_mp_call) {
      // Process the call outside the store so as not to block it.
      handle_multipart_call(device, _mp_call);
    }
  }
}
