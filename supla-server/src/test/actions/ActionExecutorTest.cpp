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

#include "actions/action_config.h"
#include "doubles/device/ValueGetterStub.h"
#include "user/user.h"
namespace testing {

ActionExecutorTest::ActionExecutorTest(void) {
  aexec = NULL;
  device = NULL;
}

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

TEST_F(ActionExecutorTest, setGetChannelId) {
  aexec->set_scene_id(2, 3);
  aexec->set_group_id(5, 6);
  aexec->set_channel_id(12345, 5, 89);
  EXPECT_EQ(aexec->get_user_id(), 12345);
  EXPECT_EQ(aexec->get_device_id(), 5);
  EXPECT_EQ(aexec->get_channel_id(), 89);
  EXPECT_EQ(aexec->get_group_id(), 0);
  EXPECT_EQ(aexec->get_scene_id(), 0);
}

TEST_F(ActionExecutorTest, setGetChannelGroupId) {
  aexec->set_scene_id(2, 3);
  aexec->set_channel_id(4, 5, 89);
  aexec->set_group_id(12345, 6);
  EXPECT_EQ(aexec->get_user_id(), 12345);
  EXPECT_EQ(aexec->get_device_id(), 0);
  EXPECT_EQ(aexec->get_channel_id(), 0);
  EXPECT_EQ(aexec->get_group_id(), 6);
  EXPECT_EQ(aexec->get_scene_id(), 0);
}

TEST_F(ActionExecutorTest, setGetSceneId) {
  aexec->set_channel_id(1, 5, 89);
  aexec->set_group_id(5, 6);
  aexec->set_scene_id(12345, 3);
  EXPECT_EQ(aexec->get_user_id(), 12345);
  EXPECT_EQ(aexec->get_device_id(), 0);
  EXPECT_EQ(aexec->get_channel_id(), 0);
  EXPECT_EQ(aexec->get_group_id(), 0);
  EXPECT_EQ(aexec->get_scene_id(), 3);
}

TEST_F(ActionExecutorTest, executeScene) {
  supla_action_config config;
  config.set_subject_id(15);
  config.set_subject_type(stScene);
  config.set_action_id(ACTION_EXECUTE);

  ValueGetterStub value_getter;

  aexec->execute_action(supla_caller(ctIPC), 12345, &config, &value_getter);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getExecuteCounter(), 1);
  EXPECT_EQ(aexec->get_scene_id(), 15);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctIPC));
}

TEST_F(ActionExecutorTest, stopScene) {
  supla_action_config config;
  config.set_subject_id(25);
  config.set_subject_type(stScene);
  config.set_action_id(ACTION_STOP);

  ValueGetterStub value_getter;

  aexec->execute_action(supla_caller(ctIPC), 12345, &config, &value_getter);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getStopCounter(), 1);
  EXPECT_EQ(aexec->get_scene_id(), 25);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctIPC));
}

} /* namespace testing */
