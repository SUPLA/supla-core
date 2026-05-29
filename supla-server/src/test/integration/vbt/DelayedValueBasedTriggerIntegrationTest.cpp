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

#include "DelayedValueBasedTriggerIntegrationTest.h"

#include <chrono>
#include <memory>
#include <string>

#include "device/value/channel_general_purpose_text_value.h"
#include "device/value/channel_onoff_value.h"
#include "log.h"

namespace testing {

DelayedValueBasedTriggerIntegrationTest::
DelayedValueBasedTriggerIntegrationTest() : IntegrationTest(), Test() {
  user = nullptr;
}

DelayedValueBasedTriggerIntegrationTest::
~DelayedValueBasedTriggerIntegrationTest() {}

void DelayedValueBasedTriggerIntegrationTest::SetUp() {
  user = supla_user::find(2, true);
  initTestDatabase();
  runSqlScript("AddDelayedValueBasedTriggers.sql");
  actionExecutor = new ActionExecutorGmock();
  propertyGetter = new ChannelPropertyGetterMock();
  scheduler = new supla_vbt_scheduler(actionExecutor, propertyGetter);
  triggers = new ValueBasedTriggersMock(user);
  ON_CALL(*triggers, get_scheduler).WillByDefault(Return(scheduler));
  triggers->load();
  Test::SetUp();
}

void DelayedValueBasedTriggerIntegrationTest::TearDown() {
  Test::TearDown();
  delete triggers;
  delete scheduler;
  delete user;
}

TEST_F(DelayedValueBasedTriggerIntegrationTest, singleAction) {
  supla_channel_onoff_value oldv, newv;
  newv.set_on(true);

  auto start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point fire;

  EXPECT_CALL(*actionExecutor, set_on(true, _))
      .WillOnce([&fire](bool on, unsigned long long duration_ms) -> void {
        fire = std::chrono::steady_clock::now();
      });

  triggers->on_value_changed(supla_caller(ctIPC), 140, &oldv, &newv,
                             actionExecutor, propertyGetter);

  usleep(1500000);

  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(fire - start)
                .count();
  EXPECT_GT(ms, 1000);
  EXPECT_LT(ms, 1400);
}

TEST_F(DelayedValueBasedTriggerIntegrationTest, inactive) {
  runSqlScript("SetValueBasedTriggerInactive.sql");
  triggers->load();

  supla_channel_onoff_value oldv, newv;
  newv.set_on(true);

  EXPECT_CALL(*actionExecutor, set_on(true, _)).Times(0);

  triggers->on_value_changed(supla_caller(ctIPC), 140, &oldv, &newv,
                             actionExecutor, propertyGetter);

  usleep(1500000);
}

TEST_F(DelayedValueBasedTriggerIntegrationTest,
       anotherMatchingValueWhileWaiting) {
  supla_channel_temphum_value oldv(SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR,
                                   SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE,
                                   25.15, 49);
  supla_channel_temphum_value newv(SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR,
                                   SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE,
                                   25.15, 50);
  supla_channel_temphum_value another_newv(
      SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR,
      SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE, 25.15, 55);

  auto start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point fire;

  EXPECT_CALL(*actionExecutor, set_on(true, _))
      .WillOnce([&fire](bool on, unsigned long long duration_ms) -> void {
        fire = std::chrono::steady_clock::now();
      });

  triggers->on_value_changed(supla_caller(ctIPC), 158, &oldv, &newv,
                             actionExecutor, propertyGetter);

  triggers->on_value_changed(supla_caller(ctIPC), 158, &newv, &another_newv,
                             actionExecutor, propertyGetter);

  usleep(1500000);

  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(fire - start)
                .count();
  EXPECT_GT(ms, 1000);
  EXPECT_LT(ms, 1500);
}

TEST_F(DelayedValueBasedTriggerIntegrationTest, resumeAndFireAgain) {
  supla_channel_temphum_value oldv(SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR,
                                   SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE,
                                   25.15, 49),
      newv(SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR,
           SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE, 25.15, 50);

  auto start = std::chrono::steady_clock::now();
  int counter = 0;

  EXPECT_CALL(*actionExecutor, set_on(true, _))
      .Times(2)
      .WillRepeatedly([&start, &counter, &oldv, &newv, this](
                          bool on, unsigned long long duration_ms) -> void {
        auto fire = std::chrono::steady_clock::now();
        auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(fire - start)
                .count();
        EXPECT_GT(ms, 1000);
        EXPECT_LT(ms, 1300);

        start = std::chrono::steady_clock::now();

        if (counter == 0) {
          // resume

          supla_channel_temphum_value resumev(
              SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR,
              SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE, 25.15, 5);

          triggers->on_value_changed(supla_caller(ctIPC), 158, &newv, &resumev,
                                     actionExecutor, propertyGetter);

          triggers->on_value_changed(supla_caller(ctIPC), 158, &oldv, &newv,
                                     actionExecutor, propertyGetter);
        }

        counter++;
      });

  triggers->on_value_changed(supla_caller(ctIPC), 158, &oldv, &newv,
                             actionExecutor, propertyGetter);

  usleep(2500000);

  EXPECT_EQ(counter, 2);
}

TEST_F(DelayedValueBasedTriggerIntegrationTest, fireAgainWithoutResume) {
  supla_channel_temphum_value oldv(SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR,
                                   SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE,
                                   25.15, 49),
      newv(SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR,
           SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE, 25.15, 50);

  auto start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point fire;

  EXPECT_CALL(*actionExecutor, set_on(true, _))
      .WillOnce([&fire, &oldv, &newv, this](
                    bool on, unsigned long long duration_ms) -> void {
        fire = std::chrono::steady_clock::now();

        triggers->on_value_changed(supla_caller(ctIPC), 158, &oldv, &newv,
                                   actionExecutor, propertyGetter);
      });

  triggers->on_value_changed(supla_caller(ctIPC), 158, &oldv, &newv,
                             actionExecutor, propertyGetter);

  usleep(2500000);

  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(fire - start)
                .count();
  EXPECT_GT(ms, 1000);
  EXPECT_LT(ms, 1500);
}

TEST_F(DelayedValueBasedTriggerIntegrationTest, timerReset) {
  supla_channel_onoff_value v_true, v_false;
  v_true.set_on(true);
  v_false.set_on(false);

  auto start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point fire;

  EXPECT_CALL(*actionExecutor, set_on(true, _))
      .WillOnce([&fire](bool on, unsigned long long duration_ms) -> void {
        fire = std::chrono::steady_clock::now();
      });

  triggers->on_value_changed(supla_caller(ctIPC), 140, &v_false, &v_true,
                             actionExecutor, propertyGetter);

  usleep(500000);

  triggers->on_value_changed(supla_caller(ctIPC), 140, &v_true, &v_false,
                             actionExecutor, propertyGetter);

  triggers->on_value_changed(supla_caller(ctIPC), 140, &v_false, &v_true,
                             actionExecutor, propertyGetter);

  usleep(1500000);

  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(fire - start)
                .count();
  EXPECT_GT(ms, 1500);
  EXPECT_LT(ms, 2000);
}

TEST_F(DelayedValueBasedTriggerIntegrationTest, multipleActions) {
  auto start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point fire;

  EXPECT_CALL(*actionExecutor, set_on(true, _))
      .Times(3)
      .WillRepeatedly([&fire](bool on, unsigned long long duration_ms) -> void {
        fire = std::chrono::steady_clock::now();
      });

  {
    supla_channel_onoff_value oldv, newv;
    newv.set_on(true);

    triggers->on_value_changed(supla_caller(ctIPC), 140, &oldv, &newv,
                               actionExecutor, propertyGetter);

    triggers->on_value_changed(supla_caller(ctIPC), 141, &oldv, &newv,
                               actionExecutor, propertyGetter);
  }

  {
    supla_channel_temphum_value oldv(SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR,
                                     SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE,
                                     25.15, 49),
        newv(SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR,
             SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE, 25.15, 50);

    triggers->on_value_changed(supla_caller(ctIPC), 158, &oldv, &newv,
                               actionExecutor, propertyGetter);
  }

  usleep(1500000);

  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(fire - start)
                .count();
  EXPECT_GT(ms, 1000);
  EXPECT_LT(ms, 1400);
}

TEST_F(DelayedValueBasedTriggerIntegrationTest,
       generalPurposeTextPushNotificationAfterDuration) {
  runSqlScript("AddDelayedGeneralPurposeTextValueBasedTrigger.sql");
  triggers->load();

  auto gpt_trigger = triggers->get(34);
  ASSERT_TRUE(gpt_trigger != nullptr);
  EXPECT_EQ(gpt_trigger->get_channel_id(), 320);
  EXPECT_EQ(gpt_trigger->get_action_config().get_subject_type(),
            stPushNotification);
  EXPECT_EQ(gpt_trigger->get_action_config().get_subject_id(), 500);
  EXPECT_EQ(gpt_trigger->get_action_config().get_action_id(), ACTION_SEND);
  EXPECT_EQ(gpt_trigger->get_on_change_cnd().get_op(), op_eq);
  EXPECT_EQ(gpt_trigger->get_on_change_cnd().get_duration_sec(), 1);
  double latitude = 0;
  double longitude = 0;
  std::string timezone = user->get_timezone(&latitude, &longitude);
  EXPECT_TRUE(gpt_trigger->get_active_period().is_now_active(
      timezone.c_str(), latitude, longitude));

  char old_raw[SUPLA_CHANNELVALUE_SIZE] = {};
  strncpy(old_raw, "before", SUPLA_CHANNELVALUE_SIZE - 1);
  supla_channel_general_purpose_text_value oldv(old_raw);

  char match_raw[SUPLA_CHANNELVALUE_SIZE] = {};
  strncpy(match_raw, "match", SUPLA_CHANNELVALUE_SIZE - 1);
  supla_channel_general_purpose_text_value matchv(match_raw);

  auto direct_match = gpt_trigger->is_condition_met(320, &oldv, &matchv);
  _supla_int64_t direct_ms_left = 0;
  EXPECT_TRUE(direct_match.is_condition_met(&direct_ms_left));
  EXPECT_GT(direct_ms_left, 0);

  auto start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point fire;

  EXPECT_CALL(*actionExecutor, send(_, _))
      .WillOnce([&](const supla_caller &caller, nlohmann::json *template_data) {
        fire = std::chrono::steady_clock::now();
        EXPECT_EQ(actionExecutor->get_push_notification_id(), 500);
        ASSERT_NE(template_data, nullptr);
        EXPECT_EQ((*template_data)["value"].get<std::string>(), "match");
      });

  triggers->on_value_changed(supla_caller(ctIPC), 320, &oldv, &matchv,
                             actionExecutor, propertyGetter);

  usleep(1500000);

  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(fire - start)
                .count();
  EXPECT_GT(ms, 1000);
  EXPECT_LT(ms, 1500);
}

TEST_F(DelayedValueBasedTriggerIntegrationTest,
       generalPurposeTextPushNotificationResetsWhenValueChangesAway) {
  runSqlScript("AddDelayedGeneralPurposeTextValueBasedTrigger.sql");
  triggers->load();

  auto gpt_trigger = triggers->get(34);
  ASSERT_TRUE(gpt_trigger != nullptr);
  EXPECT_EQ(gpt_trigger->get_channel_id(), 320);
  EXPECT_EQ(gpt_trigger->get_action_config().get_subject_type(),
            stPushNotification);
  EXPECT_EQ(gpt_trigger->get_action_config().get_subject_id(), 500);
  EXPECT_EQ(gpt_trigger->get_action_config().get_action_id(), ACTION_SEND);
  EXPECT_EQ(gpt_trigger->get_on_change_cnd().get_op(), op_eq);
  EXPECT_EQ(gpt_trigger->get_on_change_cnd().get_duration_sec(), 1);
  double latitude = 0;
  double longitude = 0;
  std::string timezone = user->get_timezone(&latitude, &longitude);
  EXPECT_TRUE(gpt_trigger->get_active_period().is_now_active(
      timezone.c_str(), latitude, longitude));

  char old_raw[SUPLA_CHANNELVALUE_SIZE] = {};
  strncpy(old_raw, "before", SUPLA_CHANNELVALUE_SIZE - 1);
  supla_channel_general_purpose_text_value oldv(old_raw);

  char match_raw[SUPLA_CHANNELVALUE_SIZE] = {};
  strncpy(match_raw, "match", SUPLA_CHANNELVALUE_SIZE - 1);
  supla_channel_general_purpose_text_value matchv(match_raw);

  char other_raw[SUPLA_CHANNELVALUE_SIZE] = {};
  strncpy(other_raw, "other", SUPLA_CHANNELVALUE_SIZE - 1);
  supla_channel_general_purpose_text_value otherv(other_raw);

  auto direct_match = gpt_trigger->is_condition_met(320, &oldv, &matchv);
  _supla_int64_t direct_ms_left = 0;
  EXPECT_TRUE(direct_match.is_condition_met(&direct_ms_left));
  EXPECT_GT(direct_ms_left, 0);

  auto start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point fire;

  EXPECT_CALL(*actionExecutor, send(_, _))
      .WillOnce([&](const supla_caller &caller, nlohmann::json *template_data) {
        fire = std::chrono::steady_clock::now();
        EXPECT_EQ(actionExecutor->get_push_notification_id(), 500);
        ASSERT_NE(template_data, nullptr);
        EXPECT_EQ((*template_data)["value"].get<std::string>(), "match");
      });

  triggers->on_value_changed(supla_caller(ctIPC), 320, &oldv, &matchv,
                             actionExecutor, propertyGetter);

  usleep(500000);

  triggers->on_value_changed(supla_caller(ctIPC), 320, &matchv, &otherv,
                             actionExecutor, propertyGetter);

  usleep(700000);

  triggers->on_value_changed(supla_caller(ctIPC), 320, &otherv, &matchv,
                             actionExecutor, propertyGetter);

  usleep(1100000);

  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(fire - start)
                .count();
  EXPECT_GT(ms, 2100);
  EXPECT_LT(ms, 2600);
}

} /* namespace testing */
