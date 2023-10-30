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

#ifndef SUPLA_ABSTRACT_ACTION_COMMAND_H_
#define SUPLA_ABSTRACT_ACTION_COMMAND_H_

#include <string>

#include "actions/action_hvac_parameters.h"
#include "actions/action_hvac_setpoint_temperature.h"
#include "actions/action_hvac_setpoint_temperatures.h"
#include "ipc/abstract_ipc_command.h"

class supla_abstract_action_command : public supla_abstract_ipc_command {
 private:
  int action;
  void _send_result(bool success, int channel_id);

 protected:
  std::string get_command_name(int action);
  virtual void on_command_match(const char *params);
  virtual bool action_open_close(int user_id, int device_id, int channel_id,
                                 bool open, const char *alexa_correlation_token,
                                 const char *google_request_id) = 0;
  virtual bool action_turn_on(int user_id, int device_id, int channel_id) = 0;
  virtual bool action_turn_off(int user_id, int device_id, int channel_id) = 0;
  virtual bool action_toggle(int user_id, int device_id, int channel_id) = 0;
  virtual bool action_stop(int user_id, int device_id, int channel_id) = 0;
  virtual bool action_up_or_stop(int user_id, int device_id,
                                 int channel_id) = 0;
  virtual bool action_down_or_stop(int user_id, int device_id,
                                   int channel_id) = 0;
  virtual bool action_step_by_step(int user_id, int device_id,
                                   int channel_id) = 0;
  virtual bool action_copy(int user_id, int device_id, int channel_id,
                           int source_device_id, int source_channel_id) = 0;

  virtual bool action_shut(int user_id, int device_id, int channel_id,
                           const char *percentage, bool delta) = 0;
  virtual bool action_hvac_set_parameters(
      int user_id, int device_id, int channel_id,
      const supla_action_hvac_parameters *params) = 0;
  virtual bool action_hvac_switch_to_manual_mode(int user_id, int device_id,
                                                 int channel_id) = 0;
  virtual bool action_hvac_switch_to_program_mode(int user_id, int device_id,
                                                  int channel_id) = 0;
  virtual bool action_hvac_set_temperature(
      int user_id, int device_id, int channel_id,
      const supla_action_hvac_setpoint_temperature *temperature) = 0;
  virtual bool action_hvac_set_temperatures(
      int user_id, int device_id, int channel_id,
      const supla_action_hvac_setpoint_temperatures *temperatures) = 0;

 public:
  explicit supla_abstract_action_command(
      supla_abstract_ipc_socket_adapter *socket_adapter, int action);
  virtual const std::string get_command_name(void);
};

#endif /* SUPLA_ABSTRACT_ACTION_COMMAND_H_ */
