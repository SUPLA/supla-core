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

#include "amazon/alexa_client2.h"

#include "device/value/channel_rs_value.h"

supla_alexa_client2::supla_alexa_client2(
    int channel_id, supla_abstract_curl_adapter *curl_adapter,
    supla_amazon_alexa_credentials2 *credentials)
    : supla_voice_assistant_client2(channel_id, curl_adapter, credentials) {
  cause_type = CAUSE_APP_INTERACTION;
  props = nullptr;
}

supla_alexa_client2::~supla_alexa_client2(void) {
  if (props) {
    cJSON_Delete(props);
  }
}

void supla_alexa_client2::set_cause_type(int cause_type) {
  this->cause_type = cause_type;
}

void supla_alexa_client2::add_props(cJSON *props) {
  if (!props) {
    return;
  }

  if (!this->props) {
    this->props = cJSON_CreateArray();
  }

  if (this->props) {
    cJSON_AddItemToArray(this->props, props);
  }
}

cJSON *supla_alexa_client2::get_percentage_controller_properties(
    short percentage) {}

void supla_alexa_client2::add_percentage_controller(void) {
  if (is_channel_connected()) {
    supla_channel_rs_value *rs_val =
        dynamic_cast<supla_channel_rs_value *>(get_channel_value());
    if (rs_val) {
      add_props(get_percentage_controller_properties(
          rs_val->get_rs_value()->position));
    }
  }
}
