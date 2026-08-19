// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef suplalog_H_
#define suplalog_H_

#if defined(ESP8266)
#include <mem.h>
#define LOG_ICACHE_FLASH ICACHE_FLASH_ATTR
#endif

#if defined(ARDUINO)
#undef LOG_ICACHE_FLASH
#endif /*defined(ARDUINO)*/

#ifndef LOG_ICACHE_FLASH
#define LOG_ICACHE_FLASH
#endif /*LOG_ICACHE_FLASH*/

#if defined(ESP8266) || defined(__AVR__) || defined(_WIN32) || \
  defined(ESP32) || defined(SUPLA_DEVICE) || defined(SUPLA_TEST)

#define LOG_EMERG 0
#define LOG_ALERT 1
#define LOG_CRIT 2
#define LOG_ERR 3
#define LOG_WARNING 4
#define LOG_NOTICE 5
#define LOG_INFO 6
#define LOG_DEBUG 7
#define LOG_VERBOSE 8

#else

#include <syslog.h>

#endif  // defined(ESP8266) || defined(__AVR__)
        // || defined(_WIN32) || defined(ESP32) || defined(SUPLA_DEVICE)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __LOG_CALLBACK
typedef int (*_supla_log_callback)(int __pri, const char *message);

void LOG_ICACHE_FLASH supla_log_set_callback(_supla_log_callback callback);
#endif /*__LOG_CALLBACK*/

void LOG_ICACHE_FLASH supla_log_set_level(int level);
int LOG_ICACHE_FLASH supla_log_get_level(void);
char LOG_ICACHE_FLASH supla_log_is_enabled(int level);
void LOG_ICACHE_FLASH supla_log(int __pri, const char *__fmt, ...);
void LOG_ICACHE_FLASH supla_write_state_file(const char *file, int __pri,
                                             const char *__fmt, ...);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* suplalog_H_ */
