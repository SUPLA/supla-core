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

#include "scene/scene.h"

supla_scene::supla_scene(int id) : supla_dobject(id) {
  operations = NULL;
  location_id = 0;
}

supla_scene::~supla_scene(void) {
  if (operations) {
    delete operations;
    operations = NULL;
  }
}

void supla_scene::set_operations(supla_scene_operations *operations) {
  if (this->operations) {
    delete this->operations;
  }

  this->operations = operations;
}

const supla_scene_operations *supla_scene::get_operations(void) {
  return operations;
}

void supla_scene::set_location_id(int location_id) {
  this->location_id = location_id;
}

void supla_scene::set_caption(const char *caption) {
  this->caption = caption ? caption : "";
}

int supla_scene::get_location_id(void) { return location_id; }

const std::string supla_scene::get_caption(void) { return caption; }

void supla_scene::set_remote_update_indicator(
    supla_dobject_change_indicator *rui) {}
