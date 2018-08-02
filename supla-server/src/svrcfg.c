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

#include "svrcfg.h"
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *svrcfg_oauth_url_base64 = NULL;

unsigned char svrcfg_init(int argc, char *argv[]) {
  char result;
  BIO *bio, *b64;
  BUF_MEM *bufferPtr;
  // !!! order is important !!!

  char *s_global = "GLOBAL";
  scfg_add_str_param(s_global, "user", NULL);
  scfg_add_str_param(s_global, "group", NULL);

  char *s_net = "NET";
  scfg_add_int_param(s_net, "tcp_port", 2015);
  scfg_add_bool_param(s_net, "tcp_enabled", 1);

  scfg_add_int_param(s_net, "ssl_port", 2016);
  scfg_add_bool_param(s_net, "ssl_enabled", 1);

  scfg_add_str_param(s_net, "cert", NULL);
  scfg_add_str_param(s_net, "private_key", NULL);

  char *s_mysql = "MySQL";
  scfg_add_str_param(s_mysql, "host", NULL);
  scfg_add_int_param(s_mysql, "port", 0);
  scfg_add_str_param(s_mysql, "database", "supla");
  scfg_add_str_param(s_mysql, "user", NULL);
  scfg_add_str_param(s_mysql, "password", NULL);

  char *s_ipc = "IPC";
  scfg_add_str_param(s_ipc, "socket_path", "/tmp/supla-server-ctrl.sock");

  char *s_oauth = "OAUTH";
  char hostname[CFG_OAUTH_URL_MAXSIZE - 10];
  memset(hostname, 0, sizeof(hostname));
  gethostname(hostname, sizeof(hostname) - 1);

  char url[CFG_OAUTH_URL_MAXSIZE];
  snprintf(url, CFG_OAUTH_URL_MAXSIZE, "https://%s", hostname);

  scfg_add_str_param(s_oauth, "url", url);
  scfg_add_int_param(s_oauth, "access_token_lifetime", 300);

  result = scfg_load(argc, argv, "/etc/supla-server/supla.cfg");
  scfg_names_free();

  if (result != 0) {
    int n = strnlen(scfg_string(CFG_OAUTH_URL), CFG_OAUTH_URL_MAXSIZE);
    if (n > 0) {
      b64 = BIO_new(BIO_f_base64());
      bio = BIO_new(BIO_s_mem());
      bio = BIO_push(b64, bio);
      BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
      BIO_write(bio, scfg_string(CFG_OAUTH_URL), n);

      BIO_get_mem_ptr(bio, &bufferPtr);
      (void)BIO_set_close(bio, BIO_NOCLOSE);

      svrcfg_oauth_url_base64 = malloc(bufferPtr->length + 1);
      memcpy(svrcfg_oauth_url_base64, bufferPtr->data, bufferPtr->length);
      svrcfg_oauth_url_base64[bufferPtr->length] = 0;

      BUF_MEM_free(bufferPtr);
      (void)BIO_flush(b64);
      BIO_free_all(b64);
    }
  }

  return result;
}

void svrcfg_free(void) {
  if (svrcfg_oauth_url_base64) {
    free(svrcfg_oauth_url_base64);
    svrcfg_oauth_url_base64 = NULL;
  }
  scfg_free();
}
