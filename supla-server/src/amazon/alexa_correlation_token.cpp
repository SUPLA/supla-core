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

#include "alexa_correlation_token.h"
#include "device/value/channel_onoff_value.h"

using std::string;

supla_alexa_correlation_token::supla_alexa_correlation_token(
    const char token[]) {
  subchannel_id = 0;
  set_token(token);
  action_id = 0;
  action_params = nullptr;
}

supla_alexa_correlation_token::~supla_alexa_correlation_token(void) {
  if (action_params) {
    delete action_params;
  }
}

void supla_alexa_correlation_token::set_token(const char token[]) {
  this->token = "";
  subchannel_id = 0;

  if (token) {
    this->token = token;
  }

  size_t sub_pos = this->token.find("::SUB=");
  if (sub_pos != string::npos) {
    subchannel_id = std::stoi(this->token.substr(sub_pos + 6));
    this->token = this->token.substr(0, sub_pos);
  }
}

void supla_alexa_correlation_token::set_action_id(int action_id) {
  this->action_id = action_id;
}

void supla_alexa_correlation_token::set_action_params(
    const supla_abstract_action_parameters *action_params) {
  if (this->action_params) {
    delete this->action_params;
    this->action_params = nullptr;
  }

  if (action_params) {
    this->action_params = action_params->copy();
  }
}

const string &supla_alexa_correlation_token::get_token(void) { return token; }

int supla_alexa_correlation_token::get_subchannel_id(void) {
  return subchannel_id;
}

void supla_alexa_correlation_token::apply_expected(supla_channel_value *value) {
  if ((action_id == ACTION_TURN_ON || action_id == ACTION_TURN_OFF) &&
      dynamic_cast<supla_channel_onoff_value *>(value)) {
    dynamic_cast<supla_channel_onoff_value *>(value)->set_on(action_id ==
                                                             ACTION_TURN_ON);
    return;
  }
}

// static
supla_alexa_correlation_token *supla_alexa_correlation_token::new_token(
    const char token[], int action_id,
    const supla_abstract_action_parameters *action_params) {
  if (token && token[0]) {
    supla_alexa_correlation_token *result =
        new supla_alexa_correlation_token(token);
    result->set_action_id(action_id);
    result->set_action_params(action_params);
    return result;
  }

  return nullptr;
}
