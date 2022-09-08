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

#include "device/RegisterDeviceTest.h"

namespace testing {

RegisterDeviceTest::RegisterDeviceTest() : Test() {}

RegisterDeviceTest::~RegisterDeviceTest() {}

void RegisterDeviceTest::SetUp() {
  Test::SetUp();
  gettimeofday(&setUpTime, nullptr);

  authkeyCache.set_cache_size_limit(0);
  rd.set_hold_time_on_failure_usec(500000);
  ON_CALL(rd, get_authkey_cache).WillByDefault(Return(&authkeyCache));
  ON_CALL(srpcAdapter, get_proto_version)
      .WillByDefault(Return(SUPLA_PROTO_VERSION));
}

void RegisterDeviceTest::TearDown() { Test::TearDown(); }

__useconds_t RegisterDeviceTest::usecFromSetUp(void) {
  struct timeval now = {};
  gettimeofday(&now, nullptr);

  return ((now.tv_sec * 1000000 + now.tv_usec) -
          (setUpTime.tv_sec * 1000000 + setUpTime.tv_usec));
}

} /* namespace testing */
