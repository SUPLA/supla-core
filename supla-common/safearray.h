/*
 ============================================================================
 Name        : safearray.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */


#ifndef SAFEARRAY_H_
#define SAFEARRAY_H_

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
void* safe_array_get(void *arr, int idx);
void* safe_array_pop(void *arr);
void safe_array_free(void *arr);
void safe_array_clean(void *arr, _func_sa_cnd del_cnd);
void* safe_array_findcnd(void *arr, _func_sa_cnd_param find_cnd, void *user_param);

#ifdef __cplusplus
}
#endif

#endif /* SAFEARRAY_H_ */
