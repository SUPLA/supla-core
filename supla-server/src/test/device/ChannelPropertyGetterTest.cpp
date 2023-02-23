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

#include "ChannelPropertyGetterTest.h"

#include "doubles/device/ChannelPropertyGetterMock.h"

namespace testing {

TEST_F(ChannelPropertyGetterTest, changeOfIdentifiersAtGetters) {
  ChannelPropertyGetterMock propertyGetter(123, 456, 789);
  EXPECT_EQ(propertyGetter.get_user_id(), 123);
  EXPECT_EQ(propertyGetter.get_device_id(), 456);
  EXPECT_EQ(propertyGetter.get_channel_id(), 789);

  EXPECT_CALL(propertyGetter,
              _get_value(Eq(234), Eq(567), Eq(890), IsNull(), IsNull()))
      .Times(1)
      .WillOnce(Return(nullptr));

  EXPECT_CALL(propertyGetter, _get_func(Eq(239), Eq(569), Eq(899)))
      .Times(1)
      .WillOnce(Return(SUPLA_CHANNELFNC_ACTIONTRIGGER));

  EXPECT_CALL(propertyGetter,
              _get_electricity_measurement(Eq(734), Eq(767), Eq(790)))
      .Times(1)
      .WillOnce(Return(nullptr));

  EXPECT_CALL(propertyGetter, _get_ic_measurement(Eq(834), Eq(867), Eq(890)))
      .Times(1)
      .WillOnce(Return(nullptr));

  EXPECT_EQ(propertyGetter.get_value(234, 567, 890), nullptr);

  EXPECT_EQ(propertyGetter.get_user_id(), 234);
  EXPECT_EQ(propertyGetter.get_device_id(), 567);
  EXPECT_EQ(propertyGetter.get_channel_id(), 890);

  EXPECT_EQ(propertyGetter.get_func(239, 569, 899),
            SUPLA_CHANNELFNC_ACTIONTRIGGER);

  EXPECT_EQ(propertyGetter.get_user_id(), 239);
  EXPECT_EQ(propertyGetter.get_device_id(), 569);
  EXPECT_EQ(propertyGetter.get_channel_id(), 899);

  EXPECT_EQ(propertyGetter.get_electricity_measurement(734, 767, 790), nullptr);

  EXPECT_EQ(propertyGetter.get_user_id(), 734);
  EXPECT_EQ(propertyGetter.get_device_id(), 767);
  EXPECT_EQ(propertyGetter.get_channel_id(), 790);

  EXPECT_EQ(propertyGetter.get_ic_measurement(834, 867, 890), nullptr);

  EXPECT_EQ(propertyGetter.get_user_id(), 834);
  EXPECT_EQ(propertyGetter.get_device_id(), 867);
  EXPECT_EQ(propertyGetter.get_channel_id(), 890);
}

} /* namespace testing */
