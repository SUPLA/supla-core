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

#include "voice_assistant_client.h"

using std::string;

supla_voice_assistant_client::supla_voice_assistant_client(
    int channel_id, supla_abstract_curl_adapter *curl_adapter,
    supla_http_oauth_credentials *credentials) {
  this->channel_id = channel_id;
  this->subchannel_id = 0;
  this->channel_connected = false;
  this->channel_value = nullptr;
  this->curl_adapter = curl_adapter;
  this->credentials = credentials;
}

supla_voice_assistant_client::~supla_voice_assistant_client(void) {
  if (channel_value) {
    delete channel_value;
  }
}

int supla_voice_assistant_client::get_channel_id(void) { return channel_id; }

short supla_voice_assistant_client::get_subchannel_id(void) {
  return subchannel_id;
}

supla_channel_value *supla_voice_assistant_client::get_channel_value(void) {
  return channel_value;
}

supla_abstract_curl_adapter *supla_voice_assistant_client::get_curl_adapter(
    void) {
  return curl_adapter;
}

supla_http_oauth_credentials *supla_voice_assistant_client::get_credentials(
    void) {
  return credentials;
}

bool supla_voice_assistant_client::is_channel_connected(void) {
  return channel_connected;
}

string supla_voice_assistant_client::get_endpoint_id(void) {
  string result;

  string uuid = credentials->get_user_short_unique_id();
  if (uuid.size() == 0) {
    return result;
  }

  result = uuid + "-" + std::to_string(channel_id);

  if (subchannel_id) {
    result.append("-" + std::to_string(subchannel_id));
  }

  return result;
}

void supla_voice_assistant_client::set_subchannel_id(short subchannel_id) {
  this->subchannel_id = subchannel_id;
}

void supla_voice_assistant_client::set_channel_connected(bool connected) {
  this->channel_connected = connected;
}

void supla_voice_assistant_client::set_channel_value(
    supla_channel_value *channel_value) {
  this->channel_value = channel_value;
}
