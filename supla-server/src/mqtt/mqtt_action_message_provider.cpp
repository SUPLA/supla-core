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

#include "mqtt_action_message_provider.h"
#include "log.h"

supla_mqtt_action_message_provider::supla_mqtt_action_message_provider(void)
    : supla_mqtt_abstract_action_message_provider() {}

supla_mqtt_action_message_provider::~supla_mqtt_action_message_provider(void) {}

const char *supla_mqtt_action_message_provider::_get_user_suid(int user_id) {
  supla_user *user = supla_user::find(user_id, false);
  if (user != NULL) {
    return user->getShortUniqueID();
  }

  return NULL;
}
