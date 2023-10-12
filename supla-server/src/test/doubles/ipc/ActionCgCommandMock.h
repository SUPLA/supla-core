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

#ifndef ACTION_CG_COMMMAND_MOCK_H_
#define ACTION_CG_COMMMAND_MOCK_H_

#include <gmock/gmock.h>

#include "ipc/abstract_action_cg_command.h"

namespace testing {

class ActionCgCommandMock : public supla_abstract_action_cg_command {
 public:
  explicit ActionCgCommandMock(
      supla_abstract_ipc_socket_adapter *socket_adapter, int action);
  MOCK_METHOD3(action_open_close,
               bool(supla_user *user, int group_id, bool open));
  MOCK_METHOD2(action_turn_on, bool(supla_user *user, int group_id));
  MOCK_METHOD2(action_turn_off, bool(supla_user *user, int group_id));
  MOCK_METHOD2(action_toggle, bool(supla_user *user, int group_id));
  MOCK_METHOD2(action_stop, bool(supla_user *user, int group_id));
  MOCK_METHOD2(action_up_or_stop, bool(supla_user *user, int group_id));
  MOCK_METHOD2(action_down_or_stop, bool(supla_user *user, int group_id));
  MOCK_METHOD2(action_step_by_step, bool(supla_user *user, int group_id));
  MOCK_METHOD4(action_copy, bool(supla_user *user, int group_id,
                                 int source_device_id, int source_channel_id));
  MOCK_METHOD4(action_shut, bool(supla_user *user, int group_id,
                                 const char *percentage, bool delta));
  MOCK_METHOD3(action_set_hvac_parameters,
               bool(supla_user *user, int group_id,
                    const supla_action_hvac_parameters *params));
};

} /* namespace testing */

#endif /* ACTION_CG_COMMMAND_MOCK_H_ */
