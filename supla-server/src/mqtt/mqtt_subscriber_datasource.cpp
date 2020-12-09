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

#include <mqtt_subscriber_datasource.h>

supla_mqtt_subscriber_datasource::supla_mqtt_subscriber_datasource(
    supla_mqtt_client_settings *settings)
    : supla_mqtt_client_datasource(settings) {
  this->provider = NULL;
}

supla_mqtt_subscriber_datasource::~supla_mqtt_subscriber_datasource(void) {}

bool supla_mqtt_subscriber_datasource::context_open(
    const _mqtt_ds_context_t *context) {
  if (context->scope == MQTTDS_SCOPE_FULL) {
    provider = new supla_mqtt_subscription_topic_provider();
    return provider != NULL;
  }
  return false;
}

bool supla_mqtt_subscriber_datasource::_fetch(const _mqtt_ds_context_t *context,
                                              char **topic_name, void **message,
                                              size_t *message_size) {
  if (provider) {
    return provider->fetch(get_settings()->getPrefix(), topic_name, NULL, NULL);
  }

  return false;
}

void supla_mqtt_subscriber_datasource::context_close(
    const _mqtt_ds_context_t *context) {
  if (provider) {
    delete provider;
    provider = NULL;
  }
}
