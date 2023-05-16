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
  EXPECT_EQ(ev.get_total_cost(), 0);
  EXPECT_EQ(ev.get_total_cost_balanced(), 0);
  EXPECT_EQ(ev.get_price_per_unit(), 0);
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
  EXPECT_EQ(ev.get_total_cost(), 37);
  EXPECT_EQ(ev.get_total_cost_balanced(), 0);
  EXPECT_EQ(ev.get_price_per_unit(), 1234);
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
  EXPECT_EQ(ev.get_total_cost(), 122);
  EXPECT_EQ(ev.get_total_cost_balanced(), 110);
  EXPECT_EQ(ev.get_price_per_unit(), 245);
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
  EXPECT_EQ(ev.get_total_cost(), 122);
  EXPECT_EQ(ev.get_total_cost_balanced(), 110);
  EXPECT_EQ(ev.get_price_per_unit(), 245);

  ev.get_raw_value(&v2);
  v2.total_forward_active_energy[1] = 5000000;
  v2.total_forward_active_energy_balanced = 9000000;
  ev.set_raw_value(&v2);

  EXPECT_EQ(ev.get_currency(), "EUR");
  EXPECT_EQ(ev.get_total_cost(), 243);
  EXPECT_EQ(ev.get_total_cost_balanced(), 220);
  EXPECT_EQ(ev.get_price_per_unit(), 245);

  v2.price_per_unit = 0;
  memset(v2.currency, 0, 3);

  ev.set_raw_value(&v2);
  EXPECT_EQ(ev.get_currency(), "");
  EXPECT_EQ(ev.get_total_cost(), 0);
  EXPECT_EQ(ev.get_total_cost_balanced(), 0);
  EXPECT_EQ(ev.get_price_per_unit(), 0);
}

}  // namespace testing
