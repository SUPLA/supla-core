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

TEST_F(ChannelEmExtendedValueTest, defaultConstructor) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  TSuplaChannelExtendedValue ev_raw2 = {};

  ev_raw1.size = sizeof(TElectricityMeter_ExtendedValue_V3);
  ev_raw1.type = -1;

  TestHelper::randomize(ev_raw1.value, ev_raw1.size);

  ((TElectricityMeter_ExtendedValue_V3 *)ev_raw1.value)->m_count =
      EM_MEASUREMENT_COUNT;

  {
    supla_channel_em_extended_value v(&ev_raw1);
    EXPECT_FALSE(v.get_raw_value(&ev_raw2));
  }

  ev_raw1.size = sizeof(TElectricityMeter_ExtendedValue_V3);
  ev_raw1.type = EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V3;

  {
    supla_channel_em_extended_value v(&ev_raw1);
    EXPECT_TRUE(v.get_raw_value(&ev_raw2));
  }

  EXPECT_EQ(memcmp(&ev_raw1, &ev_raw2, sizeof(TSuplaChannelExtendedValue)), 0);
}

TEST_F(ChannelEmExtendedValueTest, setV2GetV3) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  TSuplaChannelExtendedValue ev_raw2 = {};

  ev_raw1.size = sizeof(TElectricityMeter_ExtendedValue_V2);
  ev_raw1.type = EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2;

  TestHelper::randomize(ev_raw1.value, ev_raw1.size);

  ((TElectricityMeter_ExtendedValue_V2 *)ev_raw1.value)->m_count =
      EM_MEASUREMENT_COUNT;

  {
    supla_channel_em_extended_value v(&ev_raw1);
    EXPECT_TRUE(v.get_raw_value(&ev_raw2));
  }

  ASSERT_EQ(ev_raw2.type, EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V3);
  EXPECT_EQ(((TElectricityMeter_ExtendedValue_V2 *)ev_raw1.value)->total_cost,
            ((TElectricityMeter_ExtendedValue_V3 *)ev_raw2.value)->total_cost);
}

TEST_F(ChannelEmExtendedValueTest, converstionV1toV3) {
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

  TElectricityMeter_ExtendedValue_V3 v3 = {};
  ev.get_raw_value(&v3);

  EXPECT_EQ(v3.currency[0], 'P');
  EXPECT_EQ(v3.currency[1], 'L');
  EXPECT_EQ(v3.currency[2], 'N');
  EXPECT_EQ(v3.total_cost, 37);

  EXPECT_EQ(v1.m_count, v3.m_count);
  EXPECT_EQ(v1.total_forward_active_energy[0],
            v3.total_forward_active_energy[0]);
  EXPECT_EQ(v1.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2],
            v3.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2]);
}

TEST_F(ChannelEmExtendedValueTest, converstionV2toV3) {
  TElectricityMeter_ExtendedValue_V2 v2 = {};

  v2.total_forward_active_energy[0] = 300000;
  v2.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2] = 45;
  v2.m_count = EM_MEASUREMENT_COUNT;

  TSuplaChannelExtendedValue value = {};
  srpc_evtool_v2_emextended2extended(&v2, &value);

  supla_channel_em_extended_value ev(&value, "PLN", 1234);

  TElectricityMeter_ExtendedValue_V3 v3 = {};
  ev.get_raw_value(&v3);

  EXPECT_EQ(v3.currency[0], 'P');
  EXPECT_EQ(v3.currency[1], 'L');
  EXPECT_EQ(v3.currency[2], 'N');
  EXPECT_EQ(v3.total_cost, 37);

  EXPECT_EQ(v2.m_count, v3.m_count);
  EXPECT_EQ(v2.total_forward_active_energy[0],
            v3.total_forward_active_energy[0]);
  EXPECT_EQ(v2.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2],
            v3.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2]);
}

