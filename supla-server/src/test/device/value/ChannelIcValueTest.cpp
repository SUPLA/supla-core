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

#include "ChannelIcValueTest.h"

#include "device/extended_value/channel_ic_extended_value.h"
#include "device/value/channel_ic_value.h"
#include "devicechannel.h"  // NOLINT
#include "jsonconfig/channel/impulse_counter_config.h"

namespace testing {

TEST_F(ChannelIcValueTest, rawDataConstructor) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_ic_value v1(raw_value);

  EXPECT_EQ(v1.get_ic_value()->counter, 0);

  ((TDS_ImpulseCounter_Value *)raw_value)->counter = 3;

  supla_channel_ic_value v2(raw_value);

  EXPECT_EQ(v2.get_ic_value()->counter, 3);
}

TEST_F(ChannelIcValueTest, icValueConstructor) {
  TDS_ImpulseCounter_Value ic = {};
  ic.counter = 2;

  supla_channel_ic_value v(&ic);
  EXPECT_EQ(v.get_ic_value()->counter, 2);
}

TEST_F(ChannelIcValueTest, applyChannelProperties) {
  impulse_counter_config config;
  config.set_user_config("{\"impulsesPerUnit\":5, \"initialValue\":100}");

  const TDS_ImpulseCounter_Value icval = {};
  supla_channel_ic_value v1(&icval);

  EXPECT_EQ(v1.get_ic_value()->counter, 0);
  v1.apply_channel_properties(0, 0, 0, 0, 0, 0, &config);
  EXPECT_EQ(v1.get_ic_value()->counter, 500);
}

TEST_F(ChannelIcValueTest, convertToExtended) {
  impulse_counter_config config;
  config.set_user_config(
      "{\"impulsesPerUnit\":5, \"initialValue\":100,\"addToHistory\":false}");

  const TDS_ImpulseCounter_Value icval = {};
  supla_channel_ic_value v1(&icval);

  EXPECT_EQ(v1.get_ic_value()->counter, 0);
  v1.apply_channel_properties(0, 0, 0, 0, 0, 0, &config);
  EXPECT_EQ(v1.get_ic_value()->counter, 500);

  supla_channel_extended_value *data_logger_purpose = nullptr;
  supla_channel_extended_value *ev =
      v1.convert2extended(&config, SUPLA_CHANNELFNC_IC_ELECTRICITY_METER, "PLN",
                          "Unit", 45000, 10, &data_logger_purpose);

  EXPECT_TRUE(data_logger_purpose != nullptr);
  EXPECT_TRUE(ev != nullptr);

  if (ev) {
    supla_channel_ic_extended_value *ic_ev =
        dynamic_cast<supla_channel_ic_extended_value *>(ev);
    EXPECT_TRUE(ic_ev != nullptr);
    if (ic_ev) {
      EXPECT_EQ(ic_ev->get_total_cost(), 22500);
      EXPECT_EQ(ic_ev->get_price_per_unit(), 45000);
      EXPECT_EQ(ic_ev->get_currency(), "PLN");
      EXPECT_EQ(ic_ev->get_custom_unit(), "Unit");
      EXPECT_EQ(ic_ev->get_impulses_per_unit(), 10);
      EXPECT_EQ(ic_ev->get_counter(), 500);
      EXPECT_EQ(ic_ev->get_calculated_value(), 50000);
      EXPECT_EQ(ic_ev->get_calculated_value_dbl(), 50.0);
    }
    delete ev;
  }

  if (data_logger_purpose) {
    supla_channel_ic_extended_value *ic_ev =
        dynamic_cast<supla_channel_ic_extended_value *>(data_logger_purpose);
    EXPECT_TRUE(ic_ev != nullptr);
    if (ic_ev) {
      EXPECT_EQ(ic_ev->get_total_cost(), 0);
      EXPECT_EQ(ic_ev->get_price_per_unit(), 45000);
      EXPECT_EQ(ic_ev->get_currency(), "PLN");
      EXPECT_EQ(ic_ev->get_custom_unit(), "Unit");
      EXPECT_EQ(ic_ev->get_impulses_per_unit(), 10);
      EXPECT_EQ(ic_ev->get_counter(), 0);
      EXPECT_EQ(ic_ev->get_calculated_value(), 0);
      EXPECT_EQ(ic_ev->get_calculated_value_dbl(), 0);
    }

    delete data_logger_purpose;
  }

  config.set_user_config(
      "{\"impulsesPerUnit\":5, \"initialValue\":100,\"addToHistory\":true}");

  EXPECT_EQ(v1.get_ic_value()->counter, 500);
}

}  // namespace testing
