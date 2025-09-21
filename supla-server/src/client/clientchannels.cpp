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

#include "clientchannels.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include "clientchannel.h"
#include "db/database.h"
#include "device/device.h"
#include "log.h"
#include "safearray.h"
#include "srpc/srpc.h"
#include "user.h"

using std::function;
using std::shared_ptr;

supla_client_channels::supla_client_channels(supla_client *client)
    : supla_client_objcontainer(client) {}

supla_client_channel *supla_client_channels::find_channel(int Id) {
  return static_cast<supla_client_channel *>(find(Id, master));
}

supla_client_channel *supla_client_channels::any_channel_with_deviceid(
    int DeviceId) {
  return static_cast<supla_client_channel *>(find(DeviceId, master, true));
}

bool supla_client_channels::channel_exists(int ChannelID) {
  bool result = false;

  safe_array_lock(getArr());

  if (find_channel(ChannelID) != nullptr) result = true;

  safe_array_unlock(getArr());

  return result;
}

void supla_client_channels::_load(database *db, e_objc_scope scope) {
  db->get_client_channels(getClient()->get_id(), nullptr, this);

  safe_array_lock(getArr());
  for (int a = 0; a < safe_array_count(getArr()); a++) {
    supla_client_channel *channel =
        static_cast<supla_client_channel *>(safe_array_get(getArr(), a));
    if (channel) {
      channel->after_all_channels_loaded();
    }
  }
  safe_array_unlock(getArr());
}

void supla_client_channels::update_device_channels(int DeviceId) {
  database *db = new database();
  if (db->connect() == true) {
    safe_array_lock(getArr());
    db->get_client_channels(getClient()->get_id(), &DeviceId, this);
    safe_array_unlock(getArr());
  }
  delete db;

  void *arr = getArr();
  supla_client_channel *channel = nullptr;

  safe_array_lock(arr);
  for (int a = 0; a < safe_array_count(arr); a++) {
    channel = static_cast<supla_client_channel *>(safe_array_get(arr, a));
    if (channel && channel->get_device_id() == DeviceId) {
      channel->mark_for_remote_update(OI_REMOTEUPDATE_DATA2);
    }
  }
  safe_array_unlock(arr);
}

template <typename TSuplaDataPack, class TObjClass>
bool supla_client_channels::get_datapack_for_remote(
    supla_client_objcontainer_item *obj, void **data, int max_count) {
  TSuplaDataPack *pack = static_cast<TSuplaDataPack *>(*data);
  if (pack == nullptr) {
    pack = (TSuplaDataPack *)malloc(sizeof(TSuplaDataPack));
    memset(pack, 0, sizeof(TSuplaDataPack));
  }

  *data = pack;

  if (pack->count < max_count) {
    static_cast<TObjClass *>(obj)->proto_get(&pack->items[pack->count],
                                             getClient());
    pack->items[pack->count].EOL = 0;
    pack->count++;
    return true;
  } else {
    pack->total_left++;
  }

  return false;
}

bool supla_client_channels::get_ev_datapack_for_remote(
    supla_client_objcontainer_item *obj, void **data) {
  _supla_int_t pack_size = 0;
  TSC_SuplaChannelExtendedValuePack *pack =
      static_cast<TSC_SuplaChannelExtendedValuePack *>(*data);

  if (pack != nullptr) {
    if (pack->count >= SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXCOUNT ||
        pack->pack_size < 0 ||
        pack->pack_size >= SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXDATASIZE) {
      pack->total_left++;
      return false;
    }

    pack_size = pack->pack_size;
  }

  TSC_SuplaChannelExtendedValue cev;
  if (static_cast<supla_client_channel *>(obj)->proto_get(&cev, getClient())) {
    _supla_int_t cev_size = sizeof(TSC_SuplaChannelExtendedValue) -
                            SUPLA_CHANNELEXTENDEDVALUE_SIZE + cev.value.size;
    if (cev_size + pack_size <= SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXDATASIZE) {
      if (pack == nullptr) {
        pack = (TSC_SuplaChannelExtendedValuePack *)malloc(
            sizeof(TSC_SuplaChannelExtendedValuePack));
        memset(pack, 0, sizeof(TSC_SuplaChannelExtendedValuePack));
        *data = pack;
      }

      if (pack != nullptr) {
        memcpy(&pack->pack[pack->pack_size], &cev, cev_size);
        pack->pack_size += cev_size;
        pack->count++;
        return true;
      }
    }
  }

  if (pack != nullptr) {
    pack->total_left++;
  }

  return false;
}

