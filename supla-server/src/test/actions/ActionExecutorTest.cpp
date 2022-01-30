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

#include "ActionExecutorTest.h"

#include "user/user.h"

namespace testing {

ActionExecutorTest::ActionExecutorTest(void) { aexec = NULL; }

ActionExecutorTest::~ActionExecutorTest(void) {}

void ActionExecutorTest::SetUp() {
  supla_user *user = supla_user::find(12345, true);
  device = new DeviceStub(NULL);
  device->setID(567);
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  device->get_channels()->add_channel(89, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL,
                                      0, 0, value, 0, NULL, NULL);
  supla_user::add_device(device, user->getUserID());
  aexec = new ActionExecutorMock();
}

void ActionExecutorTest::TearDown() {
  delete supla_user::find(12345, false);
  delete aexec;
}

TEST_F(ActionExecutorTest, accessDeviceWithAndWithoutId) {
  bool device_accessed = false;
  aexec->set_channel_id(1, 2, 3);
  aexec->access_device([&device_accessed](supla_device *device) -> void {
    device_accessed = true;
  });

  EXPECT_FALSE(device_accessed);

  aexec->set_channel_id(12345, 2, 3);
  aexec->access_device([&device_accessed](supla_device *device) -> void {
    device_accessed = true;
  });

  EXPECT_FALSE(device_accessed);

  aexec->set_channel_id(12345, 567, 3);
  aexec->access_device([this, &device_accessed](supla_device *device) -> void {
    device_accessed = true;
    EXPECT_EQ(device, this->device);
  });

  EXPECT_TRUE(device_accessed);

  device_accessed = false;

  aexec->set_channel_id(12345, 5, 89);
  aexec->access_device([this, &device_accessed](supla_device *device) -> void {
    device_accessed = true;
  });

  EXPECT_FALSE(device_accessed);

  aexec->set_channel_id(12345, 0, 89);
  aexec->access_device([this, &device_accessed](supla_device *device) -> void {
    device_accessed = true;
    EXPECT_EQ(device, this->device);
  });

  EXPECT_TRUE(device_accessed);
}

} /* namespace testing */
