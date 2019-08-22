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

supla_client_channel::supla_client_channel(
    supla_client_channels *Container, int Id, int DeviceId, int LocationID,
    int Type, int Func, int Param1, int Param2, int Param3, char *TextParam1,
    char *TextParam2, char *TextParam3, const char *Caption, int AltIcon,
    int UserIcon, short ManufacturerID, short ProductID,
    unsigned char ProtocolVersion)
    : supla_client_objcontainer_item(Container, Id, Caption) {
  this->DeviceId = DeviceId;
  this->LocationId = LocationID;
  this->Type = Type;
  this->Func = Func;
  this->Param1 = Param1;
  this->Param2 = Param2;
  this->Param3 = Param3;
  this->TextParam1 = TextParam1 ? strndup(TextParam1, 255) : NULL;
  this->TextParam2 = TextParam2 ? strndup(TextParam2, 255) : NULL;
  this->TextParam3 = TextParam3 ? strndup(TextParam3, 255) : NULL;
  this->AltIcon = AltIcon;
  this->UserIcon = UserIcon;
  this->ManufacturerID = ManufacturerID;
  this->ProductID = ProductID;
  this->ProtocolVersion = ProtocolVersion;
  this->Flags = 0;
}

supla_client_channel::~supla_client_channel(void) {
  if (this->TextParam1) {
    free(this->TextParam1);
    this->TextParam1 = NULL;
  }

  if (this->TextParam2) {
    free(this->TextParam2);
    this->TextParam2 = NULL;
  }

  if (this->TextParam3) {
    free(this->TextParam3);
    this->TextParam3 = NULL;
  }
}

int supla_client_channel::getDeviceId() { return DeviceId; }

int supla_client_channel::getExtraId() { return DeviceId; }

bool supla_client_channel::remote_update_is_possible(void) {
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
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_GAS_METER:
    case SUPLA_CHANNELFNC_WATER_METER:
      return true;

    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:

      if (Param1 == 0 && Param2 == 0) {
        return true;
      }

      break;
  }

  return false;
}

void supla_client_channel::proto_get_value(TSuplaChannelValue *value,
                                           char *online, supla_client *client) {
  if (client && client->getUser()) {
    client->getUser()->get_channel_value(DeviceId, getId(), value, online);
    if (Type == SUPLA_CHANNELTYPE_IMPULSE_COUNTER) {
      switch (Func) {
        case SUPLA_CHANNELFNC_ELECTRICITY_METER:
        case SUPLA_CHANNELFNC_GAS_METER:
        case SUPLA_CHANNELFNC_WATER_METER: {
          TDS_ImpulseCounter_Value ds;
          memcpy(&ds, value->value, sizeof(TDS_ImpulseCounter_Value));
          memset(value->value, 0, SUPLA_CHANNELVALUE_SIZE);

          TSC_ImpulseCounter_Value sc;
          sc.calculated_value = supla_channel_ic_measurement::get_calculated_i(
              Param3, ds.counter);

          memcpy(value->value, &sc, sizeof(TSC_ImpulseCounter_Value));
          break;
        }
      }
    }
  }
}

void supla_client_channel::proto_get(TSC_SuplaChannel *channel,
                                     supla_client *client) {
  memset(channel, 0, sizeof(TSC_SuplaChannel));

  channel->Id = getId();
  channel->Func = Func;
  channel->LocationID = this->LocationId;

  proto_get_value(&channel->value, &channel->online, client);
  proto_get_caption(channel->Caption, &channel->CaptionSize,
                    SUPLA_CHANNEL_CAPTION_MAXSIZE);
}

void supla_client_channel::proto_get(TSC_SuplaChannel_B *channel,
                                     supla_client *client) {
  memset(channel, 0, sizeof(TSC_SuplaChannel_B));

  channel->Id = getId();
  channel->Func = Func;
  channel->LocationID = this->LocationId;
  channel->AltIcon = this->AltIcon;
  channel->ProtocolVersion = this->ProtocolVersion;
  channel->Flags = this->Flags;

  proto_get_value(&channel->value, &channel->online, client);
  proto_get_caption(channel->Caption, &channel->CaptionSize,
                    SUPLA_CHANNEL_CAPTION_MAXSIZE);
}

void supla_client_channel::proto_get(TSC_SuplaChannel_C *channel,
                                     supla_client *client) {
  memset(channel, 0, sizeof(TSC_SuplaChannel_C));

  channel->Id = getId();
  channel->DeviceID = getDeviceId();
  channel->Type = this->Type;
  channel->Func = Func;
  channel->LocationID = this->LocationId;
  channel->AltIcon = this->AltIcon;
  channel->UserIcon = this->UserIcon;
  channel->ManufacturerID = this->ManufacturerID;
  channel->ProductID = this->ProductID;
  channel->ProtocolVersion = this->ProtocolVersion;
  channel->Flags = this->Flags;

  proto_get_value(&channel->value, &channel->online, client);
  proto_get_caption(channel->Caption, &channel->CaptionSize,
                    SUPLA_CHANNEL_CAPTION_MAXSIZE);
}

void supla_client_channel::proto_get(TSC_SuplaChannelValue *channel_value,
                                     supla_client *client) {
  memset(channel_value, 0, sizeof(TSC_SuplaChannelValue));
  channel_value->Id = getId();
  proto_get_value(&channel_value->value, &channel_value->online, client);
}

bool supla_client_channel::proto_get(TSC_SuplaChannelExtendedValue *cev,
                                     supla_client *client) {
  if (cev == NULL) {
    return false;
  }

  memset(cev, 0, sizeof(TSC_SuplaChannelExtendedValue));
  cev->Id = getId();

  if (client && client->getUser() &&
      client->getUser()->get_channel_extendedvalue(DeviceId, getId(),
                                                   &cev->value)) {
    switch (cev->value.type) {
      case EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1:
        return supla_channel_electricity_measurement::update_cev(cev, Param2,
                                                                 TextParam1);

      case EV_TYPE_IMPULSE_COUNTER_DETAILS_V1:
        return supla_channel_ic_measurement::update_cev(
            cev, Func, Param2, Param3, TextParam1, TextParam2);
    }

    return true;
  }

  return false;
}

void supla_client_channel::mark_for_remote_update(int mark) {
  supla_client_objcontainer_item::mark_for_remote_update(mark);
  mark = marked_for_remote_update();
  if ((mark & OI_REMOTEUPDATE_DATA1) && (mark & OI_REMOTEUPDATE_DATA2)) {
    unmark_for_remote_update(OI_REMOTEUPDATE_DATA2);
  }
}
