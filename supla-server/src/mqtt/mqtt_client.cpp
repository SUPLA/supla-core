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

#include "mqtt_client.h"
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <unistd.h>
#include "log.h"
#include "sthread.h"

typedef struct {
  supla_mqtt_client *instance;
  ssize_t (*__sendall)(supla_mqtt_client *supla_client_instance,
                       const char *buf, size_t len, int flags);
  ssize_t (*__recvall)(supla_mqtt_client *supla_client_instance, char *buf,
                       size_t bufsz, int flags);
} _sendrecv_methods_t;

// static
ssize_t supla_mqtt_client::__mqtt_pal_sendall(
    supla_mqtt_client *supla_client_instance, const char *buf, size_t len,
    int flags) {
  return supla_client_instance->mqtt_pal_sendall(buf, len, flags);
}

// static
ssize_t supla_mqtt_client::__mqtt_pal_recvall(
    supla_mqtt_client *supla_client_instance, char *buf, size_t bufsz,
    int flags) {
  return supla_client_instance->mqtt_pal_recvall(buf, bufsz, flags);
}

ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle supla_client_instance,
                         const void *buf, size_t len, int flags) {
  _sendrecv_methods_t *m =
      static_cast<_sendrecv_methods_t *>(supla_client_instance);
  if (m) {
    return m->__sendall(m->instance, (const char *)buf, len, flags);
  }

  return MQTT_ERROR_SOCKET_ERROR;
}

ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle supla_client_instance,
                         void *buf, size_t bufsz, int flags) {
  _sendrecv_methods_t *m =
      static_cast<_sendrecv_methods_t *>(supla_client_instance);
  if (m) {
    return m->__recvall(m->instance, (char *)buf, bufsz, flags);
  }

  return MQTT_ERROR_SOCKET_ERROR;
}

// static
void supla_mqtt_client::job(void *supla_client_instance, void *sthread) {
  supla_mqtt_client *client =
      static_cast<supla_mqtt_client *>(supla_client_instance);
  if (client) {
    client->job(sthread);
  }
}

// static
void supla_mqtt_client::reconnect(struct mqtt_client *client,
                                  void **supla_client_instance) {
  if (supla_client_instance) {
    supla_mqtt_client *supla_client =
        static_cast<supla_mqtt_client *>(*supla_client_instance);
    if (supla_client) {
      supla_client->reconnect(client);
    }
  }
}

// static
void supla_mqtt_client::on_message_received(
    void **supla_client_instance, struct mqtt_response_publish *message) {
  if (supla_client_instance) {
    supla_mqtt_client *supla_client =
        static_cast<supla_mqtt_client *>(*supla_client_instance);
    if (supla_client) {
      supla_client->on_message_received(message);
    }
  }
}

supla_mqtt_client::supla_mqtt_client(supla_mqtt_client_settings *settings,
                                     supla_mqtt_client_datasource *datasource) {
  this->settings = settings;
  this->datasource = datasource;
  this->sthread = NULL;
  this->sockfd = -1;
  this->sbio = NULL;
}

supla_mqtt_client::~supla_mqtt_client(void) { stop(); }

void supla_mqtt_client::job(void *sthread) {
  struct mqtt_client client;
  memset(&client, 0, sizeof(struct mqtt_client));

  mqtt_init_reconnect(&client, supla_mqtt_client::reconnect, this,
                      supla_mqtt_client::on_message_received);

  _sendrecv_methods_t m;
  m.instance = this;
  m.__recvall = supla_mqtt_client::__mqtt_pal_recvall;
  m.__sendall = supla_mqtt_client::__mqtt_pal_sendall;

  client.socketfd = &m;
  client.publish_response_callback_state = this;
  client.reconnect_state = this;

  while (!sthread_isterminated(sthread)) {
    mqtt_sync(&client);
    usleep(1000);
  }
}

void supla_mqtt_client::connect(void) {
  disconnect();

  if (settings->isSSLEnabled()) {
  } else {
  }
}

void supla_mqtt_client::disconnect(void) {
  if (sockfd != -1) {
  } else if (sbio) {
  }
}

void supla_mqtt_client::reconnect(struct mqtt_client *client) {}

void supla_mqtt_client::on_message_received(
    struct mqtt_response_publish *message) {}

void supla_mqtt_client::start(void) {
  if (sthread == NULL && settings && settings->isMQTTEnabled()) {
    sthread = sthread_simple_run(supla_mqtt_client::job, this, 0);
  }
}

void supla_mqtt_client::stop(void) {
  if (sthread != NULL) {
    sthread_twf(sthread);
    sthread = NULL;
  }
}

ssize_t supla_mqtt_client::mqtt_pal_sendall(const char *buf, size_t len,
                                            int flags) {
  size_t sent = 0;

  if (sockfd != -1) {
    // https://github.com/LiamBindle/MQTT-C/blob/9a7cc93eb09680140ab963e1faecfe3d2f80829c/src/mqtt_pal.c#L261

    while (sent < len) {
      ssize_t tmp = send(sockfd, buf + sent, len - sent, flags);
      if (tmp < 1) {
        return MQTT_ERROR_SOCKET_ERROR;
      }
      sent += (size_t)tmp;
    }

    return sent;
  } else if (sbio != NULL) {
    // https://github.com/LiamBindle/MQTT-C/blob/9a7cc93eb09680140ab963e1faecfe3d2f80829c/src/mqtt_pal.c#L224

    size_t sent = 0;
    while (sent < len) {
      int tmp = BIO_write((BIO *)sbio, (const char *)buf + sent, len - sent);
      if (tmp > 0) {
        sent += (size_t)tmp;
      } else if (tmp <= 0 && !BIO_should_retry((BIO *)sbio)) {
        return MQTT_ERROR_SOCKET_ERROR;
      }
    }
    return sent;
  }

  return MQTT_ERROR_SOCKET_ERROR;
}

ssize_t supla_mqtt_client::mqtt_pal_recvall(char *buf, size_t bufsz,
                                            int flags) {
  char *start = buf;

  if (sockfd != -1) {
    // https://github.com/LiamBindle/MQTT-C/blob/9a7cc93eb09680140ab963e1faecfe3d2f80829c/src/mqtt_pal.c#L273
    ssize_t rv;
    do {
      rv = recv(sockfd, buf, bufsz, flags);
      if (rv > 0) {
        /* successfully read bytes from the socket */
        buf += rv;
        bufsz -= rv;
      } else if (rv < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        /* an error occurred that wasn't "nothing to read". */
        return MQTT_ERROR_SOCKET_ERROR;
      }
    } while (rv > 0);

    return buf - start;
  } else if (sbio != NULL) {
    // https://github.com/LiamBindle/MQTT-C/blob/9a7cc93eb09680140ab963e1faecfe3d2f80829c/src/mqtt_pal.c#L237
    char *bufptr = (char *)buf;
    int rv;
    do {
      rv = BIO_read((BIO *)sbio, bufptr, bufsz);
      if (rv > 0) {
        /* successfully read bytes from the socket */
        bufptr += rv;
        bufsz -= rv;
      } else if (!BIO_should_retry((BIO *)sbio)) {
        /* an error occurred that wasn't "nothing to read". */
        return MQTT_ERROR_SOCKET_ERROR;
      }
    } while (!BIO_should_read((BIO *)sbio));

    return (ssize_t)(bufptr - start);
  }

  return MQTT_ERROR_SOCKET_ERROR;
}
