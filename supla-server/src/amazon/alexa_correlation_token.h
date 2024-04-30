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

#ifndef ALEXA_CORRELATION_TOKEN_H_
#define ALEXA_CORRELATION_TOKEN_H_

#include <string>

#include "actions/abstract_action_parameters.h"
#include "device/value/channel_value.h"

class supla_alexa_correlation_token {
 private:
  std::string token;
  int action_id;
  supla_abstract_action_parameters *action_params;
  int subchannel_id;

 public:
  supla_alexa_correlation_token(const char token[]);
  ~supla_alexa_correlation_token(void);
  void set_token(const char token[]);
  void set_action_id(int action_id);
  void set_action_params(const supla_abstract_action_parameters *action_params);
  const std::string &get_token(void);
  int get_subchannel_id(void);
  void apply_expected(supla_channel_value *value);
  static supla_alexa_correlation_token *new_token(
      const char token[], int action_id,
      const supla_abstract_action_parameters *action_params);
};

#endif /* ALEXA_CORRELATION_TOKEN_H_ */
