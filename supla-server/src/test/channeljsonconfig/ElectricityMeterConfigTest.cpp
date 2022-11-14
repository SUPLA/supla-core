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

#include <limits.h>

#include "TestHelper.h"
#include "channeljsonconfig/electricity_meter_config.h"
#include "log.h"
#include "srpc/srpc.h"

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

TEST_F(ElectricityMeterConfigTest, emEvSetAvailableCounters) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.measured_values = EM_VAR_REVERSE_ACTIVE_ENERGY;

  TSuplaChannelExtendedValue ev = {};
  EXPECT_TRUE(srpc_evtool_v2_emextended2extended(&em_ev, &ev));

  EXPECT_TRUE(config->update_available_counters(&ev));
  EXPECT_FALSE(config->update_available_counters(&ev));

  char *str1 = config->get_properties();
  EXPECT_TRUE(str1 != NULL);

  if (str1) {
    // TestHelper::printEscaped(str);
    EXPECT_EQ(strncmp(str1, "{\"countersAvailable\":[\"reverseActiveEnergy\"]}",
                      1000),
              0);
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
            (long long)45678900000);
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
            (long long)45678900000);
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
            (long long)45678900000);
  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_REACTIVE_ENERGY),
            (unsigned)12300000);
  EXPECT_EQ(config->get_initial_value(EM_VAR_REVERSE_REACTIVE_ENERGY),
            (unsigned)87612345);
  EXPECT_EQ(config->get_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED),
            (long long)1000000000000);
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
  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED, 0, 0, &v);

  EXPECT_EQ(v, (unsigned)10);

  EXPECT_TRUE(config->update_available_counters(0xFFFFFFFF));

  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED, 0, 0, &v);
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
  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 1,
                            SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
                                SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED,
                            &v);

  EXPECT_EQ(v, (unsigned)77800010);

  v = 1234;
  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 1,
                            SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                                SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED,
                            &v);

  EXPECT_EQ(v, (unsigned)1234);

  v = 1234;

  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 1,
                            SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                                SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED,
                            &v);

  EXPECT_EQ(v, (unsigned)1234);

  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 2,
                            SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                                SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED,
                            &v);

  EXPECT_EQ(v, (unsigned)1234);

  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 3,
                            SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                                SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED,
                            &v);

  EXPECT_EQ(v, (unsigned)77801234);

  delete config;
}

TEST_F(ElectricityMeterConfigTest, twoPhases) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":555.00333}"
      "}");

  EXPECT_TRUE(config->update_available_counters(0xFFFFFFFF));

  unsigned _supla_int64_t v = 0;
  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 1,
                            SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED, &v);

  EXPECT_EQ(v, (unsigned)27750167);

  v = 0;
  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 2,
                            SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED, &v);

  EXPECT_EQ(v, (unsigned)27750166);

  v = 1122;
  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 2,
                            SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED, &v);

  EXPECT_EQ(v, (unsigned)27751288);

  delete config;
}

TEST_F(ElectricityMeterConfigTest, threePhases) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":2000}"
      "}");

  EXPECT_TRUE(config->update_available_counters(0xFFFFFFFF));

  unsigned _supla_int64_t v = 0;
  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 1, 0, &v);

  EXPECT_EQ(v, (unsigned)66666668);

  v = 0;
  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 2, 0, &v);

  EXPECT_EQ(v, (unsigned)66666666);

  v = 0;
  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 3, 0, &v);

  EXPECT_EQ(v, (unsigned)66666666);

  v = 10000004;
  config->add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 3, 0, &v);

  EXPECT_EQ(v, (unsigned)76666670);

  delete config;
}

