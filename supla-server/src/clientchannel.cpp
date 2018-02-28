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

#include "client.h"
#include "clientchannel.h"
#include "database.h"
#include "log.h"
#include "proto.h"
#include "safearray.h"
#include "srpc.h"
#include "user.h"

#define CC_REMOTEUPDATE_NONE 0
#define CC_REMOTEUPDATE_CHANNEL 1
#define CC_REMOTEUPDATE_CHANNELVALUE 2

supla_client_channel::supla_client_channel(int Id, int DeviceId, int LocationID,
                                           int Func, int Param1, int Param2,
                                           const char *Caption, int AltIcon,
                                           unsigned char ProtocolVersion) {
  this->Id = Id;
  this->DeviceId = DeviceId;
  this->LocationId = LocationID;
  this->Func = Func;
  this->Param1 = Param1;
  this->Param2 = Param2;
  this->AltIcon = AltIcon;
  this->ProtocolVersion = ProtocolVersion;
  this->Flags = 0;

  if (Caption) {
    this->Caption = strdup(Caption);
  } else {
    this->Caption = NULL;
  }

  remote_update = CC_REMOTEUPDATE_NONE;
}

supla_client_channel::~supla_client_channel() { setCaption(NULL); }

int supla_client_channel::getId(void) { return Id; }

int supla_client_channel::getDeviceId() { return DeviceId; }

void supla_client_channel::setCaption(const char *Caption) {
  if (this->Caption != NULL) {
    free(this->Caption);
    this->Caption = NULL;
  }

  if (Caption) {
    this->Caption = strdup(Caption);
  }
}

void supla_client_channel::mark_for_remote_update(char mark) {
  if (mark == CC_REMOTEUPDATE_NONE) {
    remote_update = CC_REMOTEUPDATE_NONE;
    return;

  } else if (mark == CC_REMOTEUPDATE_CHANNELVALUE &&
             remote_update == CC_REMOTEUPDATE_CHANNEL) {
    mark = CC_REMOTEUPDATE_CHANNEL;
  }

  switch (Func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_THERMOMETER:
    case SUPLA_CHANNELFNC_HUMIDITY:
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_DEPTHSENSOR:
    case SUPLA_CHANNELFNC_DISTANCESENSOR:
    case SUPLA_CHANNELFNC_MAILSENSOR:
    case SUPLA_CHANNELFNC_WINDSENSOR:
    case SUPLA_CHANNELFNC_PRESSURESENSOR:
    case SUPLA_CHANNELFNC_RAINSENSOR:
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
    case SUPLA_CHANNELFNC_WEATHER_STATION:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:

      remote_update = mark;
      break;

    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:

      if (Param1 == 0 && Param2 == 0) {
        remote_update = mark;
      }

      break;
  }
}

char supla_client_channel::marked_for_remote_update(void) {
  return remote_update;
}

void supla_client_channel::proto_get_channel(TSC_SuplaChannel *channel,
                                             supla_client *client) {
  memset(channel, 0, sizeof(TSC_SuplaChannel));

  channel->Id = Id;
  channel->Func = Func;
  channel->LocationID = this->LocationId;

  if (client && client->getUser()) {
    client->getUser()->get_channel_value(DeviceId, Id, &channel->value,
                                         &channel->online);
  }

  if (Caption) {
    snprintf(channel->Caption, SUPLA_CHANNEL_CAPTION_MAXSIZE, "%s", Caption);
    channel->CaptionSize =
        strnlen(channel->Caption, SUPLA_CHANNEL_CAPTION_MAXSIZE - 1) + 1;
  } else {
    channel->CaptionSize = 1;
    channel->Caption[0] = 0;
  }
}

