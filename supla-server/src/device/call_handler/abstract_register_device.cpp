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

#include "device/call_handler/abstract_register_device.h"

#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "conn/authkey_cache.h"
#include "device/device.h"
#include "device/devicechannel.h"
#include "log.h"

using std::shared_ptr;
using std::weak_ptr;

supla_abstract_register_device::supla_abstract_register_device(void)
    : supla_abstract_register_object() {
  channel_count = 0;
  register_device_c = nullptr;
  register_device_g = nullptr;
  location_id = 0;
  device_flags = 0;
  manufacturer_id = 0;
  product_id = 0;
  device_id = 0;

  location_enabled = false;
  new_device = false;
  device_enabled = true;
  channel_added = false;
  _location_id = 0;
  _original_location_id = 0;

  device_dao = nullptr;
}

supla_abstract_register_device::~supla_abstract_register_device() {}

supla_abstract_device_dao *supla_abstract_register_device::get_device_dao(
    void) {
  return device_dao;
}

std::weak_ptr<supla_device> supla_abstract_register_device::get_device(void) {
  return device;
}

bool supla_abstract_register_device::is_channel_added(void) {
  return channel_added;
}

int supla_abstract_register_device::get_device_id() { return device_id; }

int supla_abstract_register_device::get_device_flags(void) {
  return device_flags;
}

int supla_abstract_register_device::get_location_id() { return location_id; }

int supla_abstract_register_device::get_channel_count() {
  return channel_count;
}

TDS_SuplaDeviceChannel_B *supla_abstract_register_device::get_channels_b(void) {
  return register_device_c ? register_device_c->channels : nullptr;
}

TDS_SuplaDeviceChannel_E *supla_abstract_register_device::get_channels_e(void) {
  return register_device_g ? register_device_g->channels : nullptr;
}

void supla_abstract_register_device::set_hold_time_on_failure_usec(
    __useconds_t hold_time_on_failure_usec) {
  supla_abstract_register_object::set_hold_time_on_failure_usec(
      hold_time_on_failure_usec);
}

__useconds_t supla_abstract_register_device::get_hold_time_on_failure_usec(
    void) {
  return supla_abstract_register_object::get_hold_time_on_failure_usec();
}

void supla_abstract_register_device::send_result(int resultcode) {
  if (get_dba()->is_connected()) {
    if (get_should_rollback()) {
      get_dba()->rollback();
    }

    get_dba()->disconnect();
  }

  if (resultcode == SUPLA_RESULTCODE_TRUE ||
      resultcode == SUPLA_RESULTCODE_CFG_MODE_REQUESTED) {
    supla_log(LOG_INFO,
              "Device registered. ID: %i, ClientSD: %i Protocol Version: %i "
              "ThreadID: %i GUID: %02X%02X%02X%02X ResultCode: %i",
              device_id, get_client_sd(),
              get_srpc_adapter()->get_proto_version(), syscall(__NR_gettid),
              (unsigned char)get_guid()[0], (unsigned char)get_guid()[1],
              (unsigned char)get_guid()[2], (unsigned char)get_guid()[3],
              resultcode);
  } else {
    usleep(get_hold_time_on_failure_usec());
  }

  TSD_SuplaRegisterDeviceResult srdr;
  srdr.result_code = resultcode;
  srdr.activity_timeout = get_activity_timeout();
  srdr.version_min = SUPLA_PROTO_VERSION_MIN;
  srdr.version = SUPLA_PROTO_VERSION;
  get_srpc_adapter()->sd_async_registerdevice_result(&srdr);
}

bool supla_abstract_register_device::device_auth(void) {
  if (register_device_c != nullptr &&
      device_dao->location_auth(location_id, register_device_c->LocationPWD,
                                get_user_id_ptr(),
                                &location_enabled) == false) {
    send_result(SUPLA_RESULTCODE_BAD_CREDENTIALS);
    return false;
  }

  if (register_device_g != nullptr &&
      false == authkey_auth(get_guid(), register_device_g->Email, get_authkey(),
                            get_user_id_ptr())) {
    supla_log(LOG_INFO,
              "(AUTHKEY_AUTH) Bad device credentials. ClientSD: %i "
              "Protocol Version: %i "
              "ThreadID: %i GUID: %02X%02X%02X%02X",
              get_client_sd(), get_srpc_adapter()->get_proto_version(),
              syscall(__NR_gettid), (unsigned char)get_guid()[0],
              (unsigned char)get_guid()[1], (unsigned char)get_guid()[2],
              (unsigned char)get_guid()[3]);

    send_result(SUPLA_RESULTCODE_BAD_CREDENTIALS);
    return false;
  }

  if (get_user_id() == 0) {
    send_result(SUPLA_RESULTCODE_BAD_CREDENTIALS);
    return false;
  }

  return true;
}

