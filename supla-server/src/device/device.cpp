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

#include "device.h"

#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include "db/database.h"
#include "device/call_handler/call_handler_collection.h"
#include "device/device_dao.h"
#include "http/http_event_hub.h"
#include "jsonconfig/device/device_json_config.h"
#include "lck.h"
#include "log.h"
#include "safearray.h"
#include "srpc/srpc.h"
#include "user.h"

using std::dynamic_pointer_cast;
using std::list;
using std::shared_ptr;

// static
supla_device_call_handler_collection supla_device::call_handler_collection;

supla_device::supla_device(supla_connection *connection)
    : supla_abstract_connection_object(connection) {
  this->entering_cfg_mode_in_progress = false;
  this->channels = nullptr;
  this->flags = 0;
}

supla_device::~supla_device() {
  if (get_user()) {  // 1st line!
    list<int> ids = channels->get_all_ids();
    for (auto it = ids.begin(); it != ids.end(); it++) {
      get_user()->on_channel_value_changed(supla_caller(ctDevice, get_id()),
                                           get_id(), *it);
    }
  }

  delete channels;
}

supla_abstract_srpc_call_handler_collection *
supla_device::get_srpc_call_handler_collection(void) {
  return &supla_device::call_handler_collection;
}

shared_ptr<supla_device> supla_device::get_shared_ptr(void) {
  return dynamic_pointer_cast<supla_device>(
      supla_abstract_connection_object::get_shared_ptr());
}

bool supla_device::is_sleeping_object(void) {
  return (flags & SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED) &&
         channels->get_value_validity_time_left_msec() > 0;
}

unsigned int supla_device::get_time_to_wakeup_msec(void) {
  return channels->get_value_validity_time_left_msec();
}

bool supla_device::can_reconnect(void) {
  return (flags & SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED) == 0 &&
         supla_abstract_connection_object::can_reconnect();
}

// static
bool supla_device::funclist_contains_function(int funcList, int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      return (funcList & SUPLA_BIT_FUNC_CONTROLLINGTHEGATEWAYLOCK) > 0;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      return (funcList & SUPLA_BIT_FUNC_CONTROLLINGTHEGATE) > 0;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      return (funcList & SUPLA_BIT_FUNC_CONTROLLINGTHEGARAGEDOOR) > 0;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      return (funcList & SUPLA_BIT_FUNC_CONTROLLINGTHEDOORLOCK) > 0;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      return (funcList & SUPLA_BIT_FUNC_CONTROLLINGTHEROLLERSHUTTER) > 0;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      return (funcList & SUPLA_BIT_FUNC_CONTROLLINGTHEROOFWINDOW) > 0;
    case SUPLA_CHANNELFNC_POWERSWITCH:
      return (funcList & SUPLA_BIT_FUNC_POWERSWITCH) > 0;
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
      return (funcList & SUPLA_BIT_FUNC_LIGHTSWITCH) > 0;
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      return (funcList & SUPLA_BIT_FUNC_STAIRCASETIMER) > 0;
    case SUPLA_CHANNELFNC_THERMOMETER:
      return (funcList & SUPLA_BIT_FUNC_THERMOMETER) > 0;
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      return (funcList & SUPLA_BIT_FUNC_HUMIDITYANDTEMPERATURE) > 0;
    case SUPLA_CHANNELFNC_HUMIDITY:
      return (funcList & SUPLA_BIT_FUNC_HUMIDITY) > 0;
    case SUPLA_CHANNELFNC_WINDSENSOR:
      return (funcList & SUPLA_BIT_FUNC_WINDSENSOR) > 0;
    case SUPLA_CHANNELFNC_PRESSURESENSOR:
      return (funcList & SUPLA_BIT_FUNC_PRESSURESENSOR) > 0;
    case SUPLA_CHANNELFNC_RAINSENSOR:
      return (funcList & SUPLA_BIT_FUNC_RAINSENSOR) > 0;
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
      return (funcList & SUPLA_BIT_FUNC_WEIGHTSENSOR) > 0;
  }

  return false;
}

void supla_device::set_flags(int flags) { this->flags = flags; }

int supla_device::get_flags(void) { return flags; }

void supla_device::set_channels(supla_device_channels *channels) {
  this->channels = channels;
}

supla_device_channels *supla_device::get_channels(void) { return channels; }

bool supla_device::enter_cfg_mode(void) {
  if (flags & SUPLA_DEVICE_FLAG_CALCFG_ENTER_CFG_MODE) {
    entering_cfg_mode_in_progress = true;

    TSD_DeviceCalCfgRequest request = {};

    request.ChannelNumber = -1;
    request.Command = SUPLA_CALCFG_CMD_ENTER_CFG_MODE;
    request.SuperUserAuthorized = true;

    srpc_sd_async_device_calcfg_request(
        get_connection()->get_srpc_adapter()->get_srpc(), &request);
    return true;
  }

  return false;
}

void supla_device::send_config_to_device(void) {
  if (get_protocol_version() >= 21 &&
      (get_flags() & SUPLA_DEVICE_FLAG_DEVICE_CONFIG_SUPPORTED)) {
    supla_db_access_provider dba;
    supla_device_dao dao(&dba);

    device_json_config *config =
        dao.get_device_config(get_id(), nullptr, nullptr);
    if (config) {
      unsigned _supla_int64_t fields = 0xFFFFFFFFFFFFFFFF;
      unsigned _supla_int64_t available_fields = config->get_available_fields();
      while (fields) {
        TSDS_SetDeviceConfig sds_config = {};
        config->get_config(&sds_config, fields, &fields);
        sds_config.AvailableFields = available_fields;
        sds_config.EndOfDataFlag = fields == 0 ? 1 : 0;
        get_connection()
            ->get_srpc_adapter()
            ->sd_async_set_device_config_request(&sds_config);
      }

      delete config;
    }
  }
}
