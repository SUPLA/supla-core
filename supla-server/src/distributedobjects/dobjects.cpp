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

#include <distributedobjects/dobjects.h>

#include "lck.h"

supla_dobjects::supla_dobjects() { lck = lck_init(); }

supla_dobjects::~supla_dobjects() { lck_free(lck); }

void supla_dobjects::load(void) {}

void supla_dobjects::load(int id) {}

void supla_dobjects::access_object(
    int id, std::function<void(supla_dobject *object)> on_access) {
  if (!id || !on_access) {
    return;
  }

  lck_lock(lck);
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if ((*it)->get_id() == id) {
      on_access(*it);
      break;
    }
  }
  lck_unlock(lck);
}
