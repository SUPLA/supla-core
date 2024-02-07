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

#include "ElectricityAnalyzerTest.h"

#include "device/extended_value/channel_em_extended_value.h"
#include "jsonconfig/channel/electricity_meter_config.h"
#include "srpc/srpc.h"

namespace testing {

TEST_F(ElectricityAnalyzerTest, channelIdSetterAndGetter) {
  EXPECT_EQ(ea.get_channel_id(), 0);
  ea.set_channel_id(456);
  EXPECT_EQ(ea.get_channel_id(), 456);
}

TEST_F(ElectricityAnalyzerTest, aberrationOnePhase) {
  EXPECT_TRUE(ea.get_aberration_phase1() == nullptr);
  EXPECT_TRUE(ea.get_aberration_phase2() == nullptr);
  EXPECT_TRUE(ea.get_aberration_phase3() == nullptr);

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].voltage[0] = 31055;
  em_ev.m[0].voltage[1] = 31555;
  em_ev.m[0].voltage[2] = 32055;
  em_ev.measured_values = EM_VAR_VOLTAGE;

  supla_channel_em_extended_value em(&em_ev, nullptr, 0);

  electricity_meter_config config;
  config.set_user_config(
      "{\"lowerVoltageThreshold\":100,\"upperVoltageThreshold\":300}");

  ea.add_sample(SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
                    SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED,
                &config, &em);

  ASSERT_TRUE(ea.get_aberration_phase1() != nullptr);
  EXPECT_TRUE(ea.get_aberration_phase2() == nullptr);
  EXPECT_TRUE(ea.get_aberration_phase3() == nullptr);

  EXPECT_EQ(ea.get_aberration_phase1()->get_above_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_aberration_phase1()->get_max(), 310.55);
}

TEST_F(ElectricityAnalyzerTest, aberrationThreePhases) {
  EXPECT_TRUE(ea.get_aberration_phase1() == nullptr);
  EXPECT_TRUE(ea.get_aberration_phase2() == nullptr);
  EXPECT_TRUE(ea.get_aberration_phase3() == nullptr);

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].voltage[0] = 31055;
  em_ev.m[0].voltage[1] = 31555;
  em_ev.m[0].voltage[2] = 32055;
  em_ev.measured_values = EM_VAR_VOLTAGE;

  TSuplaChannelExtendedValue ev = {};
  srpc_evtool_v2_emextended2extended(&em_ev, &ev);
  supla_channel_em_extended_value em(&ev, nullptr, 0);

  electricity_meter_config config;
  config.set_user_config(
      "{\"lowerVoltageThreshold\":100,\"upperVoltageThreshold\":300}");

  ea.add_sample(0, &config, &em);

  ASSERT_TRUE(ea.get_aberration_phase1() != nullptr);
  ASSERT_TRUE(ea.get_aberration_phase2() != nullptr);
  ASSERT_TRUE(ea.get_aberration_phase3() != nullptr);

  EXPECT_EQ(ea.get_aberration_phase1()->get_above_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_aberration_phase1()->get_max(), 310.55);

  EXPECT_EQ(ea.get_aberration_phase2()->get_above_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_aberration_phase2()->get_max(), 315.55);

  EXPECT_EQ(ea.get_aberration_phase3()->get_above_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_aberration_phase3()->get_max(), 320.55);
}

TEST_F(ElectricityAnalyzerTest, noMeasurements) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.m_count = 0;
  em_ev.m[0].voltage[0] = 31055;
  em_ev.m[0].voltage[1] = 31555;
  em_ev.m[0].voltage[2] = 32055;
  em_ev.measured_values = EM_VAR_VOLTAGE;

  TSuplaChannelExtendedValue ev = {};
  srpc_evtool_v2_emextended2extended(&em_ev, &ev);
  supla_channel_em_extended_value em(&ev, nullptr, 0);

  electricity_meter_config config;
  config.set_user_config(
      "{\"lowerVoltageThreshold\":100,\"upperVoltageThreshold\":300}");

  ea.add_sample(0, &config, &em);

  ASSERT_TRUE(ea.get_aberration_phase1() == nullptr);
  ASSERT_TRUE(ea.get_aberration_phase2() == nullptr);
  ASSERT_TRUE(ea.get_aberration_phase3() == nullptr);
}

