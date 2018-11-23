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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "tools.h"

char *svrcfg_oauth_url_base64 = NULL;
int svrcfg_oauth_url_base64_len = 0;

unsigned char svrcfg_init(int argc, char *argv[]) {
  char result;
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

  char *s_http = "HTTP";
  scfg_add_int_param(s_http, "thread_count_limit", 50);
  scfg_add_int_param(s_http, "request_timeout", 5000);

  char *s_alexa = "ALEXA";
  scfg_add_int_param(s_alexa, "response_timeout", 4000);
  scfg_add_int_param(s_alexa, "changereport_timeout", 30000);

  result = scfg_load(argc, argv, "/etc/supla-server/supla.cfg");
  scfg_names_free();

  if (result != 0) {
    int n = strnlen(scfg_string(CFG_OAUTH_URL), CFG_OAUTH_URL_MAXSIZE);
    if (n > 0) {
      svrcfg_oauth_url_base64 =
          st_openssl_base64_encode(scfg_string(CFG_OAUTH_URL), n);
      svrcfg_oauth_url_base64_len =
          svrcfg_oauth_url_base64 == NULL ? 0 : strnlen(svrcfg_oauth_url_base64,
                                                        n * 2);
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
