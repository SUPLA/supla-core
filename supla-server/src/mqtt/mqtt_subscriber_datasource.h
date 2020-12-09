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

#ifndef MQTT_SUBSCRIBER_DATASOURCE_H_
#define MQTT_SUBSCRIBER_DATASOURCE_H_

#include "mqtt_client_datasource.h"
#include "mqtt_subscription_topic_provider.h"

class supla_mqtt_subscriber_datasource : public supla_mqtt_client_datasource {
 private:
  supla_mqtt_subscription_topic_provider *provider;

 protected:
  virtual bool context_open(const _mqtt_ds_context_t *context);
  virtual bool _fetch(const _mqtt_ds_context_t *context, char **topic_name,
                      void **message, size_t *message_size);
  virtual void context_close(const _mqtt_ds_context_t *context);

 public:
  explicit supla_mqtt_subscriber_datasource(
      supla_mqtt_client_settings *settings);
  virtual ~supla_mqtt_subscriber_datasource(void);
};

#endif /*MQTT_SUBSCRIBER_DATASOURCE_H_*/
