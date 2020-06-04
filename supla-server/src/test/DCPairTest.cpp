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

#include "DCPairTest.h"

namespace testing {

TEST_F(DCPairTest, popDeviceChannelIDs) {
  std::list<dcpair> pairs;

  int a, b, c;

  for (a = 1; a <= 5; a++) {
    for (b = 1; b <= a; b++) {
      dcpair p(a, a + b);
      pairs.push_back(p);
    }
  }

  ASSERT_EQ((const long unsigned int)15, pairs.size());

  int deviceId = 0;

  std::list<int> cids;
  a = 0;
  b = pairs.size();

  while (dcpair::popDeviceChannelIDs(&pairs, &deviceId, &cids)) {
    a++;
    ASSERT_EQ(a, deviceId);
    ASSERT_EQ((long unsigned int)deviceId, cids.size());
    b -= cids.size();
    ASSERT_EQ((long unsigned int)b, pairs.size());

    c = 1;
    for (std::list<int>::iterator it = cids.begin(); it != cids.end(); it++) {
      ASSERT_EQ(c + a, *it);
      c++;
    }
  }

  ASSERT_EQ((int)5, a);
}

} /* namespace testing */
