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

#include "distributedobjects/dobjects.h"

#include "assert.h"
#include "lck.h"

supla_dobjects::supla_dobjects(supla_abstract_dobject_remote_updater *updater) {
  this->lck = lck_init();
  this->updater = updater;
}

supla_dobjects::~supla_dobjects() {
  clear();
  lck_free(lck);
}

void supla_dobjects::lock(void) { lck_lock(lck); }

void supla_dobjects::unlock(void) { lck_unlock(lck); }

void supla_dobjects::clear(void) {
  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    delete *it;
  }
  objects.clear();
  unlock();
}

void supla_dobjects::remove(int id) {
  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if ((*it)->get_id() == id) {
      delete *it;
      objects.erase(it);
      break;
    }
  }
  unlock();
}

void supla_dobjects::add(supla_dobject *object) {
  if (!object) {
    return;
  }

  lock();

  if (object) {
    objects.push_back(object);
  }

  unlock();
}

void supla_dobjects::access_object(
    int id, std::function<void(supla_dobject *object)> on_access) {
  if (!id || !on_access) {
    return;
  }

  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if ((*it)->get_id() == id) {
      on_access(*it);
      break;
    }
  }
  unlock();
}

int supla_dobjects::count(void) {
  lock();
  int result = objects.size();
  unlock();

  return result;
}

bool supla_dobjects::update_remote(void) {
  if (!updater) {
    return false;
  }

  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    updater->update(*it);

    if (updater->is_transaction_should_end()) {
      break;
    }
  }
  unlock();

  return updater->end_transaction();
}
