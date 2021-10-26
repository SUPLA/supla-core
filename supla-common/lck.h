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

#ifndef LCK_H_
#define LCK_H_

#if defined(ESP8266) || defined(ESP32)
#include <mem.h>
#define LCK_ICACHE_FLASH ICACHE_FLASH_ATTR
#endif

#if defined(ARDUINO)
#undef LCK_ICACHE_FLASH
#endif /*defined(ARDUINO)*/ 

#ifndef LCK_ICACHE_FLASH
#define LCK_ICACHE_FLASH
#endif /*LCK_ICACHE_FLASH*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __DEBUG
// #define __LCK_DEBUG
#endif

#ifdef __LCK_DEBUG
#define lck_lock(ptr) __lck_lock(ptr, __FILE__, __LINE__)
void LCK_ICACHE_FLASH _lck_lock(void *lck);
void LCK_ICACHE_FLASH __lck_lock(void *lck, const char *file, int line);
void LCK_ICACHE_FLASH lck_debug_init(void);
void LCK_ICACHE_FLASH lck_debug_dump(void);
#else
void LCK_ICACHE_FLASH lck_lock(void *lck);
#endif /*__LCK_DEBUG*/

char LCK_ICACHE_FLASH lck_lock_with_timeout(void *lck, int timeout_sec);
void LCK_ICACHE_FLASH lck_unlock(void *lck);
int LCK_ICACHE_FLASH lck_unlock_r(void *lck, int result);
void *LCK_ICACHE_FLASH lck_init(void);
void LCK_ICACHE_FLASH lck_free(void *lck);

#ifdef __cplusplus
}
#endif

#endif /* LCK_H_ */
