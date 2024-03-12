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

#include "ChannelRelationTest.h"

#include "device/channel_relation.h"

namespace testing {

using std::vector;

TEST_F(ChannelRelationTest, compareLists) {
  vector<supla_channel_relation> r1;
  vector<supla_channel_relation> r2;

  EXPECT_FALSE(supla_channel_relation::equal(nullptr, nullptr));
  EXPECT_FALSE(supla_channel_relation::equal(&r1, nullptr));
  EXPECT_FALSE(supla_channel_relation::equal(nullptr, &r2));
  EXPECT_TRUE(supla_channel_relation::equal(&r1, &r2));

  r1.push_back(supla_channel_relation(1, 2, 3));

  EXPECT_FALSE(supla_channel_relation::equal(&r1, &r2));

  r2.push_back(supla_channel_relation(1, 2, 3));

  EXPECT_TRUE(supla_channel_relation::equal(&r1, &r2));

  r2.push_back(supla_channel_relation(4, 5, 6));

  EXPECT_FALSE(supla_channel_relation::equal(&r1, &r2));

  r1.push_back(supla_channel_relation(4, 5, 6));

  EXPECT_TRUE(supla_channel_relation::equal(&r1, &r2));

  r1.push_back(supla_channel_relation(7, 8, 9));

  EXPECT_FALSE(supla_channel_relation::equal(&r1, &r2));

  r2.push_back(supla_channel_relation(7, 8, 10));

  EXPECT_FALSE(supla_channel_relation::equal(&r1, &r2));
}

} /* namespace testing */
