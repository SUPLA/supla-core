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

#include <analyzer/ElectricityAnalyzerTest.h>

#include "device/extended_value/channel_em_extended_value.h"
#include "jsonconfig/channel/electricity_meter_config.h"
#include "srpc/srpc.h"

namespace testing {

TEST_F(ElectricityAnalyzerTest, channelIdSetterAndGetter) {
  EXPECT_EQ(ea.get_channel_id(), 0);
  ea.set_channel_id(456);
  EXPECT_EQ(ea.get_channel_id(), 456);
}

TEST_F(ElectricityAnalyzerTest, onePhase) {
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
  EXPECT_EQ(ea.get_aberration_phase1()->get_max(), 310.55);
}

TEST_F(ElectricityAnalyzerTest, threePhases) {
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
  EXPECT_EQ(ea.get_aberration_phase1()->get_max(), 310.55);

  EXPECT_EQ(ea.get_aberration_phase2()->get_above_count(), 1);
  EXPECT_EQ(ea.get_aberration_phase2()->get_max(), 315.55);

  EXPECT_EQ(ea.get_aberration_phase3()->get_above_count(), 1);
  EXPECT_EQ(ea.get_aberration_phase3()->get_max(), 320.55);
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
  em_ev.measured_values = EM_VAR_VOLTAGE;

  supla_channel_em_extended_value em(&em_ev, nullptr, 0);

  electricity_meter_config config;
  config.set_user_config(
      "{\"lowerVoltageThreshold\":100,\"upperVoltageThreshold\":300}");

  ea.add_sample(0, &config, &em);

  ASSERT_TRUE(ea.get_aberration_phase1() != nullptr);
  ASSERT_TRUE(ea.get_aberration_phase2() != nullptr);
  ASSERT_TRUE(ea.get_aberration_phase3() != nullptr);

  ea.set_channel_id(12345);

  supla_electricity_analyzer *ea2 =
      dynamic_cast<supla_electricity_analyzer *>(ea.copy());
  ASSERT_TRUE(ea2 != nullptr);

  ASSERT_TRUE(ea2->get_aberration_phase1() != nullptr);
  ASSERT_TRUE(ea2->get_aberration_phase2() != nullptr);
  ASSERT_TRUE(ea2->get_aberration_phase3() != nullptr);

  EXPECT_TRUE(ea2->get_aberration_phase1() != ea.get_aberration_phase1());
  EXPECT_TRUE(ea2->get_aberration_phase2() != ea.get_aberration_phase2());
  EXPECT_TRUE(ea2->get_aberration_phase3() != ea.get_aberration_phase3());

  EXPECT_EQ(ea2->get_aberration_phase1()->get_max(),
            ea.get_aberration_phase1()->get_max());
  EXPECT_EQ(ea2->get_aberration_phase2()->get_max(),
            ea.get_aberration_phase2()->get_max());
  EXPECT_EQ(ea2->get_aberration_phase3()->get_max(),
            ea.get_aberration_phase3()->get_max());

  EXPECT_EQ(ea2->get_channel_id(), ea.get_channel_id());
  delete ea2;
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
      "{\"lowerVoltageThreshold\":100,\"upperVoltageThreshold\":300}");

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

}  // namespace testing
