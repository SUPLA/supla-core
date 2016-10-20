/*
 ============================================================================
 Name        : log.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : 2015-2016 GPLv2 
 ============================================================================
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "log.h"

#if defined(_WIN32)
    #include <Windows.h>
	#include <wchar.h>
#elif defined(ARDUINO_ARCH_ESP8266) || defined(__AVR__)
    #include <Arduino.h>
#else
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
#endif

#include <string.h>



#ifdef ESP8266
        #include <osapi.h>
        #ifdef ARDUINO_ARCH_ESP8266
        #include <Arduino.h>
        #include <ets_sys.h>
        #endif
        #include <mem.h>	

	#define malloc os_malloc
	#define free os_free
	#define realloc os_realloc
	#define vsnprintf ets_vsnprintf

#else

	#ifndef __AVR__
		#include "cfg.h"
	#endif

#endif

#ifdef __ANDROID__
	#include <android/log.h>
#endif

#ifdef __LOG_CALLBACK
	_supla_log_callback __supla_log_callback = NULL;
	
	
	void supla_log_set_callback(_supla_log_callback callback) {
		__supla_log_callback = callback;
	}
#endif

char supla_log_string(char **buffer, int *size, va_list va, const char *__fmt) {

    char *nb;
    int n;

    if ( *size == 0 ) {
        *size = strlen(__fmt)+10;
        *buffer = (char *)malloc(*size);
    }

    if ( *buffer == NULL ) {
    	*size = 0;
    	return 0;
    }

	#ifdef _WIN32
		n = vsnprintf_s(*buffer, *size, _TRUNCATE,__fmt, va);
	#else
		n = vsnprintf(*buffer, *size, __fmt, va);
	#endif

	if (n < 0 || n >= *size) {

		 if (n > -1)    /* glibc 2.1 */
			  *size = n+1; /* precisely what is needed */
		 else           /* glibc 2.0 */
			  (*size) *= 2;  /* twice the old size */

		 if ((nb = (char *)realloc (*buffer, *size)) == NULL) {
			  free(*buffer);
			  *buffer = NULL;
			  *size = 0;
		 } else {
			  *buffer = nb;
			  return 1;
		 }
	}

	if ( *buffer != NULL ) {
	   (*buffer)[(*size)-1] = 0;
	}

	return 0;

}

#ifdef _WIN32

void supla_vlog(int __pri, const char *message) {

	WCHAR wstr[2048];
	size_t convertedChars;

	mbstowcs_s(
	    &convertedChars,
		wstr,
		2048,
		message,
		strlen(message)
	);

	OutputDebugStringW(wstr);
	OutputDebugStringW(L"\n");
}

#elif defined(ESP8266) || defined(__AVR__)
	void supla_vlog(int __pri, const char *message) {
        #if defined(ESP8266) && !defined(ARDUINO_ARCH_ESP8266)
		os_printf("%s\r\n", message);
        #else
		Serial.println(message);
        #endif
	}
#else
void supla_vlog(int __pri, const char *message) {

    #ifdef __ANDROID__
		switch(__pri) {
		case	LOG_CRIT:
		case	LOG_EMERG:
			__pri = ANDROID_LOG_FATAL;
			break;
		case	LOG_ERR:
			__pri = ANDROID_LOG_ERROR;
			break;
		case	LOG_ALERT:
		case	LOG_WARNING:
			__pri = ANDROID_LOG_WARN;
			break;
		case	LOG_NOTICE:
			__pri = ANDROID_LOG_DEFAULT;
			break;
		case	LOG_INFO:
			__pri = ANDROID_LOG_INFO;
			break;
		case	LOG_DEBUG:
			__pri = ANDROID_LOG_DEBUG;
			break;
		}
		 __android_log_write(ANDROID_LOG_DEBUG, "LibSuplaClient", message);
    #else

		#ifndef __LOG_CALLBACK
			struct timeval now;
		#endif

		#if defined(ESP8266) || defined(__AVR__)
			if ( message == NULL ) return;
		#else
			if ( message == NULL || ( debug_mode == 0 && __pri == LOG_DEBUG ) ) return;
		#endif

		#ifdef __LOG_CALLBACK
			if ( __supla_log_callback )
				__supla_log_callback(__pri, message);
		#else
			if ( run_as_daemon == 1 ) {
				  syslog(__pri, "%s", message);
			} else {
					switch(__pri) {
							case LOG_EMERG: printf("EMERG"); break;
							case LOG_ALERT: printf("ALERT"); break;
							case LOG_CRIT: printf("CRIT"); break;
							case LOG_ERR: printf("ERR"); break;
							case LOG_WARNING: printf("WARNING"); break;
							case LOG_NOTICE: printf("NOTICE"); break;
							case LOG_INFO: printf("INFO"); break;
							case LOG_DEBUG: printf("DEBUG"); break;
					}
		
				   #if defined(ESP8266) || defined(__AVR__)
					   os_printf("%s\r\n", message);
				   #else
					   gettimeofday(&now, NULL);
					   printf("[%li.%li] ", (unsigned long)now.tv_sec, (unsigned long)now.tv_usec);
					   printf("%s", message);
					   printf("\n");
					   fflush (stdout);
				   #endif
			}
		#endif

	#endif
}
#endif

void supla_log(int __pri, const char *__fmt, ...) {

    va_list ap;
	char *buffer = NULL;
	int size = 0;


    #if defined(ESP8266) || defined(__AVR__) || defined(_WIN32)
		if ( __fmt == NULL ) return;
    #else
		if ( __fmt == NULL || ( debug_mode == 0 && __pri == LOG_DEBUG ) ) return;
    #endif


	while(1) {
		va_start(ap, __fmt);
		if ( 0 == supla_log_string(&buffer,  &size, ap, __fmt) ) {
			va_end(ap);
			break;
		} else {
			va_end(ap);
		}
		va_end(ap);
	}

	if ( buffer == NULL )
		return;

	supla_vlog(__pri, buffer);
	free(buffer);

}


void supla_write_state_file(const char *file, int __pri, const char *__fmt, ...) {
	
	char *buffer = NULL;
	int size = 0;
	
	va_list ap;

	while(1) {
		va_start(ap, __fmt);
		if ( 0 == supla_log_string(&buffer,  &size, ap, __fmt) ) {
			va_end(ap);
			break;
		} else {
			va_end(ap);
		}
	}
	
	if ( buffer == NULL )
		return;
	
	if ( __pri > -1 ) {
		supla_vlog(__pri, buffer);
	}
	
	#if !defined(ESP8266) && !defined(__AVR__) && !defined(WIN32)
	
	int fd;

	if ( file != 0
			&& strlen(file) > 0 ) {
	
			fd = open(file, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
			if ( fd != -1 ) {
				write(fd, buffer, strlen(buffer));
				close(fd);
			}
	}
	#endif

	free(buffer);
	
}


