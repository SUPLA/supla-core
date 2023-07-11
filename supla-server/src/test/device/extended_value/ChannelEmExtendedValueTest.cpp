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

#include "ChannelEmExtendedValueTest.h"

#include "TestHelper.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "log.h"
#include "srpc/srpc.h"

namespace testing {

TEST_F(ChannelEmExtendedValueTest, defaults) {
  TSuplaChannelExtendedValue value = {};
  supla_channel_em_extended_value ev(&value, "", 0);

  EXPECT_EQ(ev.get_currency(), "");
  EXPECT_EQ(ev.get_total_cost(), 0.0);
  EXPECT_EQ(ev.get_total_cost_balanced(), 0.0);
  EXPECT_EQ(ev.get_price_per_unit(), 0.0);
}

TEST_F(ChannelEmExtendedValueTest, converstionV1toV2) {
  TElectricityMeter_ExtendedValue v1 = {};
  TSuplaChannelExtendedValue value = {};

  v1.total_forward_active_energy[0] = 300000;
  v1.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2] = 45;
  v1.m_count = EM_MEASUREMENT_COUNT;

  srpc_evtool_v1_emextended2extended(&v1, &value);

  supla_channel_em_extended_value ev(&value, "PLN", 1234);

  EXPECT_EQ(ev.get_currency(), "PLN");
  EXPECT_DOUBLE_EQ(ev.get_total_cost(), 0.37);
  EXPECT_DOUBLE_EQ(ev.get_total_cost_balanced(), 0.0);
  EXPECT_DOUBLE_EQ(ev.get_price_per_unit(), 0.1234);
  EXPECT_EQ(ev.get_measured_values(), 0);

  TElectricityMeter_ExtendedValue_V2 v2 = {};
  ev.get_raw_value(&v2);

  EXPECT_EQ(v2.currency[0], 'P');
  EXPECT_EQ(v2.currency[1], 'L');
  EXPECT_EQ(v2.currency[2], 'N');
  EXPECT_EQ(v2.total_cost, 37);

  EXPECT_EQ(v1.m_count, v2.m_count);
  EXPECT_EQ(v1.total_forward_active_energy[0],
            v2.total_forward_active_energy[0]);
  EXPECT_EQ(v1.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2],
            v2.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2]);
}

TEST_F(ChannelEmExtendedValueTest, verifyGetters) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  TSuplaChannelExtendedValue value = {};

  v2.total_forward_active_energy[2] = 5000000;
  v2.total_forward_active_energy_balanced = 4500000;
  v2.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2] = 150;
  v2.m_count = EM_MEASUREMENT_COUNT;
  v2.measured_values = EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED;

  srpc_evtool_v2_emextended2extended(&v2, &value);

  supla_channel_em_extended_value ev(&value, "EUR", 245);

  EXPECT_EQ(ev.get_currency(), "EUR");
  EXPECT_DOUBLE_EQ(ev.get_total_cost(), 1.22);
  EXPECT_DOUBLE_EQ(ev.get_total_cost_balanced(), 1.10);
  EXPECT_DOUBLE_EQ(ev.get_price_per_unit(), 0.0245);
  EXPECT_EQ(ev.get_measured_values(), EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED);

  TElectricityMeter_ExtendedValue_V2 v2b = {};
  ev.get_raw_value(&v2b);

  EXPECT_EQ(v2b.currency[0], 'E');
  EXPECT_EQ(v2b.currency[1], 'U');
  EXPECT_EQ(v2b.currency[2], 'R');
  EXPECT_EQ(v2b.total_cost, 122);
  EXPECT_EQ(v2b.total_cost_balanced, 110);

  EXPECT_EQ(v2b.total_forward_active_energy[2], 5000000);
  EXPECT_EQ(v2b.total_forward_active_energy_balanced, 4500000);
  EXPECT_EQ(v2b.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2], 150);
  EXPECT_EQ(v2b.m_count, EM_MEASUREMENT_COUNT);
}

TEST_F(ChannelEmExtendedValueTest, random) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  TSuplaChannelExtendedValue value = {};

  TestHelper::randomize((char *)&v2,
                        sizeof(TElectricityMeter_ExtendedValue_V2));
  v2.m_count = EM_MEASUREMENT_COUNT;

  srpc_evtool_v2_emextended2extended(&v2, &value);

  supla_channel_em_extended_value ev1(&value, "EUR", 245);
  supla_channel_em_extended_value ev2(&v2, "EUR", 245);

  TElectricityMeter_ExtendedValue_V2 v2b = {};
  TElectricityMeter_ExtendedValue_V2 v2c = {};
  ev1.get_raw_value(&v2b);
  ev2.get_raw_value(&v2c);

  memcpy(v2.currency, v2b.currency, sizeof(v2.currency));
  v2.total_cost = v2b.total_cost;
  v2.price_per_unit = v2b.price_per_unit;
  v2.total_cost_balanced = v2b.total_cost_balanced;

  EXPECT_EQ(memcmp(&v2, &v2b, sizeof(TElectricityMeter_ExtendedValue_V2)), 0);

  EXPECT_EQ(memcmp(&v2b, &v2c, sizeof(TElectricityMeter_ExtendedValue_V2)), 0);
}

