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
#include <list>

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

TEST_F(DCPairTest, sortByDeviceId) {
  std::list<dcpair> pairs;

  {
    dcpair p(1, 1);
    pairs.push_back(p);
  }

  {
    dcpair p(2, 1);
    pairs.push_back(p);
  }

  {
    dcpair p(3, 1);
    pairs.push_back(p);
  }

  {
    dcpair p(1, 2);
    pairs.push_back(p);
  }

  {
    dcpair p(2, 2);
    pairs.push_back(p);
  }

  {
    dcpair p(3, 2);
    pairs.push_back(p);
  }

  {
    dcpair p(1, 3);
    pairs.push_back(p);
  }

  {
    dcpair p(2, 3);
    pairs.push_back(p);
  }

  {
    dcpair p(3, 3);
    pairs.push_back(p);
  }

  int DeviceID = 1;
  int ChannelID = 1;

  dcpair::sort_by_device_id(&pairs);

  for (std::list<dcpair>::iterator it = pairs.begin(); it != pairs.end();
       it++) {
    ASSERT_EQ(DeviceID, it->getDeviceId());
    ASSERT_EQ(ChannelID, it->getChannelId());

    ChannelID++;
    if (ChannelID > 3) {
      ChannelID = 1;
      DeviceID++;
    }
  }
}

TEST_F(DCPairTest, detectingTheLastOne) {
  std::list<dcpair> pairs;

  {
    dcpair p(1, 1);
    pairs.push_back(p);
  }

  {
    dcpair p(2, 1);
    pairs.push_back(p);
  }

  {
    dcpair p(3, 1);
    pairs.push_back(p);
  }

  {
    dcpair p(1, 2);
    pairs.push_back(p);
  }

  {
    dcpair p(2, 2);
    pairs.push_back(p);
  }

  {
    dcpair p(3, 2);
    pairs.push_back(p);
  }

  {
    dcpair p(1, 3);
    pairs.push_back(p);
  }

  {
    dcpair p(2, 3);
    pairs.push_back(p);
  }

  {
    dcpair p(3, 3);
    pairs.push_back(p);
  }

  {
    dcpair p(4, 10);
    pairs.push_back(p);
  }

  dcpair::sort_by_device_id(&pairs);

  int n = 1;

  for (std::list<dcpair>::iterator it = pairs.begin(); it != pairs.end();
       it++) {
    if (n == 10) {
      ASSERT_TRUE(dcpair::last_one(&pairs, it));
    } else {
      ASSERT_EQ(n % 3 == 0 ? 1 : 0, dcpair::last_one(&pairs, it) ? 1 : 0);
    }

    n++;
  }

  ASSERT_EQ(n, 11);
}

} /* namespace testing */
