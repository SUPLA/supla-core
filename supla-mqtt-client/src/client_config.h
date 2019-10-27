/*
 * client_config.h
 *
 *  Created on: 17 paź 2019
 *      Author: Łukasz Bek
 *   Copyright: Łukasz Bek
 */

#ifndef CLIENT_CONFIG_H_
#define CLIENT_CONFIG_H_

#include <stdio.h>
#include <string.h>
#include <vector>

#include "client_command.h"
#include "client_state.h"
#include "supla-client-lib/log.h"
#include "supla-client-lib/tools.h"
#include "yaml/yaml.h"

using namespace std;

class client_config {
 private:
  std::string mqtt_host;
  int mqtt_port;
  std::string mqtt_username;
  std::string mqtt_password;
  std::string mqtt_commands;
  std::string mqtt_states;

  std::string supla_host;
  uint16_t supla_port;
  uint32_t supla_locationid;
  std::string supla_password;
  std::string supla_email;

  std::vector<client_command*> commands;
  std::vector<client_state*> states;

 public:
  client_config();
  virtual ~client_config();
  void load(const char* config_file);

  std::string getMqttHost();
  std::string getMqttUsername();
  std::string getMqttPassword();
  std::string getMqttCommands();
  std::string getMqttStates();

  int getMqttPort();
  std::string getSuplaHost();
  std::string getSuplaPassword();
  const uint16_t getSuplaPort();
  const uint32_t getSuplaLocationId();
  std::string getSuplaEmail();
  void getCommandsForTopic(std::string topic,
                           std::vector<client_command*>& output);
  void getStatesForFunction(uint16_t function,
                            std::vector<client_state*>& output);
  void getTopicsToSubscribe(vector<std::string>& vect);
};

#endif /* CLIENT_CONFIG_H_ */
