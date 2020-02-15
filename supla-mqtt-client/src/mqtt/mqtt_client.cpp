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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void* mqtt_deamon_thread;
struct reconnect_state_t* reconnect_state;
struct mqtt_client* mq_client;

void reconnect_client(struct mqtt_client* client, void** reconnect_state_vptr);

int open_nb_socket(const char* addr, uint16_t port) {
  struct addrinfo hints = {0};

  hints.ai_family = AF_UNSPEC;     /* IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* Must be TCP */
  int sockfd = -1;
  int rv;
  struct addrinfo *p, *servinfo;

  char port_buffer[6];
  sprintf(port_buffer, "%d", port);

  /* get address information */
  rv = getaddrinfo(addr, port_buffer, &hints, &servinfo);
  if (rv != 0) {
    fprintf(stderr, "Failed to open socket (getaddrinfo): %s\n",
            gai_strerror(rv));
    return -1;
  }

  /* open the first possible socket */
  for (p = servinfo; p != NULL; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) continue;

    /* connect to server */
    rv = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (rv == -1) continue;
    break;
  }

  /* free servinfo */
  freeaddrinfo(servinfo);

  /* make non-blocking */
  if (sockfd != -1) fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);

  /* return the new socket fd */
  return sockfd;
}

void mqtt_client_loop(void* client, void* sthread) {
  while (sthread_isterminated(sthread) == 0) {
    mqtt_sync((struct mqtt_client*)client);
    usleep(100000U);
  }
}

void close_client(int sockfd, pthread_t* client_daemon) {
  if (client_daemon != NULL) pthread_cancel(*client_daemon);
}

int mqtt_client_init(std::string addr, int port, std::string username,
                     std::string password, std::string client_name,
                     uint8_t protocol_version, vector<std::string>& topics,
                     void (*publish_response_callback)(
                         void** state, struct mqtt_response_publish* publish)) {
  reconnect_state = new reconnect_state_t();
  reconnect_state->sendbuf = (uint8_t*)malloc(8192 * sizeof(uint8_t));
  reconnect_state->sendbufsz = 8192 * sizeof(uint8_t);
  reconnect_state->recvbuf = (uint8_t*)malloc(2048 * sizeof(uint8_t));
  reconnect_state->recvbufsz = 2048 * sizeof(uint8_t);
  reconnect_state->hostname = addr;
  reconnect_state->port = port;
  reconnect_state->username = username;
  reconnect_state->password = password;
  reconnect_state->client_name = client_name;

  for (auto topic : topics) reconnect_state->topics.push_back(topic);

  /* setup a client */
  mq_client = new mqtt_client();
  mq_client->protocol_version = protocol_version;

  mqtt_init_reconnect(mq_client, reconnect_client, reconnect_state,
                      publish_response_callback);

  mqtt_deamon_thread =
      sthread_simple_run(mqtt_client_loop, (void*)mq_client, 0);

  return EXIT_SUCCESS;
}

void mqtt_client_publish(const char* topic, const char* payload, char retain,
                         char qos) {
  if (mq_client == NULL || mq_client->error != MQTT_OK) return;

  uint8_t publish_flags = 0;
  if (retain) {
    publish_flags |= MQTT_PUBLISH_RETAIN;
  }

  if (qos == 0)
    publish_flags |= MQTT_PUBLISH_QOS_0;
  else if (qos == 1)
    publish_flags |= MQTT_PUBLISH_QOS_1;
  else if (qos == 2)
    publish_flags |= MQTT_PUBLISH_QOS_2;

  supla_log(LOG_DEBUG, "publishing %s", topic);

  mqtt_publish(mq_client, topic, (void*)payload, strlen(payload),
               publish_flags);
}

void reconnect_client(struct mqtt_client* client, void** reconnect_state_vptr) {
  struct reconnect_state_t* reconnect_state =
      *((struct reconnect_state_t**)reconnect_state_vptr);

  /* Close the clients socket if this isn't the initial reconnect call */
  if (client->error != MQTT_ERROR_INITIAL_RECONNECT) {
    close(client->socketfd);
  }

  /* Perform error handling here. */
  if (client->error != MQTT_ERROR_INITIAL_RECONNECT) {
    cout << "mqtt client error " << mqtt_error_str(client->error) << endl;
    // sleep(5);
  }

  supla_log(LOG_DEBUG, "connecting to %s on port %d using protocol version %d",
            reconnect_state->hostname.c_str(), reconnect_state->port,
            client->protocol_version);

  if (reconnect_state->username.length() > 0) {
    supla_log(LOG_DEBUG, "using credentials %s %s",
              reconnect_state->username.c_str(),
              reconnect_state->password.c_str());
  }

  /* Open a new socket. */
  int sockfd =
      open_nb_socket(reconnect_state->hostname.c_str(), reconnect_state->port);
  if (sockfd == -1) {
    cout << "socket error" << endl;
    sleep(5);
    client->error = MQTT_ERROR_INITIAL_RECONNECT;
    return;
  }

  /* Reinitialize buffers */
  if (client->error != MQTT_ERROR_INITIAL_RECONNECT) {
    free(reconnect_state->sendbuf);
    free(reconnect_state->recvbuf);

    reconnect_state->sendbuf = (uint8_t*)malloc(8192 * sizeof(uint8_t));
    reconnect_state->sendbufsz = 8192 * sizeof(uint8_t);
    reconnect_state->recvbuf = (uint8_t*)malloc(2048 * sizeof(uint8_t));
    reconnect_state->recvbufsz = 2048 * sizeof(uint8_t);
  }

  /* Reinitialize the client. */
  mqtt_reinit(client, sockfd, reconnect_state->sendbuf,
              reconnect_state->sendbufsz, reconnect_state->recvbuf,
              reconnect_state->recvbufsz);

  const char* username = reconnect_state->username.length() > 0
                             ? reconnect_state->username.c_str()
                             : NULL;
  const char* password = reconnect_state->password.length() > 0
                             ? reconnect_state->password.c_str()
                             : NULL;
  const char* client_name = reconnect_state->client_name.c_str();

  /* Send connection request to the broker. */
  mqtt_connect(client, client_name, NULL, NULL, 0, username, password, 0, 400);

  /* Subscribe to the topic. */
  for (auto topic : reconnect_state->topics) {
    supla_log(LOG_DEBUG, "subscribing %s", topic.c_str());
    mqtt_subscribe(client, topic.c_str(), 0);
  }
}

void mqtt_client_free() {
  if (mq_client != NULL) mqtt_disconnect(mq_client);

  if (mqtt_deamon_thread != NULL) {
    sthread_twf(mqtt_deamon_thread);
  }

  if (mq_client != NULL && mq_client->socketfd != -1)
    close(mq_client->socketfd);

  free(reconnect_state->recvbuf);
  free(reconnect_state->sendbuf);
  delete reconnect_state;

  if (mq_client != NULL) delete mq_client;
}
