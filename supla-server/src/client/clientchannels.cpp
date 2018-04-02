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

#include "../database.h"
#include "../log.h"
#include "../safearray.h"
#include "../srpc.h"
#include "../user.h"
#include "clientchannel.h"
#include "clientchannels.h"

supla_client_channels::supla_client_channels(supla_client *client)
    : supla_client_objcontainer(client) {}

supla_client_channel *supla_client_channels::find_channel(int Id) {
  return static_cast<supla_client_channel *>(find(Id));
}

bool supla_client_channels::channel_exists(int ChannelID) {
  bool result = false;

  safe_array_lock(getArr());

  if (find_channel(ChannelID) != NULL) result = true;

  safe_array_unlock(getArr());

  return result;
}

void supla_client_channels::_load(database *db) {
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
}

bool supla_client_channels::get_data_for_remote(
    supla_client_objcontainer_item *obj, void **data, bool full, bool EOL,
    bool *check_more) {
  if (full) {
    *check_more = true;

    if (getClient()->getProtocolVersion() > 7) {
      TSC_SuplaChannelPack_B *pack =
          static_cast<TSC_SuplaChannelPack_B *>(*data);
      if (pack == NULL) {
        pack = (TSC_SuplaChannelPack_B *)malloc(sizeof(TSC_SuplaChannelPack_B));
        memset(pack, 0, sizeof(TSC_SuplaChannelPack_B));
      }

      *data = pack;

      if (pack->count < SUPLA_CHANNELPACK_MAXCOUNT) {
        static_cast<supla_client_channel *>(obj)->proto_get_channel(
            &pack->channels[pack->count], getClient());
        pack->channels[pack->count].EOL = 0;
        pack->count++;
        return true;
      } else {
        pack->total_left++;
      }

    } else {
      TSC_SuplaChannelPack *pack = static_cast<TSC_SuplaChannelPack *>(*data);
      if (pack == NULL) {
        pack = (TSC_SuplaChannelPack *)malloc(sizeof(TSC_SuplaChannelPack));
        memset(pack, 0, sizeof(TSC_SuplaChannelPack));
      }

      *data = pack;

      if (pack->count < SUPLA_CHANNELPACK_MAXCOUNT) {
        static_cast<supla_client_channel *>(obj)->proto_get_channel(
            &pack->channels[pack->count], getClient());
        pack->channels[pack->count].EOL = 0;
        pack->count++;
        return true;
      } else {
        pack->total_left++;
      }
    }
  } else {
    if (*data == NULL) {
      *data = malloc(sizeof(TSC_SuplaChannelValue));
    }

    static_cast<supla_client_channel *>(obj)->proto_get_channel_value(
        (TSC_SuplaChannelValue *)*data, getClient());

    return true;
  }

  return false;
}

void supla_client_channels::send_data_to_remote_and_free(void *srpc, void *data,
                                                         bool full) {
  if (full) {
    if (getClient()->getProtocolVersion() > 7) {
      TSC_SuplaChannelPack_B *pack =
          static_cast<TSC_SuplaChannelPack_B *>(data);

      if (pack->count > 0) {
        if (pack->total_left == 0) pack->channels[pack->count - 1].EOL = 1;

        srpc_sc_async_channelpack_update_b(srpc, pack);
      }
    } else {
      TSC_SuplaChannelPack *pack = static_cast<TSC_SuplaChannelPack *>(data);

      if (pack->count > 0) {
        if (pack->total_left == 0) pack->channels[pack->count - 1].EOL = 1;

        srpc_sc_async_channelpack_update(srpc, pack);
      }
    }
  } else {
    srpc_sc_async_channel_value_update(srpc, (TSC_SuplaChannelValue *)data);
    free(data);
  }
}

void supla_client_channels::_update(supla_client_objcontainer_item *obj,
                                    supla_client_objcontainer_item *source) {
  supla_client_channel *channel = dynamic_cast<supla_client_channel *>(obj);
  supla_client_channel *src = dynamic_cast<supla_client_channel *>(source);
  if (channel) {
    channel->update(src);
  }
}

supla_client_objcontainer_item *supla_client_channels::new_item(
    supla_client_objcontainer_item *obj) {
  supla_client_channel *channel = dynamic_cast<supla_client_channel *>(obj);
  if (channel) {
    return new supla_client_channel(channel);
  }

  return NULL;
}

void supla_client_channels::on_channel_value_changed(void *srpc, int DeviceId,
                                                     int ChannelId) {
  on_value_changed(srpc, ChannelId, DeviceId);
}

bool supla_client_channels::set_device_channel_new_value(
    TCS_SuplaChannelNewValue_B *channel_new_value) {
  if (channel_exists(channel_new_value->ChannelId)) {
    safe_array_lock(getArr());

    supla_client_channel *channel;
    int DeviceID = 0;

    if (NULL != (channel = find_channel(channel_new_value->ChannelId))) {
      DeviceID = channel->getDeviceId();
    }

    safe_array_unlock(getArr());

    if (DeviceID) {
      getClient()->getUser()->set_device_channel_value(
          getClient()->getID(), DeviceID, channel_new_value->ChannelId,
          channel_new_value->value);
    }
  }

  return false;
}
