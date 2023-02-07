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
    function<void(shared_ptr<supla_abstract_connection_object> obj,
                  bool *will_continue)>
        on_object) {
  vector<weak_ptr<supla_abstract_connection_object>> objects = get_all();
  bool will_continue = true;
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    shared_ptr<supla_abstract_connection_object> _obj = (*it).lock();

    if (_obj != nullptr) {
      on_object(_obj, &will_continue);
      if (!will_continue) {
        break;
      }
    }
  }
}

vector<weak_ptr<supla_abstract_connection_object>>
supla_connection_objects::get_all(void) {
  lock();
  vector<weak_ptr<supla_abstract_connection_object>> result = objects;
  unlock();

  return result;
}

bool supla_connection_objects::add(
    shared_ptr<supla_abstract_connection_object> obj) {
  bool result = false;
  lock();

  char guid[SUPLA_GUID_SIZE] = {};
  obj->get_guid(guid);
  int id = obj->get_id();

  bool ptr_exists = false;
  shared_ptr<supla_abstract_connection_object> previous = nullptr;
  vector<shared_ptr<supla_abstract_connection_object>> dont_release;

  dont_release.reserve(objects.size());

  for (auto it = objects.begin(); it != objects.end(); ++it) {
    shared_ptr<supla_abstract_connection_object> _obj = (*it).lock();
    if (_obj == nullptr) {
      // cleanup
      it = objects.erase(it);
      --it;
    } else if (_obj == obj) {
      ptr_exists = true;
    } else if (_obj->get_id() == id || _obj->guid_equal(guid)) {
      previous = _obj;
      // remove from list (will be terminated)
      it = objects.erase(it);
      --it;
    } else {
      // We don't want the object to accidentally get freed between lock() and
      // unlock()
      dont_release.push_back(_obj);
    }
  }

  if (!ptr_exists) {
    objects.push_back(obj);
    result = true;

    if (previous != nullptr) {
      // Terminate only after adding "obj" to the list
      previous->terminate();
    }
  }

  unlock();

  dont_release.clear();

  return result;
}

shared_ptr<supla_abstract_connection_object> supla_connection_objects::get(
    int id) {
  if (!id) {
    return nullptr;
  }

  shared_ptr<supla_abstract_connection_object> result;
  vector<weak_ptr<supla_abstract_connection_object>> objects =
      get_all();  // We don't want to use lock() / unlock() so that if an object
                  // is freed it won't happen during the lock. That's why we
                  // make a copy of the list.

  for (auto it = objects.begin(); it != objects.end(); ++it) {
    shared_ptr<supla_abstract_connection_object> _obj = (*it).lock();

    if (_obj != nullptr && _obj->get_id() == id) {
      result = _obj;
      break;
    }
  }

  return result;
}

int supla_connection_objects::count(void) {
  int result = 0;

  vector<weak_ptr<supla_abstract_connection_object>> objects =
      get_all();  // We don't want to use lock() / unlock() so that if an object
                  // is freed it won't happen during the lock. That's why we
                  // make a copy of the list.

  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if (!(*it).expired()) {
      result++;
    }
  }

  return result;
}

bool supla_connection_objects::reconnect_all(void) {
  bool result = false;

  for_each([&result](shared_ptr<supla_abstract_connection_object> object,
                     bool *will_continue) -> void {
    if (object->reconnect()) {
      result = true;
    }
  });

  return result;
}

bool supla_connection_objects::terminate(int id) {
  shared_ptr<supla_abstract_connection_object> obj = get(id);
  if (obj) {
    obj->terminate();
    return true;
  }

  return false;
}

bool supla_connection_objects::is_online(int id) { return get(id) != nullptr; }
