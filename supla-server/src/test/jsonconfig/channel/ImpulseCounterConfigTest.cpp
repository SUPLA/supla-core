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

#include "ImpulseCounterConfigTest.h"

#include <limits.h>

#include "TestHelper.h"
#include "jsonconfig/channel/impulse_counter_config.h"
#include "proto.h"

namespace testing {

ImpulseCounterConfigTest::ImpulseCounterConfigTest(void) {}
ImpulseCounterConfigTest::~ImpulseCounterConfigTest(void) {}

TEST_F(ImpulseCounterConfigTest, getInitialValue) {
  impulse_counter_config *config = new impulse_counter_config();
  ASSERT_TRUE(config != NULL);

  EXPECT_EQ(config->get_initial_value(), 0.0);

  config->set_user_config("{\"initialValue\":2234.56}");

  EXPECT_EQ(config->get_initial_value(), 2234.56);

  config->set_user_config("{\"initialValue\":-1}");

  EXPECT_EQ(config->get_initial_value(), -1);

  config->set_user_config("{\"initialValue\":-100000001.001}");

  EXPECT_EQ(config->get_initial_value(), -100000000.0);

  config->set_user_config("{\"initialValue\":100000001.001}");

  EXPECT_EQ(config->get_initial_value(), 100000000.0);

  config->set_user_config("{\"initialValue\":100000000.001}");

  EXPECT_EQ(config->get_initial_value(), 100000000.0);

  config->set_user_config("{\"initialValue\":99999999.999}");

  EXPECT_EQ(config->get_initial_value(), 99999999.999);

  delete config;
}

TEST_F(ImpulseCounterConfigTest, getImpulsesPerUnit) {
  impulse_counter_config *config = new impulse_counter_config();
  ASSERT_TRUE(config != NULL);

  EXPECT_EQ(config->get_impulses_per_unit(), (unsigned)0);

  config->set_user_config("{\"impulsesPerUnit\":2234.56}");

  EXPECT_EQ(config->get_impulses_per_unit(), (unsigned)2234);

  config->set_user_config("{\"impulsesPerUnit\":-1}");

  EXPECT_EQ(config->get_impulses_per_unit(), (unsigned)0);

  config->set_user_config("{\"impulsesPerUnit\":1000001.001}");

  EXPECT_EQ(config->get_impulses_per_unit(), (unsigned)1000000);

  config->set_user_config("{\"impulsesPerUnit\":1000000.001}");

  EXPECT_EQ(config->get_impulses_per_unit(), (unsigned)1000000);

  config->set_user_config("{\"impulsesPerUnit\":999999.999}");

  EXPECT_EQ(config->get_impulses_per_unit(), (unsigned)999999);

  config->set_user_config("{\"impulsesPerUnit\":999999}");

  EXPECT_EQ(config->get_impulses_per_unit(), (unsigned)999999);

  delete config;
}

TEST_F(ImpulseCounterConfigTest, addingAnInitialValue) {
  impulse_counter_config *config = new impulse_counter_config();
  ASSERT_TRUE(config != NULL);

  TDS_ImpulseCounter_Value value = {};
  config->add_initial_value(NULL);

  config->set_user_config("{\"impulsesPerUnit\":0, \"initialValue\":2234.56}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)0);

  config->set_user_config("{\"impulsesPerUnit\":100, \"initialValue\":0}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)0);

  config->set_user_config("{\"impulsesPerUnit\":1, \"initialValue\":100}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)100);

  value.counter = 0;
  config->set_user_config("{\"impulsesPerUnit\":1000, \"initialValue\":100}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)100000);

  value.counter = 0;
  config->set_user_config(
      "{\"impulsesPerUnit\":1000, \"initialValue\":100.123}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)100123);

  value.counter = 0;
  config->set_user_config(
      "{\"impulsesPerUnit\":1000000, \"initialValue\":100000000}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned long long)100000000000000);

  value.counter = -1;
  config->set_user_config(
      "{\"impulsesPerUnit\":1000000, \"initialValue\":100000000}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, ULONG_MAX);

  value.counter = 0xFFFFFFFFFFFFFF00;
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, ULONG_MAX);

  delete config;
}

TEST_F(ImpulseCounterConfigTest, overValue) {
  impulse_counter_config *config = new impulse_counter_config();
  ASSERT_TRUE(config != NULL);

  TDS_ImpulseCounter_Value value = {};

  value.counter = ULONG_MAX;
  config->set_user_config(
      "{\"impulsesPerUnit\":1000000, \"initialValue\":100000000}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, ULONG_MAX);

  value.counter = ULONG_MAX - 100;
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, ULONG_MAX);

  delete config;
}

TEST_F(ImpulseCounterConfigTest, negative) {
  impulse_counter_config *config = new impulse_counter_config();
  ASSERT_TRUE(config != NULL);

  TDS_ImpulseCounter_Value value = {};

  config->set_user_config("{\"impulsesPerUnit\":5, \"initialValue\":-100}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)0);

  value.counter = 499;

  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)0);

  value.counter = 501;

  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)1);

  delete config;
}

TEST_F(ImpulseCounterConfigTest, addToHitorryFlagTest) {
  impulse_counter_config *config = new impulse_counter_config();
  ASSERT_TRUE(config != NULL);

  ASSERT_FALSE(config->should_be_added_to_history());

  config->set_user_config("{\"addToHistory\":false}");

  ASSERT_FALSE(config->should_be_added_to_history());

  config->set_user_config("{\"addToHistory\":true}");

  ASSERT_TRUE(config->should_be_added_to_history());

  delete config;
}

} /* namespace testing */
