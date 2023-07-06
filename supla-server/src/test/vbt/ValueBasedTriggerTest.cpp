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

#include "ValueBasedTriggerTest.h"

#include "doubles/actions/ActionExecutorMock.h"
#include "doubles/device/ChannelPropertyGetterMock.h"
#include "vbt/value_based_trigger.h"

namespace testing {

using std::map;
using std::string;

ValueBasedTriggerTest::ValueBasedTriggerTest() : Test() {}

ValueBasedTriggerTest::~ValueBasedTriggerTest() {}

TEST_F(ValueBasedTriggerTest, defaultLimits) {
  supla_action_config main_ac;
  supla_value_based_trigger t(1, 2, main_ac, "");

  EXPECT_EQ(t.get_time_window_sec(), 5);
  EXPECT_EQ(t.get_fire_count_limit(), 5);
  EXPECT_EQ(t.get_min_time_between_firing_usec(), 100000);
}

TEST_F(ValueBasedTriggerTest, changeLimits) {
  supla_action_config main_ac;
  supla_value_based_trigger t(1, 2, main_ac, "");

  t.set_time_window_sec(10);
  t.set_fire_count_limit(6);
  t.set_min_time_between_firing_usec(200);

  EXPECT_EQ(t.get_time_window_sec(), 10);
  EXPECT_EQ(t.get_fire_count_limit(), 6);
  EXPECT_EQ(t.get_min_time_between_firing_usec(), 200);
}

TEST_F(ValueBasedTriggerTest, tooFastFiring) {
  supla_action_config main_ac;
  main_ac.set_action_id(ACTION_TURN_ON);
  main_ac.set_subject_id(140);
  main_ac.set_subject_type(stChannel);
  supla_value_based_trigger t(1, 2, main_ac, "");
  t.set_min_time_between_firing_usec(200000);

  ActionExecutorMock actionExecutor;
  ChannelPropertyGetterMock propertyGetter;
  map<string, string> replacement_map;

  for (int a = 0; a < 100; a++) {
    usleep(10);
    t.fire(supla_caller(ctIPC), 2, &actionExecutor, &propertyGetter,
           &replacement_map);
  }

  EXPECT_EQ(actionExecutor.counterSetCount(), 1);
  EXPECT_EQ(actionExecutor.getOnCounter(), 1);
  usleep(t.get_min_time_between_firing_usec() + 1);

  t.fire(supla_caller(ctIPC), 2, &actionExecutor, &propertyGetter,
         &replacement_map);

  EXPECT_EQ(actionExecutor.counterSetCount(), 1);
  EXPECT_EQ(actionExecutor.getOnCounter(), 2);
}

TEST_F(ValueBasedTriggerTest, tooManyFiresInAcertainAmountOfTime) {
  supla_action_config main_ac;
  main_ac.set_action_id(ACTION_TURN_ON);
  main_ac.set_subject_id(140);
  main_ac.set_subject_type(stChannel);
  supla_value_based_trigger t(1, 2, main_ac, "");
  t.set_min_time_between_firing_usec(0);
  t.set_time_window_sec(1);

  ActionExecutorMock actionExecutor;
  ChannelPropertyGetterMock propertyGetter;
  map<string, string> replacement_map;

  for (unsigned int a = 0; a <= t.get_fire_count_limit() + 10; a++) {
    t.fire(supla_caller(ctIPC), 2, &actionExecutor, &propertyGetter,
           &replacement_map);
  }

  EXPECT_EQ(actionExecutor.counterSetCount(), 1);
  EXPECT_EQ(actionExecutor.getOnCounter(), t.get_fire_count_limit());

  usleep(t.get_time_window_sec() * 1000000);

  for (unsigned int a = 0; a <= t.get_fire_count_limit() + 10; a++) {
    t.fire(supla_caller(ctIPC), 2, &actionExecutor, &propertyGetter,
           &replacement_map);
  }

  EXPECT_EQ(actionExecutor.counterSetCount(), 1);
  EXPECT_EQ(actionExecutor.getOnCounter(), t.get_fire_count_limit() * 2);
}

TEST_F(ValueBasedTriggerTest, equalityOperator) {
  supla_action_config main_ac;
  main_ac.set_action_id(2);
  main_ac.set_subject_type(stScene);
  main_ac.set_subject_id(3);
  main_ac.set_source_device_id(4);
  main_ac.set_source_channel_id(5);
  main_ac.set_percentage(6);

  supla_value_based_trigger main(5, 10, main_ac,
                                 "{\"on_change_to\":{\"eq\":1}}");

  {
    supla_value_based_trigger t1(5, 10, main_ac,
                                 "{\"on_change_to\":{\"eq\":1}}");

    supla_value_based_trigger t2(6, 10, main_ac,
                                 "{\"on_change_to\":{\"eq\":1}}");

    EXPECT_TRUE(main == t1);
    EXPECT_TRUE(main == &t1);
    EXPECT_FALSE(main != t1);
    EXPECT_FALSE(main != &t1);

    EXPECT_FALSE(main == t2);
    EXPECT_FALSE(main == &t2);
    EXPECT_TRUE(main != t2);
    EXPECT_TRUE(main != &t2);
  }

  {
    supla_value_based_trigger t1(5, 10, main_ac,
                                 "{\"on_change_to\":{\"eq\":1}}");

    supla_value_based_trigger t2(5, 11, main_ac,
                                 "{\"on_change_to\":{\"eq\":1}}");

    EXPECT_TRUE(main == t1);
    EXPECT_FALSE(main != t1);
    EXPECT_FALSE(main == t2);
    EXPECT_TRUE(main != t2);
  }

  {
    supla_value_based_trigger t1(5, 10, main_ac,
                                 "{\"on_change_to\":{\"eq\":1}}");

    supla_value_based_trigger t2(5, 10, main_ac,
                                 "{\"on_change_to\":{\"eq\":1}}");

    EXPECT_TRUE(main == t1);
    EXPECT_FALSE(main != t1);
    EXPECT_FALSE(main != t2);
    EXPECT_TRUE(main == t2);
  }

  {
    supla_value_based_trigger t1(5, 10, main_ac,
                                 "{\"on_change_to\":{\"eq\":1}}");

    supla_action_config ac(&main_ac);
    ac.set_percentage(50);
    supla_value_based_trigger t2(5, 10, ac, "{\"on_change_to\":{\"eq\":1}}");

    EXPECT_TRUE(main == t1);
    EXPECT_FALSE(main != t1);
    EXPECT_FALSE(main == t2);
    EXPECT_TRUE(main != t2);
  }

  {
    supla_value_based_trigger t1(5, 10, main_ac,
                                 "{\"on_change_to\":{\"eq\":1}}");

    supla_value_based_trigger t2(5, 10, main_ac,
                                 "{\"on_change_to\":{\"eq\":1}}");

    EXPECT_TRUE(main == t1);
    EXPECT_FALSE(main != t1);
    EXPECT_FALSE(main != t2);
    EXPECT_TRUE(main == t2);
  }

  {
    supla_value_based_trigger t1(5, 10, main_ac,
                                 "{\"on_change_to\":{\"eq\":1}}");

    supla_value_based_trigger t2(5, 10, main_ac,
                                 "{\"on_change_to\":{\"ne\":1}}");

    EXPECT_TRUE(main == t1);
    EXPECT_FALSE(main != t1);
    EXPECT_FALSE(main == t2);
    EXPECT_TRUE(main != t2);
  }
}

}  // namespace testing
