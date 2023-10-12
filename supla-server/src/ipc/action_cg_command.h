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

#ifndef SUPLA_ACTION_CG_COMMAND_H_
#define SUPLA_ACTION_CG_COMMAND_H_

#include <string>

#include "ipc/abstract_action_cg_command.h"

class supla_action_cg_command : public supla_abstract_action_cg_command {
 protected:
  virtual bool action_open_close(supla_user *user, int group_id, bool open);
  virtual bool action_turn_off(supla_user *user, int group_id);
  virtual bool action_turn_on(supla_user *user, int group_id);
  virtual bool action_toggle(supla_user *user, int group_id);
  virtual bool action_stop(supla_user *user, int group_id);
  virtual bool action_up_or_stop(supla_user *user, int group_id);
  virtual bool action_down_or_stop(supla_user *user, int group_id);
  virtual bool action_step_by_step(supla_user *user, int group_id);
  virtual bool action_copy(supla_user *user, int group_id, int source_device_id,
                           int source_channel_id);
  virtual bool action_shut(supla_user *user, int group_id,
                           const char *percentage, bool delta);
  virtual bool action_set_hvac_parameters(
      supla_user *user, int group_id,
      const supla_action_hvac_parameters *params);

 public:
  explicit supla_action_cg_command(
      supla_abstract_ipc_socket_adapter *socket_adapter, int action);
};

#endif /* SUPLA_ACTION_CG_COMMAND_H_ */
