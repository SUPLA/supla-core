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

#ifndef SUPLA_ACTION_COMMAND_H_
#define SUPLA_ACTION_COMMAND_H_

#include <string>

#include "ipc/abstract_action_command.h"

class supla_action_command : public supla_abstract_action_command {
 protected:
  virtual bool action_open_close(int user_id, int device_id, int channel_id,
                                 bool open, const char *alexa_correlation_token,
                                 const char *google_request_id);
  virtual bool action_turn_on(int user_id, int device_id, int channel_id);
  virtual bool action_turn_off(int user_id, int device_id, int channel_id);
  virtual bool action_toggle(int user_id, int device_id, int channel_id);
  virtual bool action_stop(int user_id, int device_id, int channel_id);
  virtual bool action_up_or_stop(int user_id, int device_id, int channel_id);
  virtual bool action_down_or_stop(int user_id, int device_id, int channel_id);
  virtual bool action_step_by_step(int user_id, int device_id, int channel_id);
  virtual bool action_copy(int user_id, int device_id, int channel_id,
                           int source_device_id, int source_channel_id);
  virtual bool action_shut(int user_id, int device_id, int channel_id,
                           const char *percentage, bool delta);

 public:
  explicit supla_action_command(
      supla_abstract_ipc_socket_adapter *socket_adapter, int action);
};

#endif /* SUPLA_ACTION_COMMAND_H_ */