TEST_F(ElectricityAnalyzerTest, voltageIsNotMeasured) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.m_count = 1;

  supla_channel_em_extended_value em(&em_ev, nullptr, 0);

  electricity_meter_config config;
  config.set_user_config(
      "{\"lowerVoltageThreshold\":100,\"upperVoltageThreshold\":300}");

  ea.add_sample(0, &config, &em);

  ASSERT_TRUE(ea.get_aberration_phase1() == nullptr);
  ASSERT_TRUE(ea.get_aberration_phase2() == nullptr);
  ASSERT_TRUE(ea.get_aberration_phase3() == nullptr);
}

TEST_F(ElectricityAnalyzerTest, thresholdsAreNotSet) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].voltage[0] = 31055;
  em_ev.m[0].voltage[1] = 31555;
  em_ev.m[0].voltage[2] = 32055;
  em_ev.measured_values = EM_VAR_VOLTAGE;

  TSuplaChannelExtendedValue ev = {};
  srpc_evtool_v2_emextended2extended(&em_ev, &ev);
  supla_channel_em_extended_value em(&ev, nullptr, 0);

  electricity_meter_config config;
  config.set_user_config(
      "{\"lowerVoltageThreshold\":0,\"upperVoltageThreshold\":0}");

  ea.add_sample(0, &config, &em);

  ASSERT_TRUE(ea.get_aberration_phase1() == nullptr);
  ASSERT_TRUE(ea.get_aberration_phase2() == nullptr);
  ASSERT_TRUE(ea.get_aberration_phase3() == nullptr);
}

