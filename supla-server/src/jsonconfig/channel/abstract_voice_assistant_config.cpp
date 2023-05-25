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

#include "abstract_voice_assistant_config.h"

supla_abstract_voice_assistant_config::supla_abstract_voice_assistant_config(
    void)
    : channel_json_config() {}

supla_abstract_voice_assistant_config::supla_abstract_voice_assistant_config(
    channel_json_config *root)
    : channel_json_config(root) {}

bool supla_abstract_voice_assistant_config::is_integration_disabled(void) {
  cJSON *root = get_user_root();
  if (!root) {
    return false;
  }

  root = cJSON_GetObjectItem(root, get_root_key());
  if (root && cJSON_IsObject(root)) {
    cJSON *value = cJSON_GetObjectItem(root, get_value_key());
    return value && cJSON_IsTrue(value);
  }

  return false;
}
