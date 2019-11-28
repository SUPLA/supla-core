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

#include "safearray.h"
#include <assert.h>
#include <stdlib.h>
#include "log.h"

typedef struct {
  void *lck;

  int count;
  void **arr;
} TSafeArray;

void *safe_array_init(void) {
  TSafeArray *sa = malloc(sizeof(TSafeArray));
  sa->lck = lck_init();
  sa->count = 0;
  sa->arr = NULL;

  return sa;
}

void safe_array_free(void *_arr) {
  assert(_arr != 0);

  lck_free(((TSafeArray *)_arr)->lck);
  free(((TSafeArray *)_arr)->arr);
  free(_arr);
}

#ifdef __LCK_DEBUG
void _safe_array_lock(void *_arr, const char *file, int line) {
  assert(_arr != 0);
  __lck_lock(((TSafeArray *)_arr)->lck, file, line);
}
#else
void safe_array_lock(void *_arr) {
  assert(_arr != 0);

  lck_lock(((TSafeArray *)_arr)->lck);
}
#endif /*__LCK_DEBUG*/

void safe_array_unlock(void *_arr) {
  assert(_arr != 0);

  lck_unlock(((TSafeArray *)_arr)->lck);
}

int safe_array_count(void *_arr) {
  int result;

  assert(_arr != 0);

  safe_array_lock(_arr);
  result = ((TSafeArray *)_arr)->count;
  safe_array_unlock(_arr);

  return result;
}

void safe_array_move_to_begin(void *_arr, int idx) {
  TSafeArray *arr = (TSafeArray *)_arr;

  assert(_arr != 0);

  if (idx < 1 || idx >= arr->count) return;

  void *item = arr->arr[idx];
  int a;
  for (a = idx - 1; a >= 0; a--) {
    arr->arr[a + 1] = arr->arr[a];
  }

  arr->arr[0] = item;
}

int safe_array_add(void *_arr, void *ptr) {
  int result = 0;
  TSafeArray *arr = (TSafeArray *)_arr;

  assert(_arr != 0);

  if (ptr == 0) return -1;

  safe_array_lock(_arr);

  void **new_arr = realloc(arr->arr, sizeof(void *) * (arr->count + 1));

  if (new_arr == NULL) {
    result = -1;

  } else {
    arr->arr = new_arr;
    arr->arr[arr->count] = ptr;
    result = arr->count;
    arr->count++;
  }

  safe_array_unlock(_arr);

  return result;
}

void safe_array_delete(void *_arr, int idx) {
  TSafeArray *arr = (TSafeArray *)_arr;

  assert(_arr != 0);

  if (idx < 0) return;

  safe_array_lock(_arr);

  if (idx < arr->count) {
    if (idx < arr->count - 1) arr->arr[idx] = arr->arr[arr->count - 1];

    void **new_arr = realloc(arr->arr, sizeof(void *) * (arr->count - 1));

    if (new_arr != NULL || arr->count == 1) {
      arr->arr = new_arr;
      arr->count--;
    }
  }

  safe_array_unlock(_arr);
}

void safe_array_remove(void *_arr, void *ptr) {
  assert(_arr != 0);

  safe_array_lock(_arr);
  safe_array_delete(_arr, safe_array_find(_arr, ptr));
  safe_array_unlock(_arr);
}

int safe_array_find(void *_arr, void *ptr) {
  int a;
  int result = -1;
  TSafeArray *arr = (TSafeArray *)_arr;

  assert(_arr != 0);

  if (ptr == 0) return -1;

  safe_array_lock(_arr);

  for (a = 0; a < arr->count; a++)
    if (arr->arr[a] == ptr) {
      result = a;
      break;
    }

  safe_array_unlock(_arr);

  return result;
}

void *safe_array_get(void *_arr, int idx) {
  void *result = NULL;
  TSafeArray *arr = (TSafeArray *)_arr;

  assert(_arr != 0);

  if (idx < 0) return 0;

  safe_array_lock(_arr);

  if (idx < arr->count) result = arr->arr[idx];

  safe_array_unlock(_arr);

  return result;
}

void *safe_array_pop(void *_arr) {
  int a;
  void *result = NULL;
  TSafeArray *arr = (TSafeArray *)_arr;

  safe_array_lock(_arr);

  result = safe_array_get(_arr, 0);

  if (result != NULL) {
    for (a = 1; a < arr->count; a++) arr->arr[a - 1] = arr->arr[a];
  }

  safe_array_delete(_arr, arr->count - 1);

  safe_array_unlock(_arr);

  return result;
}

void safe_array_clean(void *_arr, _func_sa_cnd del_cnd) {
  void *ptr;
  int a = 0;

  assert(_arr != 0);

  safe_array_lock(_arr);

  while ((ptr = safe_array_get(_arr, a)) != 0) {
    if (del_cnd(ptr) == 1) {
      safe_array_delete(_arr, a);
    } else {
      a++;
    }
  }
  safe_array_unlock(_arr);
}

void *safe_array_findcnd(void *_arr, _func_sa_cnd_param find_cnd,
                         void *user_param) {
  void *result = NULL;
  void *ptr = NULL;
  int a = 0;

  safe_array_lock(_arr);

  while ((ptr = safe_array_get(_arr, a)) != 0) {
    if (find_cnd(ptr, user_param) == 1) {
      result = ptr;
      break;
    } else {
      a++;
    }
  }

  safe_array_unlock(_arr);

  return result;
}
