/*
 ============================================================================
 Name        : log.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef suplalog_H_
#define suplalog_H_

#include <syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __LOG_CALLBACK
typedef int (*_supla_log_callback)(int __pri, const char *message);

void supla_log_set_callback(_supla_log_callback callback);
#endif

void supla_log(int __pri, const char *__fmt, ...);
void supla_write_state_file(const char *file, int __pri, const char *__fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* suplalog_H_ */
