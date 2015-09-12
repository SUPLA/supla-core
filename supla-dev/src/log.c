/*
 ============================================================================
 Name        : log.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "log.h"
#include "cfg.h"

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
        *buffer = malloc(*size);
    }

    if ( *buffer == NULL ) {
    	*size = 0;
    	return 0;
    }


    n = vsnprintf(*buffer, *size, __fmt, va);

	if (n < 0 || n >= *size) {

		 if (n > -1)    /* glibc 2.1 */
			  *size = n+1; /* precisely what is needed */
		 else           /* glibc 2.0 */
			  (*size) *= 2;  /* twice the old size */

		 if ((nb = realloc (*buffer, *size)) == NULL) {
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

void supla_vlog(int __pri, const char *message) {


    #ifndef __LOG_CALLBACK
    struct timeval now;
    #endif

    if ( message == NULL || ( debug_mode == 0 && __pri == LOG_DEBUG ) ) return;

    #ifdef __LOG_CALLBACK
    if ( __supla_log_callback )
    	__supla_log_callback(__pri, message);
    #else

    if ( run_as_daemon == 1 ) {
          syslog(__pri, message);
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

       gettimeofday(&now, NULL);
       printf("[%li.%li] ", (unsigned long)now.tv_sec, (unsigned long)now.tv_usec);
       printf(message);
       printf("\n");
       fflush (stdout);
    }
    #endif
}

void supla_log(int __pri, const char *__fmt, ...) {

    va_list ap;
	char *buffer = NULL;
	int size = 0;

    if ( __fmt == NULL || ( debug_mode == 0 && __pri == LOG_DEBUG ) ) return;

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
	int fd;

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


	if ( file != 0
		 && strlen(file) > 0 ) {

			fd = open(file, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
			if ( fd != -1 ) {
				write(fd, buffer, strlen(buffer));
				close(fd);
			}
	}

	free(buffer);

}