TEST_F(ChannelEmExtendedValueTest, verifyGetters) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  TSuplaChannelExtendedValue value = {};

  v3.total_forward_active_energy[2] = 5000000;
  v3.total_forward_active_energy_balanced = 4500000;
  v3.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2] = 150;
  v3.m_count = EM_MEASUREMENT_COUNT;
  v3.measured_values = EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED;

  srpc_evtool_v3_emextended2extended(&v3, &value);

  supla_channel_em_extended_value ev(&value, "EUR", 245);

  EXPECT_EQ(ev.get_currency(), "EUR");
  EXPECT_DOUBLE_EQ(ev.get_total_cost(), 1.22);
  EXPECT_DOUBLE_EQ(ev.get_total_cost_balanced(), 1.10);
  EXPECT_DOUBLE_EQ(ev.get_price_per_unit(), 0.0245);
  EXPECT_EQ(ev.get_measured_values(), EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED);

  TElectricityMeter_ExtendedValue_V3 v3b = {};
  ev.get_raw_value(&v3b);

  EXPECT_EQ(v3b.currency[0], 'E');
  EXPECT_EQ(v3b.currency[1], 'U');
  EXPECT_EQ(v3b.currency[2], 'R');
  EXPECT_EQ(v3b.total_cost, 122);
  EXPECT_EQ(v3b.total_cost_balanced, 110);

  EXPECT_EQ(v3b.total_forward_active_energy[2], 5000000);
  EXPECT_EQ(v3b.total_forward_active_energy_balanced, 4500000);
  EXPECT_EQ(v3b.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2], 150);
  EXPECT_EQ(v3b.m_count, EM_MEASUREMENT_COUNT);
}

TEST_F(ChannelEmExtendedValueTest, random) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  TSuplaChannelExtendedValue value = {};

  TestHelper::randomize((char *)&v3,
                        sizeof(TElectricityMeter_ExtendedValue_V3));
  v3.m_count = EM_MEASUREMENT_COUNT;

  srpc_evtool_v3_emextended2extended(&v3, &value);

  supla_channel_em_extended_value ev1(&value, "EUR", 245);
  supla_channel_em_extended_value ev2(&v3, "EUR", 245);

  TElectricityMeter_ExtendedValue_V3 v3b = {};
  TElectricityMeter_ExtendedValue_V3 v3c = {};
  ev1.get_raw_value(&v3b);
  ev2.get_raw_value(&v3c);

  memcpy(v3.currency, v3b.currency, sizeof(v3.currency));
  v3.total_cost = v3b.total_cost;
  v3.price_per_unit = v3b.price_per_unit;
  v3.total_cost_balanced = v3b.total_cost_balanced;

  EXPECT_EQ(memcmp(&v3, &v3b, sizeof(TElectricityMeter_ExtendedValue_V3)), 0);

  EXPECT_EQ(memcmp(&v3b, &v3c, sizeof(TElectricityMeter_ExtendedValue_V3)), 0);
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

TEST_F(ChannelEmExtendedValueTest, calculate) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};

  v3.total_forward_active_energy[2] = 5000000;
  v3.total_forward_active_energy_balanced = 4500000;
  v3.m[EM_MEASUREMENT_COUNT - 1].phase_angle[2] = 150;
  v3.m_count = EM_MEASUREMENT_COUNT;
  v3.measured_values = EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED;

  supla_channel_em_extended_value ev(&v3, "EUR", 245);

  EXPECT_EQ(ev.get_currency(), "EUR");
  EXPECT_DOUBLE_EQ(ev.get_total_cost(), 1.22);
  EXPECT_DOUBLE_EQ(ev.get_total_cost_balanced(), 1.10);
  EXPECT_DOUBLE_EQ(ev.get_price_per_unit(), 0.0245);

  ev.get_raw_value(&v3);
  v3.total_forward_active_energy[1] = 5000000;
  v3.total_forward_active_energy_balanced = 9000000;
  supla_channel_em_extended_value ev2(&v3, "EUR", 245);

  EXPECT_EQ(ev2.get_currency(), "EUR");
  EXPECT_DOUBLE_EQ(ev2.get_total_cost(), 2.45);
  EXPECT_DOUBLE_EQ(ev2.get_total_cost_balanced(), 2.20);
  EXPECT_DOUBLE_EQ(ev2.get_price_per_unit(), 0.0245);
}