TEST_F(ElectricityMeterConfigTest, extendedValue_V2) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":100.1,"
      "\"reverseActiveEnergy\":201.167,\"forwardReactiveEnergy\":1200.0001,"
      "\"reverseReactiveEnergy\":30001.1234,\"forwardActiveEnergyBalanced\":0."
      "00002,"
      "\"reverseActiveEnergyBalanced\":123.678}}");

  EXPECT_TRUE(config->update_available_counters(
      EM_VAR_REVERSE_ACTIVE_ENERGY | EM_VAR_REVERSE_REACTIVE_ENERGY |
      EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED));

  {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};

    config->add_initial_values(0, &em_ev);

    EXPECT_EQ(em_ev.total_forward_active_energy[0], (unsigned)0);
    EXPECT_EQ(em_ev.total_forward_active_energy[1], (unsigned)0);
    EXPECT_EQ(em_ev.total_forward_active_energy[2], (unsigned)0);

    EXPECT_EQ(em_ev.total_reverse_active_energy[0], (unsigned)6705568);
    EXPECT_EQ(em_ev.total_reverse_active_energy[1], (unsigned)6705566);
    EXPECT_EQ(em_ev.total_reverse_active_energy[2], (unsigned)6705566);

    EXPECT_EQ(em_ev.total_forward_reactive_energy[0], (unsigned)0);
    EXPECT_EQ(em_ev.total_forward_reactive_energy[1], (unsigned)0);
    EXPECT_EQ(em_ev.total_forward_reactive_energy[2], (unsigned)0);

    EXPECT_EQ(em_ev.total_reverse_reactive_energy[0], (unsigned)1000037448);
    EXPECT_EQ(em_ev.total_reverse_reactive_energy[1], (unsigned)1000037446);
    EXPECT_EQ(em_ev.total_reverse_reactive_energy[2], (unsigned)1000037446);

    EXPECT_EQ(em_ev.total_forward_active_energy_balanced, (unsigned)0);
    EXPECT_EQ(em_ev.total_reverse_active_energy_balanced, (unsigned)12367800);
  }

  EXPECT_TRUE(config->update_available_counters(0xFFFFFFFF));

  {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};

    config->add_initial_values(0, &em_ev);

    EXPECT_EQ(em_ev.total_forward_active_energy[0], (unsigned)3336668);
    EXPECT_EQ(em_ev.total_forward_active_energy[1], (unsigned)3336666);
    EXPECT_EQ(em_ev.total_forward_active_energy[2], (unsigned)3336666);

    EXPECT_EQ(em_ev.total_reverse_active_energy[0], (unsigned)6705568);
    EXPECT_EQ(em_ev.total_reverse_active_energy[1], (unsigned)6705566);
    EXPECT_EQ(em_ev.total_reverse_active_energy[2], (unsigned)6705566);

    EXPECT_EQ(em_ev.total_forward_reactive_energy[0], (unsigned)40000004);
    EXPECT_EQ(em_ev.total_forward_reactive_energy[1], (unsigned)40000003);
    EXPECT_EQ(em_ev.total_forward_reactive_energy[2], (unsigned)40000003);

    EXPECT_EQ(em_ev.total_reverse_reactive_energy[0], (unsigned)1000037448);
    EXPECT_EQ(em_ev.total_reverse_reactive_energy[1], (unsigned)1000037446);
    EXPECT_EQ(em_ev.total_reverse_reactive_energy[2], (unsigned)1000037446);

    EXPECT_EQ(em_ev.total_forward_active_energy_balanced, (unsigned)2);
    EXPECT_EQ(em_ev.total_reverse_active_energy_balanced, (unsigned)12367800);
  }

  {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};

    em_ev.total_forward_active_energy[0] = 1;
    em_ev.total_forward_active_energy[1] = 2;
    em_ev.total_forward_active_energy[2] = 3;

    em_ev.total_reverse_active_energy[0] = 10;
    em_ev.total_reverse_active_energy[1] = 20;
    em_ev.total_reverse_active_energy[2] = 30;

    em_ev.total_forward_reactive_energy[0] = 100;
    em_ev.total_forward_reactive_energy[1] = 200;
    em_ev.total_forward_reactive_energy[2] = 300;

    em_ev.total_reverse_reactive_energy[0] = 120;
    em_ev.total_reverse_reactive_energy[1] = 230;
    em_ev.total_reverse_reactive_energy[2] = 340;

    em_ev.total_forward_active_energy_balanced = 11;
    em_ev.total_reverse_active_energy_balanced = 36;

    config->add_initial_values(0, &em_ev);

    EXPECT_EQ(em_ev.total_forward_active_energy[0], (unsigned)3336669);
    EXPECT_EQ(em_ev.total_forward_active_energy[1], (unsigned)3336668);
    EXPECT_EQ(em_ev.total_forward_active_energy[2], (unsigned)3336669);

    EXPECT_EQ(em_ev.total_reverse_active_energy[0], (unsigned)6705578);
    EXPECT_EQ(em_ev.total_reverse_active_energy[1], (unsigned)6705586);
    EXPECT_EQ(em_ev.total_reverse_active_energy[2], (unsigned)6705596);

    EXPECT_EQ(em_ev.total_forward_reactive_energy[0], (unsigned)40000104);
    EXPECT_EQ(em_ev.total_forward_reactive_energy[1], (unsigned)40000203);
    EXPECT_EQ(em_ev.total_forward_reactive_energy[2], (unsigned)40000303);

    EXPECT_EQ(em_ev.total_reverse_reactive_energy[0], (unsigned)1000037568);
    EXPECT_EQ(em_ev.total_reverse_reactive_energy[1], (unsigned)1000037676);
    EXPECT_EQ(em_ev.total_reverse_reactive_energy[2], (unsigned)1000037786);

    EXPECT_EQ(em_ev.total_forward_active_energy_balanced, (unsigned)13);
    EXPECT_EQ(em_ev.total_reverse_active_energy_balanced, (unsigned)12367836);
  }

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardReactiveEnergy\":1200.0001}"
      "}");

  {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};

    config->add_initial_values(0, &em_ev);

    EXPECT_EQ(em_ev.total_forward_active_energy[0], (unsigned)0);
    EXPECT_EQ(em_ev.total_forward_active_energy[1], (unsigned)0);
    EXPECT_EQ(em_ev.total_forward_active_energy[2], (unsigned)0);

    EXPECT_EQ(em_ev.total_reverse_active_energy[0], (unsigned)0);
    EXPECT_EQ(em_ev.total_reverse_active_energy[1], (unsigned)0);
    EXPECT_EQ(em_ev.total_reverse_active_energy[2], (unsigned)0);

    EXPECT_EQ(em_ev.total_forward_reactive_energy[0], (unsigned)40000004);
    EXPECT_EQ(em_ev.total_forward_reactive_energy[1], (unsigned)40000003);
    EXPECT_EQ(em_ev.total_forward_reactive_energy[2], (unsigned)40000003);

    EXPECT_EQ(em_ev.total_reverse_reactive_energy[0], (unsigned)0);
    EXPECT_EQ(em_ev.total_reverse_reactive_energy[1], (unsigned)0);
    EXPECT_EQ(em_ev.total_reverse_reactive_energy[2], (unsigned)0);

    EXPECT_EQ(em_ev.total_forward_active_energy_balanced, (unsigned)0);
    EXPECT_EQ(em_ev.total_reverse_active_energy_balanced, (unsigned)0);
  }

  delete config;
}

