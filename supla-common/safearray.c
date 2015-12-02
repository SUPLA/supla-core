/*
 ============================================================================
 Name        : safearray.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include <stdlib.h>
#include <assert.h>
#include "safearray.h"
#include "lck.h"
#include "log.h"

typedef struct {

	void *lck;

	int count;
	void **arr;

}TSafeArray;

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


void safe_array_lock(void *_arr) {

	assert(_arr != 0);

	lck_lock(((TSafeArray *)_arr)->lck);
}

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

int safe_array_add(void *_arr, void *ptr) {

	int result = 0;
	TSafeArray *arr = (TSafeArray *)_arr;

	assert(_arr != 0);

	if ( ptr == 0 )
		return -1;

	safe_array_lock(_arr);
	arr->arr = realloc(arr->arr, sizeof(void *) * (arr->count+1));
	arr->arr[arr->count] = ptr;
	result = arr->count;
	arr->count++;

    safe_array_unlock(_arr);

    return result;
}

void safe_array_delete(void *_arr, int idx) {

	TSafeArray *arr = (TSafeArray *)_arr;

	assert(_arr != 0);

	if ( idx < 0 )
		return;

	safe_array_lock(_arr);

	if ( idx < arr->count ) {

		if ( idx < arr->count-1 )
			arr->arr[idx] = arr->arr[arr->count-1];

		arr->arr = realloc(arr->arr, sizeof(void *) * (arr->count-1));
		arr->count--;
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

	if ( ptr == 0 )
		return -1;

	safe_array_lock(_arr);

	for(a=0;a<arr->count;a++)
		if ( arr->arr[a] == ptr ) {
			result = a;
			break;
		}

	safe_array_unlock(_arr);

	return result;
}

void* safe_array_get(void *_arr, int idx) {

	void *result = NULL;
	TSafeArray *arr = (TSafeArray *)_arr;

	assert(_arr != 0);

	if ( idx < 0 )
		return 0;

	safe_array_lock(_arr);

	if ( idx < arr->count )
		result = arr->arr[idx];

	safe_array_unlock(_arr);

	return result;
}

void* safe_array_pop(void *_arr) {

	int a;
	void *result = NULL;
	TSafeArray *arr = (TSafeArray *)_arr;

	safe_array_lock(_arr);

	result = safe_array_get(_arr, 0);

	if ( result != NULL ) {
		for(a=1;a<arr->count;a++)
			arr->arr[a-1] = arr->arr[a];
	}

	safe_array_delete(_arr, arr->count-1);

	safe_array_unlock(_arr);

	return result;
}

void safe_array_clean(void *_arr, _func_sa_cnd del_cnd) {

	void *ptr;
	int a = 0;

	assert(_arr != 0);

	safe_array_lock(_arr);

	while( (ptr = safe_array_get(_arr, a)) != 0 ) {

		if ( del_cnd(ptr) == 1 ) {
			safe_array_delete(_arr, a);
		} else {
			a++;
		}

	}
	safe_array_unlock(_arr);
}

void* safe_array_findcnd(void *_arr, _func_sa_cnd_param find_cnd, void *user_param) {

	void *result = NULL;
	void *ptr = NULL;
	int a = 0;

	safe_array_lock(_arr);

	while( (ptr = safe_array_get(_arr, a)) != 0 ) {

		if ( find_cnd(ptr, user_param) == 1 ) {
			result = ptr;
			break;
		} else {
			a++;
		}

	}

	safe_array_unlock(_arr);

	return result;
}
