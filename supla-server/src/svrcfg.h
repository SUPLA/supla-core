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

#ifndef suplasvrcfg_H_
#define suplasvrcfg_H_

#include "cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_OAUTH_URL_MAXSIZE 96
#define CFG_OAUTH_TOKEN_SIZE 86

#define SERVER_VERSION "22.12.02"

#define CFG_UID 0
#define CFG_GID 1

#define CFG_TCP_PORT 2
#define CFG_TCP_ENABLED 3

#define CFG_SSL_PORT 4
#define CFG_SSL_ENABLED 5
#define CFG_SSL_CERT 6
#define CFG_SSL_KEY 7

#define CFG_MYSQL_HOST 8
#define CFG_MYSQL_PORT 9
#define CFG_MYSQL_DB 10
#define CFG_MYSQL_USER 11
#define CFG_MYSQL_PASSWORD 12

#define CFG_IPC_SOCKET_PATH 13

#define CFG_OAUTH_URL 14
#define CFG_OAUTH_TOKEN_LIFETIME 15

#define CFG_HTTP_THREAD_COUNT_LIMIT 16
#define CFG_HTTP_REQUESTS_PER_THREAD 17
#define CFG_HTTP_THREAD_KEEP_ALIVE_SEC 18
#define CFG_HTTP_KEEP_ALIVE_MAX_THREAD_COUNT 19
#define CFG_HTTP_REQUEST_TIMEOUT 20
#define CFG_HTTP_DELAY_WARNING_TIME 21
#define CFG_HTTP_CONNECTION_TIMEOUT 22
#define CFG_HTTP_LONG_REQUEST_TIME 23
#define CFG_ALEXA_RESPONSE_TIMEOUT 24
#define CFG_ALEXA_CHANGEREPORT_TIMEOUT 25

#define CFG_GOOGLE_HOME_SYNCREQUEST_TIMEOUT 26
#define CFG_GOOGLE_HOME_STATEREPORT_TIMEOUT 27

#define CFG_PUSH_DEVELOPMENT 28

#define CFG_LIMIT_CONCURRENT_REGISTRATIONS 29
#define CFG_LIMIT_AUTHKEY_AUTH_CACHE_SIZE 30

#define CFG_MQTT_ENABLED 31
#define CFG_MQTT_HOST 32
#define CFG_MQTT_PORT 33
#define CFG_MQTT_USERNAME 34
#define CFG_MQTT_PASSWORD 35
#define CFG_MQTT_PREFIX 36
#define CFG_MQTT_SSL 37
#define CFG_MQTT_CLIENTID 38
#define CFG_MQTT_KEEP_ALIVE_SEC 39

extern char* svrcfg_oauth_url_base64;
extern int svrcfg_oauth_url_base64_len;

unsigned char svrcfg_init(int argc, char* argv[]);
void svrcfg_free(void);

#ifdef __cplusplus
}
#endif

#endif /* suplasvrcfg_H_ */