TEST_F(ElectricityAnalyzerTest, copy) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].voltage[0] = 31055;
  em_ev.m[0].voltage[1] = 31555;
  em_ev.m[0].voltage[2] = 32055;
  em_ev.m[0].current[0] = 10000;
  em_ev.m[0].current[1] = 20000;
  em_ev.m[0].current[2] = 30000;
  em_ev.m[0].power_active[0] = 100000;
  em_ev.m[0].power_active[1] = 200000;
  em_ev.m[0].power_active[2] = 300000;
  em_ev.measured_values = EM_VAR_VOLTAGE | EM_VAR_CURRENT | EM_VAR_POWER_ACTIVE;

  supla_channel_em_extended_value em(&em_ev, nullptr, 0);

  electricity_meter_config config;
  config.set_user_config(
      "{\"voltageLoggerEnabled\":true,\"currentLoggerEnabled\":true,"
      "\"powerActiveLoggerEnabled\":true,\"lowerVoltageThreshold\":100,"
      "\"upperVoltageThreshold\":300}");

  ea.add_sample(0, &config, &em);

  ASSERT_TRUE(ea.get_aberration_phase1() != nullptr);
  ASSERT_TRUE(ea.get_aberration_phase2() != nullptr);
  ASSERT_TRUE(ea.get_aberration_phase3() != nullptr);

  ASSERT_TRUE(ea.get_voltage_phase1() != nullptr);
  ASSERT_TRUE(ea.get_voltage_phase2() != nullptr);
  ASSERT_TRUE(ea.get_voltage_phase3() != nullptr);

  ASSERT_TRUE(ea.get_current_phase1() != nullptr);
  ASSERT_TRUE(ea.get_current_phase2() != nullptr);
  ASSERT_TRUE(ea.get_current_phase3() != nullptr);

  ASSERT_TRUE(ea.get_power_active_phase1() != nullptr);
  ASSERT_TRUE(ea.get_power_active_phase2() != nullptr);
  ASSERT_TRUE(ea.get_power_active_phase3() != nullptr);

  ea.set_channel_id(12345);

  supla_electricity_analyzer *ea2 =
      dynamic_cast<supla_electricity_analyzer *>(ea.copy());
  ASSERT_TRUE(ea2 != nullptr);

  ASSERT_TRUE(ea2->get_aberration_phase1() != nullptr);
  ASSERT_TRUE(ea2->get_aberration_phase2() != nullptr);
  ASSERT_TRUE(ea2->get_aberration_phase3() != nullptr);

  ASSERT_TRUE(ea2->get_voltage_phase1() != nullptr);
  ASSERT_TRUE(ea2->get_voltage_phase2() != nullptr);
  ASSERT_TRUE(ea2->get_voltage_phase3() != nullptr);

  ASSERT_TRUE(ea2->get_current_phase1() != nullptr);
  ASSERT_TRUE(ea2->get_current_phase2() != nullptr);
  ASSERT_TRUE(ea2->get_current_phase3() != nullptr);

  ASSERT_TRUE(ea2->get_power_active_phase1() != nullptr);
  ASSERT_TRUE(ea2->get_power_active_phase2() != nullptr);
  ASSERT_TRUE(ea2->get_power_active_phase3() != nullptr);

  EXPECT_TRUE(ea2->get_aberration_phase1() != ea.get_aberration_phase1());
  EXPECT_TRUE(ea2->get_aberration_phase2() != ea.get_aberration_phase2());
  EXPECT_TRUE(ea2->get_aberration_phase3() != ea.get_aberration_phase3());

  EXPECT_TRUE(ea2->get_voltage_phase1() != ea.get_voltage_phase1());
  EXPECT_TRUE(ea2->get_voltage_phase2() != ea.get_voltage_phase2());
  EXPECT_TRUE(ea2->get_voltage_phase3() != ea.get_voltage_phase3());

  EXPECT_TRUE(ea2->get_current_phase1() != ea.get_current_phase1());
  EXPECT_TRUE(ea2->get_current_phase2() != ea.get_current_phase2());
  EXPECT_TRUE(ea2->get_current_phase3() != ea.get_current_phase3());

  EXPECT_TRUE(ea2->get_power_active_phase1() != ea.get_power_active_phase1());
  EXPECT_TRUE(ea2->get_power_active_phase2() != ea.get_power_active_phase2());
  EXPECT_TRUE(ea2->get_power_active_phase3() != ea.get_power_active_phase3());

  EXPECT_DOUBLE_EQ(ea2->get_aberration_phase1()->get_max(),
                   ea.get_aberration_phase1()->get_max());
  EXPECT_DOUBLE_EQ(ea2->get_aberration_phase2()->get_max(),
                   ea.get_aberration_phase2()->get_max());
  EXPECT_DOUBLE_EQ(ea2->get_aberration_phase3()->get_max(),
                   ea.get_aberration_phase3()->get_max());

  EXPECT_DOUBLE_EQ(ea2->get_voltage_phase1()->get_max(),
                   ea.get_voltage_phase1()->get_max());
  EXPECT_DOUBLE_EQ(ea2->get_voltage_phase2()->get_max(),
                   ea.get_voltage_phase2()->get_max());
  EXPECT_DOUBLE_EQ(ea2->get_voltage_phase3()->get_max(),
                   ea.get_voltage_phase3()->get_max());

  EXPECT_DOUBLE_EQ(ea2->get_current_phase1()->get_max(),
                   ea.get_current_phase1()->get_max());
  EXPECT_DOUBLE_EQ(ea2->get_current_phase2()->get_max(),
                   ea.get_current_phase2()->get_max());
  EXPECT_DOUBLE_EQ(ea2->get_current_phase3()->get_max(),
                   ea.get_current_phase3()->get_max());

  EXPECT_DOUBLE_EQ(ea2->get_power_active_phase1()->get_max(),
                   ea.get_power_active_phase1()->get_max());
  EXPECT_DOUBLE_EQ(ea2->get_power_active_phase2()->get_max(),
                   ea.get_power_active_phase2()->get_max());
  EXPECT_DOUBLE_EQ(ea2->get_power_active_phase3()->get_max(),
                   ea.get_power_active_phase3()->get_max());

  EXPECT_EQ(ea2->get_channel_id(), ea.get_channel_id());
  delete ea2;
}

