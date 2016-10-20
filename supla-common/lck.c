/*
 ============================================================================
 Name        : lck.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : 2015-2016 GPLv2
 ============================================================================
 */

#include "lck.h"

#if defined(__AVR__) || defined(ARDUINO_ARCH_ESP8266)
#define __SINGLE_THREAD
#else

	#ifdef _WIN32
		#include <Windows.h>
	#else
		#include <pthread.h>
		#include <time.h>
	#endif

#endif

#include <stdlib.h>



#define MUTEX_COUNT  4

#ifndef __SINGLE_THREAD

typedef struct {
	#ifdef _WIN32
	CRITICAL_SECTION critSec;
	#else
	pthread_mutex_t mutex;
	#endif
}TLckData;
#endif

void *lck_init(void) {

#ifdef __SINGLE_THREAD
	return NULL;
#else
	TLckData *lck = malloc(sizeof(TLckData));

	if ( lck != NULL ) {

		#ifdef _WIN32
		InitializeCriticalSectionEx(&lck->critSec, 4000, CRITICAL_SECTION_NO_DEBUG_INFO);
		#else
		
		pthread_mutexattr_t    attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&lck->mutex, &attr);

		#endif
	}


	return lck;
#endif
}

void lck_lock(void *lck) {

#ifndef __SINGLE_THREAD
	if (lck != NULL) {
		#ifdef _WIN32
			EnterCriticalSection(&((TLckData*)lck)->critSec);
		#else
			pthread_mutex_lock(&((TLckData*)lck)->mutex);
		#endif
	}
    	
#endif
}


void lck_unlock(void *lck) {
#ifndef __SINGLE_THREAD
	if (lck != NULL) {
		#ifdef _WIN32
			LeaveCriticalSection(&((TLckData*)lck)->critSec);
		#else
			pthread_mutex_unlock(&((TLckData*)lck)->mutex);
		#endif
	}
    	
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
		#ifdef _WIN32
			DeleteCriticalSection(&((TLckData*)lck)->critSec);
		#else
    		pthread_mutex_destroy(&((TLckData*)lck)->mutex);
		#endif
    	free(lck);
    }
#endif
}
