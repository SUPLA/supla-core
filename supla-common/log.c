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

#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <Windows.h>
#include <wchar.h>
#elif defined(ARDUINO)
void serialPrintLn(const char *);
#elif defined(ESP_PLATFORM)
#include <esp_log.h>
static const char *SUPLA_TAG = "SUPLA";
#elif defined(SUPLA_DEVICE)
// new supla-device lib
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#endif /*defined(_WIN32)*/

#include <string.h>

#ifdef ESP8266

#include <mem.h>
#include <osapi.h>

#else

#if !defined(ARDUINO) && !defined(SUPLA_DEVICE)
#include "cfg.h"
#endif /*!defined(ARDUINO) && !defined(SUPLA_DEVICE)*/

#endif /*ESP8266*/

#ifdef __ANDROID__
#include <android/log.h>
#endif /*__ANDROID__*/

#ifdef __LOG_CALLBACK
_supla_log_callback __supla_log_callback = NULL;

void LOG_ICACHE_FLASH supla_log_set_callback(_supla_log_callback callback) {
  __supla_log_callback = callback;
}
#endif /*__LOG_CALLBACK*/

char LOG_ICACHE_FLASH supla_log_string(char **buffer, int *size, va_list va,
                                       const char *__fmt) {
  char *nb;
  int n;

  if (*size == 0) {
    *size = strnlen(__fmt, 10240) + 10;
    *buffer = (char *)malloc(*size);
  }

  if (*buffer == NULL) {
    *size = 0;
    return 0;
  }

#ifdef _WIN32
  n = vsnprintf_s(*buffer, *size, _TRUNCATE, __fmt, va);
#else
  n = vsnprintf(*buffer, *size, __fmt, va);
#endif /*_WIN32*/

  if (n < 0 || n >= *size) {
    if (n > -1)      /* glibc 2.1 */
      *size = n + 1; /* precisely what is needed */
    else             /* glibc 2.0 */
      (*size) *= 2;  /* twice the old size */

    if ((nb = (char *)realloc(*buffer, *size)) == NULL) {
      free(*buffer);
      *buffer = NULL;
      *size = 0;
    } else {
      *buffer = nb;
      return 1;
    }
  }

  if (*buffer != NULL) {
    (*buffer)[(*size) - 1] = 0;
  }

  return 0;
}

#ifdef _WIN32
// Windows variant
void supla_vlog(int __pri, const char *message) {
  WCHAR wstr[2048];
  size_t convertedChars;

  mbstowcs_s(&convertedChars, wstr, 2048, message, strnlen(message, 10240));

  OutputDebugStringW(wstr);
  OutputDebugStringW(L"\n");
}
#elif defined(ARDUINO)
// supla-device variant for Arduino IDE
void supla_vlog(int __pri, const char *message) {
  (void)(__pri);
  serialPrintLn(message);
}
#elif defined(ESP_PLATFORM)
// variant for ESP8266 RTOS and ESP-IDF
void supla_vlog(int __pri, const char *message) {
  switch (__pri) {
    case LOG_VERBOSE:
      ESP_LOGV(SUPLA_TAG, "%s", message);
      break;
    case LOG_DEBUG:
      ESP_LOGD(SUPLA_TAG, "%s", message);
      break;
    case LOG_INFO:
    case LOG_NOTICE:
      ESP_LOGI(SUPLA_TAG, "%s", message);
      break;
    case LOG_WARNING:
      ESP_LOGW(SUPLA_TAG, "%s", message);
      break;
    case LOG_ERR:
    case LOG_CRIT:
    case LOG_EMERG:
    case LOG_ALERT:
      ESP_LOGE(SUPLA_TAG, "%s", message);
      break;
    default:
      ESP_LOGE(SUPLA_TAG, "%s", message);
  }
}
#elif defined(SUPLA_DEVICE)
// Keep it empty - supla_vlog is defined in target specific file in porting
// folder
void supla_vlog(int __pri, const char *message);

#elif defined(ESP8266)
// supla-espressif-esp variant
void LOG_ICACHE_FLASH supla_vlog(int __pri, const char *message) {
#ifndef ESP8266_LOG_DISABLED
  os_printf("%s\r\n", message);
#endif
}
#else

