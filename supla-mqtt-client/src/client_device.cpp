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

#include "client_device.h"

client_device_channel::client_device_channel(
    int Id, int Number, int Type, int Func, int Param1, int Param2, int Param3,
    char *TextParam1, char *TextParam2, char *TextParam3, bool Hidden,
    bool Online, char *Caption, const std::vector<client_state *> &States)
    : supla_device_channel(Id, Number, Type, Func, Param1, Param2, Param3,
                           TextParam1, TextParam2, TextParam3, Hidden) {
  this->Online = Online;
  this->Caption =
      Caption ? strndup(Caption, SUPLA_CHANNEL_CAPTION_MAXSIZE) : NULL;
  for (auto state : States) this->states.push_back(state);
  memset(this->Sub_value, 0, SUPLA_CHANNELVALUE_SIZE);
}

client_device_channel::~client_device_channel() {
  if (this->Caption) {
    free(this->Caption);
    this->Caption = NULL;
  }
  this->states.clear();
  std::vector<client_state *>().swap(states);
}

char *client_device_channel::getCaption(void) { return Caption; }

const std::vector<client_state *> client_device_channel::getStates(void) const {
  return this->states;
}

void client_device_channel::setSubValue(
    char sub_value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(this->Sub_value, sub_value, SUPLA_CHANNELVALUE_SIZE);
}

void client_device_channel::getSubValue(
    char sub_value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(sub_value, this->Sub_value, SUPLA_CHANNELVALUE_SIZE);
}

void client_device_channel::setOnline(bool value) { this->Online = value; }

bool client_device_channel::getOnline() { return this->Online; }

void client_device_channels::add_channel(
    int Id, int Number, int Type, int Func, int Param1, int Param2, int Param3,
    char *TextParam1, char *TextParam2, char *TextParam3, bool Hidden,
    bool Online, char *Caption, const std::vector<client_state *> &States) {
  safe_array_lock(arr);

  client_device_channel *channel = find_channel(Id);
  if (channel == 0) {
    if (Type == 0) {
      /* enable support for proto version < 10 */
      switch (Func) {
        case SUPLA_CHANNELFNC_THERMOMETER:
          Type = SUPLA_CHANNELTYPE_THERMOMETERDS18B20;
          break;
        case SUPLA_CHANNELFNC_DISTANCESENSOR:
          Type = SUPLA_CHANNELTYPE_DISTANCESENSOR;
          break;
        case SUPLA_CHANNELFNC_DIMMER:
          Type = SUPLA_CHANNELTYPE_DIMMER;
          break;
        case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
          Type = SUPLA_CHANNELTYPE_DIMMERANDRGBLED;
          break;
        case SUPLA_CHANNELFNC_THERMOSTAT:
          Type = SUPLA_CHANNELTYPE_THERMOSTAT;
          break;
        case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
          Type = SUPLA_CHANNELTYPE_THERMOSTAT_HEATPOL_HOMEPLUS;
          break;
        case SUPLA_CHANNELFNC_RGBLIGHTING:
          Type = SUPLA_CHANNELTYPE_RGBLEDCONTROLLER;
          break;
        case SUPLA_CHANNELFNC_GAS_METER:
        case SUPLA_CHANNELFNC_WATER_METER:
          Type = SUPLA_CHANNELTYPE_IMPULSE_COUNTER;
          break;
        case SUPLA_CHANNELFNC_ELECTRICITY_METER:
          Type = SUPLA_CHANNELTYPE_ELECTRICITY_METER;
          break;
        case SUPLA_CHANNELFNC_HUMIDITY:
        case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
          Type = SUPLA_CHANNELTYPE_DHT11;
          break;
        case SUPLA_CHANNELFNC_WEATHER_STATION:
          Type = SUPLA_CHANNELTYPE_WEATHER_STATION;
          break;
        case SUPLA_CHANNELFNC_WEIGHTSENSOR:
          Type = SUPLA_CHANNELTYPE_WEIGHTSENSOR;
          break;
        case SUPLA_CHANNELFNC_RAINSENSOR:
          Type = SUPLA_CHANNELTYPE_RAINSENSOR;
          break;
        case SUPLA_CHANNELFNC_PRESSURESENSOR:
          Type = SUPLA_CHANNELTYPE_PRESSURESENSOR;
          break;
        case SUPLA_CHANNELFNC_WINDSENSOR:
          Type = SUPLA_CHANNELTYPE_WINDSENSOR;
          break;
        case SUPLA_CHANNELFNC_MAILSENSOR:
          Type = SUPLA_CHANNELTYPE_SENSORNC;
          break;
        case SUPLA_CHANNELFNC_LIGHTSWITCH:
        case SUPLA_CHANNELFNC_POWERSWITCH:
          Type = SUPLA_CHANNELTYPE_RELAY;
          break;
      }
    }

    client_device_channel *c = new client_device_channel(
        Id, Number, Type, Func, Param1, Param2, Param3, TextParam1, TextParam2,
        TextParam3, Hidden, Online, Caption, States);

    if (c != NULL && safe_array_add(arr, c) == -1) {
      delete c;
      c = NULL;
    }
  } else {
    channel->setOnline(Online);
  }

  safe_array_unlock(arr);
}

void client_device_channels::set_channel_sub_value(
    int ChannelID, char sub_value[SUPLA_CHANNELVALUE_SIZE]) {
  if (ChannelID == 0) return;

  safe_array_lock(arr);

  client_device_channel *channel = find_channel(ChannelID);

  if (channel) {
    channel->setSubValue(sub_value);
  }
  safe_array_unlock(arr);
}

client_device_channel *client_device_channels::find_channel(int ChannelId) {
  return (client_device_channel *)safe_array_findcnd(arr, arr_findcmp,
                                                     &ChannelId);
}
