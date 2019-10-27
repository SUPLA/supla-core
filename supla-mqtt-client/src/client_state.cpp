/*
 * client_state.cpp
 *
 *  Created on: 22 paź 2019
 *      Author: Łukasz Bek
 *   Copyright: Łukasz Bek
 */

#include "client_state.h"

client_state::client_state() {
  this->function = 0;
  this->payload = "";
  this->enabled = true;
  this->topic = "";
}

uint16_t client_state::getFunction() { return this->function; }
std::string client_state::getPayload(int id, std::string caption) {
  std::string result = this->payload;
  replace_string_in_place(result, "$id", std::to_string(id));
  replace_string_in_place(result, "$caption", caption);
  return result;
}
std::string client_state::getTopic(int id, int type) {
  std::string result = this->topic;

  replace_string_in_place(result, "$id", std::to_string(id));
  replace_string_in_place(result, "$type", std::to_string(type));

  return result;
}

bool client_state::getEnabled() { return this->enabled; }

void client_state::setFunction(std::uint16_t value) { this->function = value; }
void client_state::setPayload(std::string value) { this->payload = value; }
void client_state::setEnabled(bool value) { this->enabled = value; }
void client_state::setTopic(std::string value) { this->topic = value; }
