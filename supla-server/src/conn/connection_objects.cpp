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

#include "conn/connection_objects.h"

using std::function;
using std::shared_ptr;
using std::vector;

supla_connection_objects::supla_connection_objects() { lck = lck_init(); }

supla_connection_objects::~supla_connection_objects() { lck_free(lck); }

void supla_connection_objects::lock(void) { lck_lock(lck); }

void supla_connection_objects::unlock(void) { lck_unlock(lck); }

void supla_connection_objects::for_each(
    function<bool(shared_ptr<supla_connection_object> obj)> on_object) {
  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if (!on_object(*it)) {
      break;
    }
  }
  unlock();
}

std::vector<std::shared_ptr<supla_connection_object> >
supla_connection_objects::get_all(void) {
  lock();
  std::vector<std::shared_ptr<supla_connection_object> > result = objects;
  unlock();

  return result;
}

bool supla_connection_objects::exists(shared_ptr<supla_connection_object> obj) {
  bool result = false;
  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if (*it == obj) {
      result = true;
      break;
    }
  }
  unlock();

  return result;
}

bool supla_connection_objects::add(shared_ptr<supla_connection_object> obj) {
  bool result = false;
  lock();

  if (!exists(obj)) {
    char GUID[SUPLA_GUID_SIZE] = {};
    obj->get_guid(GUID);

    shared_ptr<supla_connection_object> _obj = find_by_guid(GUID);
    objects.push_back(obj);
    result = true;

    if (_obj != nullptr) {
      _obj->terminate();
    }
  }

  unlock();

  return result;
}

shared_ptr<supla_connection_object> supla_connection_objects::find_by_id(
    int id) {
  shared_ptr<supla_connection_object> result;
  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if ((*it)->get_id() == id) {
      result = *it;
      break;
    }
  }
  unlock();
  return result;
}

shared_ptr<supla_connection_object> supla_connection_objects::find_by_guid(
    const char guid[SUPLA_GUID_SIZE]) {
  shared_ptr<supla_connection_object> result;
  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if ((*it)->guid_equal(guid)) {
      result = *it;
      break;
    }
  }
  unlock();
  return result;
}

int supla_connection_objects::count(void) {
  int result = 0;
  lock();
  result = objects.size();
  unlock();
  return result;
}

bool supla_connection_objects::terminate_all(void) {
  lock();
  vector<shared_ptr<supla_connection_object> > copy = objects;
  unlock();

  bool result = false;

  for (auto it = copy.begin(); it != copy.end(); ++it) {
    (*it)->terminate();
    result = true;
  }

  return result;
}

bool supla_connection_objects::terminate(int id) {
  shared_ptr<supla_connection_object> obj = find_by_id(id);
  if (obj) {
    obj->terminate();
    return true;
  }

  return false;
}
