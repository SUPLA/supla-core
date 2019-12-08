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

#include "client_config.h"
#include <string>

client_config::client_config() {
  this->mqtt_host = "localhost";
  this->mqtt_port = 1883;
  this->mqtt_username = "";
  this->mqtt_password = "";
  this->mqtt_commands = "";
  this->mqtt_states = "";
  this->mqtt_client_name = "supla_mqtt_client";
  this->mqtt_protocol_version = 5;
  this->mqtt_publish_events = true;

  this->supla_host = "localhost";
  this->supla_port = 2016;
  this->supla_accessid = 1;
  this->supla_password = "";
  this->supla_email = "";
  this->supla_protocol_version = 10;
}

client_config::~client_config() {
  for (auto command : commands) {
    delete command;
  }

  commands.clear();
  std::vector<client_command*>().swap(commands);

  for (auto state : states) {
    delete state;
  }

  states.clear();
  std::vector<client_state*>().swap(states);
}

bool client_config::load(const char* config_file) {
  try {
    if (!st_file_exists(config_file)) {
      std::cout << "configuration file missing [parameter -config]"
                << std::endl;
      exit(2);
    }

    Yaml::Node root;
    Yaml::Parse(root, config_file);

    if (!root["mqtt"].IsNone()) {
      this->mqtt_host = root["mqtt"]["host"].As<std::string>("localhost");
      this->mqtt_port = root["mqtt"]["port"].As<uint16_t>(1883);
      this->mqtt_username = root["mqtt"]["username"].As<std::string>("");
      this->mqtt_password = root["mqtt"]["password"].As<std::string>("");
      this->mqtt_commands =
          root["mqtt"]["commands_file_path"].As<std::string>("command.yaml");
      this->mqtt_states =
          root["mqtt"]["states_file_path"].As<std::string>("state.yaml");
      this->mqtt_client_name =
          root["mqtt"]["client_name"].As<std::string>("supla_mqtt_client");
      this->mqtt_protocol_version =
          root["mqtt"]["protocol_version"].As<uint16_t>(5);
      this->mqtt_publish_events =
          root["mqtt"]["publish_supla_events"].As<bool>(true);
    }

    if (!root["supla"].IsNone()) {
      this->supla_host = root["supla"]["host"].As<std::string>("localhost");
      this->supla_password = root["supla"]["password"].As<std::string>("");
      this->supla_accessid = root["supla"]["access_id"].As<uint32_t>(2016);
      this->supla_port = root["supla"]["port"].As<uint16_t>(2016);
      this->supla_email = root["supla"]["email"].As<std::string>("");
      this->supla_protocol_version =
          root["supla"]["protocol_version"].As<uint16_t>(10);
    }

    if (st_file_exists(this->mqtt_commands.c_str())) {
      supla_log(LOG_DEBUG, "reading commands YAML flie");
      Yaml::Node commandsRoot;
      Yaml::Parse(commandsRoot, this->mqtt_commands.c_str());

      for (auto itN = commandsRoot.Begin(); itN != commandsRoot.End(); itN++) {
        Yaml::Node& command = (*itN).second;

        client_command* cc = new client_command();
        cc->setId(command["id"].As<std::string>(""));
        cc->setTopic(
            command["topic"].As<std::string>("supla/channels/command/#"));
        cc->setBrightness(command["brightness"].As<std::string>(""));
        cc->setColor(command["color"].As<std::string>(""));
        cc->setColorB(command["color_b"].As<std::string>(""));
        cc->setColorR(command["color_r"].As<std::string>(""));
        cc->setColorG(command["color_g"].As<std::string>(""));
        cc->setOnOff(command["on_off"].As<std::string>(""));
        cc->setOnValue(command["on_value"].As<std::string>("1"));
        cc->setOffValue(command["off_value"].As<std::string>("0"));
        cc->setShut(command["shut"].As<std::string>(""));
        cc->setColorBrightness(command["color_brightness"].As<std::string>(""));

        this->commands.push_back(cc);
      }

    } else {
      supla_log(LOG_INFO, "command file not exists.");
      return false;
    }

    if (st_file_exists(this->mqtt_states.c_str())) {
      Yaml::Node statesRoot;
      Yaml::Parse(statesRoot, this->mqtt_states.c_str());

      bool enabled = true;
      std::string default_topic =
          statesRoot["default_state_topic"].As<std::string>("");

      for (auto itN = statesRoot["channels"].Begin();
           itN != statesRoot["channels"].End(); itN++) {
        Yaml::Node& state = (*itN).second;
        client_state* cs = new client_state();
        cs->setFunction(state["channel_type"].As<int>(0));
        cs->setPayload(state["payload_template"].As<std::string>(""));
        cs->setEnabled(state["enabled"].As<bool>(enabled));
        cs->setTopic(state["state_topic"].As<std::string>(default_topic));

        this->states.push_back(cs);
      }
    } else {
      supla_log(LOG_INFO, "states file not exists");
      return false;
    }
    return true;
  } catch (std::exception& exception) {
    std::cout << exception.what() << std::endl;
    return false;
  }
}

