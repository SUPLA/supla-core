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
#include "../safearray.h"
#include "../database.h"
#include "clientobjcontainer.h"

// static
char supla_client_objcontainer::arr_delcnd(void *ptr) {
  delete (supla_client_location *)ptr;
  return 1;
}

// static
char supla_client_objcontainer::arr_findcmp(void *ptr, void *id) {
  return ((supla_client_channel *)ptr)->getId() == *((int *)id) ? 1 : 0;
}

void supla_client_objcontainer::arr_clean(void) {
  safe_array_lock(arr);
  safe_array_clean(arr, arr_delcnd);
  safe_array_unlock(arr);
}

supla_client_objcontainer_item *supla_client_objcontainer::find(int Id) {
  return (supla_client_channel *)safe_array_findcnd(arr, arr_findcmp, &Id);
}

supla_client_objcontainer::supla_client_objcontainer(supla_client *client) {
  this->client = client;
  this->arr = safe_array_init();
}

supla_client_objcontainer::~supla_client_objcontainer() {}

void *supla_client_objcontainer::getArr(void) { return arr; }

supla_client *supla_client_objcontainer::getClient() { return client; }

int supla_client_objcontainer::count(void) { return safe_array_count(arr); }

void supla_client_objcontainer::load(void) {
  database *db = new database();

  if (db->connect() == true) {
    safe_array_lock(getArr());
    arr_clean();

    _load(db);

    safe_array_unlock(getArr());
  }

  delete db;
}

void supla_client_objcontainer::update(supla_client_objcontainer_item *_obj) {
  safe_array_lock(getArr());

  supla_client_objcontainer_item *obj = NULL;

  if ((obj = find(_obj->getId())) == NULL) {
    obj = new_item(_obj);
    if (obj && safe_array_add(getArr(), obj) == -1) {
      delete obj;
      obj = NULL;
    }

  } else {
    obj->setCaption(_obj->getCaption());
    _update(obj, _obj);
  }

  if (obj) {
    obj->mark_for_remote_update(OI_REMOTEUPDATE_FULL);
  }

  safe_array_unlock(getArr());
}

supla_client_objcontainer_item *supla_client_objcontainer::get_marked(void) {
  supla_client_objcontainer_item *obj = NULL;

  for (int a = 0; a < safe_array_count(getArr()); a++) {
    obj = static_cast<supla_client_objcontainer_item *>(
        safe_array_get(getArr(), a));
    if (obj->marked_for_remote_update() != OI_REMOTEUPDATE_NONE) {
      break;
    } else {
      obj = NULL;
    }
  }

  return obj;
}

bool supla_client_objcontainer::do_remote_update(void *srpc, bool full) {
  void *data = NULL;
  supla_client_objcontainer_item *obj = NULL;

  safe_array_lock(getArr());

  char mark = full ? OI_REMOTEUPDATE_FULL : OI_REMOTEUPDATE_VALUE;
  bool check_more = false;
  bool result = false;

  for (int a = 0; a < safe_array_count(getArr()); a++) {
    obj = static_cast<supla_client_objcontainer_item *>(
        safe_array_get(getArr(), a));
    if (obj->marked_for_remote_update() == mark) {
      if (get_data_for_remote(obj, &data, full, get_marked() == NULL ? 1 : 0,
                              &check_more)) {
        obj->mark_for_remote_update(OI_REMOTEUPDATE_NONE);
        result = true;
      }

      if (!check_more) {
        break;
      }
    }
  }

  safe_array_unlock(getArr());

  if (data) {
    send_data_to_remote_and_free(srpc, data, full);
  }

  return result;
}

bool supla_client_objcontainer::remote_update(void *srpc) {
  if (do_remote_update(srpc, true)) {
    return true;
  }

  return do_remote_update(srpc, false);
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
