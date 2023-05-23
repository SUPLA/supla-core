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

#ifndef MQTT_STATE_MESSAGE_PROVIDER_H_
#define MQTT_STATE_MESSAGE_PROVIDER_H_

#include <memory.h>

#include "device/channel_property_getter.h"
#include "devicechannel.h"
#include "mqtt_abstract_state_message_provider.h"
#include "mqtt_db.h"

class supla_mqtt_state_message_provider
    : public supla_mqtt_abstract_state_message_provider {
 private:
  supla_cahnnel_property_getter prop_getter;

 protected:
  virtual const char *_get_user_suid(void);
  virtual supla_abstract_channel_property_getter *_get_channel_property_getter(
      void);

 public:
  supla_mqtt_state_message_provider(void);
  virtual ~supla_mqtt_state_message_provider(void);
};

#endif /*MQTT_STATE_MESSAGE_PROVIDER_H_*/
