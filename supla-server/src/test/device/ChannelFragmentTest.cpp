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

#include "ChannelFragmentTest.h"

#include "device/channel_fragment.h"
#include "device/devicechannel.h"
#include "doubles/device/DeviceStub.h"

namespace testing {

TEST_F(ChannelFragmentTest, defaultContructor) {
  supla_channel_fragment f;
  EXPECT_EQ(f.get_device_id(), 0);
  EXPECT_EQ(f.get_channel_id(), 0);
  EXPECT_EQ(f.get_function(), 0);
  EXPECT_EQ(f.get_type(), 0);
  EXPECT_EQ(f.get_flags(), 0);
  EXPECT_TRUE(f.is_hidden());
}

TEST_F(ChannelFragmentTest, constructorWithArgs) {
  supla_channel_fragment f1(1, 2, 3, 4, 5, false);
  EXPECT_EQ(f1.get_device_id(), 1);
  EXPECT_EQ(f1.get_channel_id(), 2);
  EXPECT_EQ(f1.get_type(), 3);
  EXPECT_EQ(f1.get_function(), 4);
  EXPECT_EQ(f1.get_flags(), 5U);
  EXPECT_FALSE(f1.is_hidden());

  supla_channel_fragment f2(12, 22, 32, 42, 54, true);
  EXPECT_EQ(f2.get_device_id(), 12);
  EXPECT_EQ(f2.get_channel_id(), 22);
  EXPECT_EQ(f2.get_type(), 32);
  EXPECT_EQ(f2.get_function(), 42);
  EXPECT_EQ(f2.get_flags(), 54U);
  EXPECT_TRUE(f2.is_hidden());
}

TEST_F(ChannelFragmentTest, channelAssignment) {
  DeviceStub device(nullptr);
  device.set_id(286);
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_device_channel channel(
      &device, 686, 0, SUPLA_CHANNELTYPE_ACTIONTRIGGER,
      SUPLA_CHANNELFNC_ACTIONTRIGGER, 0, 0, 0, 0, nullptr, nullptr, nullptr,
      false, SUPLA_CHANNEL_FLAG_CALCFG_RECALIBRATE, value, 0, nullptr, nullptr);

  supla_channel_fragment f;
  f = &channel;

  EXPECT_EQ(f.get_device_id(), 286);
  EXPECT_EQ(f.get_channel_id(), 686);
  EXPECT_EQ(f.get_type(), SUPLA_CHANNELTYPE_ACTIONTRIGGER);
  EXPECT_EQ(f.get_function(), SUPLA_CHANNELFNC_ACTIONTRIGGER);
  EXPECT_EQ(f.get_flags(), SUPLA_CHANNEL_FLAG_CALCFG_RECALIBRATE);
  EXPECT_FALSE(f.is_hidden());
}

TEST_F(ChannelFragmentTest, functionSetter) {
  supla_channel_fragment f;
  f.set_function(SUPLA_CHANNELFNC_ACTIONTRIGGER);
  EXPECT_EQ(f.get_function(), SUPLA_CHANNELFNC_ACTIONTRIGGER);
}

} /* namespace testing */
