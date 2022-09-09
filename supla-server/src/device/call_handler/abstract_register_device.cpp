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
#include "device/devicechannel.h"
#include "log.h"

supla_ch_abstract_register_device::supla_ch_abstract_register_device(void)
    : supla_ch_abstract_register_object() {
  guid = nullptr;
  authkey = nullptr;
  name = nullptr;
  softver = nullptr;
  channel_count = 0;
  register_device_c = nullptr;
  register_device_e = nullptr;
  location_id = 0;
  device_flags = 0;
  manufacturer_id = 0;
  product_id = 0;
  flags = 0;
  device_id = 0;

  user_id = 0;
  location_enabled = false;
  new_device = false;
  device_enabled = true;
  channels_added = false;
  _location_id = 0;
  _original_location_id = 0;

  should_rollback = true;

  srpc_adapter = nullptr;
  dba = nullptr;
  dao = nullptr;
  client_sd = -1;
  client_ipv4 = 0;
  activity_timeout = 0;

  hold_time_on_failure_usec = 2000000;
}

supla_ch_abstract_register_device::~supla_ch_abstract_register_device() {}

void supla_ch_abstract_register_device::send_result(int resultcode) {
  if (should_rollback) {
    dba->rollback();
  }

  if (dba->is_connected()) {
    dba->disconnect();
  }

  if (resultcode == SUPLA_RESULTCODE_TRUE) {
    supla_log(LOG_INFO,
              "Device registered. ID: %i, ClientSD: %i Protocol Version: %i "
              "ThreadID: %i GUID: %02X%02X%02X%02X",
              device_id, client_sd, srpc_adapter->get_proto_version(),
              syscall(__NR_gettid), (unsigned char)guid[0],
              (unsigned char)guid[1], (unsigned char)guid[2],
              (unsigned char)guid[3]);
  } else {
    usleep(hold_time_on_failure_usec);
  }

  TSD_SuplaRegisterDeviceResult srdr;
  srdr.result_code = resultcode;
  srdr.activity_timeout = activity_timeout;
  srdr.version_min = SUPLA_PROTO_VERSION_MIN;
  srdr.version = SUPLA_PROTO_VERSION;
  srpc_adapter->sd_async_registerdevice_result(&srdr);
}

bool supla_ch_abstract_register_device::device_auth(void) {
  if (register_device_c != nullptr &&
      dao->location_auth(location_id, register_device_c->LocationPWD, &user_id,
                         &location_enabled) == false) {
    send_result(SUPLA_RESULTCODE_BAD_CREDENTIALS);
    return false;
  };

  if (register_device_e != nullptr &&
      false ==
          authkey_auth(guid, register_device_e->Email, authkey, &user_id)) {
    supla_log(LOG_INFO,
              "(AUTHKEY_AUTH) Bad device credentials. ClientSD: %i "
              "Protocol Version: %i "
              "ThreadID: %i GUID: %02X%02X%02X%02X",
              client_sd, srpc_adapter->get_proto_version(),
              syscall(__NR_gettid), (unsigned char)guid[0],
              (unsigned char)guid[1], (unsigned char)guid[2],
              (unsigned char)guid[3]);

    send_result(SUPLA_RESULTCODE_BAD_CREDENTIALS);
    return false;
  };

  if (user_id == 0) {
    send_result(SUPLA_RESULTCODE_BAD_CREDENTIALS);
    return false;
  }

  return true;
}

bool supla_ch_abstract_register_device::add_device(void) {
  if (!dao->get_device_reg_enabled(user_id)) {
    send_result(SUPLA_RESULTCODE_REGISTRATION_DISABLED);
    return false;
  }

  if (dao->get_device_limit_left(user_id) <= 0) {
    send_result(SUPLA_RESULTCODE_DEVICE_LIMITEXCEEDED);
    return false;
  }

  if (location_id == 0 && register_device_e != nullptr) {
    if ((location_id = dao->get_location_id(user_id, true)) != 0) {
      location_enabled = true;

    } else if ((location_id = dao->get_location_id(user_id, false)) != 0) {
      location_enabled = false;

    } else {
      send_result(SUPLA_RESULTCODE_NO_LOCATION_AVAILABLE);
      return false;
    }
  }

  if (location_id != 0) {
    new_device = true;

    _location_id = location_id;

    device_id =
        dao->add_device(location_id, guid, authkey, name, client_ipv4, softver,
                        srpc_adapter->get_proto_version(), manufacturer_id,
                        product_id, device_flags, user_id);

    if (device_id == 0) {
      send_result(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE);
      return false;
    } else {
      return true;
    }
  }

  return false;
}