TEST_F(ElectricityAnalyzerTest, reset) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].voltage[0] = 31055;
  em_ev.m[0].voltage[1] = 31555;
  em_ev.m[0].voltage[2] = 32055;
  em_ev.m[0].current[0] = 10000;
  em_ev.m[0].current[1] = 20000;
  em_ev.m[0].current[2] = 30000;
  em_ev.m[0].power_active[0] = 100000;
  em_ev.m[0].power_active[1] = 200000;
  em_ev.m[0].power_active[2] = 300000;
  em_ev.measured_values = EM_VAR_VOLTAGE | EM_VAR_CURRENT | EM_VAR_POWER_ACTIVE;

  supla_channel_em_extended_value em(&em_ev, nullptr, 0);

  electricity_meter_config config;
  config.set_user_config(
      "{\"voltageLoggerEnabled\":true,\"currentLoggerEnabled\":true,"
      "\"powerActiveLoggerEnabled\":true,\"lowerVoltageThreshold\":100,"
      "\"upperVoltageThreshold\":300}");

  ea.add_sample(0, &config, &em);

  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_aberration_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_voltage_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_current_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_power_active_for_logging_purpose());

  ea.reset_power_active();

  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_aberration_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_voltage_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_current_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_power_active_for_logging_purpose());

  ea.reset_current();

  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_aberration_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_voltage_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_current_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_power_active_for_logging_purpose());

  ea.reset_voltage();

  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_aberration_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_voltage_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_current_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_power_active_for_logging_purpose());

  ea.reset_aberrations();

  EXPECT_FALSE(ea.is_any_data_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_aberration_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_voltage_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_current_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_power_active_for_logging_purpose());

  ea.add_sample(0, &config, &em);

  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_aberration_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_voltage_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_current_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_power_active_for_logging_purpose());

  ea.reset();

  EXPECT_FALSE(ea.is_any_data_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_aberration_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_voltage_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_current_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_power_active_for_logging_purpose());
}

TEST_F(ElectricityAnalyzerTest, resetTestAndCheckingIfAnyThresholdIsExceeded) {
  EXPECT_FALSE(ea.is_any_data_for_logging_purpose());

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].voltage[0] = 31055;
  em_ev.m[0].voltage[1] = 20000;
  em_ev.m[0].voltage[2] = 20000;
  em_ev.measured_values = EM_VAR_VOLTAGE;

  supla_channel_em_extended_value em(&em_ev, nullptr, 0);

  electricity_meter_config config;
  config.set_user_config(
      "{\"voltageLoggerEnabled\":true,\"currentLoggerEnabled\":true,"
      "\"powerActiveLoggerEnabled\":true,\"lowerVoltageThreshold\":100,"
      "\"upperVoltageThreshold\":300}");

  ea.add_sample(0, &config, &em);

  ASSERT_TRUE(ea.get_aberration_phase1() != nullptr);
  ASSERT_TRUE(ea.get_aberration_phase2() != nullptr);
  ASSERT_TRUE(ea.get_aberration_phase3() != nullptr);

  EXPECT_EQ(ea.get_aberration_phase1()->get_above_count(), 1);
  EXPECT_EQ(ea.get_aberration_phase2()->get_above_count(), 0);
  EXPECT_EQ(ea.get_aberration_phase3()->get_above_count(), 0);

  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  ea.reset();
  EXPECT_FALSE(ea.is_any_data_for_logging_purpose());

  em_ev.m[0].voltage[0] = 20000;
  em_ev.m[0].voltage[1] = 31055;

  em.set_raw_value(&em_ev);
  ea.add_sample(0, &config, &em);

  ASSERT_TRUE(ea.get_aberration_phase1() != nullptr);
  ASSERT_TRUE(ea.get_aberration_phase2() != nullptr);
  ASSERT_TRUE(ea.get_aberration_phase3() != nullptr);

  EXPECT_EQ(ea.get_aberration_phase1()->get_above_count(), 0);
  EXPECT_EQ(ea.get_aberration_phase2()->get_above_count(), 1);
  EXPECT_EQ(ea.get_aberration_phase3()->get_above_count(), 0);

  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  ea.reset();
  EXPECT_FALSE(ea.is_any_data_for_logging_purpose());

  em_ev.m[0].voltage[1] = 20000;
  em_ev.m[0].voltage[2] = 31055;

  em.set_raw_value(&em_ev);
  ea.add_sample(0, &config, &em);

  ASSERT_TRUE(ea.get_aberration_phase1() != nullptr);
  ASSERT_TRUE(ea.get_aberration_phase2() != nullptr);
  ASSERT_TRUE(ea.get_aberration_phase3() != nullptr);

  EXPECT_EQ(ea.get_aberration_phase1()->get_above_count(), 0);
  EXPECT_EQ(ea.get_aberration_phase2()->get_above_count(), 0);
  EXPECT_EQ(ea.get_aberration_phase3()->get_above_count(), 1);

  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  ea.reset();
  EXPECT_FALSE(ea.is_any_data_for_logging_purpose());
}

