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

#include <device/value/ChannelHvacValueTest.h>

#include "device/value/channel_hvac_value.h"
#include "devicechannel.h"  // NOLINT

namespace testing {

TEST_F(ChannelHvacValueTest, mode) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.set_mode(100);
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.set_mode(SUPLA_HVAC_MODE_HEAT);
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_HEAT);
}

TEST_F(ChannelHvacValueTest, turnOn) {
  supla_channel_hvac_value value;
  EXPECT_FALSE(value.is_on());
  value.turn_on();
  EXPECT_FALSE(value.is_on());
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_CMD_TURN_ON);
}

TEST_F(ChannelHvacValueTest, turnOff) {
  supla_channel_hvac_value value;
  EXPECT_FALSE(value.is_on());
  value.set_mode(SUPLA_HVAC_MODE_HEAT);
  EXPECT_TRUE(value.is_on());
  value.turn_off();
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_OFF);
}

TEST_F(ChannelHvacValueTest, switchToManual) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.switch_to_manual();
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_CMD_SWITCH_TO_MANUAL);
}

}  // namespace testing
