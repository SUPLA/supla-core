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

#ifndef SUPLA_CH_ABSTRACT_EXECUTE_ACTION_H_
#define SUPLA_CH_ABSTRACT_EXECUTE_ACTION_H_

#include <functional>

#include "actions/abstract_action_executor.h"
#include "client/call_handler/abstract_client_srpc_call_handler.h"
#include "client/client.h"

class supla_ch_abstract_execute_action
    : public supla_abstract_client_srpc_call_handler {
 protected:
  void send_result(TCS_Action* action,
                   supla_abstract_srpc_adapter* srpc_adapter, int resultcode);
  void execute_action(
      int user_id, int client_id, supla_abstract_action_executor* aexec,
      TCS_Action* action, supla_abstract_srpc_adapter* srpc_adapter,
      std::function<bool(int subject_type, int subject_id)> subject_exists,
      std::function<bool(int channel_id)> is_channel_online);

 public:
  supla_ch_abstract_execute_action(void);
  virtual ~supla_ch_abstract_execute_action();
};

#endif /* SUPLA_CH_ABSTRACT_EXECUTE_ACTION_H_*/
