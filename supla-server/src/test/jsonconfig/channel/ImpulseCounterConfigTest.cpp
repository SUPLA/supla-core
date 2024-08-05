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

  config->set_user_config("{\"initialValue\":2234560}");

  EXPECT_EQ(config->get_initial_value(), 2234.56);

  config->set_user_config("{\"initialValue\":-1000}");

  EXPECT_EQ(config->get_initial_value(), -1);

  config->set_user_config("{\"initialValue\":-100000001001}");

  EXPECT_EQ(config->get_initial_value(), -100000000.0);

  config->set_user_config("{\"initialValue\":100000001001}");

  EXPECT_EQ(config->get_initial_value(), 100000000.0);

  config->set_user_config("{\"initialValue\":100000000001}");

  EXPECT_EQ(config->get_initial_value(), 100000000.0);

  config->set_user_config("{\"initialValue\":99999999999}");

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

  config->set_user_config("{\"impulsesPerUnit\":0, \"initialValue\":2234560}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)0);

  config->set_user_config("{\"impulsesPerUnit\":100, \"initialValue\":0}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)0);

  config->set_user_config("{\"impulsesPerUnit\":1, \"initialValue\":100000}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)100);

  value.counter = 0;
  config->set_user_config(
      "{\"impulsesPerUnit\":1000, \"initialValue\":100000}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)100000);

  value.counter = 0;
  config->set_user_config(
      "{\"impulsesPerUnit\":1000, \"initialValue\":100123}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned)100123);

  value.counter = 0;
  config->set_user_config(
      "{\"impulsesPerUnit\":1000000, \"initialValue\":100000000000}");
  config->add_initial_value(&value.counter);

  EXPECT_EQ(value.counter, (unsigned long long)100000000000000);

  value.counter = -1;
  config->set_user_config(
      "{\"impulsesPerUnit\":1000000, \"initialValue\":100000000000}");
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
      "{\"impulsesPerUnit\":1000000, \"initialValue\":100000000000}");
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

  config->set_user_config("{\"impulsesPerUnit\":5, \"initialValue\":-100000}");
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

TEST_F(ImpulseCounterConfigTest, customUnit) {
  impulse_counter_config cfg;
  char custom_unit[9] = {};
  cfg.get_custom_unit(custom_unit, 0);
  EXPECT_STREQ(custom_unit, "");

  cfg.get_custom_unit(custom_unit, SUPLA_CHANNELFNC_IC_ELECTRICITY_METER);
  EXPECT_STREQ(custom_unit, "kWh");

  cfg.get_custom_unit(custom_unit, SUPLA_CHANNELFNC_IC_GAS_METER);
  EXPECT_STREQ(custom_unit, "m³");

  *custom_unit = {};

  cfg.get_custom_unit(custom_unit, SUPLA_CHANNELFNC_IC_WATER_METER);
  EXPECT_STREQ(custom_unit, "m³");

  cfg.get_custom_unit(custom_unit, SUPLA_CHANNELFNC_IC_HEAT_METER);
  EXPECT_STREQ(custom_unit, "GJ");

  *custom_unit = {};

  cfg.set_user_config("{\"unit\":\"abcdefgh1\"}");

  cfg.get_custom_unit(custom_unit, SUPLA_CHANNELFNC_IC_HEAT_METER);
  EXPECT_STREQ(custom_unit, "GJ");

  *custom_unit = {};

  cfg.get_custom_unit(custom_unit, 0);
  EXPECT_STREQ(custom_unit, "");

  cfg.set_user_config("{\"unit\":\"abcdefgh\"}");

  cfg.get_custom_unit(custom_unit, SUPLA_CHANNELFNC_IC_HEAT_METER);
  EXPECT_STREQ(custom_unit, "abcdefgh");

  *custom_unit = {};

  cfg.get_custom_unit(custom_unit, 0);
  EXPECT_STREQ(custom_unit, "abcdefgh");
}

TEST_F(ImpulseCounterConfigTest, pricePerUnit) {
  impulse_counter_config cfg;
  EXPECT_EQ(cfg.get_price_per_unit(), 0);
  cfg.set_user_config("{\"pricePerUnit\":1234}");
  EXPECT_EQ(cfg.get_price_per_unit(), 1234);
}

TEST_F(ImpulseCounterConfigTest, currency) {
  impulse_counter_config cfg;
  EXPECT_STREQ(cfg.get_currency().c_str(), "");

  cfg.set_user_config("{\"currency\":\"PLN1\"}");
  EXPECT_STREQ(cfg.get_currency().c_str(), "");

  cfg.set_user_config("{\"currency\":\"PLN\"}");
  EXPECT_STREQ(cfg.get_currency().c_str(), "PLN");
}

TEST_F(ImpulseCounterConfigTest, setGetRawConfig) {
  impulse_counter_config cfg;
  TChannelConfig_ImpulseCounter raw = {};
  raw.PricePerUnit = 12345;
  raw.Currency[0] = 'E';
  raw.Currency[1] = 'U';
  raw.Currency[2] = 'R';
  snprintf(raw.CustomUnit, sizeof(raw.CustomUnit), "%s", "CUnit");
  raw.ImpulsesPerUnit = 5678;
  raw.InitialValue = 8765;
  raw.AddToHistory = 1;

  cfg.set_config(&raw);

  raw = {};
  cfg.get_config(&raw);

  EXPECT_EQ(raw.PricePerUnit, 12345);
  EXPECT_EQ(raw.Currency[0], 'E');
  EXPECT_EQ(raw.Currency[1], 'U');
  EXPECT_EQ(raw.Currency[2], 'R');
  EXPECT_STREQ(raw.CustomUnit, "CUnit");
  EXPECT_EQ(raw.ImpulsesPerUnit, 5678);
  EXPECT_EQ(raw.InitialValue, 8765);
  EXPECT_EQ(raw.AddToHistory, 1);

  char *str = cfg.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"pricePerUnit\":12345,\"currency\":\"EUR\",\"unit\":\"CUnit\","
      "\"impulsesPerUnit\":5678,\"initialValue\":8765,\"addToHistory\":true}");
  free(str);
}

TEST_F(ImpulseCounterConfigTest, merge) {
  impulse_counter_config config1;

  config1.set_user_config(
      "{\"a\":\"b\",\"pricePerUnit\":12345,\"currency\":\"EUR\",\"unit\":"
      "\"CUnit\",\"impulsesPerUnit\":5678,\"initialValue\":8765,"
      "\"addToHistory\":true}");

  TChannelConfig_ImpulseCounter raw = {};
  raw.ImpulsesPerUnit = 6000;
  raw.Currency[0] = 'P';
  raw.Currency[1] = 'L';
  raw.Currency[2] = 'N';

  impulse_counter_config config2;
  config2.set_config(&raw);
  config2.merge(&config1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"a\":\"b\",\"pricePerUnit\":0,\"currency\":\"PLN\",\"unit\":\"\","
      "\"impulsesPerUnit\":6000,\"initialValue\":0,\"addToHistory\":false}");

  free(str);
}

} /* namespace testing */
