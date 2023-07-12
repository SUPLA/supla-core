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

#include "SetChannelGroupCaptionIntegrationTest.h"

#include "log.h"

using std::string;

namespace testing {

SetChannelGroupCaptionIntegrationTest::SetChannelGroupCaptionIntegrationTest() {
  expectedResultCode = 0;
  expectedChannelGroupID = 0;
  match = 0;
  memset(expectedCaption, 0, SUPLA_CHANNEL_GROUP_CAPTION_MAXSIZE);
}

SetChannelGroupCaptionIntegrationTest::
    ~SetChannelGroupCaptionIntegrationTest() {}

void SetChannelGroupCaptionIntegrationTest::channelGroupMatch(
    TSC_SetCaptionResult *result, TSC_SuplaChannelGroup_B *channel_group) {
  if (result) {
    ASSERT_EQ(result->ResultCode, expectedResultCode);
    ASSERT_EQ(result->ID, expectedChannelGroupID);
    ASSERT_EQ(strncmp(expectedCaption, result->Caption,
                      SUPLA_CHANNEL_GROUP_CAPTION_MAXSIZE),
              0);

    match |= 0x1;
  } else if (channel_group && channel_group->Id == expectedChannelGroupID) {
    if (strncmp(expectedCaption, channel_group->Caption,
                SUPLA_CHANNEL_GROUP_CAPTION_MAXSIZE) == 0) {
      match |= 0x2;
    }
  }

  if ((match & 0x1) &&
      ((match & 0x2) || expectedResultCode != SUPLA_RESULTCODE_TRUE)) {
    cancelIteration();
  }
}

void SetChannelGroupCaptionIntegrationTest::onChannelGroupCaptionSetResult(
    TSC_SetCaptionResult *result) {
  ASSERT_FALSE(result == NULL);
  channelGroupMatch(result, NULL);
}

void SetChannelGroupCaptionIntegrationTest::channelGroupUpdate(
    TSC_SuplaChannelGroup_B *channel_group) {
  ASSERT_FALSE(channel_group == NULL);
  channelGroupMatch(NULL, channel_group);
}

void SetChannelGroupCaptionIntegrationTest::reconnect() {
  match = 0;
  GetChannelBasicCfg::reconnect();
}

string SetChannelGroupCaptionIntegrationTest::dbGetCaption(int groupId) {
  char sql[100];
  snprintf(sql, sizeof(sql),
           "SELECT caption FROM supla_dev_channel_group WHERE id = %i",
           groupId);
  return sqlQuery(sql);
}

TEST_F(SetChannelGroupCaptionIntegrationTest,
       SetCaptionWhtnClientIsNotAuthorized) {
  expectedCaption[0] = 0;
  expectedResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  expectedChannelGroupID = 1;
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_set_channel_group_caption(
                sclient, expectedChannelGroupID, expectedCaption),
            0);

  iterateUntilDefaultTimeout();
}

TEST_F(SetChannelGroupCaptionIntegrationTest, SetFewViarintsOfCaption) {
  expectedResultCode = SUPLA_RESULTCODE_TRUE;
  expectedChannelGroupID = 1;
  ASSERT_FALSE(sclient == NULL);

  superuserAuthorize();

  ASSERT_EQ(dbGetCaption(expectedChannelGroupID).compare("caption\n\n"), 0);

  snprintf(expectedCaption, SUPLA_CHANNEL_GROUP_CAPTION_MAXSIZE, "ABCDF");

  ASSERT_GT(supla_client_set_channel_group_caption(
                sclient, expectedChannelGroupID, expectedCaption),
            0);

  iterateUntilDefaultTimeout();

  ASSERT_EQ(dbGetCaption(expectedChannelGroupID).compare("caption\nABCDF\n"),
            0);

  // Empty -----------------

  reconnect();
  superuserAuthorize();
  expectedCaption[0] = 0;
  ASSERT_GT(supla_client_set_channel_group_caption(
                sclient, expectedChannelGroupID, expectedCaption),
            0);

  iterateUntilDefaultTimeout();

  ASSERT_EQ(dbGetCaption(expectedChannelGroupID).compare("caption\nNULL\n"), 0);

  // Emoji -----------------
  reconnect();
  superuserAuthorize();
  snprintf(expectedCaption, SUPLA_CHANNEL_GROUP_CAPTION_MAXSIZE,
           "Caption with emoji 🤓");
  ASSERT_GT(supla_client_set_channel_group_caption(
                sclient, expectedChannelGroupID, expectedCaption),
            0);

  iterateUntilDefaultTimeout();

  // FullSize -----------------
  // Database field is limited to 255 characters

  reconnect();
  superuserAuthorize();

  const char LongCaption[] =
      "FullSizeCaption........................................................."
      "........................................................................"
      "........................................................................"
      ".................................123456789";

  snprintf(expectedCaption, SUPLA_CHANNEL_GROUP_CAPTION_MAXSIZE, "%s",
           LongCaption);

  ASSERT_GT(supla_client_set_channel_group_caption(
                sclient, expectedChannelGroupID, expectedCaption),
            0);
  expectedCaption[255] = 0;

  iterateUntilDefaultTimeout();
  ASSERT_EQ(
      dbGetCaption(expectedChannelGroupID)
          .compare("caption\nFullSizeCaption..................................."
                   "..........................................................."
                   "..........................................................."
                   "..........................................................."
                   "......................123456\n"),
      0);
}

} /* namespace testing */