void LOG_ICACHE_FLASH supla_vlog(int __pri, const char *message) {
#ifdef __ANDROID__
  switch (__pri) {
    case LOG_CRIT:
    case LOG_EMERG:
      __pri = ANDROID_LOG_FATAL;
      break;
    case LOG_ERR:
      __pri = ANDROID_LOG_ERROR;
      break;
    case LOG_ALERT:
    case LOG_WARNING:
      __pri = ANDROID_LOG_WARN;
      break;
    case LOG_NOTICE:
      __pri = ANDROID_LOG_DEFAULT;
      break;
    case LOG_INFO:
      __pri = ANDROID_LOG_INFO;
      break;
    case LOG_DEBUG:
      __pri = ANDROID_LOG_DEBUG;
      break;
  }
  __android_log_write(ANDROID_LOG_DEBUG, "LibSuplaClient", message);
#else

#ifndef __LOG_CALLBACK
  struct timeval now;
#endif

  if (message == NULL || (debug_mode == 0 && __pri == LOG_DEBUG)) return;

#ifdef __LOG_CALLBACK
  if (__supla_log_callback) __supla_log_callback(__pri, message);
#else
  if (run_as_daemon == 1) {
    syslog(__pri, "%s", message);
  } else {
    switch (__pri) {
      case LOG_EMERG:
        printf("EMERG");
        break;
      case LOG_ALERT:
        printf("ALERT");
        break;
      case LOG_CRIT:
        printf("CRIT");
        break;
      case LOG_ERR:
        printf("ERR");
        break;
      case LOG_WARNING:
        printf("WARNING");
        break;
      case LOG_NOTICE:
        printf("NOTICE");
        break;
      case LOG_INFO:
        printf("INFO");
        break;
      case LOG_DEBUG:
        printf("DEBUG");
        break;
    }

    gettimeofday(&now, NULL);
    time_t now_time = now.tv_sec;
    struct tm now_tm;
    localtime_r(&now_time, &now_tm);
    char time_buf[64] = {};

    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &now_tm);
#ifdef __APPLE__
    printf("[%s.%06i] ", time_buf, now.tv_usec);
#else
    printf("[%s.%06ld] ", time_buf, now.tv_usec);
#endif
    printf("%s", message);
    printf("\n");
    fflush(stdout);
  }
#endif

#endif
}
#endif

void LOG_ICACHE_FLASH supla_log(int __pri, const char *__fmt, ...) {
  va_list ap;
  char *buffer = NULL;
  int size = 0;

#if defined(ESP8266) || defined(ARDUINO) || defined(_WIN32) || \
    defined(SUPLA_DEVICE)
  if (__fmt == NULL) return;
#else
  if (__fmt == NULL || (debug_mode == 0 && __pri == LOG_DEBUG)) return;
#endif

  while (1) {
    va_start(ap, __fmt);
    if (0 == supla_log_string(&buffer, &size, ap, __fmt)) {
      va_end(ap);
      break;
    } else {
      va_end(ap);
    }
    va_end(ap);
  }

  if (buffer == NULL) return;

  supla_vlog(__pri, buffer);
  free(buffer);
}

void LOG_ICACHE_FLASH supla_write_state_file(const char *file, int __pri,
                                             const char *__fmt, ...) {
  (void)(file);
  (void)(__pri);
  (void)(__fmt);
  char *buffer = NULL;
  int size = 0;

  va_list ap;

  while (1) {
    va_start(ap, __fmt);
    if (0 == supla_log_string(&buffer, &size, ap, __fmt)) {
      va_end(ap);
      break;
    } else {
      va_end(ap);
    }
  }

  if (buffer == NULL) return;

  if (__pri > -1) {
    supla_vlog(__pri, buffer);
  }

#if !defined(ESP8266) && !defined(ARDUINO) && !defined(WIN32) && \
    !defined(SUPLA_DEVICE)

  int fd;

  if (file != 0 && strnlen(file, 1024) > 0) {
    fd = open(file, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd != -1) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
      write(fd, buffer, strnlen(buffer, size));
#pragma GCC diagnostic pop
      close(fd);
    }
  }
#endif

  free(buffer);
}
