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

void supla_client_channels::update_channel(int Id, int DeviceId, int LocationID,
                                           int Func, int Param1, int Param2,
                                           const char *Caption, int AltIcon,
                                           unsigned char ProtocolVersion) {
  safe_array_lock(getArr());

  supla_client_channel *channel = NULL;

  if ((channel = find_channel(Id)) == NULL) {
    channel =
        new supla_client_channel(Id, DeviceId, LocationID, Func, Param1, Param2,
                                 Caption, AltIcon, ProtocolVersion);
    if (safe_array_add(getArr(), channel) == -1) {
      delete channel;
      channel = NULL;
    }

  } else {
    channel->setCaption(Caption);
  }

  if (channel) {
    channel->mark_for_remote_update(CC_REMOTEUPDATE_CHANNEL);
  }

  safe_array_unlock(getArr());
}

bool supla_client_channels::channel_exists(int ChannelID) {
  bool result = false;

  safe_array_lock(getArr());

  if (find_channel(ChannelID) != NULL) result = true;

  safe_array_unlock(getArr());

  return result;
}

void supla_client_channels::load(void) {
  database *db = new database();

  if (db->connect() == true) {
    safe_array_lock(getArr());
    arr_clean();

    db->get_client_channels(getClient()->getID(), NULL, this);

    safe_array_unlock(getArr());
  }

  delete db;
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

supla_client_channel *supla_client_channels::get_marked(void) {
  supla_client_channel *channel = NULL;

  for (int a = 0; a < safe_array_count(getArr()); a++) {
    channel = (supla_client_channel *)safe_array_get(getArr(), a);
    if (channel->marked_for_remote_update() != CC_REMOTEUPDATE_NONE) {
      break;
    } else {
      channel = NULL;
    }
  }

  return channel;
}

bool supla_client_channels::remote_update_cv(void *srpc) {
  bool result = false;
  TSC_SuplaChannelValue channel_value;

  safe_array_lock(getArr());

  for (int a = 0; a < safe_array_count(getArr()); a++) {
    supla_client_channel *channel =
        (supla_client_channel *)safe_array_get(getArr(), a);
    if (channel->marked_for_remote_update() == CC_REMOTEUPDATE_CHANNELVALUE) {
      channel->proto_get_channel_value(&channel_value, getClient());
      channel->mark_for_remote_update(CC_REMOTEUPDATE_NONE);
      channel_value.EOL = get_marked() == NULL ? 1 : 0;

      result = true;
      break;
    }
  }

  safe_array_unlock(getArr());

  if (result) {
    srpc_sc_async_channel_value_update(srpc, &channel_value);
  }

  return result;
}

bool supla_client_channels::remote_update_c(void *srpc) {
  TSC_SuplaChannelPack pack;
  memset(&pack, 0, sizeof(TSC_SuplaChannelPack));

  safe_array_lock(getArr());

  for (int a = 0; a < safe_array_count(getArr()); a++) {
    supla_client_channel *channel =
        (supla_client_channel *)safe_array_get(getArr(), a);

    if (channel->marked_for_remote_update() == CC_REMOTEUPDATE_CHANNEL) {
      if (pack.count < SUPLA_CHANNELPACK_MAXCOUNT) {
        channel->proto_get_channel(&pack.channels[pack.count], getClient());
        pack.channels[pack.count].EOL = 0;
        channel->mark_for_remote_update(CC_REMOTEUPDATE_NONE);
        pack.count++;

      } else {
        pack.total_left++;
      }
    }
  }

  safe_array_unlock(getArr());

  if (pack.count > 0) {
    if (pack.total_left == 0) pack.channels[pack.count - 1].EOL = 1;

    srpc_sc_async_channelpack_update(srpc, &pack);
  }

  return pack.count > 0;
}

bool supla_client_channels::remote_update_c_b(void *srpc) {
  TSC_SuplaChannelPack_B pack;
  memset(&pack, 0, sizeof(TSC_SuplaChannelPack_B));

  safe_array_lock(getArr());

  for (int a = 0; a < safe_array_count(getArr()); a++) {
    supla_client_channel *channel =
        (supla_client_channel *)safe_array_get(getArr(), a);

    if (channel->marked_for_remote_update() == CC_REMOTEUPDATE_CHANNEL) {
      if (pack.count < SUPLA_CHANNELPACK_MAXCOUNT) {
        channel->proto_get_channel(&pack.channels[pack.count], getClient());
        pack.channels[pack.count].EOL = 0;
        channel->mark_for_remote_update(CC_REMOTEUPDATE_NONE);
        pack.count++;

      } else {
        pack.total_left++;
      }
    }
  }

  safe_array_unlock(getArr());

  if (pack.count > 0) {
    if (pack.total_left == 0) pack.channels[pack.count - 1].EOL = 1;

    srpc_sc_async_channelpack_update_b(srpc, &pack);
  }

  return pack.count > 0;
}

bool supla_client_channels::remote_update(void *srpc) {
  if (getClient()->getProtocolVersion() > 7) {
    if (remote_update_c_b(srpc)) {
      return true;
    }
  } else {
    if (remote_update_c(srpc)) {
      return true;
    }
  }

  return remote_update_cv(srpc);
}

void supla_client_channels::on_channel_value_changed(void *srpc, int DeviceId,
                                                     int ChannelId) {
  supla_client_channel *channel;
  bool r = false;

  safe_array_lock(getArr());

  for (int a = 0; a < safe_array_count(getArr()); a++) {
    channel = (supla_client_channel *)safe_array_get(getArr(), a);
    if (channel && channel->getDeviceId() == DeviceId &&
        (ChannelId == 0 || channel->getId() == ChannelId)) {
      channel->mark_for_remote_update(CC_REMOTEUPDATE_CHANNELVALUE);
      r = true;
    }
  }

  safe_array_unlock(getArr());

  if (srpc && r) {
    remote_update(srpc);
  }
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