TEST_F(ChannelEmExtendedValueTest, voltage) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.m_count = 1;
  v3.m[0].voltage[0] = 24512;
  v3.m[0].voltage[1] = 23123;
  v3.m[0].voltage[2] = 21000;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_voltage(1), 245.12);
  EXPECT_DOUBLE_EQ(ev.get_voltage(2), 231.23);
  EXPECT_DOUBLE_EQ(ev.get_voltage(3), 210.00);
  EXPECT_DOUBLE_EQ(ev.get_voltage_avg(), 228.78333333333333);

  v3.m[0].voltage[1] = 0;
  ev.set_raw_value(&v3);
  EXPECT_DOUBLE_EQ(ev.get_voltage_avg(), 227.56);
}

TEST_F(ChannelEmExtendedValueTest, current) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.m_count = 1;
  v3.m[0].current[0] = 12345;
  v3.m[0].current[1] = 58912;
  v3.m[0].current[2] = 58766;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_current(1), 12.345);
  EXPECT_DOUBLE_EQ(ev.get_current(2), 58.912);
  EXPECT_DOUBLE_EQ(ev.get_current(3), 58.766);
  EXPECT_DOUBLE_EQ(ev.get_current_sum(), 130.023);
}

TEST_F(ChannelEmExtendedValueTest, currentOver65A) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.m_count = 1;
  v3.measured_values = EM_VAR_CURRENT_OVER_65A;
  v3.m[0].current[0] = 12345;
  v3.m[0].current[1] = 58912;
  v3.m[0].current[2] = 58766;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_current(1), 123.45);
  EXPECT_DOUBLE_EQ(ev.get_current(2), 589.12);
  EXPECT_DOUBLE_EQ(ev.get_current(3), 587.66);
  EXPECT_DOUBLE_EQ(ev.get_current_sum(), 1300.230);
}

TEST_F(ChannelEmExtendedValueTest, powerActive_W) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.m_count = 1;
  v3.m[0].power_active[0] = 1234578;
  v3.m[0].power_active[1] = 5891298;
  v3.m[0].power_active[2] = 5876630;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_power_active(1), 12.34578);
  EXPECT_DOUBLE_EQ(ev.get_power_active(2), 58.91298);
  EXPECT_DOUBLE_EQ(ev.get_power_active(3), 58.76630);
  EXPECT_GE(ev.get_power_active_sum(), 130.025);
  EXPECT_LE(ev.get_power_active_sum(), 130.0251);
}

TEST_F(ChannelEmExtendedValueTest, powerActive_kW) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.m_count = 1;
  v3.measured_values = EM_VAR_POWER_ACTIVE_KW;
  v3.m[0].power_active[0] = 1234578;
  v3.m[0].power_active[1] = 5891298;
  v3.m[0].power_active[2] = 5876630;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_power_active(1), 12345.78);
  EXPECT_DOUBLE_EQ(ev.get_power_active(2), 58912.98);
  EXPECT_DOUBLE_EQ(ev.get_power_active(3), 58766.30);
  EXPECT_DOUBLE_EQ(ev.get_power_active_sum(), 130025.06);
}

TEST_F(ChannelEmExtendedValueTest, powerReactive_var) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.m_count = 1;
  v3.m[0].power_reactive[0] = 1234578;
  v3.m[0].power_reactive[1] = 5891298;
  v3.m[0].power_reactive[2] = 5876630;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive(1), 12.34578);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive(2), 58.91298);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive(3), 58.76630);
  EXPECT_GE(ev.get_power_reactive_sum(), 130.025);
  EXPECT_LE(ev.get_power_reactive_sum(), 130.0251);
}

TEST_F(ChannelEmExtendedValueTest, powerReactive_kvar) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.m_count = 1;
  v3.measured_values = EM_VAR_POWER_REACTIVE_KVAR;
  v3.m[0].power_reactive[0] = 1234578;
  v3.m[0].power_reactive[1] = 5891298;
  v3.m[0].power_reactive[2] = 5876630;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive(1), 12345.78);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive(2), 58912.98);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive(3), 58766.30);
  EXPECT_DOUBLE_EQ(ev.get_power_reactive_sum(), 130025.06);
}

