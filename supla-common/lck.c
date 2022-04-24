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

#include "lck.h"

#ifdef __LCK_DEBUG
#include <stdio.h>
#include <string.h>

#include "log.h"
#endif /*__LCK_DEBUG*/

#if defined(__AVR__) || defined(ARDUINO_ARCH_ESP8266) || \
    defined(ARDUINO_ARCH_ESP32)
#define __SINGLE_THREAD
#else

#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#include <time.h>
#endif /*_WIN32*/

#endif  // defined(__AVR__) || defined(ARDUINO_ARCH_ESP8266)
        // || defined(ARDUINO_ARCH_ESP32)

#include <stdlib.h>

#define MUTEX_COUNT 4

#ifndef __SINGLE_THREAD

typedef struct {
#ifdef _WIN32
  CRITICAL_SECTION critSec;
#else
  pthread_mutex_t mutex;
#ifdef __LCK_DEBUG

  pthread_t thread;
  int count;
  int lineNumber;
  char fileName[100];

#endif /*__LCK_DEBUG*/
#endif /*_WIN32*/
} TLckData;
#endif

#ifdef __LCK_DEBUG
void *ptrs[500];

void LCK_ICACHE_FLASH lck_debug_init(void) { memset(ptrs, 0, sizeof(ptrs)); }

void LCK_ICACHE_FLASH lck_debug_dump(void) {
  supla_log(LOG_INFO, "LCK DEBUG DUMP\n");
  int a;
  int n = sizeof(ptrs) / sizeof(void *);
  TLckData *l = 0;

  for (a = 0; a < n; a++) {
    if ((l = (TLckData *)ptrs[a]) != 0 && l->count != 0) {
      supla_log(LOG_INFO, "%p:%p %s:%i count=%i\n", (void *)l,
                (void *)l->thread, l->fileName, l->lineNumber, l->count);
    }
  }

  supla_log(LOG_INFO, "<<-----\n");
}

#endif /*__LCK_DEBUG*/

void *LCK_ICACHE_FLASH lck_init(void) {
#ifdef __SINGLE_THREAD
  return NULL;
#else
  TLckData *lck = malloc(sizeof(TLckData));

  if (lck != NULL) {
#ifdef __LCK_DEBUG
    memset(lck, 0, sizeof(TLckData));
    int a;
    int n = sizeof(ptrs) / sizeof(void *);
    for (a = 0; a < n; a++) {
      if (ptrs[a] == 0) {
        ptrs[a] = lck;
        break;
      }
    }
#endif /*__LCK_DEBUG*/

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

#ifdef __LCK_DEBUG

void LCK_ICACHE_FLASH __lck_lock(void *lck, const char *file, int line) {
  _lck_lock(lck);

  ((TLckData *)lck)->thread = pthread_self();
  ((TLckData *)lck)->count++;
  if (((TLckData *)lck)->count == 1) {
    snprintf(((TLckData *)lck)->fileName, sizeof(((TLckData *)lck)->fileName),
             "%s", file);
    ((TLckData *)lck)->lineNumber = line;
  }
}

void LCK_ICACHE_FLASH _lck_lock(void *lck) {
#else
void lck_lock(void *lck) {
#endif /*__LCK_DEBUG*/
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
#ifdef __LCK_DEBUG
  ((TLckData *)lck)->count--;
#endif /*__LCK_DEBUG*/
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
#ifndef __SINGLE_THREAD
  lck_unlock(lck);
#endif /*__SINGLE_THREAD*/
  return result;
}

void LCK_ICACHE_FLASH lck_free(void *lck) {
#ifdef __LCK_DEBUG
  int a;
  int n = sizeof(ptrs) / sizeof(void *);
  for (a = 0; a < n; a++) {
    if (ptrs[a] == lck) {
      ptrs[a] = 0;
      break;
    }
  }
#endif /*__LCK_DEBUG*/

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
