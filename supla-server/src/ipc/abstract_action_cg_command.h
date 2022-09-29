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

#ifndef SUPLA_ABSTRACT_ACTION_CG_COMMAND_H_
#define SUPLA_ABSTRACT_ACTION_CG_COMMAND_H_

#include <string>

#include "ipc/abstract_ipc_command.h"
#include "user.h"

class supla_abstract_action_cg_command : public supla_abstract_ipc_command {
 private:
  int action;
  void _send_result(bool success, int channel_id);

 protected:
  std::string get_command_name(int action);
  virtual void on_command_match(const char *params);
  virtual bool action_open_close(supla_user *user, int group_id, bool open) = 0;
  virtual bool action_toggle(supla_user *user, int group_id) = 0;
  virtual bool action_stop(supla_user *user, int group_id) = 0;
  virtual bool action_up_or_stop(supla_user *user, int group_id) = 0;
  virtual bool action_down_or_stop(supla_user *user, int group_id) = 0;
  virtual bool action_step_by_step(supla_user *user, int group_id) = 0;
  virtual bool action_copy(supla_user *user, int group_id, int source_device_id,
                           int source_channel_id) = 0;
  virtual bool action_shut(supla_user *user, int group_id,
                           const char *percentage, bool delta) = 0;

 public:
  explicit supla_abstract_action_cg_command(
      supla_abstract_ipc_socket_adapter *socket_adapter, int action);
  virtual const std::string get_command_name(void);
};

#endif /* SUPLA_ABSTRACT_ACTION_CG_COMMAND_H_ */
