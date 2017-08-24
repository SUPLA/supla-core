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
char *st_str2hex(char *buffer, const char *str, size_t maxlen);

char st_read_randkey_from_file(char *file, char *KEY, int size, char create);
char st_read_guid_from_file(char *file, char *GUID, char create);
char st_read_authkey_from_file(char *file, char *AuthKey, char create);

time_t st_get_utc_time(void);

#ifdef __BCRYPT

char st_bcrypt_gensalt(char *salt, int salt_buffer_size, char rounds);
char st_bcrypt_hash(char *str, char *salt, char *hash, int hash_buffer_size);
char st_bcrypt_crypt(char *str, char *hash, int hash_buffer_size, char rounds);
char st_bcrypt_check(char *str, char *hash, int hash_len);

#endif

#ifdef __cplusplus
}
#endif

#endif /* suplatools_H_ */
