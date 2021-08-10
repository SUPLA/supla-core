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

#include <mqtt_ds_context.h>

void supla_mqtt_ds_context::set_vars(MQTTDataSourceScope scope, int user_id,
                                     int device_id, int channel_id) {
  this->scope = scope;

  this->user_id = 0;
  this->device_id = 0;
  this->channel_id = 0;

  if (scope != MQTTDS_SCOPE_NONE && scope != MQTTDS_SCOPE_FULL) {
    this->user_id = user_id;
  }

  if (scope == MQTTDS_SCOPE_DEVICE || scope == MQTTDS_SCOPE_CHANNEL_STATE) {
    this->device_id = device_id;
  }

  if (scope == MQTTDS_SCOPE_CHANNEL_STATE) {
    this->channel_id = channel_id;
  }
}

supla_mqtt_ds_context::supla_mqtt_ds_context(void) {
  set_vars(MQTTDS_SCOPE_NONE, 0, 0, 0);
}

supla_mqtt_ds_context::supla_mqtt_ds_context(MQTTDataSourceScope scope) {
  set_vars(scope, 0, 0, 0);
}

supla_mqtt_ds_context::supla_mqtt_ds_context(MQTTDataSourceScope scope,
                                             int user_id) {
  set_vars(scope, user_id, 0, 0);
}

supla_mqtt_ds_context::supla_mqtt_ds_context(MQTTDataSourceScope scope,
                                             int user_id, int device_id) {
  set_vars(scope, user_id, device_id, 0);
}

supla_mqtt_ds_context::supla_mqtt_ds_context(MQTTDataSourceScope scope,
                                             int user_id, int device_id,
                                             int channel_id) {
  set_vars(scope, user_id, device_id, channel_id);
}

void supla_mqtt_ds_context::set_user_id(int user_id) {
  if (scope == MQTTDS_SCOPE_FULL &&
      ((!user_id && this->user_id) || (user_id && !this->user_id))) {
    this->user_id = user_id;
  }
}

MQTTDataSourceScope supla_mqtt_ds_context::get_scope(void) { return scope; }

int supla_mqtt_ds_context::get_user_id(void) { return user_id; }

int supla_mqtt_ds_context::get_device_id(void) { return device_id; }

int supla_mqtt_ds_context::get_channel_id(void) { return channel_id; }
