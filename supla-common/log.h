/*
 ============================================================================
 Name        : log.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef suplalog_H_
#define suplalog_H_

#ifdef ESP8266

#define LOG_EMERG       0       
#define LOG_ALERT       1      
#define LOG_CRIT        2   
#define LOG_ERR         3      
#define LOG_WARNING     4     
#define LOG_NOTICE      5    
#define LOG_INFO        6   
#define LOG_DEBUG       7  

#else

#include <syslog.h>

#endif

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
