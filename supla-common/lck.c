// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "lck.h"

#if defined(__AVR__) || defined(ARDUINO_ARCH_ESP8266) || \
    defined(ARDUINO_ARCH_ESP32) || defined(SUPLA_DEVICE)
#include <stdlib.h>
#define __SINGLE_THREAD
#else

#ifdef _WIN32
#include <Windows.h>
#else
#define _XOPEN_SOURCE 700
#include <pthread.h>
#include <time.h>
#endif /*_WIN32*/

#include <stdlib.h>

#endif  // defined(__AVR__) || defined(ARDUINO_ARCH_ESP8266)
        // || defined(ARDUINO_ARCH_ESP32)


#ifndef __SINGLE_THREAD

typedef struct {
#ifdef _WIN32
  CRITICAL_SECTION critSec;
#else
  pthread_mutex_t mutex;
#endif /*_WIN32*/
} TLckData;
#endif

void *LCK_ICACHE_FLASH lck_init(void) {
#ifdef __SINGLE_THREAD
  return NULL;
#else
  TLckData *lck = malloc(sizeof(TLckData));

  if (lck != NULL) {
#ifdef _WIN32
    InitializeCriticalSectionEx(&lck->critSec, 4000,
                                CRITICAL_SECTION_NO_DEBUG_INFO);
#else

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&lck->mutex, &attr);

#endif /*_WIN32*/
  }

  return lck;
#endif /*__SINGLE_THREAD*/
}

void lck_lock(void *lck) {
  (void)(lck);
#ifndef __SINGLE_THREAD
  if (lck != NULL) {
#ifdef _WIN32
    EnterCriticalSection(&((TLckData *)lck)->critSec);  // NOLINT
#else
    pthread_mutex_lock(&((TLckData *)lck)->mutex);     // NOLINT
#endif /*_WIN32*/
  }

#endif /*__SINGLE_THREAD*/
}

void LCK_ICACHE_FLASH lck_unlock(void *lck) {
  (void)(lck);
#ifndef __SINGLE_THREAD
  if (lck != NULL) {
#ifdef _WIN32
    LeaveCriticalSection(&((TLckData *)lck)->critSec);  // NOLINT
#else
    pthread_mutex_unlock(&((TLckData *)lck)->mutex);   // NOLINT
#endif /*_WIN32*/
  }

#endif /*__SINGLE_THREAD*/
}

int LCK_ICACHE_FLASH lck_unlock_r(void *lck, int result) {
  (void)(lck);
#ifndef __SINGLE_THREAD
  lck_unlock(lck);
#endif /*__SINGLE_THREAD*/
  return result;
}

void LCK_ICACHE_FLASH lck_free(void *lck) {
  (void)(lck);

#ifndef __SINGLE_THREAD
  if (lck != NULL) {
#ifdef _WIN32
    DeleteCriticalSection(&((TLckData *)lck)->critSec);  // NOLINT
#else
    pthread_mutex_destroy(&((TLckData *)lck)->mutex);  // NOLINT
#endif /*_WIN32*/
    free(lck);
  }
#endif /*__SINGLE_THREAD*/
}