void supla_client_channel::proto_get_channel(TSC_SuplaChannel_B *channel,
                                             supla_client *client) {
  memset(channel, 0, sizeof(TSC_SuplaChannel_B));

  channel->Id = Id;
  channel->Func = Func;
  channel->LocationID = this->LocationId;
  channel->AltIcon = this->AltIcon;
  channel->ProtocolVersion = this->ProtocolVersion;
  channel->Flags = this->Flags;

  if (client && client->getUser()) {
    client->getUser()->get_channel_value(DeviceId, Id, &channel->value,
                                         &channel->online);
  }

  if (Caption) {
    snprintf(channel->Caption, SUPLA_CHANNEL_CAPTION_MAXSIZE, "%s", Caption);
    channel->CaptionSize =
        strnlen(channel->Caption, SUPLA_CHANNEL_CAPTION_MAXSIZE - 1) + 1;
  } else {
    channel->CaptionSize = 1;
    channel->Caption[0] = 0;
  }
}

void supla_client_channel::proto_get_channel_value(
    TSC_SuplaChannelValue *channel_value, supla_client *client) {
  memset(channel_value, 0, sizeof(TSC_SuplaChannelValue));
  channel_value->Id = Id;

  if (client && client->getUser()) {
    client->getUser()->get_channel_value(DeviceId, Id, &channel_value->value,
                                         &channel_value->online);
  }
}

// -----------------------------------------
// -----------------------------------------
// -----------------------------------------

char supla_client_channels::arr_findcmp(void *ptr, void *id) {
  return ((supla_client_channel *)ptr)->getId() == *((int *)id) ? 1 : 0;
}

char supla_client_channels::arr_delcnd(void *ptr) {
  delete (supla_client_location *)ptr;
  return 1;
}

void supla_client_channels::arr_clean(void) {
  safe_array_lock(arr);
  safe_array_clean(arr, arr_delcnd);
  safe_array_unlock(arr);
}

supla_client_channels::supla_client_channels(supla_client *client) {
  this->client = client;
  this->arr = safe_array_init();
}

supla_client_channels::~supla_client_channels() {
  arr_clean();
  safe_array_free(arr);
}

supla_client_channel *supla_client_channels::find_channel(int Id) {
  return (supla_client_channel *)safe_array_findcnd(arr, arr_findcmp, &Id);
}

void supla_client_channels::update_channel(int Id, int DeviceId, int LocationID,
                                           int Func, int Param1, int Param2,
                                           const char *Caption, int AltIcon,
                                           unsigned char ProtocolVersion) {
  safe_array_lock(arr);

  supla_client_channel *channel = NULL;

  if ((channel = find_channel(Id)) == NULL) {
    channel =
        new supla_client_channel(Id, DeviceId, LocationID, Func, Param1, Param2,
                                 Caption, AltIcon, ProtocolVersion);
    if (safe_array_add(arr, channel) == -1) {
      delete channel;
      channel = NULL;
    }

  } else {
    channel->setCaption(Caption);
  }

  if (channel) {
    channel->mark_for_remote_update(CC_REMOTEUPDATE_CHANNEL);
  }

  safe_array_unlock(arr);
}

bool supla_client_channels::channel_exists(int ChannelID) {
  bool result = false;

  safe_array_lock(arr);

  if (find_channel(ChannelID) != NULL) result = true;

  safe_array_unlock(arr);

  return result;
}

void supla_client_channels::load(void) {
  database *db = new database();

  if (db->connect() == true) {
    safe_array_lock(arr);
    arr_clean();

    db->get_client_channels(client->getID(), NULL, this);

    safe_array_unlock(arr);
  }

  delete db;
}

int supla_client_channels::count(void) { return safe_array_count(arr); }

void supla_client_channels::update_device_channels(int DeviceId) {
  database *db = new database();

  if (db->connect() == true) {
    safe_array_lock(arr);
    db->get_client_channels(client->getID(), &DeviceId, this);
    safe_array_unlock(arr);
  }

  delete db;
}

