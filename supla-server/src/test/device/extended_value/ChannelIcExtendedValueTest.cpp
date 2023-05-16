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

#include "ChannelIcExtendedValueTest.h"

#include "device/extended_value/channel_ic_extended_value.h"

namespace testing {

TEST_F(ChannelIcExtendedValueTest, defaults) {
  TDS_ImpulseCounter_Value ic_val = {};
  supla_channel_ic_extended_value ev(0, &ic_val, "", "", 0, 0);

  EXPECT_EQ(ev.get_total_cost(), 0);
  EXPECT_EQ(ev.get_price_per_unit(), 0);
  EXPECT_EQ(ev.get_currency(), "");
  EXPECT_EQ(ev.get_custom_unit(), "");
  EXPECT_EQ(ev.get_impulses_per_unit(), 0);
  EXPECT_EQ(ev.get_counter(), 0);
  EXPECT_EQ(ev.get_calculated_value(), 0);
}

TEST_F(ChannelIcExtendedValueTest, electricityMeterDefaults) {
  TDS_ImpulseCounter_Value ic_val = {};
  supla_channel_ic_extended_value ev(SUPLA_CHANNELFNC_IC_ELECTRICITY_METER,
                                     &ic_val, "", "", 0, 0);
  EXPECT_EQ(ev.get_custom_unit(), "kWh");
}

TEST_F(ChannelIcExtendedValueTest, gasMeterDefaults) {
  TDS_ImpulseCounter_Value ic_val = {};
  supla_channel_ic_extended_value ev(SUPLA_CHANNELFNC_IC_GAS_METER, &ic_val, "",
                                     "", 0, 0);
  EXPECT_EQ(ev.get_custom_unit(), "m³");
}

TEST_F(ChannelIcExtendedValueTest, gasWaterDefaults) {
  TDS_ImpulseCounter_Value ic_val = {};
  supla_channel_ic_extended_value ev(SUPLA_CHANNELFNC_IC_WATER_METER, &ic_val,
                                     "", "", 0, 0);
  EXPECT_EQ(ev.get_custom_unit(), "m³");
}

TEST_F(ChannelIcExtendedValueTest, gasHeatDefaults) {
  TDS_ImpulseCounter_Value ic_val = {};
  supla_channel_ic_extended_value ev(SUPLA_CHANNELFNC_IC_HEAT_METER, &ic_val,
                                     "", "", 0, 0);
  EXPECT_EQ(ev.get_custom_unit(), "GJ");
}

TEST_F(ChannelIcExtendedValueTest, vefityAllGetters) {
  TDS_ImpulseCounter_Value ic_val = {};
  ic_val.counter = 150;

  supla_channel_ic_extended_value ev(SUPLA_CHANNELFNC_IC_ELECTRICITY_METER,
                                     &ic_val, "PLN", "Unit", 45000, 10);

  EXPECT_EQ(ev.get_total_cost(), 6750);
  EXPECT_EQ(ev.get_price_per_unit(), 45000);
  EXPECT_EQ(ev.get_currency(), "PLN");
  EXPECT_EQ(ev.get_custom_unit(), "Unit");
  EXPECT_EQ(ev.get_impulses_per_unit(), 10);
  EXPECT_EQ(ev.get_counter(), 150);
  EXPECT_EQ(ev.get_calculated_value(), 15000);
}

TEST_F(ChannelIcExtendedValueTest, copy) {
  TDS_ImpulseCounter_Value ic_val = {};
  ic_val.counter = 150;

  supla_channel_ic_extended_value ev(SUPLA_CHANNELFNC_IC_ELECTRICITY_METER,
                                     &ic_val, "PLN", "Unit", 45000, 10);

  supla_channel_ic_extended_value *copy =
      dynamic_cast<supla_channel_ic_extended_value *>(ev.copy());
  ASSERT_TRUE(copy != nullptr);

  TSuplaChannelExtendedValue ev1 = {};
  TSuplaChannelExtendedValue ev2 = {};
  ev.get_raw_value(&ev1);
  copy->get_raw_value(&ev2);

  EXPECT_EQ(memcmp(&ev1, &ev2, sizeof(TSuplaChannelExtendedValue)), 0);

  delete copy;
}

}  // namespace testing