TEST_F(ChannelEmExtendedValueTest, powerApparent_VA) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.m_count = 1;
  v3.m[0].power_apparent[0] = 1234578;
  v3.m[0].power_apparent[1] = 5891298;
  v3.m[0].power_apparent[2] = 5876630;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent(1), 12.34578);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent(2), 58.91298);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent(3), 58.76630);
  EXPECT_GE(ev.get_power_apparent_sum(), 130.025);
  EXPECT_LE(ev.get_power_apparent_sum(), 130.0251);
}

TEST_F(ChannelEmExtendedValueTest, powerApparent_kVA) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.m_count = 1;
  v3.measured_values = EM_VAR_POWER_APPARENT_KVA;
  v3.m[0].power_apparent[0] = 1234578;
  v3.m[0].power_apparent[1] = 5891298;
  v3.m[0].power_apparent[2] = 5876630;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent(1), 12345.78);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent(2), 58912.98);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent(3), 58766.30);
  EXPECT_DOUBLE_EQ(ev.get_power_apparent_sum(), 130025.06);
}

TEST_F(ChannelEmExtendedValueTest, forwardActiveEnergy) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.total_forward_active_energy[0] = 122001;
  v3.total_forward_active_energy[1] = 233002;
  v3.total_forward_active_energy[2] = 344003;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_fae(1), 1.22001);
  EXPECT_DOUBLE_EQ(ev.get_fae(2), 2.33002);
  EXPECT_DOUBLE_EQ(ev.get_fae(3), 3.44003);
  EXPECT_DOUBLE_EQ(ev.get_fae_sum(), 6.99006);
}

TEST_F(ChannelEmExtendedValueTest, reverseActiveEnergy) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.total_reverse_active_energy[0] = 122002;
  v3.total_reverse_active_energy[1] = 233004;
  v3.total_reverse_active_energy[2] = 344006;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_rae(1), 1.22002);
  EXPECT_DOUBLE_EQ(ev.get_rae(2), 2.33004);
  EXPECT_DOUBLE_EQ(ev.get_rae(3), 3.44006);
  EXPECT_DOUBLE_EQ(ev.get_rae_sum(), 6.99012);
}

TEST_F(ChannelEmExtendedValueTest, forwardActiveEnergyBalanced) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.total_forward_active_energy_balanced = 123456;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_fae_balanced(), 1.23456);
}

TEST_F(ChannelEmExtendedValueTest, reverseActiveEnergyBalanced) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.total_reverse_active_energy_balanced = 223456;
  supla_channel_em_extended_value ev(&v3, nullptr, 0);
  EXPECT_DOUBLE_EQ(ev.get_rae_balanced(), 2.23456);
}

TEST_F(ChannelEmExtendedValueTest, dependsOnProtocolVersion) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  supla_channel_em_extended_value ev(&v3, nullptr, 0);

  TSuplaChannelExtendedValue value = {};

  EXPECT_TRUE(ev.get_raw_value(&value, 0));
  EXPECT_EQ(value.type, EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V3);

  EXPECT_TRUE(ev.get_raw_value(&value, 12));
  EXPECT_EQ(value.type, EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1);

  EXPECT_TRUE(ev.get_raw_value(&value, 24));
  EXPECT_EQ(value.type, EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2);

  EXPECT_TRUE(ev.get_raw_value(&value, 25));
  EXPECT_EQ(value.type, EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V3);
}

