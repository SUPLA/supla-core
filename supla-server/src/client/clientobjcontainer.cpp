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

#include "client.h"
#include "../database.h"
#include "../safearray.h"
#include "clientobjcontainer.h"

// static
char supla_client_objcontainer::arr_delcnd(void *ptr) {
  delete static_cast<supla_client_objcontainer_item *>(ptr);
  return 1;
}

// static
char supla_client_objcontainer::arr_findcmp(void *ptr, void *id) {
  return static_cast<supla_client_objcontainer_item *>(ptr)->getId() ==
                 *((int *)id)
             ? 1
             : 0;
}

void supla_client_objcontainer::arr_clean(void *arr) {
  safe_array_lock(arr);
  safe_array_clean(arr, arr_delcnd);
  safe_array_unlock(arr);
}

supla_client_objcontainer_item *supla_client_objcontainer::find(
    int Id, e_objc_scope scope) {
  return (supla_client_channel *)safe_array_findcnd(getArr(scope), arr_findcmp,
                                                    &Id);
}

supla_client_objcontainer::supla_client_objcontainer(supla_client *client) {
  this->client = client;
  for (int a = 0; a < OBJC_SCOPE_COUNT; a++) {
    this->arr[a] = safe_array_init();
  }
}

supla_client_objcontainer::~supla_client_objcontainer() {
  for (int a = 0; a < OBJC_SCOPE_COUNT; a++) {
    arr_clean(arr[a]);
    safe_array_free(arr[a]);
    arr[a] = NULL;
  }
}

void *supla_client_objcontainer::getArr(e_objc_scope scope) {
  return arr[scope];
}

void *supla_client_objcontainer::getArr(void) { return getArr(master); }

supla_client *supla_client_objcontainer::getClient() { return client; }

int supla_client_objcontainer::count(e_objc_scope scope) {
  return safe_array_count(getArr(scope));
}

int supla_client_objcontainer::count(void) { return count(master); }

void supla_client_objcontainer::load(e_objc_scope scope) {
  database *db = new database();

  if (db->connect() == true) {
    safe_array_lock(getArr(scope));
    arr_clean(getArr(scope));

    _load(db, scope);

    safe_array_unlock(getArr(scope));
  }

  delete db;
}

void supla_client_objcontainer::load(void) { load(master); }

void supla_client_objcontainer::update(supla_client_objcontainer_item *_obj,
                                       e_objc_scope scope) {
  safe_array_lock(getArr(scope));

  supla_client_objcontainer_item *obj = NULL;

  if ((obj = find(_obj->getId(), scope)) == NULL) {
    obj = new_item(_obj, scope);
    if (obj && safe_array_add(getArr(scope), obj) == -1) {
      delete obj;
      obj = NULL;
    }

  } else {
    obj->setCaption(_obj->getCaption());
    _update(obj, _obj, scope);
  }

  if (obj) {
    obj->mark_for_remote_update(OI_REMOTEUPDATE_FULL);
  }

  safe_array_unlock(getArr(scope));
}

void supla_client_objcontainer::update(supla_client_objcontainer_item *_obj) {
  update(_obj, master);
}

supla_client_objcontainer_item *supla_client_objcontainer::get_marked(
    e_objc_scope scope) {
  supla_client_objcontainer_item *obj = NULL;

  for (int a = 0; a < safe_array_count(getArr(scope)); a++) {
    obj = static_cast<supla_client_objcontainer_item *>(
        safe_array_get(getArr(scope), a));
    if (obj->marked_for_remote_update() != OI_REMOTEUPDATE_NONE) {
      break;
    } else {
      obj = NULL;
    }
  }

  return obj;
}

bool supla_client_objcontainer::do_remote_update(void *srpc, bool full,
                                                 e_objc_scope scope) {
  void *data = NULL;
  supla_client_objcontainer_item *obj = NULL;

  safe_array_lock(getArr(scope));

  char mark = full ? OI_REMOTEUPDATE_FULL : OI_REMOTEUPDATE_VALUE;
  bool check_more = false;
  bool result = false;

  for (int a = 0; a < safe_array_count(getArr(scope)); a++) {
    obj = static_cast<supla_client_objcontainer_item *>(
        safe_array_get(getArr(scope), a));
    if (obj->marked_for_remote_update() == mark) {
      if (get_data_for_remote(obj, &data, full,
                              get_marked(scope) == NULL ? 1 : 0, &check_more)) {
        obj->mark_for_remote_update(OI_REMOTEUPDATE_NONE);
        result = true;
      }

      if (!check_more) {
        break;
      }
    }
  }

  safe_array_unlock(getArr(scope));

  if (data) {
    send_data_to_remote_and_free(srpc, data, full);
  }

  return result;
}

bool supla_client_objcontainer::remote_update(void *srpc) {
  for (int a = 0; a < OBJC_SCOPE_COUNT; a++) {
    e_objc_scope scope = static_cast<e_objc_scope>(a);

    if (count(scope) > 0) {
      if (do_remote_update(srpc, true, scope)) {
        return true;
      }

      if (do_remote_update(srpc, false, scope)) {
        return true;
      }
    }
  }

  return false;
}

void supla_client_objcontainer::on_value_changed(void *srpc, int objId,
                                                 int extraId) {
  supla_client_objcontainer_item *obj;
  bool r = false;

  safe_array_lock(getArr());

  for (int a = 0; a < safe_array_count(getArr()); a++) {
    obj = static_cast<supla_client_objcontainer_item *>(
        safe_array_get(getArr(), a));
    if (obj && obj->getExtraId() == extraId &&
        (objId == 0 || obj->getId() == objId)) {
      obj->mark_for_remote_update(OI_REMOTEUPDATE_VALUE);
      r = true;
    }
  }

  safe_array_unlock(getArr());

  if (srpc && r) {
    remote_update(srpc);
  }
}

void supla_client_objcontainer::on_value_changed(void *srpc, int objId) {
  on_value_changed(srpc, objId, 0);
}
