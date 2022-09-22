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

#include "client/call_handler/abstract_execute_action.h"

#include "actions/action_executor.h"
#include "log.h"
using std::function;

supla_ch_abstract_execute_action::supla_ch_abstract_execute_action(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_abstract_execute_action::~supla_ch_abstract_execute_action() {}

void supla_ch_abstract_execute_action::send_result(
    TCS_Action* action, supla_abstract_srpc_adapter* srpc_adapter,
    int resultcode) {
  TSC_ActionExecutionResult result = {};
  result.ActionId = action->ActionId;
  result.SubjectId = action->SubjectId;
  result.SubjectType = action->SubjectType;
  result.ResultCode = resultcode;
supla_log(LOG_DEBUG, "Result %i", result.ResultCode);
  srpc_adapter->sc_async_action_execution_result(&result);
}

void supla_ch_abstract_execute_action::execute_action(
    int user_id, int client_id, supla_abstract_action_executor* aexec,
    TCS_Action* action, supla_abstract_srpc_adapter* srpc_adapter,
    function<bool(int subject_type, int subject_id)> subject_exists) {
  if (action->SubjectType == ACTION_SUBJECT_TYPE_UNKNOWN ||
      action->SubjectId == 0 ||
      !subject_exists(action->SubjectType, action->SubjectId)) {
    send_result(action, srpc_adapter, SUPLA_RESULTCODE_SUBJECT_NOT_FOUND);
    return;
  }

  _subjectType_e subject_type = stUnknown;
  char percentage = 0;
  _action_config_rgbw_t rgbw = {};

  switch (action->SubjectType) {
    case ACTION_SUBJECT_TYPE_CHANNEL:
      subject_type = stChannel;
      break;
    case ACTION_SUBJECT_TYPE_CHANNEL_GROUP:
      subject_type = stChannelGroup;
      break;
    case ACTION_SUBJECT_TYPE_SCENE:
      subject_type = stScene;
      break;
  }

  switch (action->ActionId) {
    case ACTION_SHUT_PARTIALLY:
    case ACTION_REVEAL_PARTIALLY:
      if (action->ParamSize == sizeof(TAction_RS_Parameters)) {
        TAction_RS_Parameters* p = (TAction_RS_Parameters*)action->Param;
        percentage = p->Percentage;
      } else {
        send_result(action, srpc_adapter,
                    SUPLA_RESULTCODE_INCORRECT_PARAMETERS);
      }
      break;
    case ACTION_SET_RGBW_PARAMETERS:
      if (action->ParamSize == sizeof(TAction_RGBW_Parameters)) {
        TAction_RGBW_Parameters* p = (TAction_RGBW_Parameters*)action->Param;
        rgbw.brightness = p->Brightness;
        rgbw.color = p->Color;
        rgbw.color_brightness = p->ColorBrightness;
        rgbw.color_random = p->ColorRandom;
      } else {
        send_result(action, srpc_adapter,
                    SUPLA_RESULTCODE_INCORRECT_PARAMETERS);
      }
      break;
  }

  aexec->execute_action(supla_caller(ctClient, client_id), user_id,
                        action->ActionId, subject_type, action->SubjectId,
                        nullptr, percentage, &rgbw, 0, 0, 0);

  send_result(action, srpc_adapter, SUPLA_RESULTCODE_TRUE);
}