TEST_F(ElectricityMeterConfigTest, channelExtendedValue_EV) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":100.1,"
      "\"reverseActiveEnergy\":201.167,\"forwardReactiveEnergy\":1200.0001,"
      "\"reverseReactiveEnergy\":30001.1234,\"forwardActiveEnergyBalanced\":0."
      "00002,"
      "\"reverseActiveEnergyBalanced\":123.678}}");

  EXPECT_TRUE(config->update_available_counters(
      EM_VAR_REVERSE_ACTIVE_ENERGY | EM_VAR_REVERSE_REACTIVE_ENERGY |
      EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED));

  TSuplaChannelExtendedValue ev = {};
  {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};
    srpc_evtool_v2_emextended2extended(&em_ev, &ev);
  }

  config->add_initial_values(0, (TSuplaChannelExtendedValue *)NULL);
  config->add_initial_values(0, &ev);

  {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};
    srpc_evtool_v2_extended2emextended(&ev, &em_ev);

    EXPECT_EQ(em_ev.total_forward_active_energy[0], (unsigned)0);
    EXPECT_EQ(em_ev.total_forward_active_energy[1], (unsigned)0);
    EXPECT_EQ(em_ev.total_forward_active_energy[2], (unsigned)0);

    EXPECT_EQ(em_ev.total_reverse_active_energy[0], (unsigned)6705568);
    EXPECT_EQ(em_ev.total_reverse_active_energy[1], (unsigned)6705566);
    EXPECT_EQ(em_ev.total_reverse_active_energy[2], (unsigned)6705566);

    EXPECT_EQ(em_ev.total_forward_reactive_energy[0], (unsigned)0);
    EXPECT_EQ(em_ev.total_forward_reactive_energy[1], (unsigned)0);
    EXPECT_EQ(em_ev.total_forward_reactive_energy[2], (unsigned)0);

    EXPECT_EQ(em_ev.total_reverse_reactive_energy[0], (unsigned)1000037448);
    EXPECT_EQ(em_ev.total_reverse_reactive_energy[1], (unsigned)1000037446);
    EXPECT_EQ(em_ev.total_reverse_reactive_energy[2], (unsigned)1000037446);

    EXPECT_EQ(em_ev.total_forward_active_energy_balanced, (unsigned)0);
    EXPECT_EQ(em_ev.total_reverse_active_energy_balanced, (unsigned)12367800);
  }

  delete config;
}

