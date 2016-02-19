/*
 ============================================================================
 Name        : lck.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.2
 Copyright   : 2015-2016 GPLv2
 ============================================================================
 */

#include "lck.h"

#ifdef __AVR__
#define __SINGLE_THREAD
#else
#include <pthread.h>
#include <time.h>
#endif

#include <stdlib.h>



#define MUTEX_COUNT  4

#ifndef __SINGLE_THREAD

typedef struct {

	pthread_mutex_t mutex;

}TLckData;
#endif

void *lck_init(void) {

#ifdef __SINGLE_THREAD
	return NULL;
#else
	TLckData *lck = malloc(sizeof(TLckData));

	if ( lck != NULL ) {

	    pthread_mutexattr_t    attr;
	    pthread_mutexattr_init(&attr);
	    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	    pthread_mutex_init(&lck->mutex, &attr);

	}


	return lck;
#endif
}

void lck_lock(void *lck) {

#ifndef __SINGLE_THREAD
    if ( lck != NULL )
    	pthread_mutex_lock(&((TLckData*)lck)->mutex);
#endif
}


void lck_unlock(void *lck) {
#ifndef __SINGLE_THREAD
    if ( lck != NULL )
    	pthread_mutex_unlock(&((TLckData*)lck)->mutex);
#endif
}

int lck_unlock_r(void *lck, int result) {
#ifndef __SINGLE_THREAD
	lck_unlock(lck);
#endif
	return result;
}

void lck_free(void *lck) {
#ifndef __SINGLE_THREAD
    if ( lck != NULL ) {
    	pthread_mutex_destroy(&((TLckData*)lck)->mutex);
    	free(lck);
    }
#endif
}