supla_client_channel *supla_client_channels::get_marked(void) {
  supla_client_channel *channel = NULL;

  for (int a = 0; a < safe_array_count(arr); a++) {
    channel = (supla_client_channel *)safe_array_get(arr, a);
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

  safe_array_lock(arr);

  for (int a = 0; a < safe_array_count(arr); a++) {
    supla_client_channel *channel =
        (supla_client_channel *)safe_array_get(arr, a);
    if (channel->marked_for_remote_update() == CC_REMOTEUPDATE_CHANNELVALUE) {
      channel->proto_get_channel_value(&channel_value, client);
      channel->mark_for_remote_update(CC_REMOTEUPDATE_NONE);
      channel_value.EOL = get_marked() == NULL ? 1 : 0;

      result = true;
      break;
    }
  }

  safe_array_unlock(arr);

  if (result) {
    srpc_sc_async_channel_value_update(srpc, &channel_value);
  }

  return result;
}

bool supla_client_channels::remote_update_c(void *srpc) {
  TSC_SuplaChannelPack pack;
  memset(&pack, 0, sizeof(TSC_SuplaChannelPack));

  safe_array_lock(arr);

  for (int a = 0; a < safe_array_count(arr); a++) {
    supla_client_channel *channel =
        (supla_client_channel *)safe_array_get(arr, a);

    if (channel->marked_for_remote_update() == CC_REMOTEUPDATE_CHANNEL) {
      if (pack.count < SUPLA_CHANNELPACK_MAXCOUNT) {
        channel->proto_get_channel(&pack.channels[pack.count], client);
        pack.channels[pack.count].EOL = 0;
        channel->mark_for_remote_update(CC_REMOTEUPDATE_NONE);
        pack.count++;

      } else {
        pack.total_left++;
      }
    }
  }

  safe_array_unlock(arr);

  if (pack.count > 0) {
    if (pack.total_left == 0) pack.channels[pack.count - 1].EOL = 1;

    srpc_sc_async_channelpack_update(srpc, &pack);
  }

  return pack.count > 0;
}

bool supla_client_channels::remote_update_c_b(void *srpc) {
  TSC_SuplaChannelPack_B pack;
  memset(&pack, 0, sizeof(TSC_SuplaChannelPack_B));

  safe_array_lock(arr);

  for (int a = 0; a < safe_array_count(arr); a++) {
    supla_client_channel *channel =
        (supla_client_channel *)safe_array_get(arr, a);

    if (channel->marked_for_remote_update() == CC_REMOTEUPDATE_CHANNEL) {
      if (pack.count < SUPLA_CHANNELPACK_MAXCOUNT) {
        channel->proto_get_channel(&pack.channels[pack.count], client);
        pack.channels[pack.count].EOL = 0;
        channel->mark_for_remote_update(CC_REMOTEUPDATE_NONE);
        pack.count++;

      } else {
        pack.total_left++;
      }
    }
  }

  safe_array_unlock(arr);

  if (pack.count > 0) {
    if (pack.total_left == 0) pack.channels[pack.count - 1].EOL = 1;

    srpc_sc_async_channelpack_update_b(srpc, &pack);
  }

  return pack.count > 0;
}

bool supla_client_channels::remote_update(void *srpc) {
  if (client->getProtocolVersion() > 7) {
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

  safe_array_lock(arr);

  for (int a = 0; a < safe_array_count(arr); a++) {
    channel = (supla_client_channel *)safe_array_get(arr, a);
    if (channel && channel->getDeviceId() == DeviceId &&
        (ChannelId == 0 || channel->getId() == ChannelId)) {
      channel->mark_for_remote_update(CC_REMOTEUPDATE_CHANNELVALUE);
      r = true;
    }
  }

  safe_array_unlock(arr);

  if (srpc && r) {
    remote_update(srpc);
  }
}

bool supla_client_channels::set_device_channel_new_value(
    TCS_SuplaChannelNewValue_B *channel_new_value) {
  if (channel_exists(channel_new_value->ChannelId)) {
    safe_array_lock(arr);

    supla_client_channel *channel;
    int DeviceID = 0;

    if (NULL != (channel = find_channel(channel_new_value->ChannelId))) {
      DeviceID = channel->getDeviceId();
    }

    safe_array_unlock(arr);

    if (DeviceID) {
      client->getUser()->set_device_channel_value(client->getID(), DeviceID,
                                                  channel_new_value->ChannelId,
                                                  channel_new_value->value);
    }
  }

  return false;
}