bool supla_client_channels::get_data_for_remote(
    supla_client_objcontainer_item *obj, void **data, int data_type,
    bool *check_more, e_objc_scope scope) {
  *check_more = true;
  if (data_type & OI_REMOTEUPDATE_DATA1) {
    if (getClient()->get_protocol_version() >= 23) {
      return get_datapack_for_remote<TSC_SuplaChannelPack_E,
                                     supla_client_channel>(
          obj, data, SUPLA_CHANNELPACK_MAXCOUNT);

    } else if (getClient()->get_protocol_version() >= 15) {
      return get_datapack_for_remote<TSC_SuplaChannelPack_D,
                                     supla_client_channel>(
          obj, data, SUPLA_CHANNELPACK_MAXCOUNT);

    } else if (getClient()->get_protocol_version() >= 10) {
      return get_datapack_for_remote<TSC_SuplaChannelPack_C,
                                     supla_client_channel>(
          obj, data, SUPLA_CHANNELPACK_MAXCOUNT);

    } else if (getClient()->get_protocol_version() >= 8) {
      return get_datapack_for_remote<TSC_SuplaChannelPack_B,
                                     supla_client_channel>(
          obj, data, SUPLA_CHANNELPACK_MAXCOUNT);

    } else {
      return get_datapack_for_remote<TSC_SuplaChannelPack,
                                     supla_client_channel>(
          obj, data, SUPLA_CHANNELPACK_MAXCOUNT);
    }
  } else if (data_type & OI_REMOTEUPDATE_DATA2) {
    if (getClient()->get_protocol_version() >= 15) {
      return get_datapack_for_remote<TSC_SuplaChannelValuePack_B,
                                     supla_client_channel>(
          obj, data, SUPLA_CHANNELVALUE_PACK_MAXCOUNT);
    } else if (getClient()->get_protocol_version() >= 9) {
      return get_datapack_for_remote<TSC_SuplaChannelValuePack,
                                     supla_client_channel>(
          obj, data, SUPLA_CHANNELVALUE_PACK_MAXCOUNT);

    } else {
      if (*data == nullptr) {
        *data = malloc(sizeof(TSC_SuplaChannelValue));
      }

      static_cast<supla_client_channel *>(obj)->proto_get(
          (TSC_SuplaChannelValue *)*data, getClient());
    }

    return true;
  } else if (data_type & OI_REMOTEUPDATE_DATA3) {
    if (getClient()->get_protocol_version() >= 10) {
      return get_ev_datapack_for_remote(obj, data);
    }
  }

  return false;
}

template <typename TSuplaDataPack>
void supla_client_channels::set_pack_eol(void *data) {
  TSuplaDataPack *pack = static_cast<TSuplaDataPack *>(data);

  if (pack && pack->count > 0) {
    if (pack->total_left == 0) pack->items[pack->count - 1].EOL = 1;
  }
}

void supla_client_channels::send_data_to_remote_and_free(void *srpc, void *data,
                                                         int data_type,
                                                         e_objc_scope scope) {
  if (data_type & OI_REMOTEUPDATE_DATA1) {
    if (getClient()->get_protocol_version() >= 23) {
      set_pack_eol<TSC_SuplaChannelPack_E>(data);
      srpc_sc_async_channelpack_update_e(
          srpc, static_cast<TSC_SuplaChannelPack_E *>(data));

    } else if (getClient()->get_protocol_version() >= 15) {
      set_pack_eol<TSC_SuplaChannelPack_D>(data);
      srpc_sc_async_channelpack_update_d(
          srpc, static_cast<TSC_SuplaChannelPack_D *>(data));

    } else if (getClient()->get_protocol_version() >= 10) {
      set_pack_eol<TSC_SuplaChannelPack_C>(data);
      srpc_sc_async_channelpack_update_c(
          srpc, static_cast<TSC_SuplaChannelPack_C *>(data));

    } else if (getClient()->get_protocol_version() >= 8) {
      set_pack_eol<TSC_SuplaChannelPack_B>(data);
      srpc_sc_async_channelpack_update_b(
          srpc, static_cast<TSC_SuplaChannelPack_B *>(data));

    } else {
      set_pack_eol<TSC_SuplaChannelPack>(data);
      srpc_sc_async_channelpack_update(
          srpc, static_cast<TSC_SuplaChannelPack *>(data));
    }
  } else if (data_type & OI_REMOTEUPDATE_DATA2) {
    if (getClient()->get_protocol_version() >= 15) {
      set_pack_eol<TSC_SuplaChannelValuePack_B>(data);
      srpc_sc_async_channelvalue_pack_update_b(
          srpc, static_cast<TSC_SuplaChannelValuePack_B *>(data));
    } else if (getClient()->get_protocol_version() >= 9) {
      set_pack_eol<TSC_SuplaChannelValuePack>(data);
      srpc_sc_async_channelvalue_pack_update(
          srpc, static_cast<TSC_SuplaChannelValuePack *>(data));
    } else {
      srpc_sc_async_channel_value_update(srpc, (TSC_SuplaChannelValue *)data);
    }
  } else if (data_type & OI_REMOTEUPDATE_DATA3) {
    srpc_sc_async_channelextendedvalue_pack_update(
        srpc, static_cast<TSC_SuplaChannelExtendedValuePack *>(data));
  }

  free(data);
}