bool supla_abstract_register_device::add_device(void) {
  if (!device_dao->get_device_reg_enabled(get_user_id())) {
    send_result(SUPLA_RESULTCODE_REGISTRATION_DISABLED);
    return false;
  }

  if (device_dao->get_device_limit_left(get_user_id()) <= 0) {
    send_result(SUPLA_RESULTCODE_DEVICE_LIMITEXCEEDED);
    return false;
  }

  if (location_id == 0 && register_device_g != nullptr) {
    if ((location_id = device_dao->get_location_id(get_user_id(), true)) != 0) {
      location_enabled = true;

    } else if ((location_id =
                    device_dao->get_location_id(get_user_id(), false)) != 0) {
      location_enabled = false;

    } else {
      send_result(SUPLA_RESULTCODE_NO_LOCATION_AVAILABLE);
      return false;
    }
  }

  if (location_id != 0) {
    new_device = true;

    _location_id = location_id;

    device_id = device_dao->add_device(
        location_id, get_guid(), get_authkey(), get_name(), get_client_ipv4(),
        get_softver(), get_srpc_adapter()->get_proto_version(), manufacturer_id,
        product_id, device_flags, get_user_id());

    if (device_id == 0) {
      send_result(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE);
      return false;
    } else {
      return true;
    }
  }

  return false;
}

bool supla_abstract_register_device::add_channels(void) {
  int processed_count = 0;

  TDS_SuplaDeviceChannel_B *dev_channels_b = get_channels_b();
  TDS_SuplaDeviceChannel_E *dev_channels_e = get_channels_e();

  for (int a = 0; a < SUPLA_CHANNELMAXCOUNT; a++) {
    if (a >= channel_count) {
      break;
    } else {
      processed_count++;

      unsigned char number = 0;
      _supla_int_t type = 0;
      _supla_int_t func_list = 0;
      _supla_int_t db_func_list = 0;
      _supla_int_t default_func = 0;
      _supla_int64_t channel_flags = 0;
      unsigned char alt_icon = 0;
      unsigned short sub_channel_id = 0;
      int db_channel_type = 0;
      int channel_id = 0;

      if (dev_channels_b != nullptr) {
        number = dev_channels_b[a].Number;
        type = dev_channels_b[a].Type;
        func_list = dev_channels_b[a].FuncList;
        default_func = dev_channels_b[a].Default;
      } else {
        number = dev_channels_e[a].Number;
        type = dev_channels_e[a].Type;
        func_list = dev_channels_e[a].FuncList;
        default_func = dev_channels_e[a].Default;
        channel_flags = dev_channels_e[a].Flags;
        alt_icon = dev_channels_e[a].DefaultIcon;
        sub_channel_id = dev_channels_e[a].SubDeviceId;
      }

      if (type == 0) {
        break;
      }

      if ((channel_id = device_dao->get_channel_properties(
               device_id, number, &db_channel_type, &db_func_list)) == 0) {
        db_channel_type = 0;
      }

      if (type == SUPLA_CHANNELTYPE_IMPULSE_COUNTER &&
          default_func == SUPLA_CHANNELFNC_ELECTRICITY_METER) {
        // Issue #115
        default_func = SUPLA_CHANNELFNC_IC_ELECTRICITY_METER;
      }

      supla_device_channel::func_list_filter(func_list, type);

      if (db_channel_type == 0) {
        int Param1 = 0;
        int Param2 = 0;
        supla_device_channel::get_defaults(type, default_func, &Param1,
                                           &Param2);
        supla_device_channel::trim_alt_icon_index(default_func, &alt_icon);

        channel_id = device_dao->add_channel(
            device_id, number, type, default_func, Param1, Param2, func_list,
            channel_flags, alt_icon, sub_channel_id, get_user_id());

        if (channel_id == 0) {
          processed_count = -1;
          break;
        } else {
          channel_added = true;
          device_dao->on_channel_added(device_id, channel_id);
        }

      } else if (db_channel_type != type) {
        processed_count = -1;
        break;
      } else if ((db_func_list | func_list) != db_func_list) {
        device_dao->update_channel_functions(channel_id, get_user_id(),
                                             db_func_list | func_list);
      }
    }
  }

  if (processed_count == -1 ||
      device_dao->get_device_channel_count(device_id) != processed_count) {
    send_result(SUPLA_RESULTCODE_CHANNEL_CONFLICT);
    return false;
  }

  return true;
}

