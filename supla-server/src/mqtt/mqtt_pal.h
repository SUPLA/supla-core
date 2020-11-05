#ifndef __MQTT_PAL_H__
#define __MQTT_PAL_H__

/*
MIT License

Copyright(c) 2018 Liam Bindle

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#define MQTT_PAL_HTONS(s) htons(s)
#define MQTT_PAL_NTOHS(s) ntohs(s)

#define MQTT_PAL_TIME() time(NULL)

typedef time_t mqtt_pal_time_t;
typedef void* mqtt_pal_mutex_t;
typedef void* mqtt_pal_socket_handle;

#define MQTT_PAL_MUTEX_INIT(mtx_ptr)
#define MQTT_PAL_MUTEX_LOCK(mtx_ptr)
#define MQTT_PAL_MUTEX_UNLOCK(mtx_ptr)

ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle supla_client_instance,
                         const void* buf, size_t len, int flags);
ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle supla_client_instance,
                         void* buf, size_t bufsz, int flags);

#ifdef __cplusplus
}
#endif

#endif /*__MQTT_PAL_H__*/