TEST_F(ChannelEmExtendedValueTest, getVbtValue) {
  TElectricityMeter_ExtendedValue_V3 v3 = {};
  v3.m_count = 1;

  v3.m[0].voltage[0] = 23010;
  v3.m[0].voltage[1] = 24020;
  v3.m[0].voltage[2] = 25030;

  v3.m[0].current[0] = 12345;
  v3.m[0].current[1] = 58912;
  v3.m[0].current[2] = 58766;

  v3.m[0].power_active[0] = 1234578;
  v3.m[0].power_active[1] = 5891298;
  v3.m[0].power_active[2] = 5876630;

  v3.m[0].power_reactive[0] = 1234579;
  v3.m[0].power_reactive[1] = 5891299;
  v3.m[0].power_reactive[2] = 5876639;

  v3.m[0].power_apparent[0] = 2234578;
  v3.m[0].power_apparent[1] = 6891298;
  v3.m[0].power_apparent[2] = 7876630;

  v3.total_forward_active_energy[0] = 122001;
  v3.total_forward_active_energy[1] = 233002;
  v3.total_forward_active_energy[2] = 344003;

  v3.total_forward_active_energy_balanced = 123456;

  v3.total_reverse_active_energy[0] = 222001;
  v3.total_reverse_active_energy[1] = 333002;
  v3.total_reverse_active_energy[2] = 444003;

  v3.total_reverse_active_energy_balanced = 523456;

  supla_channel_em_extended_value value(&v3, nullptr, 0);

  double vbt_value = 0;
  EXPECT_FALSE(value.get_vbt_value(var_name_none, &vbt_value));

  EXPECT_TRUE(value.get_vbt_value(var_name_voltage_avg, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 240.2);

  EXPECT_TRUE(value.get_vbt_value(var_name_voltage1, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 230.1);

  EXPECT_TRUE(value.get_vbt_value(var_name_voltage2, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 240.2);

  EXPECT_TRUE(value.get_vbt_value(var_name_voltage3, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 250.3);

  EXPECT_TRUE(value.get_vbt_value(var_name_current_sum, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 130.023);

  EXPECT_TRUE(value.get_vbt_value(var_name_current1, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 12.345);

  EXPECT_TRUE(value.get_vbt_value(var_name_current2, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 58.912);

  EXPECT_TRUE(value.get_vbt_value(var_name_current3, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 58.766);

  EXPECT_TRUE(value.get_vbt_value(var_name_power_active_sum, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 130.02506);

  EXPECT_TRUE(value.get_vbt_value(var_name_power_active1, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 12.34578);

  EXPECT_TRUE(value.get_vbt_value(var_name_power_active2, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 58.91298);

  EXPECT_TRUE(value.get_vbt_value(var_name_power_active3, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 58.76630);

  EXPECT_TRUE(value.get_vbt_value(var_name_power_reactive_sum, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 130.02517);

  EXPECT_TRUE(value.get_vbt_value(var_name_power_reactive1, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 12.34579);

  EXPECT_TRUE(value.get_vbt_value(var_name_power_reactive2, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 58.91299);

  EXPECT_TRUE(value.get_vbt_value(var_name_power_reactive3, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 58.76639);

  EXPECT_TRUE(value.get_vbt_value(var_name_power_apparent_sum, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 170.02506);

  EXPECT_TRUE(value.get_vbt_value(var_name_power_apparent1, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 22.34578);

  EXPECT_TRUE(value.get_vbt_value(var_name_power_apparent2, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 68.91298);

  EXPECT_TRUE(value.get_vbt_value(var_name_power_apparent3, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 78.76630);

  EXPECT_TRUE(value.get_vbt_value(var_name_fae1, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 1.22001);

  EXPECT_TRUE(value.get_vbt_value(var_name_fae2, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 2.33002);

  EXPECT_TRUE(value.get_vbt_value(var_name_fae3, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 3.44003);

  EXPECT_TRUE(value.get_vbt_value(var_name_fae_sum, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 6.99006);

  EXPECT_TRUE(value.get_vbt_value(var_name_fae_balanced, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 1.23456);

  EXPECT_TRUE(value.get_vbt_value(var_name_rae1, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 2.22001);

  EXPECT_TRUE(value.get_vbt_value(var_name_rae2, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 3.33002);

  EXPECT_TRUE(value.get_vbt_value(var_name_rae3, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 4.44003);

  EXPECT_TRUE(value.get_vbt_value(var_name_rae_sum, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 9.99006);

  EXPECT_TRUE(value.get_vbt_value(var_name_rae_balanced, &vbt_value));
  EXPECT_DOUBLE_EQ(vbt_value, 5.23456);
}

}  // namespace testing
