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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clientchannel.h"
#include "clientchannels.h"
#include "commontypes.h"
#include "database.h"
#include "log.h"
#include "safearray.h"
#include "srpc.h"
#include "user.h"

supla_client_channels::supla_client_channels(supla_client *client)
    : supla_client_objcontainer(client) {}

supla_client_channel *supla_client_channels::find_channel(int Id) {
  return static_cast<supla_client_channel *>(find(Id, master));
}

bool supla_client_channels::channel_exists(int ChannelID) {
  bool result = false;

  safe_array_lock(getArr());

  if (find_channel(ChannelID) != NULL) result = true;

  safe_array_unlock(getArr());

  return result;
}

void supla_client_channels::_load(database *db, e_objc_scope scope) {
  db->get_client_channels(getClient()->getID(), NULL, this);
}

void supla_client_channels::update_device_channels(int DeviceId) {
  database *db = new database();
  if (db->connect() == true) {
    safe_array_lock(getArr());
    db->get_client_channels(getClient()->getID(), &DeviceId, this);
    safe_array_unlock(getArr());
  }
  delete db;

  void *arr = getArr();
  supla_client_channel *channel = NULL;

  safe_array_lock(arr);
  for (int a = 0; a < safe_array_count(arr); a++) {
    channel = static_cast<supla_client_channel *>(safe_array_get(arr, a));
    if (channel && channel->getDeviceId() == DeviceId) {
      channel->mark_for_remote_update(OI_REMOTEUPDATE_DATA2);
    }
  }
  safe_array_unlock(arr);
}

template <typename TSuplaDataPack, class TObjClass>
bool supla_client_channels::get_datapack_for_remote(
    supla_client_objcontainer_item *obj, void **data, int max_count) {
  TSuplaDataPack *pack = static_cast<TSuplaDataPack *>(*data);
  if (pack == NULL) {
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

  if (pack != NULL) {
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
      if (pack == NULL) {
        pack = (TSC_SuplaChannelExtendedValuePack *)malloc(
            sizeof(TSC_SuplaChannelExtendedValuePack));
        memset(pack, 0, sizeof(TSC_SuplaChannelExtendedValuePack));
        *data = pack;
      }

      if (pack != NULL) {
        memcpy(&pack->pack[pack->pack_size], &cev, cev_size);
        pack->pack_size += cev_size;
        pack->count++;
        return true;
      }
    }
  }

  if (pack != NULL) {
    pack->total_left++;
  }

  return false;
}

bool supla_client_channels::get_data_for_remote(
    supla_client_objcontainer_item *obj, void **data, int data_type,
    bool *check_more, e_objc_scope scope) {
  *check_more = true;
  if (data_type & OI_REMOTEUPDATE_DATA1) {
    if (getClient()->getProtocolVersion() >= 10) {
      return get_datapack_for_remote<TSC_SuplaChannelPack_C,
                                     supla_client_channel>(
          obj, data, SUPLA_CHANNELPACK_MAXCOUNT);

    } else if (getClient()->getProtocolVersion() >= 8) {
      return get_datapack_for_remote<TSC_SuplaChannelPack_B,
                                     supla_client_channel>(
          obj, data, SUPLA_CHANNELPACK_MAXCOUNT);

    } else {
      return get_datapack_for_remote<TSC_SuplaChannelPack,
                                     supla_client_channel>(
          obj, data, SUPLA_CHANNELPACK_MAXCOUNT);
    }
  } else if (data_type & OI_REMOTEUPDATE_DATA2) {
    if (getClient()->getProtocolVersion() >= 9) {
      return get_datapack_for_remote<TSC_SuplaChannelValuePack,
                                     supla_client_channel>(
          obj, data, SUPLA_CHANNELVALUE_PACK_MAXCOUNT);

    } else {
      if (*data == NULL) {
        *data = malloc(sizeof(TSC_SuplaChannelValue));
      }

      static_cast<supla_client_channel *>(obj)->proto_get(
          (TSC_SuplaChannelValue *)*data, getClient());
    }

    return true;
  } else if (data_type & OI_REMOTEUPDATE_DATA3) {
    if (getClient()->getProtocolVersion() >= 10) {
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
    if (getClient()->getProtocolVersion() >= 10) {
      set_pack_eol<TSC_SuplaChannelPack_C>(data);
      srpc_sc_async_channelpack_update_c(
          srpc, static_cast<TSC_SuplaChannelPack_C *>(data));

    } else if (getClient()->getProtocolVersion() >= 8) {
      set_pack_eol<TSC_SuplaChannelPack_B>(data);
      srpc_sc_async_channelpack_update_b(
          srpc, static_cast<TSC_SuplaChannelPack_B *>(data));

    } else {
      set_pack_eol<TSC_SuplaChannelPack>(data);
      srpc_sc_async_channelpack_update(
          srpc, static_cast<TSC_SuplaChannelPack *>(data));
    }
  } else if (data_type & OI_REMOTEUPDATE_DATA2) {
    if (getClient()->getProtocolVersion() >= 9) {
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

void supla_client_channels::on_channel_value_changed(void *srpc, int DeviceId,
                                                     int ChannelId,
                                                     bool Extended) {
  on_value_changed(srpc, ChannelId, DeviceId, master,
                   Extended ? OI_REMOTEUPDATE_DATA3 : OI_REMOTEUPDATE_DATA2);
}

bool supla_client_channels::set_device_channel_new_value(int ChannelId,
                                                         char *value) {
  if (channel_exists(ChannelId)) {
    safe_array_lock(getArr());

    supla_client_channel *channel;
    int DeviceID = 0;

    if (NULL != (channel = find_channel(ChannelId))) {
      DeviceID = channel->getDeviceId();
    }

    safe_array_unlock(getArr());

    if (DeviceID) {
      return getClient()->getUser()->set_device_channel_value(
          EST_CLIENT, getClient()->getID(), DeviceID, ChannelId, value);
    }
  }

  return false;
}

bool supla_client_channels::set_device_channel_new_value(
    TCS_SuplaChannelNewValue_B *channel_new_value) {
  return set_device_channel_new_value(channel_new_value->ChannelId,
                                      channel_new_value->value);
}

bool supla_client_channels::set_device_channel_new_value(
    TCS_SuplaNewValue *new_value) {
  return set_device_channel_new_value(new_value->Id, new_value->value);
}

bool supla_client_channels::device_calcfg_request(
    TCS_DeviceCalCfgRequest *request) {
  if (request == NULL) return false;

  if (channel_exists(request->ChannelID)) {
    safe_array_lock(getArr());

    supla_client_channel *channel;
    int DeviceID = 0;

    if (NULL != (channel = find_channel(request->ChannelID))) {
      DeviceID = channel->getDeviceId();
    }

    safe_array_unlock(getArr());

    if (DeviceID) {
      return getClient()->getUser()->device_calcfg_request(getClient()->getID(),
                                                           DeviceID, request);
    }
  }

  return false;
}
