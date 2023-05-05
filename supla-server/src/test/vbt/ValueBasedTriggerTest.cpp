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

#include "vbt/value_based_trigger.h"

namespace testing {

ValueBasedTriggerTest::ValueBasedTriggerTest() : Test() {}

ValueBasedTriggerTest::~ValueBasedTriggerTest() {}

TEST_F(ValueBasedTriggerTest, equalityOperator) {
  supla_action_config main_ac;
  main_ac.set_action_id(2);
  main_ac.set_subject_type(stScene);
  main_ac.set_subject_id(3);
  main_ac.set_source_device_id(4);
  main_ac.set_source_channel_id(5);
  main_ac.set_percentage(6);

  supla_value_based_trigger main(5, 10, supla_action_config(&main_ac),
                                 "{\"on_change_to\":{\"eq\":1}}");

  {
    supla_value_based_trigger t1(5, 10, supla_action_config(&main_ac),
                                 "{\"on_change_to\":{\"eq\":1}}");

    supla_value_based_trigger t2(6, 10, supla_action_config(&main_ac),
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
    supla_value_based_trigger t1(5, 10, supla_action_config(&main_ac),
                                 "{\"on_change_to\":{\"eq\":1}}");

    supla_value_based_trigger t2(5, 11, supla_action_config(&main_ac),
                                 "{\"on_change_to\":{\"eq\":1}}");

    EXPECT_TRUE(main == t1);
    EXPECT_FALSE(main != t1);
    EXPECT_FALSE(main == t2);
    EXPECT_TRUE(main != t2);
  }

  {
    supla_value_based_trigger t1(5, 10, supla_action_config(&main_ac),
                                 "{\"on_change_to\":{\"eq\":1}}");

    supla_value_based_trigger t2(5, 10, supla_action_config(&main_ac),
                                 "{\"on_change_to\":{\"eq\":1}}");

    EXPECT_TRUE(main == t1);
    EXPECT_FALSE(main != t1);
    EXPECT_FALSE(main != t2);
    EXPECT_TRUE(main == t2);
  }

  {
    supla_value_based_trigger t1(5, 10, supla_action_config(&main_ac),
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
    supla_value_based_trigger t1(5, 10, supla_action_config(&main_ac),
                                 "{\"on_change_to\":{\"eq\":1}}");

    supla_value_based_trigger t2(5, 10, supla_action_config(&main_ac),
                                 "{\"on_change_to\":{\"eq\":1}}");

    EXPECT_TRUE(main == t1);
    EXPECT_FALSE(main != t1);
    EXPECT_FALSE(main != t2);
    EXPECT_TRUE(main == t2);
  }

  {
    supla_value_based_trigger t1(5, 10, supla_action_config(&main_ac),
                                 "{\"on_change_to\":{\"eq\":1}}");

    supla_value_based_trigger t2(5, 10, supla_action_config(&main_ac),
                                 "{\"on_change_to\":{\"ne\":1}}");

    EXPECT_TRUE(main == t1);
    EXPECT_FALSE(main != t1);
    EXPECT_FALSE(main == t2);
    EXPECT_TRUE(main != t2);
  }
}

}  // namespace testing
