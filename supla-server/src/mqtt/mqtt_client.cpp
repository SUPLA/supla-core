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
#include <fcntl.h>
#include <netdb.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "log.h"
#include "sthread.h"

#define SEND_BUFFER_SIZE 8192
#define RECV_BUFFER_SIZE 262144

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
  this->eh = NULL;
  this->recvbuf = NULL;
  this->sendbuf = NULL;
  this->unable_to_connect_notified = false;
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

  client.socketfd = &m;  // !This is not socketfd!
  client.publish_response_callback_state = this;
  client.reconnect_state = this;

  while (!sthread_isterminated(sthread)) {
    mqtt_sync(&client);
    eh_wait(eh, 1000000);
    on_iterate();
  }

  disconnect();
}

bool supla_mqtt_client::posix_connect(void) {
  // The source of this code fragment
  // https://github.com/LiamBindle/MQTT-C/blob/9a7cc93eb09680140ab963e1faecfe3d2f80829c/examples/templates/posix_sockets.h#L16
  struct addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;     /* IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* Must be TCP */
  sockfd = -1;
  int rv;
  struct addrinfo *p, *servinfo;

  char port[15];
  snprintf(port, sizeof(port), "%i", settings->getPort());

  /* get address information */
  rv = getaddrinfo(settings->getHost(), port, &hints, &servinfo);
  if (rv != 0) {
    supla_log(LOG_ERR, "MQTT: Failed to open socket (getaddrinfo): %s\n",
              gai_strerror(rv));
    return false;
  }

  /* open the first possible socket */
  for (p = servinfo; p != NULL; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) continue;

    /* connect to server */
    rv = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (rv == -1) {
      close(sockfd);
      sockfd = -1;
      continue;
    }
    break;
  }

  /* free servinfo */
  freeaddrinfo(servinfo);

  /* make non-blocking */
  if (sockfd != -1) {
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);
    unable_to_connect_notified = false;
    return true;
  }

  if (!unable_to_connect_notified) {
    supla_log(LOG_ERR, "MQTT: Can't connect to %s", settings->getHost());
    unable_to_connect_notified = true;
  }

  return false;
}

bool supla_mqtt_client::ssl_connect(void) { return false; }

void supla_mqtt_client::disconnect(void) {
  if (eh != NULL) {
    eh_free(eh);
    eh = NULL;
  }

  if (sbio) {
  } else if (sockfd != -1) {
    close(sockfd);
    sockfd = -1;
  }
}

void supla_mqtt_client::reconnect(struct mqtt_client *client) {
  if (client->error != MQTT_ERROR_INITIAL_RECONNECT) {
    if (sockfd != -1) {
      supla_log(LOG_ERR, "%s", mqtt_error_str(client->error));
    }
    usleep(5000000);
  }

  disconnect();

  if (settings->isSSLEnabled()) {
    ssl_connect();
  } else {
    posix_connect();
  }

  if (sockfd != -1) {
    if (sendbuf == NULL) {
      sendbuf = malloc(SEND_BUFFER_SIZE);
    }

    if (recvbuf == NULL) {
      recvbuf = malloc(RECV_BUFFER_SIZE);
    }

    if (eh == NULL) {
      eh = eh_init();
      eh_add_fd(eh, sockfd);
    }

    mqtt_reinit(client, client->socketfd, (uint8_t *)sendbuf, SEND_BUFFER_SIZE,
                (uint8_t *)recvbuf, RECV_BUFFER_SIZE);

    char clientId[CLIENTID_MAX_SIZE];
    getClientId(clientId, sizeof(clientId));

    mqtt_connect(client, clientId, NULL, NULL, 0, settings->getUsername(),
                 settings->getPassword(), MQTT_CONNECT_CLEAN_SESSION,
                 settings->getKeepAlive());

    on_connected();
  }

  eh_raise_event(eh);
}

void supla_mqtt_client::on_message_received(
    struct mqtt_response_publish *message) {}

void supla_mqtt_client::on_connected(void) {}

void supla_mqtt_client::on_iterate(void) {}

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

  if (recvbuf != NULL) {
    free(recvbuf);
    recvbuf = NULL;
  }

  if (sendbuf != NULL) {
    free(sendbuf);
    sendbuf = NULL;
  }
}

ssize_t supla_mqtt_client::mqtt_pal_sendall(const char *buf, size_t len,
                                            int flags) {
  size_t sent = 0;

  if (sbio != NULL) {
    // The source of this code fragment
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
  } else if (sockfd != -1) {
    // The source of this code fragment
    // https://github.com/LiamBindle/MQTT-C/blob/9a7cc93eb09680140ab963e1faecfe3d2f80829c/src/mqtt_pal.c#L261

    while (sent < len) {
      ssize_t tmp = send(sockfd, buf + sent, len - sent, flags);
      if (tmp < 1) {
        return MQTT_ERROR_SOCKET_ERROR;
      }
      sent += (size_t)tmp;
    }

    return sent;
  }

  return MQTT_ERROR_SOCKET_ERROR;
}

ssize_t supla_mqtt_client::mqtt_pal_recvall(char *buf, size_t bufsz,
                                            int flags) {
  char *start = buf;

  if (sbio != NULL) {
    // The source of this code fragment
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
  } else if (sockfd != -1) {
    // The source of this code fragment
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
  }

  return MQTT_ERROR_SOCKET_ERROR;
}