void client_config::getTopicsToSubscribe(std::vector<std::string>* vect) {
  for (auto command : this->commands) {
    std::string topic = command->getTopic();
    if (!exists_in_vect(vect, topic)) vect->push_back(topic);
  }
}

bool client_config::getMqttPublishEvents() { return this->mqtt_publish_events; }

std::string client_config::getMqttCommands() { return this->mqtt_commands; }

std::string client_config::getMqttStates() { return this->mqtt_states; }

void client_config::getCommandsForTopic(std::string topic,
                                        std::vector<client_command*>* output) {
  std::vector<std::string> topic_tokens;
  std::string token;
  std::istringstream tokenStream(topic);
  while (std::getline(tokenStream, token, '/')) {
    topic_tokens.push_back(token);
  }

  for (auto command : this->commands) {
    std::vector<std::string> command_tokens;
    std::istringstream tokenStream(command->getTopic());

    while (std::getline(tokenStream, token, '/')) {
      command_tokens.push_back(token);
    }

    bool added = false;

    if (topic_tokens.size() < command_tokens.size()) continue;
    unsigned int i = 0;

    for (i = 0; i < command_tokens.size(); i++) {
      if (command_tokens[i].compare("#") == 0) {
        output->push_back(command);
        added = true;
      }

      if (command_tokens[i].compare("+") == 0) continue;
      if (command_tokens[i].compare(topic_tokens[i]) != 0) break;
    }

    if (!added && (i == command_tokens.size() - 1)) {
      output->push_back(command);
    }
  }
}

void client_config::getStatesForFunction(uint16_t function,
                                         std::vector<client_state*>* output) {
  for (auto state : this->states) {
    if (state->getFunction() == function && state->getEnabled())
      output->push_back(state);
  }
}

std::string client_config::getMqttClientName() {
  return this->mqtt_client_name;
}
std::string client_config::getMqttHost() { return this->mqtt_host; }
std::string client_config::getMqttUsername() { return this->mqtt_username; }
std::string client_config::getMqttPassword() { return this->mqtt_password; }
int client_config::getMqttPort() { return this->mqtt_port; }
const uint16_t client_config::getMqttProtocolVersion() {
  return this->mqtt_protocol_version;
}
std::string client_config::getSuplaHost() { return this->supla_host; }
std::string client_config::getSuplaPassword() { return this->supla_password; }
const uint16_t client_config::getSuplaPort() { return this->supla_port; }
const uint32_t client_config::getSuplaAccessId() {
  return this->supla_accessid;
}
const uint16_t client_config::getSuplaProtocolVersion() {
  return this->supla_protocol_version;
}

std::string client_config::getSuplaEmail() { return this->supla_email; }
