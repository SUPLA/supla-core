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

void client_device_channel::setCaption(char *caption) {
  if (this->Caption) {
    free(this->Caption);
    this->Caption = NULL;
  }
  this->Caption =
      caption ? strndup(Caption, SUPLA_CHANNEL_CAPTION_MAXSIZE) : NULL;
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
    client_device_channel *c = new client_device_channel(
        Id, Number, Type, Func, Param1, Param2, Param3, TextParam1, TextParam2,
        TextParam3, Hidden, Online, Caption, States);

    if (c != NULL && safe_array_add(arr, c) == -1) {
      delete c;
      c = NULL;
    }
  } else {
    channel->setOnline(Online);
    channel->setCaption(Caption);
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