TEST_F(ChannelEmExtendedValueTest, copy) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  TSuplaChannelExtendedValue value = {};

  TestHelper::randomize((char *)&v2,
                        sizeof(TElectricityMeter_ExtendedValue_V2));
  v2.m_count = EM_MEASUREMENT_COUNT;

  srpc_evtool_v2_emextended2extended(&v2, &value);

  supla_channel_em_extended_value ev(&value, "EUR", 245);
  supla_channel_em_extended_value *copy =
      dynamic_cast<supla_channel_em_extended_value *>(ev.copy());
  ASSERT_TRUE(copy != nullptr);

  TSuplaChannelExtendedValue ev1 = {};
  TSuplaChannelExtendedValue ev2 = {};
  ev.get_raw_value(&ev1);
  copy->get_raw_value(&ev2);

  EXPECT_EQ(memcmp(&ev1, &ev2, sizeof(TSuplaChannelExtendedValue)), 0);

  delete copy;
}

TEST_F(ChannelEmExtendedValueTest, recalculate) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};

  v2.total_forward_active_energy[2] = 5000000;
  v2.total_forward_active_energy_balanced = 4500000;
  v2.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2] = 150;
  v2.m_count = EM_MEASUREMENT_COUNT;
  v2.measured_values = EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED;

  supla_channel_em_extended_value ev(&v2, "EUR", 245);

  EXPECT_EQ(ev.get_currency(), "EUR");
  EXPECT_DOUBLE_EQ(ev.get_total_cost(), 1.22);
  EXPECT_DOUBLE_EQ(ev.get_total_cost_balanced(), 1.10);
  EXPECT_DOUBLE_EQ(ev.get_price_per_unit(), 0.0245);

  ev.get_raw_value(&v2);
  v2.total_forward_active_energy[1] = 5000000;
  v2.total_forward_active_energy_balanced = 9000000;
  ev.set_raw_value(&v2);

  EXPECT_EQ(ev.get_currency(), "EUR");
  EXPECT_DOUBLE_EQ(ev.get_total_cost(), 2.45);
  EXPECT_DOUBLE_EQ(ev.get_total_cost_balanced(), 2.20);
  EXPECT_DOUBLE_EQ(ev.get_price_per_unit(), 0.0245);

  v2.price_per_unit = 0;
  memset(v2.currency, 0, 3);

  ev.set_raw_value(&v2);
  EXPECT_EQ(ev.get_currency(), "");
  EXPECT_DOUBLE_EQ(ev.get_total_cost(), 0.0);
  EXPECT_DOUBLE_EQ(ev.get_total_cost_balanced(), 0.0);
  EXPECT_DOUBLE_EQ(ev.get_price_per_unit(), 0);
}

TEST_F(ChannelEmExtendedValueTest, voltage) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.m_count = 1;
  v2.m[0].voltage[0] = 24512;
  v2.m[0].voltage[1] = 23123;
  v2.m[0].voltage[2] = 21000;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_voltage(1), 245.12);
  EXPECT_DOUBLE_EQ(ev.get_voltage(2), 231.23);
  EXPECT_DOUBLE_EQ(ev.get_voltage(3), 210.00);
  EXPECT_DOUBLE_EQ(ev.get_voltage_avg(), 228.78333333333333);

  v2.m[0].voltage[1] = 0;
  ev.set_raw_value(&v2);
  EXPECT_DOUBLE_EQ(ev.get_voltage_avg(), 227.56);
}

TEST_F(ChannelEmExtendedValueTest, current) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.m_count = 1;
  v2.m[0].current[0] = 12345;
  v2.m[0].current[1] = 58912;
  v2.m[0].current[2] = 58766;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_current(1), 12.345);
  EXPECT_DOUBLE_EQ(ev.get_current(2), 58.912);
  EXPECT_DOUBLE_EQ(ev.get_current(3), 58.766);
  EXPECT_DOUBLE_EQ(ev.get_current_sum(), 130.023);
}

TEST_F(ChannelEmExtendedValueTest, currentOver65A) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.m_count = 1;
  v2.measured_values = EM_VAR_CURRENT_OVER_65A;
  v2.m[0].current[0] = 12345;
  v2.m[0].current[1] = 58912;
  v2.m[0].current[2] = 58766;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_current(1), 123.45);
  EXPECT_DOUBLE_EQ(ev.get_current(2), 589.12);
  EXPECT_DOUBLE_EQ(ev.get_current(3), 587.66);
  EXPECT_DOUBLE_EQ(ev.get_current_sum(), 1300.230);
}

