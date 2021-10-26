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

#ifndef suplalog_H_
#define suplalog_H_

#if defined(ESP8266) || defined(ESP32)
#include <mem.h>
#define LOG_ICACHE_FLASH ICACHE_FLASH_ATTR
#endif

#if defined(ARDUINO)
#undef LOG_ICACHE_FLASH
#endif /*defined(ARDUINO)*/ 

#ifndef LOG_ICACHE_FLASH
#define LOG_ICACHE_FLASH
#endif /*LOG_ICACHE_FLASH*/

#if defined(ESP8266) || defined(__AVR__) || defined(_WIN32) || defined(ESP32)

#define LOG_EMERG 0
#define LOG_ALERT 1
#define LOG_CRIT 2
#define LOG_ERR 3
#define LOG_WARNING 4
#define LOG_NOTICE 5
#define LOG_INFO 6
#define LOG_DEBUG 7

#else

#include <syslog.h>

#endif  // defined(ESP8266) || defined(__AVR__)
        // || defined(_WIN32) || defined(ESP32)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __LOG_CALLBACK
typedef int (*_supla_log_callback)(int __pri, const char *message);

void LOG_ICACHE_FLASH supla_log_set_callback(_supla_log_callback callback);
#endif /*__LOG_CALLBACK*/

void LOG_ICACHE_FLASH supla_log(int __pri, const char *__fmt, ...);
void LOG_ICACHE_FLASH supla_write_state_file(const char *file, int __pri,
                                             const char *__fmt, ...);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* suplalog_H_ */
