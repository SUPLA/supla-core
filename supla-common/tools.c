/*
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#include "tools.h"
#include "log.h"
#include "eh.h"

unsigned char st_app_terminate = 0;
pthread_t main_thread;

TEventHandler *st_eh = NULL;

void st_signal_handler(int sig) {

	  if ( pthread_self() == main_thread ) {
		  st_app_terminate = 1;
	  }


      if ( st_eh != 0 )
    	  eh_raise_event(st_eh);
}

void st_hook_signals(void) {

	main_thread = pthread_self();

	signal(SIGHUP, st_signal_handler);
	signal(SIGINT, st_signal_handler);
	signal(SIGTERM, st_signal_handler);
	signal(SIGQUIT, st_signal_handler);
	signal(SIGPIPE, SIG_IGN);

}

unsigned char st_file_exists(const char *fp) {

	unsigned char file_exists = 0;

	if ( fp != NULL
		  && strnlen(fp, 1024) >  0 ) {

		file_exists = access(fp, R_OK ) != -1;
		/*
		 FILE *F = fopen(fp, "r");

		 if ( F ) {
		     file_exists = 1;
		     fclose(F);
		 }
		*/

	}

    return file_exists;

}

char st_try_fork(void) {


	    pid_t pid, sid;
	    pid = fork();
	    if (pid < 0) {
	       supla_log(LOG_ERR, "Can't fork");
	       return 0;
	    }

	    if (pid > 0) {
	        exit(EXIT_SUCCESS);
	    }

	    sid = setsid();
	    if (sid < 0) {

	    	 supla_log(LOG_ERR, "Can't fork");
		     return 0;
	    }

	    if ((chdir("/")) < 0) {

	    	 supla_log(LOG_ERR, "Can't fork");
		     return 0;
	    }

	    close(STDIN_FILENO);
	    close(STDOUT_FILENO);
	    close(STDERR_FILENO);

return 1;

}

char st_set_ug_id(int uid, int gid) {

	if ( setgid(gid) != 0 ) {
    	supla_log(LOG_ERR, "Can't change group");
    	return 0;
    }

    if ( setuid(uid) != 0 ) {
    	supla_log(LOG_ERR, "Can't change user");
    	return 0;
    }

    return 1;
}

char st_setpidfile(char *pidfile_path) {

        if ( pidfile_path != 0
        	 && strnlen(pidfile_path, 1024) > 0 ) {

                FILE *F = fopen(pidfile_path, "w");
                if ( F ) {

                        char str[32];
                        snprintf(str, 31,"%i\n",getpid());
                        fwrite(str, (int)1, strnlen(str, 31), F);
                        fclose(F);

                        return 1;
                }

                supla_log(LOG_ERR, "Can't create pid file: %s", pidfile_path);
        }

        return 0;
}

void st_delpidfile(char *pidfile_path) {


   if ( st_file_exists(pidfile_path) == 1 )
		unlink(pidfile_path);

}

void st_mainloop_init(void) {

	st_eh = eh_init();

}

void st_mainloop_free(void) {

	eh_free(st_eh);

}

void st_mainloop_wait(int usec) {
	eh_wait(st_eh, usec);
}

size_t st_strlen(char *str, size_t maxlen) {

	str[maxlen] = 0;
	return strnlen(str, maxlen-1);
}

void st_guid2hex(char GUIDHEX[SUPLA_GUID_HEXSIZE], const char GUID[SUPLA_GUID_SIZE]) {

	snprintf(GUIDHEX,
			 SUPLA_GUID_HEXSIZE,
	                        "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
	                        (unsigned char)GUID[0],
	                        (unsigned char)GUID[1],
	                        (unsigned char)GUID[2],
	                        (unsigned char)GUID[3],
	                        (unsigned char)GUID[4],
	                        (unsigned char)GUID[5],
	                        (unsigned char)GUID[6],
	                        (unsigned char)GUID[7],
	                        (unsigned char)GUID[8],
	                        (unsigned char)GUID[9],
	                        (unsigned char)GUID[10],
	                        (unsigned char)GUID[11],
	                        (unsigned char)GUID[12],
	                        (unsigned char)GUID[13],
	                        (unsigned char)GUID[14],
	                        (unsigned char)GUID[15]);

}

char *st_str2hex(char *buffer, const char *str, size_t maxlen) {

	int a, b;

	if ( str == 0 || buffer == 0 )
		return buffer;

	b=0;

	for(a=0;a<strnlen(str, maxlen);a++) {
		snprintf(&buffer[b], 3, "%02X", (unsigned char)str[a]);
		b+=2;
	}

	return buffer;
}

char st_read_randkey_from_file(char *file, char *KEY, int size, char create) {

	FILE *F;
	int a;
	char result = 0;

	if ( st_file_exists(file) != 1 ) {

		if ( create == 1 && file != 0 && strnlen(file, 1024) > 0 ) {

             F = fopen(file, "w");
             if ( F ) {

            	     struct timeval tv;
            	     gettimeofday(&tv, NULL);

            	     srand(tv.tv_usec);
            	     gettimeofday(&tv, NULL);

            	     for(a=0;a<size;a++)
            	    	 KEY[a] = (unsigned char)(rand()+tv.tv_usec);

                     if ( fwrite(KEY, size, (int)1, F) == 1 ) {
                    	 result = 1;
                     } else {
                    	 supla_log(LOG_ERR, "Can't write to file %s", file);
                     }

                     fclose(F);
                     return result;

             } else {
            	 supla_log(LOG_ERR, "Can't create file %s", file);
             }

		}

		return 0;
	};

    F = fopen(file, "r");
    if ( F ) {
    	fseek(F, 0, SEEK_END);
    	if ( ftell(F) == size ) {
    		fseek(F, 0, SEEK_SET);

            if ( fread(KEY, size, (int)1, F) == 1 ) {
           	 result = 1;
            } else {
           	 supla_log(LOG_ERR, "Can't read file %s", file);
            }

    	} else {
    		supla_log(LOG_ERR, "%s - wrong size", file);
    	}

    } else {
    	supla_log(LOG_ERR, "Can't open file %s", file);
    }

    if ( result == 1 ) {
    	result = 0;
    	for(a=0;a<size;a++)
    		if ( (int)KEY[a] != 0 ) {
    			result = 1;
    			break;
    		}

    	if ( result == 0 )
    		supla_log(LOG_ERR, "%s - format error", file);
    }

	return result;
}

char st_read_guid_from_file(char *file, char *GUID, char create) {
	return st_read_randkey_from_file(file, GUID, SUPLA_GUID_SIZE, create);
}

char st_read_authkey_from_file(char *file, char *AuthKey, char create) {
	return st_read_randkey_from_file(file, AuthKey, SUPLA_AUTHKEY_SIZE, create);
}

time_t st_get_utc_time(void) {

	time_t now = time(0);
	struct tm* now_tm = gmtime(&now);
	return mktime(now_tm);

}
