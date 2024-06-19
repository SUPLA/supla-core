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

#ifndef supla_socket_H_
#define supla_socket_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

typedef void supla_socket;
typedef void supla_socket_data;

supla_socket_data *ssocket_server_init(const char cert[], const char key[],
                                       int port, unsigned char secure);
void ssocket_ssl_new(void *_ssd, void *_supla_socket);
char ssocket_accept(void *_ssd, unsigned int *ipv4, void **_supla_socket);
char ssocket_accept_ssl(void *_ssd, void *_supla_socket);
supla_socket_data *ssocket_client_init(const char host[], int port,
                                       unsigned char secure);
unsigned char ssocket_client_connect(void *ssd, const char *state_file,
                                     int *err, int conn_timeout_ms);

char ssocket_openlistener(void *_ssd);

int ssocket_read(void *_ssd, void *supla_socket, void *buf, int count);
int ssocket_write(void *_ssd, void *supla_socket, const void *buf, int count);

int ssocket_get_fd(void *ssd);
char ssocket_is_secure(void *_ssd);

void ssocket_supla_socket_close(void *supla_socket);
void ssocket_supla_socket__close(void *_ssd);
void ssocket_supla_socket_free(void *_supla_socket);
int ssocket_supla_socket_getsfd(void *_supla_socket);
void ssocket_close(void *ssd);
void ssocket_free(void *ssd);
void ssocket_ssl_error_log(void);
int ssocket_get_last_accept_errno(void);
long int ssocket_get_last_accept_error_time_sec(void);

#ifdef __cplusplus
}
#endif

#endif /* supla_socket_H_ */
