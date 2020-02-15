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

#ifndef CLIENT_CONFIG_H_
#define CLIENT_CONFIG_H_

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "client_command.h"
#include "client_state.h"
#include "supla-client-lib/log.h"
#include "supla-client-lib/tools.h"
#include "yaml/yaml.h"

class client_config {
 private:
  std::string mqtt_host;
  int mqtt_port;
  std::string mqtt_username;
  std::string mqtt_password;
  std::string mqtt_commands;
  std::string mqtt_states;
  std::string mqtt_client_name;
  bool mqtt_publish_events;
  uint16_t mqtt_protocol_version;

  std::string supla_host;
  uint16_t supla_port;
  uint32_t supla_accessid;
  std::string supla_password;
  std::string supla_email;
  uint16_t supla_protocol_version;

  std::vector<client_command *> commands;
  std::vector<client_state *> states;

 public:
  client_config();
  virtual ~client_config();
  bool load(const char *config_file);

  std::string getMqttHost();
  std::string getMqttUsername();
  std::string getMqttPassword();
  std::string getMqttCommands();
  std::string getMqttStates();
  std::string getMqttClientName();
  bool getMqttPublishEvents();
  const uint16_t getMqttProtocolVersion();

  int getMqttPort();
  std::string getSuplaHost();
  std::string getSuplaPassword();
  const uint16_t getSuplaPort();
  const uint32_t getSuplaAccessId();
  std::string getSuplaEmail();
  const uint16_t getSuplaProtocolVersion();

  void getCommandsForTopic(std::string topic,
                           std::vector<client_command *> *output);
  void getStatesForFunction(uint16_t function,
                            std::vector<client_state *> *output);
  void getTopicsToSubscribe(std::vector<std::string> *vect);
};

#endif /* CLIENT_CONFIG_H_ */
