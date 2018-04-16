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

#include "objcontainer.h"
#include <string.h>
#include "database.h"
#include "objcontaineritem.h"
#include "safearray.h"

// static
char supla_objcontainer::arr_delcnd(void *ptr) {
  delete static_cast<supla_objcontainer_item *>(ptr);
  return 1;
}

// static
char supla_objcontainer::arr_findcmp(void *ptr, void *_f) {
  supla_objcontainer_item *item = static_cast<supla_objcontainer_item *>(ptr);
  _t_objc_search_fields *f = static_cast<_t_objc_search_fields *>(_f);

  return (f->id == item->getId() &&
          (!f->use_both || (f->use_both && f->extra_id == item->getExtraId())))
             ? 1
             : 0;
}

void supla_objcontainer::arr_clean(void *arr) {
  safe_array_lock(arr);
  safe_array_clean(arr, arr_delcnd);
  safe_array_unlock(arr);
}

supla_objcontainer_item *supla_objcontainer::find(_t_objc_search_fields *f,
                                                  e_objc_scope scope) {
  return static_cast<supla_objcontainer_item *>(
      safe_array_findcnd(getArr(scope), arr_findcmp, f));
}

supla_objcontainer_item *supla_objcontainer::find(int id, e_objc_scope scope) {
  _t_objc_search_fields f;
  f.id = id;
  f.extra_id = 0;
  f.use_both = false;
  return supla_objcontainer::find(&f, scope);
}

supla_objcontainer::supla_objcontainer(void) {
  for (int a = 0; a < OBJC_SCOPE_COUNT; a++) {
    this->arr[a] = safe_array_init();
    this->id_cmp_use_both[a] = false;
  }
}

supla_objcontainer::~supla_objcontainer() {
  for (int a = 0; a < OBJC_SCOPE_COUNT; a++) {
    arr_clean(arr[a]);
    safe_array_free(arr[a]);
    arr[a] = NULL;
  }
}

void *supla_objcontainer::getArr(e_objc_scope scope) { return arr[scope]; }

void *supla_objcontainer::getArr(void) { return getArr(master); }

int supla_objcontainer::count(e_objc_scope scope) {
  return safe_array_count(getArr(scope));
}

int supla_objcontainer::count(void) { return count(master); }

void supla_objcontainer::load(e_objc_scope scope) {
  database *db = new database();

  if (db->connect() == true) {
    safe_array_lock(getArr(scope));
    arr_clean(getArr(scope));

    _load(db, scope);

    safe_array_unlock(getArr(scope));
  }

  delete db;
}

void supla_objcontainer::load(void) { load(master); }

bool supla_objcontainer::add(supla_objcontainer_item *obj, e_objc_scope scope) {
  bool result = false;
  if (obj == NULL) {
    return false;
  }

  safe_array_lock(getArr(scope));

  _t_objc_search_fields f;

  f.id = obj->getId();
  f.extra_id = obj->getExtraId();
  f.use_both = id_cmp_use_both[scope];

  if (find(&f, scope) == NULL && safe_array_add(getArr(scope), obj) != -1) {
    result = true;

    if (obj) {
      obj->mark_for_remote_update(OI_REMOTEUPDATE_FULL);
    }
  }
  safe_array_unlock(getArr(scope));

  return result;
}

bool supla_objcontainer::add(supla_objcontainer_item *obj) {
  return add(obj, master);
}
