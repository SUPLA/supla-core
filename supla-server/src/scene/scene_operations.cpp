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

#include <scene/scene_operations.h>

supla_scene_operations::supla_scene_operations() {}

supla_scene_operations::~supla_scene_operations() {
  while (operations.size()) {
    delete operations.back();
    operations.pop_back();
  }
}

int supla_scene_operations::count(void) { return operations.size(); }

void supla_scene_operations::push(supla_scene_operation *operation) {
  operations.push_back(operation);
}

supla_scene_operation *supla_scene_operations::pop(void) {
  supla_scene_operation *result = NULL;
  if (operations.size()) {
    result = operations.front();
    operations.pop_front();
  }

  return result;
}

int supla_scene_operations::get_delay_ms(void) {
  if (operations.size()) {
    return operations.front()->get_delay_ms();
  }

  return 0;
}

supla_scene_operations *supla_scene_operations::clone(void) const {
  supla_scene_operations *result = new supla_scene_operations();
  if (result) {
    for (auto it = operations.begin(); it != operations.end(); ++it) {
      result->push((*it)->clone());
    }
  }

  return result;
}
