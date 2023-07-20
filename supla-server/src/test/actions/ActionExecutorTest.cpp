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
#include "device/devicechannels.h"
#include "doubles/device/ChannelPropertyGetterMock.h"
#include "doubles/device/DeviceDaoMock.h"
#include "user/user.h"

namespace testing {

using std::make_shared;
using std::shared_ptr;
using std::vector;

ActionExecutorTest::ActionExecutorTest(void) {
  aexec = nullptr;
  device = nullptr;
}

ActionExecutorTest::~ActionExecutorTest(void) {}

void ActionExecutorTest::SetUp() {
  supla_user *user = supla_user::find(12345, true);
  device = make_shared<DeviceStub>(nullptr);
  device->set_id(567);

  DeviceDaoMock dao;

  EXPECT_CALL(dao, get_channels(_)).Times(1).WillOnce([](supla_device *device) {
    vector<supla_device_channel *> result;

    char value[SUPLA_CHANNELVALUE_SIZE] = {};
    supla_device_channel *channel = new supla_device_channel(
        device, 89, 0, 0, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, 0, 0, value,
        0, nullptr, nullptr, nullptr);

    result.push_back(channel);

    return result;
  });

  supla_device_channels *channels =
      new supla_device_channels(&dao, device.get(), nullptr, nullptr, 0);

  device->set_channels(channels);

  supla_user::add_device(device, user->getUserID());
  aexec = new ActionExecutorMock();
}

void ActionExecutorTest::TearDown() {
  delete supla_user::find(12345, false);
  delete aexec;
}

TEST_F(ActionExecutorTest, getDeviceWithAndWithoutId) {
  aexec->set_channel_id(1, 2, 3);
  shared_ptr<supla_device> device = aexec->get_device();

  EXPECT_TRUE(device == nullptr);

  aexec->set_channel_id(12345, 2, 3);
  device = aexec->get_device();

  EXPECT_TRUE(device == nullptr);

  aexec->set_channel_id(12345, 567, 3);
  device = aexec->get_device();

  EXPECT_EQ(device, this->device);
  EXPECT_TRUE(device != nullptr);

  aexec->set_channel_id(12345, 5, 89);
  device = aexec->get_device();

  EXPECT_TRUE(device == nullptr);

  aexec->set_channel_id(12345, 0, 89);
  device = aexec->get_device();

  EXPECT_EQ(device, this->device);
  EXPECT_TRUE(device != nullptr);
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

  ChannelPropertyGetterMock property_getter;

  aexec->execute_action(supla_caller(ctIPC), 12345, &config, &property_getter);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getExecuteCounter(), 1);
  EXPECT_EQ(aexec->get_scene_id(), 15);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctIPC));
}

TEST_F(ActionExecutorTest, interruptAndExecuteScene) {
  supla_action_config config;
  config.set_subject_id(15);
  config.set_subject_type(stScene);
  config.set_action_id(ACTION_INTERRUPT_AND_EXECUTE);

  ChannelPropertyGetterMock property_getter;

  aexec->execute_action(supla_caller(ctIPC), 12345, &config, &property_getter);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getInterruptAndExecuteCounter(), 1);
  EXPECT_EQ(aexec->get_scene_id(), 15);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctIPC));
}

TEST_F(ActionExecutorTest, interruptScene) {
  supla_action_config config;
  config.set_subject_id(25);
  config.set_subject_type(stScene);
  config.set_action_id(ACTION_STOP);

  ChannelPropertyGetterMock property_getter;

  aexec->execute_action(supla_caller(ctIPC), 12345, &config, &property_getter);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getStopCounter(), 1);
  EXPECT_EQ(aexec->get_scene_id(), 25);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctIPC));
}

TEST_F(ActionExecutorTest, enableSchedule) {
  supla_action_config config;
  config.set_subject_id(25);
  config.set_subject_type(stSchedule);
  config.set_action_id(ACTION_ENABLE);

  ChannelPropertyGetterMock property_getter;

  aexec->execute_action(supla_caller(ctScene), 12345, &config,
                        &property_getter);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getEnableCounter(), 1);
  EXPECT_EQ(aexec->get_schedule_id(), 25);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctScene));
}

TEST_F(ActionExecutorTest, disableSchedule) {
  supla_action_config config;
  config.set_subject_id(26);
  config.set_subject_type(stSchedule);
  config.set_action_id(ACTION_DISABLE);

  ChannelPropertyGetterMock property_getter;

  aexec->execute_action(supla_caller(ctScene), 12345, &config,
                        &property_getter);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getDisableCounter(), 1);
  EXPECT_EQ(aexec->get_schedule_id(), 26);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctScene));
}

TEST_F(ActionExecutorTest, sendPush) {
  supla_action_config config;
  config.set_subject_id(155);
  config.set_subject_type(stPushNotification);
  config.set_action_id(ACTION_SEND);

  ChannelPropertyGetterMock property_getter;

  aexec->execute_action(supla_caller(ctIPC), 12345, &config, &property_getter);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getSentCounter(), 1);
  EXPECT_EQ(aexec->get_push_notification_id(), 155);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctIPC));
}

} /* namespace testing */