void supla_abstract_register_device::register_device(
    weak_ptr<supla_device> device, TDS_SuplaRegisterDevice_C *register_device_c,
    TDS_SuplaRegisterDevice_G *register_device_g,
    supla_abstract_srpc_adapter *srpc_adapter,
    supla_abstract_db_access_provider *dba,
    supla_abstract_connection_dao *conn_dao,
    supla_abstract_device_dao *device_dao, int client_sd, int client_ipv4,
    unsigned char activity_timeout) {
  this->device = device;
  this->register_device_c = register_device_c;
  this->register_device_g = register_device_g;
  set_srpc_adapter(srpc_adapter);
  set_dba(dba);
  set_conn_dao(conn_dao);
  this->device_dao = device_dao;
  set_client_sd(client_sd);
  set_client_ipv4(client_ipv4);
  set_activity_timeout(activity_timeout);

  set_user_id(0);
  location_enabled = false;
  new_device = false;
  device_enabled = true;
  channel_added = false;
  _location_id = 0;
  _original_location_id = 0;

  set_should_rollback(true);

  if (register_device_c != nullptr) {
    set_guid(register_device_c->GUID);
    set_name(register_device_c->Name);
    set_softver(register_device_c->SoftVer);
    channel_count = register_device_c->channel_count;
    location_id = register_device_c->LocationID;
  } else {
    set_guid(register_device_g->GUID);
    set_authkey(register_device_g->AuthKey);
    set_name(register_device_g->Name);
    set_softver(register_device_g->SoftVer);
    device_flags = register_device_g->Flags;
    channel_count = register_device_g->channel_count;
    manufacturer_id = register_device_g->ManufacturerID;
    product_id = register_device_g->ProductID;
  }

  if (strnlen(get_name(), SUPLA_DEVICE_NAME_MAXSIZE - 1) < 1) {
    snprintf(get_name(), SUPLA_DEVICE_NAME_MAXSIZE, "unknown");
  }

  if (!is_valid_guid()) {
    send_result(SUPLA_RESULTCODE_GUID_ERROR);
    return;
  }

  if (register_device_g != nullptr && !is_valid_authkey()) {
    send_result(SUPLA_RESULTCODE_AUTHKEY_ERROR);
    return;
  }

  supla_log(LOG_INFO,
            "Device registration started. ClientSD: %i "
            "Protocol Version: %i "
            "ThreadID: %i GUID: %02X%02X%02X%02X",
            client_sd, get_srpc_adapter()->get_proto_version(),
            syscall(__NR_gettid), (unsigned char)get_guid()[0],
            (unsigned char)get_guid()[1], (unsigned char)get_guid()[2],
            (unsigned char)get_guid()[3]);

  if (!get_dba()->connect()) {
    send_result(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE);
    return;
  }

  if (!device_auth()) {
    return;
  }

  device_id = device_dao->get_device_id(get_user_id(), get_guid());

  int _device_flags = 0;

  if (device_id && !device_dao->get_device_variables(
                       device_id, &device_enabled, &_original_location_id,
                       &_location_id, &location_enabled, &_device_flags)) {
    supla_log(LOG_WARNING, "Unable to get variables for the device with id: %i",
              device_id);
    send_result(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE);
    return;
  }

  if (location_id == 0) {
    location_id = _location_id;
  }

  get_dba()->start_transaction();

  if (!device_id && !add_device()) {
    return;
  }

  if (!device_enabled) {
    send_result(SUPLA_RESULTCODE_DEVICE_DISABLED);
    return;
  }

  if (!location_enabled) {
    send_result(SUPLA_RESULTCODE_LOCATION_DISABLED);
    return;
  }

  if (location_id == 0 ||
      (location_id != _location_id && location_id != _original_location_id)) {
    send_result(SUPLA_RESULTCODE_LOCATION_CONFLICT);
    return;
  }

  if (!add_channels()) {
    return;
  }

  if (new_device) {
    device_dao->on_new_device(device_id);
  } else {
    if (get_authkey() != nullptr) {
      _original_location_id = 0;
    } else {
      if (location_id == _location_id) _original_location_id = location_id;
    }

    if (_device_flags & SUPLA_DEVICE_FLAG_DEVICE_LOCKED) {
      device_flags |= SUPLA_DEVICE_FLAG_DEVICE_LOCKED;
    } else {
      device_flags ^= device_flags & SUPLA_DEVICE_FLAG_DEVICE_LOCKED;
    }

    if (!device_dao->update_device(
            device_id, _original_location_id, get_authkey(), get_name(),
            client_ipv4, get_softver(), get_srpc_adapter()->get_proto_version(),
            device_flags)) {
      send_result(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE);
      return;
    }
  }

  get_dba()->commit();

  set_should_rollback(false);

  int resultcode = SUPLA_RESULTCODE_TRUE;

  if ((device_flags & SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED) &&
      is_prev_entering_cfg_mode()) {
    resultcode = SUPLA_RESULTCODE_CFG_MODE_REQUESTED;
  }

  if ((device_flags & SUPLA_DEVICE_FLAG_DEVICE_LOCKED) &&
      (new_device || (_device_flags & SUPLA_DEVICE_FLAG_DEVICE_LOCKED))) {
    send_result(SUPLA_RESULTCODE_DEVICE_LOCKED);
    return;
  }

  on_registration_success();

  send_result(resultcode);

  after_registration_success();
}
