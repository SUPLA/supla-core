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

#include "ElectricityMeterConfigTest.h"

#include "TestHelper.h"
#include "channeljsonconfig/electricity_meter_config.h"
#include "proto.h"

namespace testing {

ElectricityMeterConfigTest::ElectricityMeterConfigTest(void) {}
ElectricityMeterConfigTest::~ElectricityMeterConfigTest(void) {}

TEST_F(ElectricityMeterConfigTest, setAllAvailableCounters) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  char *str1 = config->get_properties();
  EXPECT_TRUE(str1 != NULL);

  if (str1) {
    EXPECT_EQ(strncmp(str1, "{}", 5), 0);
    free(str1);
    str1 = NULL;
  }

  EXPECT_TRUE(config->update_available_counters(0xFFFFFFFF));
  EXPECT_FALSE(config->update_available_counters(0xFFFFFFFF));

  str1 = config->get_properties();
  EXPECT_TRUE(str1 != NULL);

  if (str1) {
    // TestHelper::printEscaped(str);
    EXPECT_EQ(
        strncmp(str1,
                "{\"countersAvailable\":[\"forwardActiveEnergy\","
                "\"reverseActiveEnergy\",\"forwardReactiveEnergy\","
                "\"reverseReactiveEnergy\",\"forwardActiveEnergyBalanced\","
                "\"reverseActiveEnergyBalanced\"]}",
                1000),
        0);

    EXPECT_FALSE(config->update_available_counters(0x1));

    char *str2 = config->get_properties();
    EXPECT_TRUE(str2 != NULL);

    if (str2) {
      // All counters that have ever been available are added to the list. Once
      // added, they are no longer removed.
      EXPECT_EQ(strncmp(str1, str2, 1000), 0);
      free(str2);
      str2 = NULL;
    }

    free(str1);
    str1 = NULL;
  }

  delete config;
}

TEST_F(ElectricityMeterConfigTest, getInitialValue) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":0.00001,"
      "\"reverseActiveEnergy\":456789,\"forwardReactiveEnergy\":123,"
      "\"reverseReactiveEnergy\":876.12345,\"forwardActiveEnergyBalanced\":1."
      "456,"
      "\"reverseActiveEnergyBalanced\":123.678}}");

  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY),
            (unsigned)0);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_ACTIVE_ENERGY),
            (unsigned)0);
  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_REACTIVE_ENERGY),
            (unsigned)0);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_REACTIVE_ENERGY),
            (unsigned)0);
  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED),
            (unsigned)0);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED),
            (unsigned)0);

  EXPECT_TRUE(config->update_available_counters(EM_VAR_REVERSE_ACTIVE_ENERGY));

  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY),
            (unsigned)0);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_ACTIVE_ENERGY),
            (unsigned long long)45678900000);
  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_REACTIVE_ENERGY),
            (unsigned)0);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_REACTIVE_ENERGY),
            (unsigned)0);
  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED),
            (unsigned)0);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED),
            (unsigned)0);

  EXPECT_TRUE(config->update_available_counters(0xFFFFFFFF));

  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY),
            (unsigned)1);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_ACTIVE_ENERGY),
            (unsigned long long)45678900000);
  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_REACTIVE_ENERGY),
            (unsigned)12300000);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_REACTIVE_ENERGY),
            (unsigned)87612345);
  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED),
            (unsigned)145600);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED),
            (unsigned)12367800);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":0.00001,"
      "\"reverseActiveEnergy\":456789,\"forwardReactiveEnergy\":123,"
      "\"reverseReactiveEnergy\":876.12345,\"forwardActiveEnergyBalanced\":"
      "10000000}}");

  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY),
            (unsigned)1);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_ACTIVE_ENERGY),
            (unsigned long long)45678900000);
  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_REACTIVE_ENERGY),
            (unsigned)12300000);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_REACTIVE_ENERGY),
            (unsigned)87612345);
  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED),
            (unsigned long long)1000000000000);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED),
            (unsigned)0);

  delete config;
}

TEST_F(ElectricityMeterConfigTest, noPhases) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":0,"
      "\"reverseActiveEnergy\":0,\"forwardReactiveEnergy\":0,"
      "\"reverseReactiveEnergy\":0,\"forwardActiveEnergyBalanced\":1.456,"
      "\"reverseActiveEnergyBalanced\":123.678}}");

  unsigned _supla_int64_t v = 10;
  config->add(EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED, 0, 0, &v);

  EXPECT_EQ(v, (unsigned)10);

  EXPECT_TRUE(config->update_available_counters(0xFFFFFFFF));

  config->add(EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED, 0, 0, &v);
  EXPECT_EQ(v, (unsigned)145610);

  delete config;
}

TEST_F(ElectricityMeterConfigTest, onePhase) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":778}}");

  EXPECT_TRUE(config->update_available_counters(0xFFFFFFFF));

  unsigned _supla_int64_t v = 10;
  config->add(EM_VAR_FORWARD_ACTIVE_ENERGY, 1,
              SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
                  SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED,
              &v);

  EXPECT_EQ(v, (unsigned)77800010);

  v = 1234;
  config->add(EM_VAR_FORWARD_ACTIVE_ENERGY, 1,
              SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                  SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED,
              &v);

  EXPECT_EQ(v, (unsigned)1234);

  v = 1235;
  config->add(EM_VAR_FORWARD_ACTIVE_ENERGY, 3,
              SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                  SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED,
              &v);

  EXPECT_EQ(v, (unsigned)77801234);

  delete config;
}

} /* namespace testing */
