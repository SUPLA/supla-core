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

#include "ChannelAvailabilityStatusTest.h"

#include "TestHelper.h"
#include "device/channel_availability_status.h"

namespace testing {

TEST_F(ChannelAvailabilityStatusTest, defaultConstructor) {
  supla_channel_availability_status status;
  EXPECT_TRUE(status.is_online());
  EXPECT_FALSE(status.is_offline());
  EXPECT_FALSE(status.is_online_but_not_available());

  EXPECT_EQ(status.get_proto_online(), 1);
  EXPECT_EQ(status.get_proto_offline(), 0);
}

TEST_F(ChannelAvailabilityStatusTest, constructorWithBool) {
  supla_channel_availability_status status1(true);
  EXPECT_FALSE(status2.is_online());
  EXPECT_TRUE(status2.is_offline());
  EXPECT_FALSE(status2.is_online_but_not_available());

  EXPECT_EQ(status1.get_proto_online(), 0);
  EXPECT_EQ(status1.get_proto_offline(), 1);

  supla_channel_availability_status status2(false);
  EXPECT_TRUE(status1.is_online());
  EXPECT_FALSE(status1.is_offline());
  EXPECT_FALSE(status1.is_online_but_not_available());

  EXPECT_EQ(status1.get_proto_online(), 1);
  EXPECT_EQ(status1.get_proto_offline(), 0);
}

TEST_F(ChannelAvailabilityStatusTest, constructorWithProtoOffline) {
  supla_channel_availability_status status1(0, true);
  EXPECT_TRUE(status1.is_online());
  EXPECT_FALSE(status1.is_offline());
  EXPECT_FALSE(status1.is_online_but_not_available());

  EXPECT_EQ(status1.get_proto_online(), 1);
  EXPECT_EQ(status1.get_proto_offline(), 0);

  supla_channel_availability_status status2(1, true);
  EXPECT_FALSE(status2.is_online());
  EXPECT_TRUE(status2.is_offline());
  EXPECT_FALSE(status2.is_online_but_not_available());

  EXPECT_EQ(status2.get_proto_online(), 0);
  EXPECT_EQ(status2.get_proto_offline(), 1);

  supla_channel_availability_status status3(2, true);
  EXPECT_FALSE(status3.is_online());
  EXPECT_FALSE(status3.is_offline());
  EXPECT_TRUE(status3.is_online_but_not_available());

  EXPECT_EQ(status3.get_proto_online(), 2);
  EXPECT_EQ(status3.get_proto_offline(), 2);

  supla_channel_availability_status status4(3, true);
  EXPECT_FALSE(status4.is_online());
  EXPECT_TRUE(status4.is_offline());
  EXPECT_FALSE(status4.is_online_but_not_available());

  EXPECT_EQ(status2.get_proto_online(), 0);
  EXPECT_EQ(status2.get_proto_offline(), 1);
}

TEST_F(ChannelAvailabilityStatusTest, constructorWithProtoOnline) {
  supla_channel_availability_status status1(1, false);
  EXPECT_TRUE(status1.is_online());
  EXPECT_FALSE(status1.is_offline());
  EXPECT_FALSE(status1.is_online_but_not_available());

  EXPECT_EQ(status1.get_proto_online(), 1);
  EXPECT_EQ(status1.get_proto_offline(), 0);

  supla_channel_availability_status status2(0, false);
  EXPECT_FALSE(status2.is_online());
  EXPECT_TRUE(status2.is_offline());
  EXPECT_FALSE(status2.is_online_but_not_available());

  EXPECT_EQ(status2.get_proto_online(), 0);
  EXPECT_EQ(status2.get_proto_offline(), 1);

  supla_channel_availability_status status3(2, false);
  EXPECT_FALSE(status3.is_online());
  EXPECT_FALSE(status3.is_offline());
  EXPECT_TRUE(status3.is_online_but_not_available());

  EXPECT_EQ(status3.get_proto_online(), 2);
  EXPECT_EQ(status3.get_proto_offline(), 2);

  supla_channel_availability_status status4(3, false);
  EXPECT_TRUE(status4.is_online());
  EXPECT_FALSE(status4.is_offline());
  EXPECT_FALSE(status4.is_online_but_not_available());

  EXPECT_EQ(status4.get_proto_online(), 1);
  EXPECT_EQ(status4.get_proto_offline(), 0);
}

TEST_F(ChannelAvailabilityStatusTest, set_proto_offline) {
  supla_channel_availability_status status;

  status.set_proto_offline(0);
  EXPECT_TRUE(status.is_online());
  EXPECT_FALSE(status.is_offline());
  EXPECT_FALSE(status.is_online_but_not_available());

  EXPECT_EQ(status.get_proto_online(), 1);
  EXPECT_EQ(status.get_proto_offline(), 0);

  status.set_proto_offline(1);
  EXPECT_FALSE(status.is_online());
  EXPECT_TRUE(status.is_offline());
  EXPECT_FALSE(status.is_online_but_not_available());

  EXPECT_EQ(status.get_proto_online(), 0);
  EXPECT_EQ(status.get_proto_offline(), 1);

  status.set_proto_offline(2);
  EXPECT_FALSE(status.is_online());
  EXPECT_FALSE(status.is_offline());
  EXPECT_TRUE(status.is_online_but_not_available());

  EXPECT_EQ(status.get_proto_online(), 2);
  EXPECT_EQ(status.get_proto_offline(), 2);

  status.set_proto_offline(3);
  EXPECT_FALSE(status.is_online());
  EXPECT_TRUE(status.is_offline());
  EXPECT_FALSE(status.is_online_but_not_available());

  EXPECT_EQ(status.get_proto_online(), 0);
  EXPECT_EQ(status.get_proto_offline(), 1);
}

TEST_F(ChannelAvailabilityStatusTest, set_proto_online) {
  supla_channel_availability_status status;

  status.set_proto_online(1);
  EXPECT_TRUE(status.is_online());
  EXPECT_FALSE(status.is_offline());
  EXPECT_FALSE(status.is_online_but_not_available());

  EXPECT_EQ(status.get_proto_online(), 1);
  EXPECT_EQ(status.get_proto_offline(), 0);

  status.set_proto_online(0);
  EXPECT_FALSE(status.is_online());
  EXPECT_TRUE(status.is_offline());
  EXPECT_FALSE(status.is_online_but_not_available());

  EXPECT_EQ(status.get_proto_online(), 0);
  EXPECT_EQ(status.get_proto_offline(), 1);

  status.set_proto_online(2);
  EXPECT_FALSE(status.is_online());
  EXPECT_FALSE(status.is_offline());
  EXPECT_TRUE(status.is_online_but_not_available());

  EXPECT_EQ(status.get_proto_online(), 2);
  EXPECT_EQ(status.get_proto_offline(), 2);

  status.set_proto_online(3);
  EXPECT_TRUE(status.is_online());
  EXPECT_FALSE(status.is_offline());
  EXPECT_FALSE(status.is_online_but_not_available());

  EXPECT_EQ(status.get_proto_online(), 1);
  EXPECT_EQ(status.get_proto_offline(), 0);
}

TEST_F(ChannelAvailabilityStatusTest, compare) {
  supla_channel_availability_status status1(0, 0);
  supla_channel_availability_status status2(1, 0);
  supla_channel_availability_status status3(2, 0);
  supla_channel_availability_status status4(0, 1);

  EXPECT_NE(status1, status2);
  EXPECT_NE(status1, status3);
  EXPECT_NE(status1, status4);
  EXPECT_EQ(status2, status4);
}

TEST_F(ChannelAvailabilityStatusTest, set_offline) {
  supla_channel_availability_status status;

  status.set_offline(false);
  EXPECT_TRUE(status.is_online());
  EXPECT_FALSE(status.is_offline());
  EXPECT_FALSE(status.is_online_but_not_available());

  EXPECT_EQ(status.get_proto_online(), 1);
  EXPECT_EQ(status.get_proto_offline(), 0);

  status.set_offline(true);
  EXPECT_FALSE(status.is_online());
  EXPECT_TRUE(status.is_offline());
  EXPECT_FALSE(status.is_online_but_not_available());

  EXPECT_EQ(status.get_proto_online(), 0);
  EXPECT_EQ(status.get_proto_offline(), 1);
}

} /* namespace testing */
