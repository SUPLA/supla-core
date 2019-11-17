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
#include "../supla-client-lib/log.h"
#include "../supla-client-lib/sthread.h"
#include "mqtt.h"

using namespace std;

struct reconnect_state_t {
  std::string hostname;
  int port;
  std::string username;
  std::string password;
  std::string client_name;
  uint8_t* sendbuf;
  size_t sendbufsz;
  uint8_t* recvbuf;
  size_t recvbufsz;
  vector<std::string> topics;
};

int mqtt_client_init(std::string addr, int port, std::string username,
                     std::string password, std::string client_name,
                     uint8_t protocol_version, vector<std::string>& topics,
                     void (*publish_response_callback)(
                         void** state, struct mqtt_response_publish* publish));

void mqtt_client_free();
void mqtt_client_publish(const char* topic, const char* payload, char retain,
                         char qos);

#endif
