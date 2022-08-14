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

supla_connection_objects::supla_connection_objects() {
  arr = safe_array_init();
  trash_arr = safe_array_init();
}

supla_connection_objects::~supla_connection_objects() {
  safe_array_free(arr);
  safe_array_free(trash_arr);
}

bool supla_connection_objects::exists(supla_connection_object *cd) {
  return safe_array_find(arr, cd) > -1;
}

supla_connection_object *supla_connection_objects::find(
    _func_sa_cnd_param find_cnd, void *user_param) {
  supla_connection_object *result = NULL;

  safe_array_lock(arr);
  result = static_cast<supla_connection_object *>(
      safe_array_findcnd(arr, find_cnd, user_param));
  if (result != NULL) {
    result = result->retain_ptr();
  }
  safe_array_unlock(arr);

  return result;
}

supla_connection_object *supla_connection_objects::get(int idx) {
  supla_connection_object *result = NULL;

  safe_array_lock(arr);
  result = static_cast<supla_connection_object *>(safe_array_get(arr, idx));
  if (result != NULL) {
    result = result->retain_ptr();
  }
  safe_array_unlock(arr);

  return result;
}

void supla_connection_objects::releasePtr(supla_connection_object *cd) {
  if (cd != NULL) {
    cd->release_ptr();
  }
}

void supla_connection_objects::addToList(supla_connection_object *cd) {
  if (cd) {
    safe_array_lock(trash_arr);
    safe_array_lock(arr);

    safe_array_remove(trash_arr, cd);

    if (safe_array_find(arr, cd) == -1) {
      safe_array_add(arr, cd);
    }

    safe_array_unlock(arr);
    safe_array_unlock(trash_arr);
  }
}

void supla_connection_objects::moveToTrash(supla_connection_object *cd) {
  if (cd) {
    safe_array_lock(trash_arr);
    safe_array_lock(arr);

    safe_array_add(trash_arr, cd);
    safe_array_remove(arr, cd);

    safe_array_unlock(arr);
    safe_array_unlock(trash_arr);
  }

  emptyTrash();
}

bool supla_connection_objects::emptyTrash(void) {
  safe_array_lock(trash_arr);

  for (int a = 0; a < safe_array_count(trash_arr); a++) {
    supla_connection_object *cd =
        static_cast<supla_connection_object *>(safe_array_get(trash_arr, a));
    if (cd && !cd->ptr_is_used()) {
      cd_delete(cd);
      safe_array_delete(trash_arr, a);
      a--;
    }
  }

  safe_array_unlock(trash_arr);

  return safe_array_count(trash_arr) == 0;
}

bool supla_connection_objects::emptyTrash(unsigned char timeout_sec) {
  struct timeval now, start;
  gettimeofday(&now, NULL);
  start = now;

  while (!emptyTrash()) {
    if (now.tv_sec - start.tv_sec >= timeout_sec) {
      break;
    }
    gettimeofday(&now, NULL);
  }

  return safe_array_count(trash_arr) == 0;
}

void supla_connection_objects::moveAllToTrash() {
  safe_array_lock(arr);
  while (safe_array_count(arr)) {
    moveToTrash(static_cast<supla_connection_object *>(safe_array_get(arr, 0)));
  }
  safe_array_unlock(arr);
}

bool supla_connection_objects::deleteAll(unsigned char timeout_sec) {
  moveAllToTrash();
  return emptyTrash(timeout_sec);
}

int supla_connection_objects::trashCount(void) {
  return safe_array_count(trash_arr);
}

int supla_connection_objects::count(void) { return safe_array_count(arr); }