TEST_F(ElectricityMeterConfigTest, overValue) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"reverseReactiveEnergy\":30001."
      "1234}}");

  EXPECT_TRUE(
      config->update_available_counters(EM_VAR_REVERSE_REACTIVE_ENERGY));

  TElectricityMeter_ExtendedValue_V2 em_ev = {};

  em_ev.total_reverse_reactive_energy[0] = 0xFFFFFFFFFFFFFF00;

  config->add_initial_values(0, &em_ev);

  EXPECT_EQ(em_ev.total_reverse_reactive_energy[0], ULONG_MAX);
  EXPECT_EQ(em_ev.total_reverse_reactive_energy[1], (unsigned)1000037446);
  EXPECT_EQ(em_ev.total_reverse_reactive_energy[2], (unsigned)1000037446);

  delete config;
}

TEST_F(ElectricityMeterConfigTest, substractionAboveValue) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"reverseReactiveEnergy\":-30001."
      "1234}}");

  EXPECT_TRUE(
      config->update_available_counters(EM_VAR_REVERSE_REACTIVE_ENERGY));

  TElectricityMeter_ExtendedValue_V2 em_ev = {};

  em_ev.total_reverse_reactive_energy[0] = 1000037448;
  em_ev.total_reverse_reactive_energy[1] = 1;
  em_ev.total_reverse_reactive_energy[2] = 1000037447;

  config->add_initial_values(0, &em_ev);

  EXPECT_EQ(em_ev.total_reverse_reactive_energy[0], (unsigned)0);
  EXPECT_EQ(em_ev.total_reverse_reactive_energy[1], (unsigned)0);
  EXPECT_EQ(em_ev.total_reverse_reactive_energy[2], (unsigned)0);

  delete config;
}

TEST_F(ElectricityMeterConfigTest, substractionWithEqualPhases) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"reverseReactiveEnergy\":-300}}");

  EXPECT_TRUE(
      config->update_available_counters(EM_VAR_REVERSE_REACTIVE_ENERGY));

  TElectricityMeter_ExtendedValue_V2 em_ev = {};

  em_ev.total_reverse_reactive_energy[0] = 10000005;
  em_ev.total_reverse_reactive_energy[1] = 10000005;
  em_ev.total_reverse_reactive_energy[2] = 10000005;

  config->add_initial_values(0, &em_ev);

  EXPECT_EQ(em_ev.total_reverse_reactive_energy[0], (unsigned)5);
  EXPECT_EQ(em_ev.total_reverse_reactive_energy[1], (unsigned)5);
  EXPECT_EQ(em_ev.total_reverse_reactive_energy[2], (unsigned)5);

  delete config;
}

TEST_F(ElectricityMeterConfigTest, substractonWithPhaseImbalance) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":-3427.56}}");

  EXPECT_TRUE(config->update_available_counters(EM_VAR_FORWARD_ACTIVE_ENERGY));

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  TElectricityMeter_Value value = {};

  em_ev.total_forward_active_energy[0] = 64066352;
  em_ev.total_forward_active_energy[1] = 213112468;
  em_ev.total_forward_active_energy[2] = 65583238;

  value.total_forward_active_energy = (em_ev.total_forward_active_energy[0] +
                                       em_ev.total_forward_active_energy[1] +
                                       em_ev.total_forward_active_energy[2]) /
                                      1000;

  config->add_initial_values(0, &em_ev);
  config->add_initial_value(&value);

  EXPECT_EQ(value.total_forward_active_energy,
            (em_ev.total_forward_active_energy[0] +
             em_ev.total_forward_active_energy[1] +
             em_ev.total_forward_active_energy[2]) /
                1000);

  EXPECT_EQ(em_ev.total_forward_active_energy[0], (unsigned)0);
  EXPECT_EQ(em_ev.total_forward_active_energy[1], (unsigned)6058);
  EXPECT_EQ(em_ev.total_forward_active_energy[2], (unsigned)0);

  delete config;
}

TEST_F(ElectricityMeterConfigTest, channelValue) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":100.123,"
      "\"reverseActiveEnergy\":201.167,\"forwardReactiveEnergy\":1200.0001,"
      "\"reverseReactiveEnergy\":30001.1234,\"forwardActiveEnergyBalanced\":0."
      "00002,"
      "\"reverseActiveEnergyBalanced\":123.678}}");

  EXPECT_TRUE(config->update_available_counters(0xFFFFFFFF));

  TElectricityMeter_Value value = {};
  value.total_forward_active_energy = 5;

  config->add_initial_value(&value);
  EXPECT_EQ(value.total_forward_active_energy, (unsigned)10017);

  delete config;
}

TEST_F(ElectricityMeterConfigTest, channelValueOverflow) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":10000000}}");

  EXPECT_TRUE(config->update_available_counters(0xFFFFFFFF));

  TElectricityMeter_Value value = {};
  value.total_forward_active_energy = UINT_MAX - 1000;

  config->add_initial_value(&value);
  EXPECT_EQ(value.total_forward_active_energy, UINT_MAX);

  delete config;
}

