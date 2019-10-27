#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "../supla-client-lib/sthread.h"
#include "mqtt.h"

using namespace std;

struct reconnect_state_t {
  std::string hostname;
  int port;
  std::string username;
  std::string password;
  uint8_t* sendbuf;
  size_t sendbufsz;
  uint8_t* recvbuf;
  size_t recvbufsz;
  vector<std::string> topics;
};

int mqtt_client_init(std::string addr, int port, std::string username,
                     std::string password, vector<std::string>& topics,
                     void (*publish_response_callback)(
                         void** state, struct mqtt_response_publish* publish));

void mqtt_client_free();
void mqtt_client_publish(const char* topic, const char* payload, char retain,
                         char qos);

#endif
