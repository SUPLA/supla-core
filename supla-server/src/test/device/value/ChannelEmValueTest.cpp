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

#include "ChannelEmValueTest.h"

#include "device/value/channel_em_value.h"
#include "devicechannel.h"  // NOLINT
#include "jsonconfig/channel/electricity_meter_config.h"

namespace testing {

TEST_F(ChannelEmValueTest, rawDataConstructor) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_em_value v1(raw_value);

  EXPECT_EQ(v1.get_em_value()->flags, 0);
  EXPECT_EQ(v1.get_em_value()->total_forward_active_energy, 0);

  ((TElectricityMeter_Value*)raw_value)->flags = 3;
  ((TElectricityMeter_Value*)raw_value)->total_forward_active_energy = 4;

  supla_channel_em_value v2(raw_value);

  EXPECT_EQ(v2.get_em_value()->flags, 3);
  EXPECT_EQ(v2.get_em_value()->total_forward_active_energy, 4);
}

TEST_F(ChannelEmValueTest, emValueConstructor) {
  TElectricityMeter_Value em = {};
  em.flags = 1;
  em.total_forward_active_energy = 2;

  supla_channel_em_value v(&em);
  EXPECT_EQ(v.get_em_value()->flags, 1);
  EXPECT_EQ(v.get_em_value()->total_forward_active_energy, 2);
}

TEST_F(ChannelEmValueTest, applyChannelProperties) {
  electricity_meter_config config;
  config.set_user_config(
      "{\"electricityMeterInitialValues\":{\"forwardActiveEnergy\":10000000}}");
  config.update_available_counters(0xFFFFFFFF);

  TElectricityMeter_Value em = {};
  supla_channel_em_value v(&em);

  v.apply_channel_properties(0, 0, 0, 0, 0, 0, &config, nullptr);

  EXPECT_EQ(v.get_em_value()->total_forward_active_energy, 1000000000);
}

}  // namespace testing
