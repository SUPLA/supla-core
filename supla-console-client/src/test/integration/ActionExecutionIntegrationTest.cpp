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

#include "ActionExecutionIntegrationTest.h"

#include "proto.h"

namespace testing {

ActionExecutionIntegrationTest::ActionExecutionIntegrationTest() {
  memset(&result, 0, sizeof(TSC_ActionExecutionResult));
}

void ActionExecutionIntegrationTest::SetUp() {
  ProperlyRegistered::SetUp();
  memset(&result, 0, sizeof(TSC_ActionExecutionResult));
}

ActionExecutionIntegrationTest::~ActionExecutionIntegrationTest() {}

void ActionExecutionIntegrationTest::onActionExecutionResult(
    TSC_ActionExecutionResult *result) {
  ASSERT_FALSE(result == NULL);
  memcpy(&this->result, result, sizeof(TSC_ActionExecutionResult));
  cancelIteration();
}

TEST_F(ActionExecutionIntegrationTest, channelNotFound) {
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_execute_action(sclient, ACTION_OPEN, nullptr, 0,
                                        ACTION_SUBJECT_TYPE_CHANNEL, 55667788),
            0);
  iterateUntilDefaultTimeout();

  ASSERT_EQ(result.ResultCode, SUPLA_RESULTCODE_SUBJECT_NOT_FOUND);
  ASSERT_EQ(result.ActionId, ACTION_OPEN);
  ASSERT_EQ(result.SubjectType, ACTION_SUBJECT_TYPE_CHANNEL);
  ASSERT_EQ(result.SubjectId, 55667788);
}

TEST_F(ActionExecutionIntegrationTest, channelGroupNotFound) {
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(
      supla_client_execute_action(sclient, ACTION_OPEN, nullptr, 0,
                                  ACTION_SUBJECT_TYPE_CHANNEL_GROUP, 11667788),
      0);
  iterateUntilDefaultTimeout();

  ASSERT_EQ(result.ResultCode, SUPLA_RESULTCODE_SUBJECT_NOT_FOUND);
  ASSERT_EQ(result.ActionId, ACTION_OPEN);
  ASSERT_EQ(result.SubjectType, ACTION_SUBJECT_TYPE_CHANNEL_GROUP);
  ASSERT_EQ(result.SubjectId, 11667788);
}

TEST_F(ActionExecutionIntegrationTest, sceneNotFound) {
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_execute_action(sclient, ACTION_EXECUTE, nullptr, 0,
                                        ACTION_SUBJECT_TYPE_SCENE, 22667788),
            0);
  iterateUntilDefaultTimeout();

  ASSERT_EQ(result.ResultCode, SUPLA_RESULTCODE_SUBJECT_NOT_FOUND);
  ASSERT_EQ(result.ActionId, ACTION_EXECUTE);
  ASSERT_EQ(result.SubjectType, ACTION_SUBJECT_TYPE_SCENE);
  ASSERT_EQ(result.SubjectId, 22667788);
}

TEST_F(ActionExecutionIntegrationTest, rollerShutterIncorrectParameters) {
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_execute_action(sclient, ACTION_SHUT_PARTIALLY, nullptr,
                                        0, ACTION_SUBJECT_TYPE_CHANNEL, 303),
            0);
  iterateUntilDefaultTimeout();

  ASSERT_EQ(result.ResultCode, SUPLA_RESULTCODE_INCORRECT_PARAMETERS);
  ASSERT_EQ(result.ActionId, ACTION_SHUT_PARTIALLY);
  ASSERT_EQ(result.SubjectType, ACTION_SUBJECT_TYPE_CHANNEL);
  ASSERT_EQ(result.SubjectId, 303);
}

TEST_F(ActionExecutionIntegrationTest, shutPartiallyWithSuccess) {
  TAction_RS_Parameters rs = {};
  rs.Percentage = 10;

  // The server version on the day of test preparation does not check group
  // function. If starts checking, it will be necessary to add the appropriate
  // group to the database before performing the test.

  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_execute_action(sclient, ACTION_SHUT_PARTIALLY, &rs,
                                        sizeof(TAction_RS_Parameters),
                                        ACTION_SUBJECT_TYPE_CHANNEL_GROUP, 1),
            0);
  iterateUntilDefaultTimeout();

  ASSERT_EQ(result.ResultCode, SUPLA_RESULTCODE_TRUE);
  ASSERT_EQ(result.ActionId, ACTION_SHUT_PARTIALLY);
  ASSERT_EQ(result.SubjectType, ACTION_SUBJECT_TYPE_CHANNEL_GROUP);
  ASSERT_EQ(result.SubjectId, 1);
}

TEST_F(ActionExecutionIntegrationTest, rgbwIncorrectParameters) {
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(
      supla_client_execute_action(sclient, ACTION_SET_RGBW_PARAMETERS, nullptr,
                                  0, ACTION_SUBJECT_TYPE_CHANNEL, 170),
      0);
  iterateUntilDefaultTimeout();

  ASSERT_EQ(result.ResultCode, SUPLA_RESULTCODE_INCORRECT_PARAMETERS);
  ASSERT_EQ(result.ActionId, ACTION_SET_RGBW_PARAMETERS);
  ASSERT_EQ(result.SubjectType, ACTION_SUBJECT_TYPE_CHANNEL);
  ASSERT_EQ(result.SubjectId, 170);
}

TEST_F(ActionExecutionIntegrationTest, setBrightnessWithSuccess) {
  TAction_RGBW_Parameters rgbw = {};
  rgbw.Brightness = 44;

  // The server version on the day of test preparation does not check group
  // function. If starts checking, it will be necessary to add the appropriate
  // group to the database before performing the test.

  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_execute_action(sclient, ACTION_SET_RGBW_PARAMETERS,
                                        &rgbw, sizeof(TAction_RGBW_Parameters),
                                        ACTION_SUBJECT_TYPE_CHANNEL_GROUP, 1),
            0);
  iterateUntilDefaultTimeout();

  ASSERT_EQ(result.ResultCode, SUPLA_RESULTCODE_TRUE);
  ASSERT_EQ(result.ActionId, ACTION_SET_RGBW_PARAMETERS);
  ASSERT_EQ(result.SubjectType, ACTION_SUBJECT_TYPE_CHANNEL_GROUP);
  ASSERT_EQ(result.SubjectId, 1);
}

TEST_F(ActionExecutionIntegrationTest, channelIsOffline) {
  TAction_RS_Parameters rs = {};
  rs.Percentage = 10;

  // The server version on the day of test preparation does not check group
  // function. If starts checking, it will be necessary to add the appropriate
  // group to the database before performing the test.

  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_execute_action(sclient, ACTION_SHUT_PARTIALLY, &rs,
                                        sizeof(TAction_RS_Parameters),
                                        ACTION_SUBJECT_TYPE_CHANNEL, 303),
            0);
  iterateUntilDefaultTimeout();

  ASSERT_EQ(result.ResultCode, SUPLA_RESULTCODE_CHANNEL_IS_OFFLINE);
  ASSERT_EQ(result.ActionId, ACTION_SHUT_PARTIALLY);
  ASSERT_EQ(result.SubjectType, ACTION_SUBJECT_TYPE_CHANNEL);
  ASSERT_EQ(result.SubjectId, 303);
}

} /* namespace testing */
