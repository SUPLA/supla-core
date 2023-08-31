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

#include "integration/vbt/ValueBasedTriggerIntegrationTest.h"

#include <string>

#include "device/value/channel_onoff_value.h"
#include "device/value/channel_temphum_value.h"
#include "doubles/actions/ActionExecutorMock.h"
#include "doubles/device/ChannelPropertyGetterMock.h"
#include "vbt/value_based_triggers.h"

using std::map;
using std::shared_ptr;
using std::string;

namespace testing {

ValueBasedTriggerIntegrationTest::ValueBasedTriggerIntegrationTest()
    : IntegrationTest(), Test() {
  user = nullptr;
}

ValueBasedTriggerIntegrationTest::~ValueBasedTriggerIntegrationTest() {}

void ValueBasedTriggerIntegrationTest::SetUp() {
  user = supla_user::find(2, true);
  initTestDatabase();
  runSqlScript("AddValueBasedTriggers.sql");
  Test::SetUp();
}

void ValueBasedTriggerIntegrationTest::TearDown() {
  Test::TearDown();
  delete user;
}

TEST_F(ValueBasedTriggerIntegrationTest, loadFromUserWhoHasNoTriggers) {
  supla_user *user = supla_user::find(12345, true);

  supla_value_based_triggers triggers(user);
  triggers.load();

  EXPECT_EQ(triggers.count(), 0);

  delete user;
}

TEST_F(ValueBasedTriggerIntegrationTest, loadThenDisableThenEnableThenModify) {
  supla_value_based_triggers triggers(user);
  triggers.load();

  EXPECT_EQ(triggers.count(), 7);

  shared_ptr<supla_value_based_trigger> t20 = triggers.get(20);
  shared_ptr<supla_value_based_trigger> t21 = triggers.get(21);
  shared_ptr<supla_value_based_trigger> t22 = triggers.get(22);
  shared_ptr<supla_value_based_trigger> t30 = triggers.get(30);
  shared_ptr<supla_value_based_trigger> t31 = triggers.get(31);
  shared_ptr<supla_value_based_trigger> t32 = triggers.get(32);
  shared_ptr<supla_value_based_trigger> t33 = triggers.get(33);

  EXPECT_TRUE(t20 != nullptr);
  EXPECT_TRUE(t21 != nullptr);
  EXPECT_TRUE(t22 != nullptr);
  EXPECT_TRUE(t30 != nullptr);
  EXPECT_TRUE(t31 != nullptr);
  EXPECT_TRUE(t32 != nullptr);
  EXPECT_TRUE(t33 != nullptr);

  runSqlScript("DisableTwoValueBasedTriggers.sql");
  triggers.load();

  EXPECT_EQ(triggers.count(), 5);

  // They shouldn't be relocated
  EXPECT_TRUE(triggers.get(20) == t20);
  EXPECT_TRUE(triggers.get(21) == t21);
  EXPECT_TRUE(triggers.get(30) == t30);
  EXPECT_TRUE(triggers.get(32) == t32);
  EXPECT_TRUE(triggers.get(33) == t33);

  runSqlScript("EnableTwoValueBasedTriggers.sql");
  triggers.load();

  EXPECT_EQ(triggers.count(), 7);

  // They shouldn't be relocated
  EXPECT_TRUE(triggers.get(20) == t20);
  EXPECT_TRUE(triggers.get(21) == t21);
  EXPECT_TRUE(triggers.get(30) == t30);
  EXPECT_TRUE(triggers.get(32) == t32);
  EXPECT_TRUE(triggers.get(33) == t33);

  // These should be re-created
  EXPECT_TRUE(t22 != nullptr);
  EXPECT_TRUE(t31 != nullptr);
  EXPECT_TRUE(triggers.get(22) != t22);
  EXPECT_TRUE(triggers.get(31) != t31);
  t22 = triggers.get(22);
  t31 = triggers.get(31);

  // We re-create only those that have been modified. Otherwise, you can remove
  // the "paused" flag, which we don't want.
  runSqlScript("ModifyValueBasedTrigger.sql");
  triggers.load();

  EXPECT_EQ(triggers.count(), 7);

  // They shouldn't be relocated
  EXPECT_TRUE(triggers.get(20) == t20);
  EXPECT_TRUE(triggers.get(22) == t22);
  EXPECT_TRUE(triggers.get(30) == t30);
  EXPECT_TRUE(triggers.get(31) == t31);
  EXPECT_TRUE(triggers.get(32) == t32);
  EXPECT_TRUE(triggers.get(33) == t33);

  // Thes should be re-created
  EXPECT_TRUE(t21 != nullptr);
  EXPECT_TRUE(triggers.get(21) != t21);
}

TEST_F(ValueBasedTriggerIntegrationTest, loadAll) {
  runSqlScript("SetVbtActivityConditions.sql");

  supla_value_based_triggers triggers(user);
  triggers.load();
  //
  EXPECT_EQ(triggers.count(), 7);

  shared_ptr<supla_value_based_trigger> t = triggers.get(20);
  EXPECT_TRUE(t != nullptr);
  if (t) {
    EXPECT_EQ(t->get_id(), 20);
    EXPECT_EQ(t->get_channel_id(), 140);
    EXPECT_EQ(t->get_action_config().get_subject_type(), stChannel);
    EXPECT_EQ(t->get_action_config().get_subject_id(), 141);
    EXPECT_EQ(t->get_action_config().get_action_id(), ACTION_TURN_ON);
    EXPECT_EQ(t->get_on_change_cnd().get_op(), op_eq);
    EXPECT_EQ(t->get_on_change_cnd().get_value(), 1);

    supla_active_period p(1692551804, 1692626454, ",14,",
                          "[[{\"afterSunset\": 15}]]");
    EXPECT_TRUE(p == t->get_active_period());
  }

  t = triggers.get(21);
  EXPECT_TRUE(t != nullptr);
  if (t) {
    EXPECT_EQ(t->get_id(), 21);
    EXPECT_EQ(t->get_channel_id(), 140);
    EXPECT_EQ(t->get_action_config().get_subject_type(), stChannel);
    EXPECT_EQ(t->get_action_config().get_subject_id(), 173);
    EXPECT_EQ(t->get_action_config().get_action_id(), ACTION_SHUT_PARTIALLY);
    EXPECT_EQ(t->get_action_config().get_percentage(), 45);
    EXPECT_EQ(t->get_on_change_cnd().get_op(), op_eq);
    EXPECT_EQ(t->get_on_change_cnd().get_value(), 1);
  }

  t = triggers.get(22);
  EXPECT_TRUE(t != nullptr);
  if (t) {
    EXPECT_EQ(t->get_id(), 22);
    EXPECT_EQ(t->get_channel_id(), 140);
    EXPECT_EQ(t->get_action_config().get_subject_type(), stChannelGroup);
    EXPECT_EQ(t->get_action_config().get_subject_id(), 1);
    EXPECT_EQ(t->get_action_config().get_action_id(), ACTION_TURN_OFF);
    EXPECT_EQ(t->get_on_change_cnd().get_op(), op_eq);
    EXPECT_EQ(t->get_on_change_cnd().get_value(), 1);
  }

  t = triggers.get(30);
  EXPECT_TRUE(t != nullptr);
  if (t) {
    EXPECT_EQ(t->get_id(), 30);
    EXPECT_EQ(t->get_channel_id(), 141);
    EXPECT_EQ(t->get_action_config().get_subject_type(), stScene);
    EXPECT_EQ(t->get_action_config().get_subject_id(), 6);
    EXPECT_EQ(t->get_action_config().get_action_id(), ACTION_EXECUTE);
    EXPECT_EQ(t->get_on_change_cnd().get_op(), op_ne);
    EXPECT_EQ(t->get_on_change_cnd().get_value(), 0);
  }

  t = triggers.get(31);
  EXPECT_TRUE(t != nullptr);
  if (t) {
    EXPECT_EQ(t->get_id(), 31);
    EXPECT_EQ(t->get_channel_id(), 141);
    EXPECT_EQ(t->get_action_config().get_subject_type(), stSchedule);
    EXPECT_EQ(t->get_action_config().get_subject_id(), 1);
    EXPECT_EQ(t->get_action_config().get_action_id(), ACTION_DISABLE);
    EXPECT_EQ(t->get_on_change_cnd().get_op(), op_eq);
    EXPECT_EQ(t->get_on_change_cnd().get_value(), 1);
  }

  t = triggers.get(32);
  EXPECT_TRUE(t != nullptr);
  if (t) {
    EXPECT_EQ(t->get_id(), 32);
    EXPECT_EQ(t->get_channel_id(), 141);
    EXPECT_EQ(t->get_action_config().get_subject_type(), stPushNotification);
    EXPECT_EQ(t->get_action_config().get_subject_id(), 500);
    EXPECT_EQ(t->get_action_config().get_action_id(), ACTION_SEND);
    EXPECT_EQ(t->get_on_change_cnd().get_op(), op_eq);
    EXPECT_EQ(t->get_on_change_cnd().get_value(), 1);
  }

  t = triggers.get(33);
  EXPECT_TRUE(t != nullptr);
  if (t) {
    EXPECT_EQ(t->get_id(), 33);
    EXPECT_EQ(t->get_channel_id(), 158);
    EXPECT_EQ(t->get_action_config().get_subject_type(), stPushNotification);
    EXPECT_EQ(t->get_action_config().get_subject_id(), 500);
    EXPECT_EQ(t->get_action_config().get_action_id(), ACTION_SEND);
    EXPECT_EQ(t->get_on_change_cnd().get_op(), op_ge);
    EXPECT_EQ(t->get_on_change_cnd().get_value(), 50);
    EXPECT_EQ(t->get_on_change_cnd().get_var_name(), var_name_humidity);
  }
}

TEST_F(ValueBasedTriggerIntegrationTest, deleteFirst) {
  supla_value_based_triggers triggers(user);
  triggers.load();

  EXPECT_EQ(triggers.count(), 7);

  auto t = triggers.get(20);
  EXPECT_TRUE(t != nullptr);

  runSqlScript("DeleteVbtFirst.sql");
  EXPECT_EQ(triggers.count(), 6);

  triggers.load();
  t = triggers.get(20);
  EXPECT_TRUE(t == nullptr);
}

TEST_F(ValueBasedTriggerIntegrationTest, deleteVbt22) {
  supla_value_based_triggers triggers(user);
  triggers.load();

  EXPECT_EQ(triggers.count(), 7);

  auto t = triggers.get(22);
  EXPECT_TRUE(t != nullptr);

  runSqlScript("DeleteVbt22.sql");

  triggers.load();
  EXPECT_EQ(triggers.count(), 6);

  t = triggers.get(22);
  EXPECT_TRUE(t == nullptr);
}

TEST_F(ValueBasedTriggerIntegrationTest, deleteLast) {
  supla_value_based_triggers triggers(user);
  triggers.load();

  EXPECT_EQ(triggers.count(), 7);

  auto t = triggers.get(33);
  EXPECT_TRUE(t != nullptr);

  runSqlScript("DeleteVbtLast.sql");

  triggers.load();
  EXPECT_EQ(triggers.count(), 6);

  t = triggers.get(33);
  EXPECT_TRUE(t == nullptr);
}

TEST_F(ValueBasedTriggerIntegrationTest, delete31and32) {
  supla_value_based_triggers triggers(user);
  triggers.load();
  EXPECT_EQ(triggers.count(), 7);

  auto t = triggers.get(31);
  EXPECT_TRUE(t != nullptr);
  t = triggers.get(32);
  EXPECT_TRUE(t != nullptr);

  runSqlScript("DeleteVbt31and32.sql");

  triggers.load();
  EXPECT_EQ(triggers.count(), 5);

  t = triggers.get(31);
  EXPECT_TRUE(t == nullptr);
  t = triggers.get(32);
  EXPECT_TRUE(t == nullptr);
}

TEST_F(ValueBasedTriggerIntegrationTest, fireForChannel140) {
  supla_value_based_triggers triggers(user);
  triggers.load();

  supla_channel_onoff_value oldv, newv;
  newv.set_on(true);

  ActionExecutorMock actionExecutor;
  ChannelPropertyGetterMock propertyGetter;

  triggers.on_channel_value_changed(supla_caller(ctIPC), 140, &oldv, &newv,
                                    &actionExecutor, &propertyGetter);

  EXPECT_EQ(actionExecutor.counterSetCount(), 3);
  EXPECT_EQ(actionExecutor.getOnCounter(), 1);
  EXPECT_EQ(actionExecutor.getOffCounter(), 1);
  EXPECT_EQ(actionExecutor.getShutCounter(), 1);
}

TEST_F(ValueBasedTriggerIntegrationTest, fireForChannel141) {
  supla_value_based_triggers triggers(user);
  triggers.load();

  supla_channel_onoff_value oldv, newv;
  newv.set_on(true);

  ActionExecutorMock actionExecutor;
  ChannelPropertyGetterMock propertyGetter;

  triggers.on_channel_value_changed(supla_caller(ctIPC), 141, &oldv, &newv,
                                    &actionExecutor, &propertyGetter);

  EXPECT_EQ(actionExecutor.counterSetCount(), 3);
  EXPECT_EQ(actionExecutor.getExecuteCounter(), 1);
  EXPECT_EQ(actionExecutor.getDisableCounter(), 1);
  EXPECT_EQ(actionExecutor.getSentCounter(), 1);
}

TEST_F(ValueBasedTriggerIntegrationTest, fireForChannel158) {
  supla_value_based_triggers triggers(user);
  triggers.load();

  supla_channel_temphum_value oldv(true, 25.15, 49), newv(true, 25.15, 50);

  ActionExecutorMock actionExecutor;
  ChannelPropertyGetterMock propertyGetter;

  triggers.on_channel_value_changed(supla_caller(ctIPC), 158, &oldv, &newv,
                                    &actionExecutor, &propertyGetter);

  EXPECT_EQ(actionExecutor.get_push_notification_id(), 500);
  EXPECT_EQ(actionExecutor.getSentCounter(), 1);

  map<string, string> m = actionExecutor.get_replacement_map();
  EXPECT_EQ(m.size(), 2);
  EXPECT_EQ(m["temperature"], "25.15");
  EXPECT_EQ(m["humidity"], "50.00");
}

} /* namespace testing */