TEST_F(ElectricityMeterConfigTest, channelValueNeg) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":-10000000}"
      "}");

  EXPECT_TRUE(config->update_available_counters(0xFFFFFFFF));

  TElectricityMeter_Value value = {};
  value.total_forward_active_energy = 1000;

  config->add_initial_value(&value);
  EXPECT_EQ(value.total_forward_active_energy, (unsigned int)0);

  value.total_forward_active_energy = 1000000001;

  config->add_initial_value(&value);
  EXPECT_EQ(value.total_forward_active_energy, (unsigned int)1);

  delete config;
}

TEST_F(ElectricityMeterConfigTest, addToHitorryFlagTest) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  ASSERT_FALSE(config->should_be_added_to_history());

  config->set_user_config("{\"addToHistory\":false}");

  ASSERT_FALSE(config->should_be_added_to_history());

  config->set_user_config("{\"addToHistory\":true}");

  ASSERT_TRUE(config->should_be_added_to_history());

  delete config;
}

TEST_F(ElectricityMeterConfigTest, upperVoltageThreshold) {
  electricity_meter_config config;
  EXPECT_EQ(config.get_upper_voltage_threshold(), 0);

  config.set_user_config("{\"upperVoltageThreshold\":null}");

  EXPECT_EQ(config.get_upper_voltage_threshold(), 0);

  config.set_user_config("{\"upperVoltageThreshold\":268.45}");

  EXPECT_EQ(config.get_upper_voltage_threshold(), 268.45);
}

TEST_F(ElectricityMeterConfigTest, lowerVoltageThreshold) {
  electricity_meter_config config;
  EXPECT_EQ(config.get_lower_voltage_threshold(), 0);

  config.set_user_config("{\"lowerVoltageThreshold\":null}");

  EXPECT_EQ(config.get_upper_voltage_threshold(), 0);

  config.set_user_config("{\"lowerVoltageThreshold\":123.45}");

  EXPECT_EQ(config.get_lower_voltage_threshold(), 123.45);
}

TEST_F(ElectricityMeterConfigTest, disabledPhases) {
  electricity_meter_config config;

  EXPECT_FALSE(config.is_phase_disabled(1));
  EXPECT_FALSE(config.is_phase_disabled(2));
  EXPECT_FALSE(config.is_phase_disabled(3));
  EXPECT_EQ(config.get_channel_user_flags(), 0);

  config.set_user_config("{\"disabledPhases\":[5,6,4,0]}");

  EXPECT_FALSE(config.is_phase_disabled(1));
  EXPECT_FALSE(config.is_phase_disabled(2));
  EXPECT_FALSE(config.is_phase_disabled(3));
  EXPECT_EQ(config.get_channel_user_flags(), 0);

  config.set_user_config("{\"disabledPhases\":[3]}");

  EXPECT_FALSE(config.is_phase_disabled(1));
  EXPECT_FALSE(config.is_phase_disabled(2));
  EXPECT_TRUE(config.is_phase_disabled(3));
  EXPECT_EQ(config.get_channel_user_flags(),
            SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED);

  config.set_user_config("{\"disabledPhases\":[5,6,2,0]}");

  EXPECT_FALSE(config.is_phase_disabled(1));
  EXPECT_TRUE(config.is_phase_disabled(2));
  EXPECT_FALSE(config.is_phase_disabled(3));
  EXPECT_EQ(config.get_channel_user_flags(),
            SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED);

  config.set_user_config("{\"disabledPhases\":[5,1]}");

  EXPECT_TRUE(config.is_phase_disabled(1));
  EXPECT_FALSE(config.is_phase_disabled(2));
  EXPECT_FALSE(config.is_phase_disabled(3));
  EXPECT_EQ(config.get_channel_user_flags(),
            SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED);

  config.set_user_config("{\"disabledPhases\":[1,2,3]}");

  EXPECT_TRUE(config.is_phase_disabled(1));
  EXPECT_TRUE(config.is_phase_disabled(2));
  EXPECT_TRUE(config.is_phase_disabled(3));

  EXPECT_EQ(config.get_channel_user_flags(),
            SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
                SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED);

  config.set_user_config("{\"disabledPhases\":[2,3,1]}");

  EXPECT_TRUE(config.is_phase_disabled(1));
  EXPECT_TRUE(config.is_phase_disabled(2));
  EXPECT_TRUE(config.is_phase_disabled(3));

  EXPECT_EQ(config.get_channel_user_flags(),
            SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
                SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED);
}

} /* namespace testing */
