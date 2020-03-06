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

#include "cdcontainer.h"

cdcontainer::cdcontainer() {
  arr = safe_array_init();
  trash_arr = safe_array_init();
}

cdcontainer::~cdcontainer() {
  safe_array_free(arr);
  safe_array_free(trash_arr);
}

bool cdcontainer::exists(cdbase *cd) { return safe_array_find(arr, cd) > -1; }

cdbase *cdcontainer::find(_func_sa_cnd_param find_cnd, void *user_param) {
  cdbase *result = NULL;

  safe_array_lock(arr);
  result = static_cast<cdbase *>(safe_array_findcnd(arr, find_cnd, user_param));
  if (result != NULL) {
    result = result->retainPtr();
  }
  safe_array_unlock(arr);

  return result;
}

cdbase *cdcontainer::get(int idx) {
  cdbase *result = NULL;

  safe_array_lock(arr);
  result = static_cast<cdbase *>(safe_array_get(arr, idx));
  if (result != NULL) {
    result = result->retainPtr();
  }
  safe_array_unlock(arr);

  return result;
}

void cdcontainer::releasePtr(cdbase *cd) {
  if (cd != NULL) {
    cd->releasePtr();
  }
}

void cdcontainer::addToList(cdbase *cd) {
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

void cdcontainer::moveToTrash(cdbase *cd) {
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

bool cdcontainer::emptyTrash(void) {
  safe_array_lock(trash_arr);

  for (int a = 0; a < safe_array_count(trash_arr); a++) {
    cdbase *cd = static_cast<cdbase *>(safe_array_get(trash_arr, a));
    if (cd && !cd->ptrIsUsed()) {
      cd_delete(cd);
      safe_array_delete(trash_arr, a);
      a--;
    }
  }

  safe_array_unlock(trash_arr);

  return safe_array_count(trash_arr) == 0;
}

bool cdcontainer::emptyTrash(unsigned char timeout_sec) {
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

void cdcontainer::moveAllToTrash() {
  safe_array_lock(arr);
  while (safe_array_count(arr)) {
    moveToTrash(static_cast<cdbase *>(safe_array_get(arr, 0)));
  }
  safe_array_unlock(arr);
}

bool cdcontainer::deleteAll(unsigned char timeout_sec) {
  moveAllToTrash();
  return emptyTrash(timeout_sec);
}

int cdcontainer::trashCount(void) { return safe_array_count(trash_arr); }

int cdcontainer::count(void) { return safe_array_count(arr); }
