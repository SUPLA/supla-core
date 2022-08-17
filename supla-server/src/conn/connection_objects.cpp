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
using std::weak_ptr;

supla_connection_objects::supla_connection_objects() { lck = lck_init(); }

supla_connection_objects::~supla_connection_objects() { lck_free(lck); }

void supla_connection_objects::lock(void) { lck_lock(lck); }

void supla_connection_objects::unlock(void) { lck_unlock(lck); }

void supla_connection_objects::for_each(
    function<bool(shared_ptr<supla_connection_object> obj)> on_object) {
  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    shared_ptr<supla_connection_object> _obj = (*it).lock();

    if (_obj != nullptr && !on_object(_obj)) {
      break;
    }
  }
  unlock();
}

vector<shared_ptr<supla_connection_object> > supla_connection_objects::get_all(
    void) {
  lock();
  vector<shared_ptr<supla_connection_object> > result;

  for (auto it = objects.begin(); it != objects.end(); ++it) {
    shared_ptr<supla_connection_object> _obj = (*it).lock();
    if (_obj != nullptr) {
      result.push_back(_obj);
    }
  }

  unlock();

  return result;
}

bool supla_connection_objects::add(shared_ptr<supla_connection_object> obj) {
  bool result = false;
  lock();

  char guid[SUPLA_GUID_SIZE] = {};
  obj->get_guid(guid);

  bool ptr_exists = false;
  shared_ptr<supla_connection_object> obj_with_same_guid = nullptr;

  for (auto it = objects.begin(); it != objects.end(); ++it) {
    shared_ptr<supla_connection_object> _obj = (*it).lock();
    if (_obj == nullptr) {
      // cleanup
      it = objects.erase(it);
      --it;
    } else if (_obj == obj) {
      ptr_exists = true;
    } else if (_obj->guid_equal(guid)) {
      obj_with_same_guid = _obj;
      // remove from list (will be terminated)
      it = objects.erase(it);
      --it;
    }
  }

  if (!ptr_exists) {
    objects.push_back(obj);
    result = true;

    if (obj_with_same_guid != nullptr) {
      // Terminate only after adding "obj" to the list
      obj_with_same_guid->terminate();
    }
  }

  unlock();

  return result;
}

shared_ptr<supla_connection_object> supla_connection_objects::find_by_id(
    int id) {
  if (!id) {
    return nullptr;
  }

  shared_ptr<supla_connection_object> result;
  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    shared_ptr<supla_connection_object> _obj = (*it).lock();

    if (_obj != nullptr && _obj->get_id() == id) {
      result = _obj;
      break;
    }
  }
  unlock();
  return result;
}

int supla_connection_objects::count(void) {
  int result = 0;
  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if (!(*it).expired()) {
      result++;
    }
  }
  unlock();
  return result;
}

bool supla_connection_objects::terminate_all(void) {
  vector<shared_ptr<supla_connection_object> > objects = get_all();

  bool result = false;

  for (auto it = objects.begin(); it != objects.end(); ++it) {
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

bool supla_connection_objects::is_online(int id) {
  return find_by_id(id) != nullptr;
}
