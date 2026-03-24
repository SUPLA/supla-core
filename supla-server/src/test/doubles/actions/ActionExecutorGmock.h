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

#ifndef ACTION_EXECUTOR_GMOCK_H_
#define ACTION_EXECUTOR_GMOCK_H_

#include <gmock/gmock.h>

#include <list>
#include <map>
#include <memory>
#include <string>

#include "actions/abstract_action_executor.h"

namespace testing {

class ActionExecutorGmock : public supla_abstract_action_executor {
 public:
  ActionExecutorGmock();
  virtual ~ActionExecutorGmock();

  MOCK_METHOD2(on_correlation_token, void(const char *alexa_correlation_token,
                                          const char *google_request_id));

  MOCK_METHOD2(set_on, void(bool on, unsigned long long duration_ms));
  MOCK_METHOD1(set_color, void(unsigned int color));

  MOCK_METHOD1(set_brightness, void(char brightness));
  MOCK_METHOD1(set_color_brightness, void(char brightness));
  MOCK_METHOD6(set_rgbw, void(unsigned int *color, char *color_brightness,
                              char *brightness, char *on_off, char *command,
                              char *white_temperature));
  MOCK_METHOD0(toggle, void(void));
  MOCK_METHOD1(shut,
               void(const supla_action_shading_system_parameters *params));
  MOCK_METHOD0(reveal, void(void));
  MOCK_METHOD0(up, void(void));
  MOCK_METHOD0(down, void(void));
  MOCK_METHOD0(up_or_stop, void(void));
  MOCK_METHOD0(down_or_stop, void(void));
  MOCK_METHOD0(step_by_step, void(void));
  MOCK_METHOD0(enable, void(void));
  MOCK_METHOD0(disable, void(void));
  MOCK_METHOD2(send,
               void(const supla_caller &caller, nlohmann::json *template_data));
  MOCK_METHOD0(execute, void(void));
  MOCK_METHOD0(interrupt, void(void));
  MOCK_METHOD0(interrupt_and_execute, void(void));
  MOCK_METHOD0(stop, void(void));
  MOCK_METHOD0(open, void(void));
  MOCK_METHOD0(close, void(void));
  MOCK_METHOD0(open_close, void(void));
  MOCK_METHOD0(open_close_without_canceling_tasks, void(void));
  MOCK_METHOD1(forward_outside, void(int cap));
  MOCK_METHOD1(hvac_set_parameters, void(supla_action_hvac_parameters *params));
  MOCK_METHOD0(hvac_switch_to_program_mode, void(void));
  MOCK_METHOD0(hvac_switch_to_manual_mode, void(void));
  MOCK_METHOD1(hvac_set_temperature,
               void(supla_action_hvac_setpoint_temperature *temperature));
  MOCK_METHOD1(hvac_set_temperatures,
               void(supla_action_hvac_setpoint_temperatures *temperatures));
};

} /* namespace testing */

#endif /* ACTION_EXECUTOR_GMOCK_H_ */
