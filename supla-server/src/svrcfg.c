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

  char *s_mqtt = "MQTT-BROKER";
  // Start with the highest index (FG_MQTT_KEEP_ALIVE_SEC == 39)
  // This ensures that realloc will only be called once
  scfg_add_int_param(CFG_MQTT_KEEP_ALIVE_SEC, s_mqtt, "CFG_MQTT_KEEP_ALIVE_SEC",
                     30);
  // -----

  scfg_add_int_param(CFG_MQTT_ENABLED, s_mqtt, "enabled", 0);
  scfg_add_str_param(CFG_MQTT_HOST, s_mqtt, "host", NULL);
  scfg_add_int_param(CFG_MQTT_PORT, s_mqtt, "port", 8883);
  scfg_add_str_param(CFG_MQTT_USERNAME, s_mqtt, "username", NULL);
  scfg_add_str_param(CFG_MQTT_PASSWORD, s_mqtt, "password", NULL);
  scfg_add_str_param(CFG_MQTT_PREFIX, s_mqtt, "prefix", NULL);
  scfg_add_int_param(CFG_MQTT_SSL, s_mqtt, "ssl", 1);
  scfg_add_str_param(CFG_MQTT_CLIENTID, s_mqtt, "client_id", NULL);

  char *s_global = "GLOBAL";
  scfg_add_str_param(CFG_UID, s_global, "user", NULL);
  scfg_add_str_param(CFG_GID, s_global, "group", NULL);

  char *s_net = "NET";
  scfg_add_int_param(CFG_TCP_PORT, s_net, "tcp_port", 2015);
  scfg_add_bool_param(CFG_TCP_ENABLED, s_net, "tcp_enabled", 1);

  scfg_add_int_param(CFG_SSL_PORT, s_net, "ssl_port", 2016);
  scfg_add_bool_param(CFG_SSL_ENABLED, s_net, "ssl_enabled", 1);

  scfg_add_str_param(CFG_SSL_CERT, s_net, "cert", NULL);
  scfg_add_str_param(CFG_SSL_KEY, s_net, "private_key", NULL);

  char *s_mysql = "MySQL";
  scfg_add_str_param(CFG_MYSQL_HOST, s_mysql, "host", NULL);
  scfg_add_int_param(CFG_MYSQL_PORT, s_mysql, "port", 0);
  scfg_add_str_param(CFG_MYSQL_DB, s_mysql, "database", "supla");
  scfg_add_str_param(CFG_MYSQL_USER, s_mysql, "user", NULL);
  scfg_add_str_param(CFG_MYSQL_PASSWORD, s_mysql, "password", NULL);

  char *s_ipc = "IPC";
  scfg_add_str_param(CFG_IPC_SOCKET_PATH, s_ipc, "socket_path",
                     "/var/run/supla/supla-server-ctrl.sock");

  char *s_oauth = "OAUTH";
  char hostname[CFG_OAUTH_URL_MAXSIZE - 10];
  memset(hostname, 0, sizeof(hostname));
  gethostname(hostname, sizeof(hostname) - 1);

  char url[CFG_OAUTH_URL_MAXSIZE];
  snprintf(url, CFG_OAUTH_URL_MAXSIZE, "https://%s", hostname);

  scfg_add_str_param(CFG_OAUTH_URL, s_oauth, "url", url);
  scfg_add_int_param(CFG_OAUTH_TOKEN_LIFETIME, s_oauth, "access_token_lifetime",
                     300);

  char *s_http =
      "HTTP";  // Does not apply to supla_google_home_sync_thread_pool
  scfg_add_int_param(CFG_HTTP_THREAD_COUNT_LIMIT, s_http, "thread_count_limit",
                     50);
  scfg_add_int_param(CFG_HTTP_REQUESTS_PER_THREAD, s_http,
                     "requests_per_thread", 10);
  scfg_add_int_param(CFG_HTTP_THREAD_KEEP_ALIVE_SEC, s_http,
                     "thread_keep_alive", 10);
  scfg_add_int_param(CFG_HTTP_KEEP_ALIVE_MAX_THREAD_COUNT, s_http,
                     "keep_alive_max_thread_count", 5);

  scfg_add_int_param(CFG_HTTP_REQUEST_TIMEOUT, s_http, "request_timeout",
                     10000);

  scfg_add_int_param(CFG_HTTP_DELAY_WARNING_TIME, s_http, "delay_warning_time",
                     5000);

  scfg_add_int_param(CFG_HTTP_CONNECTION_TIMEOUT, s_http, "connection_timeout",
                     10);

  scfg_add_int_param(CFG_HTTP_LONG_REQUEST_TIME, s_http, "long_request_time",
                     1000000);

  char *s_alexa = "ALEXA";
  scfg_add_int_param(CFG_ALEXA_RESPONSE_TIMEOUT, s_alexa, "response_timeout",
                     4000);
  scfg_add_int_param(CFG_ALEXA_CHANGEREPORT_TIMEOUT, s_alexa,
                     "changereport_timeout", 30000);

  char *s_google_home = "GOOGLE-HOME";
  scfg_add_int_param(CFG_GOOGLE_HOME_SYNCREQUEST_TIMEOUT, s_google_home,
                     "syncrequest_timeout", 30000);
  scfg_add_int_param(CFG_GOOGLE_HOME_STATEREPORT_TIMEOUT, s_google_home,
                     "statereport_timeout", 30000);

  char *s_push = "PUSH";

  scfg_add_int_param(CFG_PUSH_DEVELOPMENT, s_push, "development", 0);

  char *s_limit = "LIMITS";
  scfg_add_int_param(CFG_LIMIT_CONCURRENT_REGISTRATIONS, s_limit,
                     "concurrent_registrations", 50);
  scfg_add_int_param(CFG_LIMIT_AUTHKEY_AUTH_CACHE_SIZE, s_limit,
                     "authkey_auth_cache_size", 2000);

#ifdef __TEST
  result = scfg_load(argc, argv, "/etc/supla-server/supla-test.cfg");
#else
  result = scfg_load(argc, argv, "/etc/supla-server/supla.cfg");
#endif /*__TEST*/

  scfg_names_free();

  if (result != 0) {
    int n = strnlen(scfg_string(CFG_OAUTH_URL), CFG_OAUTH_URL_MAXSIZE);
    if (n > 0) {
      svrcfg_oauth_url_base64 =
          st_openssl_base64_encode(scfg_string(CFG_OAUTH_URL), n);
      svrcfg_oauth_url_base64_len =
          svrcfg_oauth_url_base64 == NULL
              ? 0
              : strnlen(svrcfg_oauth_url_base64, n * 2);
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