TEST_F(ChannelEmExtendedValueTest, powerActive_W) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.m_count = 1;
  v2.m[0].power_active[0] = 1234578;
  v2.m[0].power_active[1] = 5891298;
  v2.m[0].power_active[2] = 5876630;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_power_active(1), 12.34578);
  EXPECT_DOUBLE_EQ(ev.get_power_active(2), 58.91298);
  EXPECT_DOUBLE_EQ(ev.get_power_active(3), 58.76630);
  EXPECT_GE(ev.get_power_active_sum(), 130.025);
  EXPECT_LE(ev.get_power_active_sum(), 130.0251);
}

TEST_F(ChannelEmExtendedValueTest, powerActive_kW) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.m_count = 1;
  v2.measured_values = EM_VAR_POWER_ACTIVE_KW;
  v2.m[0].power_active[0] = 1234578;
  v2.m[0].power_active[1] = 5891298;
  v2.m[0].power_active[2] = 5876630;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_power_active(1), 12345.78);
  EXPECT_DOUBLE_EQ(ev.get_power_active(2), 58912.98);
  EXPECT_DOUBLE_EQ(ev.get_power_active(3), 58766.30);
  EXPECT_DOUBLE_EQ(ev.get_power_active_sum(), 130025.06);
}

TEST_F(ChannelEmExtendedValueTest, powerReactive_var) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.m_count = 1;
  v2.m[0].power_reactive[0] = 1234578;
  v2.m[0].power_reactive[1] = 5891298;
  v2.m[0].power_reactive[2] = 5876630;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive(1), 12.34578);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive(2), 58.91298);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive(3), 58.76630);
  EXPECT_GE(ev.get_power_reactive_sum(), 130.025);
  EXPECT_LE(ev.get_power_reactive_sum(), 130.0251);
}

TEST_F(ChannelEmExtendedValueTest, powerReactive_kvar) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.m_count = 1;
  v2.measured_values = EM_VAR_POWER_REACTIVE_KVAR;
  v2.m[0].power_reactive[0] = 1234578;
  v2.m[0].power_reactive[1] = 5891298;
  v2.m[0].power_reactive[2] = 5876630;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive(1), 12345.78);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive(2), 58912.98);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive(3), 58766.30);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive_sum(), 130025.06);
}

TEST_F(ChannelEmExtendedValueTest, powerApparent_VA) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.m_count = 1;
  v2.m[0].power_apparent[0] = 1234578;
  v2.m[0].power_apparent[1] = 5891298;
  v2.m[0].power_apparent[2] = 5876630;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent(1), 12.34578);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent(2), 58.91298);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent(3), 58.76630);
  EXPECT_GE(ev.get_power_apparent_sum(), 130.025);
  EXPECT_LE(ev.get_power_apparent_sum(), 130.0251);
}

TEST_F(ChannelEmExtendedValueTest, powerApparent_kVA) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.m_count = 1;
  v2.measured_values = EM_VAR_POWER_APPARENT_KVA;
  v2.m[0].power_apparent[0] = 1234578;
  v2.m[0].power_apparent[1] = 5891298;
  v2.m[0].power_apparent[2] = 5876630;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent(1), 12345.78);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent(2), 58912.98);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent(3), 58766.30);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent_sum(), 130025.06);
}

TEST_F(ChannelEmExtendedValueTest, forwardActiveEnergy) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.total_forward_active_energy[0] = 122001;
  v2.total_forward_active_energy[1] = 233002;
  v2.total_forward_active_energy[2] = 344003;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_fae(1), 1.22001);
  EXPECT_DOUBLE_EQ(ev.get_fae(2), 2.33002);
  EXPECT_DOUBLE_EQ(ev.get_fae(3), 3.44003);
  EXPECT_DOUBLE_EQ(ev.get_fae_sum(), 6.99006);
}

TEST_F(ChannelEmExtendedValueTest, reverseActiveEnergy) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.total_reverse_active_energy[0] = 122002;
  v2.total_reverse_active_energy[1] = 233004;
  v2.total_reverse_active_energy[2] = 344006;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_rae(1), 1.22002);
  EXPECT_DOUBLE_EQ(ev.get_rae(2), 2.33004);
  EXPECT_DOUBLE_EQ(ev.get_rae(3), 3.44006);
  EXPECT_DOUBLE_EQ(ev.get_rae_sum(), 6.99012);
}

TEST_F(ChannelEmExtendedValueTest, forwardActiveEnergyBalanced) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.total_forward_active_energy_balanced = 123456;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_fae_balanced(), 1.23456);
}

TEST_F(ChannelEmExtendedValueTest, reverseActiveEnergyBalanced) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};
  v2.total_reverse_active_energy_balanced = 223456;
  supla_channel_em_extended_value ev(&v2, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_rae_balanced(), 2.23456);
}

}  // namespace testing
