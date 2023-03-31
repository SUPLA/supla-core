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

#ifndef SAFEARRAY_H_
#define SAFEARRAY_H_

#include "lck.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef char (*_func_sa_cnd)(void *ptr);
typedef char (*_func_sa_cnd_param)(void *ptr, void *user_param);

void *safe_array_init(void);
void safe_array_lock(void *arr);
void safe_array_unlock(void *arr);
int safe_array_count(void *arr);
int safe_array_add(void *arr, void *ptr);
void safe_array_delete(void *arr, int idx);
void safe_array_remove(void *arr, void *ptr);
int safe_array_find(void *arr, void *ptr);
void *safe_array_get(void *arr, int idx);
void *safe_array_pop(void *arr);
void safe_array_free(void *arr);
void safe_array_clean(void *arr, _func_sa_cnd del_cnd);
void *safe_array_findcnd(void *arr, _func_sa_cnd_param find_cnd,
                         void *user_param);
void safe_array_move_to_begin(void *_arr, int idx);

#ifdef __cplusplus
}
#endif

#endif /* SAFEARRAY_H_ */
