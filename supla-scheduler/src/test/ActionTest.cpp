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

#include "ActionTest.h"

#include <list>

#include "action.h"
#include "action_switch_to_manual_mode.h"
#include "action_switch_to_program_mode.h"
#include "doubles/WorkerMock.h"

namespace testing {

namespace {

class TestableSwitchToProgramMode
    : public s_worker_action_switch_to_program_mode {
 public:
  explicit TestableSwitchToProgramMode(s_abstract_worker *worker)
      : s_worker_action_switch_to_program_mode(worker) {}

  bool is_allowed() { return is_action_allowed(); }
  bool is_success() { return result_success(NULL); }
};

class TestableSwitchToManualMode
    : public s_worker_action_switch_to_manual_mode {
 public:
  explicit TestableSwitchToManualMode(s_abstract_worker *worker)
      : s_worker_action_switch_to_manual_mode(worker) {}

  bool is_success() { return result_success(NULL); }
};

}  // namespace

ActionTest::ActionTest() {}

ActionTest::~ActionTest() {}

TEST_F(ActionTest, time) {
  WorkerMock worker(NULL);
  for (auto it = AbstractActionFactory::factories.begin();
       it != AbstractActionFactory::factories.end(); it++) {
    s_worker_action *action = (*it)->create(&worker);
    ASSERT_FALSE(action == NULL);

    EXPECT_GE(action->waiting_time_to_retry(), MIN_RETRY_TIME);
    EXPECT_GE(action->waiting_time_to_check(), MIN_CHECK_TIME);

    int diff =
        action->waiting_time_to_retry() - action->waiting_time_to_check();
    EXPECT_GT(diff, 0);

    EXPECT_LT(action->get_max_time(), 280);  // Max time 4 min 40 sec.

    delete action;
  }
}

TEST_F(ActionTest, switchToModeEligibilityAndConfirmation) {
  WorkerMock worker(NULL);
  int func = 0;
  EXPECT_CALL(worker, get_channel_func())
      .WillRepeatedly([&func]() { return func; });
  TestableSwitchToProgramMode action(&worker);

  const int relay_and_button_functions[] = {
      SUPLA_CHANNELFNC_LIGHTSWITCH,
      SUPLA_CHANNELFNC_POWERSWITCH,
      SUPLA_CHANNELFNC_STAIRCASETIMER,
      SUPLA_CHANNELFNC_CONTROLLINGTHEGATE,
      SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK,
      SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR,
      SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK,
      SUPLA_CHANNELFNC_ACTIONTRIGGER,
  };

  for (int allowed_func : relay_and_button_functions) {
    func = allowed_func;
    EXPECT_TRUE(action.is_allowed());
    EXPECT_EQ(action.try_limit(), 2);
  }

  func = SUPLA_CHANNELFNC_HVAC_THERMOSTAT;
  EXPECT_TRUE(action.is_allowed());
  EXPECT_EQ(action.try_limit(), 2);

  func = SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER;
  EXPECT_TRUE(action.is_allowed());
  EXPECT_EQ(action.try_limit(), 2);

  func = SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS;
  EXPECT_TRUE(action.is_allowed());
  EXPECT_EQ(action.try_limit(), 2);

  func = SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL;
  EXPECT_TRUE(action.is_allowed());
  EXPECT_EQ(action.try_limit(), 2);

  func = SUPLA_CHANNELFNC_DIMMER;
  EXPECT_FALSE(action.is_allowed());
}

TEST_F(ActionTest, switchToModeChecksRelayAndButtonFlags) {
  WorkerMock worker(NULL);
  int func = SUPLA_CHANNELFNC_LIGHTSWITCH;
  EXPECT_CALL(worker, get_channel_func())
      .WillRepeatedly([&func]() { return func; });
  EXPECT_CALL(worker, ipcc_get_hvac_value).Times(0);

  TestableSwitchToProgramMode program(&worker);
  TestableSwitchToManualMode manual(&worker);

  EXPECT_CALL(worker, ipcc_get_relay_value)
      .WillOnce([](TRelayChannel_Value *value) {
        value->flags = SUPLA_RELAY_FLAG_WEEKLY_SCHEDULE_ENABLED;
        return true;
      })
      .WillOnce([](TRelayChannel_Value *value) {
        value->flags = SUPLA_RELAY_FLAG_WEEKLY_SCHEDULE_ENABLED;
        return true;
      })
      .WillOnce([](TRelayChannel_Value *value) { return true; })
      .WillOnce([](TRelayChannel_Value *value) { return true; })
      .WillOnce([](TRelayChannel_Value *value) { return false; });

  EXPECT_TRUE(program.is_success());
  EXPECT_FALSE(manual.is_success());
  EXPECT_TRUE(manual.is_success());
  EXPECT_FALSE(program.is_success());
  EXPECT_FALSE(program.is_success());

  func = SUPLA_CHANNELFNC_ACTIONTRIGGER;
  EXPECT_CALL(worker, ipcc_get_action_trigger_value)
      .WillOnce([](TActionTriggerProperties *value) {
        value->Flags = SUPLA_ACTION_TRIGGER_FLAG_WEEKLY_SCHEDULE_ENABLED;
        return true;
      })
      .WillOnce([](TActionTriggerProperties *value) {
        value->Flags = SUPLA_ACTION_TRIGGER_FLAG_WEEKLY_SCHEDULE_ENABLED;
        return true;
      })
      .WillOnce([](TActionTriggerProperties *value) { return true; })
      .WillOnce([](TActionTriggerProperties *value) { return true; })
      .WillOnce([](TActionTriggerProperties *value) { return false; });

  EXPECT_TRUE(program.is_success());
  EXPECT_FALSE(manual.is_success());
  EXPECT_TRUE(manual.is_success());
  EXPECT_FALSE(program.is_success());
  EXPECT_FALSE(program.is_success());
}

}  // namespace testing
