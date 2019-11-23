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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../client_device.h"
#include "../client_state.h"
#include "gtest/gtest.h"

#include <string>
#include <vector>

namespace {

class ClientDeviceTest : public ::testing::Test {
 protected:
};

TEST_F(ClientDeviceTest, ClientDeviceClass) {
  std::vector<client_state*> states;

  client_device_channel* channel = new client_device_channel(
      1, 2, 3, 4, 5, 6, 7, NULL, NULL, NULL, true, false, "test", states);

  char value[SUPLA_CHANNELVALUE_SIZE];
  char result[SUPLA_CHANNELVALUE_SIZE];

  value[0] = 1;

  channel->setValue(value);
  channel->getValue(result);

  ASSERT_TRUE(result[0] == value[0]);

  value[0] = 100;
  channel->setSubValue(value);
  channel->getSubValue(result);

  ASSERT_TRUE(result[0] == value[0]);

  channel->setOnline(true);

  ASSERT_TRUE(channel->getOnline() == true);

  channel->setOnline(false);

  ASSERT_TRUE(channel->getOnline() == false);

  delete channel;
}
}  // namespace