int supla_client_channels::available_data_types_for_remote(e_objc_scope scope) {
  return OI_REMOTEUPDATE_DATA1 | OI_REMOTEUPDATE_DATA2 | OI_REMOTEUPDATE_DATA3;
}

void supla_client_channels::on_channel_value_changed(int DeviceId,
                                                     int ChannelId,
                                                     bool Extended) {
  on_value_changed(ChannelId, DeviceId, master,
                   Extended ? OI_REMOTEUPDATE_DATA3 : OI_REMOTEUPDATE_DATA2);
}

void supla_client_channels::set_channel_function(int ChannelId, int Func) {
  supla_client_channel *channel = find_channel(ChannelId);

  if (channel != nullptr) {
    channel->set_func(Func);
    getClient()->remote_update_lists();
  }
}

void supla_client_channels::set_channel_caption(int ChannelId, char *Caption) {
  supla_client_channel *channel = find_channel(ChannelId);

  if (channel != nullptr) {
    channel->setCaption(Caption);
    getClient()->remote_update_lists();
  }
}

unsigned _supla_int64_t supla_client_channels::value_validity_time_usec(void) {
  unsigned _supla_int64_t result = 0;

  void *arr = getArr();
  supla_client_channel *channel = nullptr;

  safe_array_lock(arr);
  for (int a = 0; a < safe_array_count(arr); a++) {
    channel = static_cast<supla_client_channel *>(safe_array_get(arr, a));
    if (channel) {
      unsigned _supla_int64_t t = channel->getValueValidityTimeUSec();
      if (t > 0 && (result == 0 || t < result)) {
        result = t;
      }
    }
  }
  safe_array_unlock(arr);

  return result;
}

void supla_client_channels::update_expired(void) {
  void *arr = getArr();
  supla_client_channel *channel = nullptr;

  bool remote_update = false;

  safe_array_lock(arr);
  for (int a = 0; a < safe_array_count(arr); a++) {
    channel = static_cast<supla_client_channel *>(safe_array_get(arr, a));
    if (channel) {
      if (channel->isValueValidityTimeSet() &&
          channel->getValueValidityTimeUSec() == 0) {
        channel->resetValueValidityTime();
        channel->mark_for_remote_update(OI_REMOTEUPDATE_DATA2);
        remote_update = true;
      }
    }
  }
  safe_array_unlock(arr);

  if (remote_update) {
    getClient()->remote_update_lists();
  }
}

void supla_client_channels::for_each(
    function<void(supla_client_channel *, bool *will_continue)> on_channel) {
  void *arr = getArr();
  supla_client_channel *channel = nullptr;

  safe_array_lock(arr);
  bool will_continue = true;
  for (int a = 0; a < safe_array_count(arr); a++) {
    channel = static_cast<supla_client_channel *>(safe_array_get(arr, a));
    if (channel) {
      on_channel(channel, &will_continue);
    }
    if (!will_continue) {
      break;
    }
  }
  safe_array_unlock(arr);
}

void supla_client_channels::channel_access(
    int channel_id, function<void(supla_client_channel *)> on_channel) {
  safe_array_lock(getArr());

  supla_client_channel *channel = nullptr;

  if (nullptr != (channel = find_channel(channel_id))) {
    on_channel(channel);
  }

  safe_array_unlock(getArr());
}

void supla_client_channels::device_access(
    int channel_id, function<void(supla_device *)> method) {
  int device_id = 0;

  channel_access(channel_id,
                 [&device_id](supla_client_channel *channel) -> void {
                   device_id = channel->get_device_id();
                 });

  if (!device_id) {
    return;
  }

  shared_ptr<supla_device> device =
      getClient()->get_user()->get_devices()->get(device_id);
  if (device != nullptr) {
    method(device.get());
  }
}

int supla_client_channels::get_device_id(int channel_id) {
  int result = 0;
  channel_access(channel_id, [&result](supla_client_channel *channel) -> void {
    result = channel->get_device_id();
  });

  return result;
}

int supla_client_channels::get_any_channel_id_with_deviceid(int device_id) {
  int result = 0;
  safe_array_lock(getArr());
  supla_client_channel *channel = any_channel_with_deviceid(device_id);
  if (channel) {
    result = channel->get_id();
  }
  safe_array_unlock(getArr());

  return result;
}

int supla_client_channels::get_channel_id(unsigned char channel_number) {
  int result = 0;
  for_each([&result, channel_number](supla_client_channel *channel,
                                     bool *will_continue) -> void {
    if (channel->get_channel_number() == channel_number) {
      *will_continue = false;
      result = channel->get_id();
    }
  });

  return result;
}
