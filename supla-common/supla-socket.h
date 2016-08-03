/*
 ============================================================================
 Name        : supla-socket.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef supla_socket_H_
#define supla_socket_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

typedef void supla_socket;
typedef void supla_socket_data;

supla_socket_data *ssocket_server_init(const char cert[], const char key[], int port, unsigned char secure);
char ssocket_accept(void *_ssd, unsigned int *ipv4, void **_supla_socket);
char ssocket_accept_ssl(void *_ssd, void *_supla_socket);
supla_socket_data *ssocket_client_init(const char host[], int port, unsigned char secure);
unsigned char ssocket_client_connect(void *ssd, const char *state_file, int *err);

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

#ifdef __cplusplus
}
#endif

#endif /* supla_socket_H_ */
