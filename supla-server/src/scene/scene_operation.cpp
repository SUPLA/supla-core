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

#include <scene/scene_operation.h>

supla_scene_operation::supla_scene_operation(void) { this->delay_ms = 0; }

supla_scene_operation::~supla_scene_operation() {}

void supla_scene_operation::set_action_config(
    const supla_action_config &action_config) {
  this->action_config = action_config;
}

supla_action_config *supla_scene_operation::get_action_config(void) {
  return &action_config;
}

unsigned int supla_scene_operation::get_delay_ms(void) { return delay_ms; }

void supla_scene_operation::set_delay_ms(unsigned int delay_ms) {
  this->delay_ms = delay_ms;
}

supla_scene_operation *supla_scene_operation::clone(void) {
  supla_scene_operation *result = new supla_scene_operation();
  if (result) {
    *result = *this;
  }

  return result;
}
