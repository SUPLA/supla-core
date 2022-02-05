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

#ifndef MQTT_ACTION_MESSAGE_PROVIDER_H_
#define MQTT_ACTION_MESSAGE_PROVIDER_H_

#include "mqtt_abstract_action_message_provider.h"
#include "mqtt_db.h"

class supla_mqtt_action_message_provider
    : public supla_mqtt_abstract_action_message_provider {
 protected:
  virtual const char *_get_user_suid(int user_id);

 public:
  supla_mqtt_action_message_provider(void);
  virtual ~supla_mqtt_action_message_provider(void);
};

#endif /*MQTT_ACTION_MESSAGE_PROVIDER_H_*/