bool supla_ch_abstract_register_device::add_channels(void) {
  int processed_count = 0;
  int channel_type = 0;

  TDS_SuplaDeviceChannel_B *dev_channels_b = nullptr;
  TDS_SuplaDeviceChannel_C *dev_channels_c = nullptr;

  if (register_device_c != nullptr) {
    dev_channels_b = register_device_c->channels;
  } else {
    dev_channels_c = register_device_e->channels;
  }

  for (int a = 0; a < SUPLA_CHANNELMAXCOUNT; a++) {
    if (a >= channel_count) {
      break;
    } else {
      processed_count++;

      unsigned char number = 0;
      _supla_int_t type = 0;
      _supla_int_t func_list = 0;
      _supla_int_t default_func = 0;
      _supla_int_t channel_flags = 0;

      if (dev_channels_b != nullptr) {
        number = dev_channels_b[a].Number;
        type = dev_channels_b[a].Type;
        func_list = dev_channels_b[a].FuncList;
        default_func = dev_channels_b[a].Default;
      } else {
        number = dev_channels_c[a].Number;
        type = dev_channels_c[a].Type;
        func_list = dev_channels_c[a].FuncList;
        default_func = dev_channels_c[a].Default;
        channel_flags = dev_channels_c[a].Flags;
      }

      if (type == 0) {
        break;
      }

      if (dao->get_channel_id_and_type(device_id, number, &channel_type) == 0) {
        channel_type = 0;
      }

      if (type == SUPLA_CHANNELTYPE_IMPULSE_COUNTER &&
          default_func == SUPLA_CHANNELFNC_ELECTRICITY_METER) {
        // Issue #115
        default_func = SUPLA_CHANNELFNC_IC_ELECTRICITY_METER;
      }

      if (channel_type == 0) {
        int Param1 = 0;
        int Param2 = 0;
        supla_device_channel::getDefaults(type, default_func, &Param1, &Param2);

        int channel_id = dao->add_channel(
            device_id, number, type, default_func, Param1, Param2,
            supla_device_channel::funcListFilter(func_list, type),
            channel_flags, user_id);

        if (channel_id == 0) {
          processed_count = -1;
          break;
        } else {
          channels_added = true;
          dao->on_channel_added(device_id, channel_id);
        }

      } else if (channel_type != type) {
        processed_count = -1;
        break;
      }
    }
  }

  if (processed_count == -1 ||
      dao->get_device_channel_count(device_id) != processed_count) {
    send_result(SUPLA_RESULTCODE_CHANNEL_CONFLICT);
    return false;
  }

  return true;
}

void supla_ch_abstract_register_device::register_device(
    TDS_SuplaRegisterDevice_C *register_device_c,
    TDS_SuplaRegisterDevice_E *register_device_e,
    supla_abstract_srpc_adapter *srpc_adapter,
    supla_abstract_db_access_provider *dba, supla_abstract_device_dao *dao,
    int client_sd, int client_ipv4, unsigned char activity_timeout) {
  this->register_device_c = register_device_c;
  this->register_device_e = register_device_e;
  this->srpc_adapter = srpc_adapter;
  this->dba = dba;
  this->dao = dao;
  this->client_sd = client_sd;
  this->client_ipv4 = client_ipv4;
  this->activity_timeout = activity_timeout;

  user_id = 0;
  location_enabled = false;
  new_device = false;
  device_enabled = true;
  channels_added = false;
  _location_id = 0;
  _original_location_id = 0;

  should_rollback = true;

  if (register_device_c != nullptr) {
    guid = register_device_c->GUID;
    name = register_device_c->Name;
    softver = register_device_c->SoftVer;
    channel_count = register_device_c->channel_count;
    location_id = register_device_c->LocationID;
  } else {
    guid = register_device_e->GUID;
    authkey = register_device_e->AuthKey;
    name = register_device_e->Name;
    softver = register_device_e->SoftVer;
    device_flags = register_device_e->Flags;
    channel_count = register_device_e->channel_count;
    manufacturer_id = register_device_e->ManufacturerID;
    product_id = register_device_e->ProductID;
    flags = register_device_e->Flags;
  }

  if (strnlen(name, SUPLA_DEVICE_NAME_MAXSIZE - 1) < 1) {
    snprintf(name, SUPLA_DEVICE_NAME_MAXSIZE, "unknown");
  }

  if (!is_valid_guid(guid)) {
    send_result(SUPLA_RESULTCODE_GUID_ERROR);
    return;
  }

  if (register_device_e != nullptr &&
      !is_valid_authkey(register_device_e->AuthKey)) {
    send_result(SUPLA_RESULTCODE_AUTHKEY_ERROR);
    return;
  }

  supla_log(LOG_INFO,
            "Device registration started. ClientSD: %i "
            "Protocol Version: %i "
            "ThreadID: %i GUID: %02X%02X%02X%02X",
            client_sd, srpc_adapter->get_proto_version(), syscall(__NR_gettid),
            (unsigned char)guid[0], (unsigned char)guid[1],
            (unsigned char)guid[2], (unsigned char)guid[3]);

  if (!dba->connect()) {
    send_result(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE);
    return;
  }

  if (!device_auth()) {
    return;
  }

  device_id = dao->get_device_id(user_id, guid);

  if (device_id && !dao->get_device_variables(
                       device_id, &device_enabled, &_original_location_id,
                       &_location_id, &location_enabled)) {
    supla_log(LOG_WARNING, "Unable to get variables for the device with id: %i",
              device_id);
    send_result(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE);
    return;
  }

  if (location_id == 0) {
    location_id = _location_id;
  }

  dba->start_transaction();

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
    dao->on_new_device(device_id);
  } else {
    if (authkey != nullptr) {
      _original_location_id = 0;
    } else {
      if (location_id == _location_id) _original_location_id = location_id;
    }

    if (!dao->update_device(device_id, _original_location_id, authkey, name,
                            client_ipv4, softver,
                            srpc_adapter->get_proto_version(), flags)) {
      send_result(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE);
      return;
    }
  }

  dba->commit();

  should_rollback = false;

  on_registraction_success(device_id, channels_added);

  send_result(SUPLA_RESULTCODE_TRUE);
}

__useconds_t supla_ch_abstract_register_device::get_hold_time_on_failure_usec(
    void) {
  return hold_time_on_failure_usec;
}