TEST_F(ElectricityAnalyzerTest, voltage) {
  EXPECT_TRUE(ea.get_voltage_phase1() == nullptr);
  EXPECT_TRUE(ea.get_voltage_phase2() == nullptr);
  EXPECT_TRUE(ea.get_voltage_phase3() == nullptr);
  EXPECT_FALSE(ea.is_any_data_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_voltage_for_logging_purpose());

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].voltage[0] = 31055;
  em_ev.m[0].voltage[1] = 31555;
  em_ev.m[0].voltage[2] = 32055;
  em_ev.measured_values = EM_VAR_VOLTAGE;

  supla_channel_em_extended_value em(&em_ev, nullptr, 0);

  electricity_meter_config config;
  ea.add_sample(0, &config, &em);

  EXPECT_FALSE(ea.is_any_data_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_voltage_for_logging_purpose());

  config.set_user_config("{\"voltageLoggerEnabled\":true}");

  ea.add_sample(SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
                    SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED,
                &config, &em);

  ASSERT_TRUE(ea.get_voltage_phase1() != nullptr);
  EXPECT_TRUE(ea.get_voltage_phase2() == nullptr);
  EXPECT_TRUE(ea.get_voltage_phase3() == nullptr);
  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_voltage_for_logging_purpose());

  EXPECT_EQ(ea.get_voltage_phase1()->get_sample_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_voltage_phase1()->get_max(), 310.55);

  ea.add_sample(SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                    SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED,
                &config, &em);

  ASSERT_TRUE(ea.get_voltage_phase1() != nullptr);
  EXPECT_TRUE(ea.get_voltage_phase2() != nullptr);
  EXPECT_TRUE(ea.get_voltage_phase3() == nullptr);
  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_voltage_for_logging_purpose());

  ea.add_sample(SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                    SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED,
                &config, &em);

  ASSERT_TRUE(ea.get_voltage_phase1() != nullptr);
  EXPECT_TRUE(ea.get_voltage_phase2() != nullptr);
  EXPECT_TRUE(ea.get_voltage_phase3() != nullptr);
  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_voltage_for_logging_purpose());

  EXPECT_EQ(ea.get_voltage_phase1()->get_sample_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_voltage_phase1()->get_max(), 310.55);

  EXPECT_EQ(ea.get_voltage_phase2()->get_sample_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_voltage_phase2()->get_max(), 315.55);

  EXPECT_EQ(ea.get_voltage_phase3()->get_sample_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_voltage_phase3()->get_max(), 320.55);

  ea.reset_voltage();
  EXPECT_FALSE(ea.is_any_voltage_for_logging_purpose());
}

TEST_F(ElectricityAnalyzerTest, current) {
  EXPECT_TRUE(ea.get_current_phase1() == nullptr);
  EXPECT_TRUE(ea.get_current_phase2() == nullptr);
  EXPECT_TRUE(ea.get_current_phase3() == nullptr);
  EXPECT_FALSE(ea.is_any_data_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_current_for_logging_purpose());

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].current[0] = 31055;
  em_ev.m[0].current[1] = 31555;
  em_ev.m[0].current[2] = 32055;
  em_ev.measured_values = EM_VAR_CURRENT;

  supla_channel_em_extended_value em(&em_ev, nullptr, 0);

  electricity_meter_config config;
  ea.add_sample(0, &config, &em);

  EXPECT_FALSE(ea.is_any_data_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_current_for_logging_purpose());

  config.set_user_config("{\"currentLoggerEnabled\":true}");

  ea.add_sample(SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
                    SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED,
                &config, &em);

  ASSERT_TRUE(ea.get_current_phase1() != nullptr);

  EXPECT_TRUE(ea.get_current_phase2() == nullptr);
  EXPECT_TRUE(ea.get_current_phase3() == nullptr);
  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_current_for_logging_purpose());

  EXPECT_EQ(ea.get_current_phase1()->get_sample_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_current_phase1()->get_max(), 31.055);

  ea.add_sample(SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                    SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED,
                &config, &em);

  ASSERT_TRUE(ea.get_current_phase1() != nullptr);
  EXPECT_TRUE(ea.get_current_phase2() != nullptr);
  EXPECT_TRUE(ea.get_current_phase3() == nullptr);
  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_current_for_logging_purpose());

  ea.add_sample(SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                    SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED,
                &config, &em);

  ASSERT_TRUE(ea.get_current_phase1() != nullptr);
  EXPECT_TRUE(ea.get_current_phase2() != nullptr);
  EXPECT_TRUE(ea.get_current_phase3() != nullptr);
  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_current_for_logging_purpose());

  EXPECT_EQ(ea.get_current_phase1()->get_sample_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_current_phase1()->get_max(), 31.055);

  EXPECT_EQ(ea.get_current_phase2()->get_sample_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_current_phase2()->get_max(), 31.555);

  EXPECT_EQ(ea.get_current_phase3()->get_sample_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_current_phase3()->get_max(), 32.055);

  ea.reset_current();
  EXPECT_FALSE(ea.is_any_current_for_logging_purpose());

  em_ev.measured_values = EM_VAR_CURRENT_OVER_65A;
  em.set_raw_value(&em_ev);

  ea.add_sample(0, &config, &em);

  ASSERT_TRUE(ea.get_current_phase1() != nullptr);
  EXPECT_DOUBLE_EQ(ea.get_current_phase1()->get_max(), 310.55);
}

