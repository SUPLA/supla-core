// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LCK_H_
#define LCK_H_

#if defined(ESP8266)
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

void LCK_ICACHE_FLASH lck_lock(void *lck);

char LCK_ICACHE_FLASH lck_lock_with_timeout(void *lck, int timeout_sec);
void LCK_ICACHE_FLASH lck_unlock(void *lck);
int LCK_ICACHE_FLASH lck_unlock_r(void *lck, int result);
void *LCK_ICACHE_FLASH lck_init(void);
void LCK_ICACHE_FLASH lck_free(void *lck);

#ifdef __cplusplus
}
#endif

#endif /* LCK_H_ */
