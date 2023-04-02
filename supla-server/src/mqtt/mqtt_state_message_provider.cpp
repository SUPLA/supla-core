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

#include "mqtt_state_message_provider.h"

#include <stdlib.h>
#include <string.h>

#include <memory>

#include "device/channel_property_getter.h"
#include "user.h"

using std::shared_ptr;

supla_mqtt_state_message_provider::supla_mqtt_state_message_provider(void)
    : supla_mqtt_abstract_state_message_provider() {}

supla_mqtt_state_message_provider::~supla_mqtt_state_message_provider(void) {}

const char *supla_mqtt_state_message_provider::_get_user_suid(void) {
  supla_user *user = supla_user::find(get_user_id(), false);
  if (user != NULL) {
    return user->getShortUniqueID();
  }

  return NULL;
}

supla_abstract_channel_property_getter *
supla_mqtt_state_message_provider::_get_channel_property_getter(void) {
  return &prop_getter;
}
