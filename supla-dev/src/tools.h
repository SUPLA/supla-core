/*
 ============================================================================
 Name        : tools.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */


#ifndef suplatools_H_
#define suplatools_H_

#include "proto.h"

#ifdef __cplusplus
extern "C" {
#endif


extern unsigned char st_app_terminate;

unsigned char st_file_exists(const char *fp);
void st_hook_signals(void);
char st_try_fork(void);
char st_set_ug_id(int uid, int gid);
char st_setpidfile(char *pidfile_path);
void st_delpidfile(char *pidfile_path);
void st_mainloop_init();
void st_mainloop_wait(int usec);
void st_mainloop_free();

size_t st_strlen(char *str, size_t maxlen);
void st_guid2hex(char GUIDHEX[SUPLA_GUID_HEXSIZE], const char GUID[SUPLA_GUID_SIZE]);
char *st_str2hex(char *buffer, const char *str);

char st_read_guid_from_file(char *file, char *GUID, char create);

#ifdef __cplusplus
}
#endif

#endif /* suplatools_H_ */
