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

#include "client_state.h"

client_state::client_state() {
  this->function = 0;
  this->payload = "";
  this->enabled = true;
  this->topic = "";
}

uint16_t client_state::getFunction() { return this->function; }
std::string client_state::getPayload(int id, std::string caption, bool online) {
  std::string result = this->payload;
  replace_string_in_place(&result, "$id$", std::to_string(id));
  replace_string_in_place(&result, "$caption$", caption);
  replace_string_in_place(&result, "$online$", std::to_string(online));
  return result;
}
std::string client_state::getTopic(int id, int type) {
  std::string result = this->topic;

  replace_string_in_place(&result, "$id$", std::to_string(id));
  replace_string_in_place(&result, "$type$", std::to_string(type));

  return result;
}

bool client_state::getEnabled() { return this->enabled; }

void client_state::setFunction(std::uint16_t value) { this->function = value; }
void client_state::setPayload(std::string value) { this->payload = value; }
void client_state::setEnabled(bool value) { this->enabled = value; }
void client_state::setTopic(std::string value) { this->topic = value; }