TEST_F(ElectricityAnalyzerTest, powerActive) {
  EXPECT_TRUE(ea.get_power_active_phase1() == nullptr);
  EXPECT_TRUE(ea.get_power_active_phase2() == nullptr);
  EXPECT_TRUE(ea.get_power_active_phase3() == nullptr);
  EXPECT_FALSE(ea.is_any_data_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_power_active_for_logging_purpose());

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].power_active[0] = 310550;
  em_ev.m[0].power_active[1] = 315550;
  em_ev.m[0].power_active[2] = 320550;
  em_ev.measured_values = EM_VAR_POWER_ACTIVE;

  supla_channel_em_extended_value em(&em_ev, nullptr, 0);

  electricity_meter_config config;
  ea.add_sample(0, &config, &em);

  EXPECT_FALSE(ea.is_any_data_for_logging_purpose());
  EXPECT_FALSE(ea.is_any_power_active_for_logging_purpose());

  config.set_user_config("{\"powerActiveLoggerEnabled\":true}");

  ea.add_sample(SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
                    SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED,
                &config, &em);

  ASSERT_TRUE(ea.get_power_active_phase1() != nullptr);
  EXPECT_TRUE(ea.get_power_active_phase2() == nullptr);
  EXPECT_TRUE(ea.get_power_active_phase3() == nullptr);
  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_power_active_for_logging_purpose());

  EXPECT_EQ(ea.get_power_active_phase1()->get_sample_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_power_active_phase1()->get_max(), 3.1055);

  ea.add_sample(SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                    SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED,
                &config, &em);

  ASSERT_TRUE(ea.get_power_active_phase1() != nullptr);
  EXPECT_TRUE(ea.get_power_active_phase2() != nullptr);
  EXPECT_TRUE(ea.get_power_active_phase3() == nullptr);
  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_power_active_for_logging_purpose());

  ea.add_sample(SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                    SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED,
                &config, &em);

  ASSERT_TRUE(ea.get_power_active_phase1() != nullptr);
  EXPECT_TRUE(ea.get_power_active_phase2() != nullptr);
  EXPECT_TRUE(ea.get_power_active_phase3() != nullptr);
  EXPECT_TRUE(ea.is_any_data_for_logging_purpose());
  EXPECT_TRUE(ea.is_any_power_active_for_logging_purpose());

  EXPECT_EQ(ea.get_power_active_phase1()->get_sample_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_power_active_phase1()->get_max(), 3.1055);

  EXPECT_EQ(ea.get_power_active_phase2()->get_sample_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_power_active_phase2()->get_max(), 3.1555);

  EXPECT_EQ(ea.get_power_active_phase3()->get_sample_count(), 1);
  EXPECT_DOUBLE_EQ(ea.get_power_active_phase3()->get_max(), 3.2055);

  ea.reset_power_active();
  EXPECT_FALSE(ea.is_any_current_for_logging_purpose());

  em_ev.measured_values |= EM_VAR_POWER_ACTIVE_KW;
  em.set_raw_value(&em_ev);

  ea.add_sample(0, &config, &em);

  ASSERT_TRUE(ea.get_power_active_phase1() != nullptr);
  EXPECT_DOUBLE_EQ(ea.get_power_active_phase1()->get_max(), 3105.5);
}

}  // namespace testing
