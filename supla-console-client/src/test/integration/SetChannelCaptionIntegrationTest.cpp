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

#include "SetChannelCaptionIntegrationTest.h"
#include "log.h"

namespace testing {

SetChannelCaptionIntegrationTest::SetChannelCaptionIntegrationTest() {
  expectedResultCode = 0;
  expectedChannelID = 0;
  memset(expectedCaption, 0, SUPLA_CHANNEL_CAPTION_MAXSIZE);
}

SetChannelCaptionIntegrationTest::~SetChannelCaptionIntegrationTest() {}

void SetChannelCaptionIntegrationTest::channelMatch(
    TSC_SetChannelCaptionResult *result, TSC_SuplaChannel_C *channel) {
  if (result) {
    ASSERT_EQ(result->ResultCode, expectedResultCode);
    ASSERT_EQ(result->ChannelID, expectedChannelID);
    ASSERT_EQ(strncmp(expectedCaption, result->Caption,
                      SUPLA_CHANNEL_CAPTION_MAXSIZE),
              0);

    match |= 0x1;
  } else if (channel && channel->Id == expectedChannelID) {
    if (strncmp(expectedCaption, channel->Caption,
                SUPLA_CHANNEL_CAPTION_MAXSIZE) == 0) {
      match |= 0x2;
    }
  }

  if (match & 0x1 &&
      (match & 0x2 || expectedResultCode != SUPLA_RESULTCODE_TRUE)) {
    cancelIteration();
  }
}

void SetChannelCaptionIntegrationTest::onChannelCaptionSetResult(
    TSC_SetChannelCaptionResult *result) {
  ASSERT_FALSE(result == NULL);
  channelMatch(result, NULL);
}

void SetChannelCaptionIntegrationTest::channelUpdate(
    TSC_SuplaChannel_C *channel) {
  ASSERT_FALSE(channel == NULL);
  channelMatch(NULL, channel);
}

void SetChannelCaptionIntegrationTest::reconnect() {
  match = 0;
  GetChannelBasicCfg::reconnect();
}

TEST_F(SetChannelCaptionIntegrationTest, SetCaptionWhtnClientIsNotAuthorized) {
  expectedCaption[0] = 0;
  expectedResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  expectedChannelID = 167;
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_set_channel_caption(sclient, expectedChannelID,
                                             expectedCaption),
            0);

  iterateUntilDefaultTimeout();
}

TEST_F(SetChannelCaptionIntegrationTest, SetFewViarintsOfCaption) {
  expectedResultCode = SUPLA_RESULTCODE_TRUE;
  expectedChannelID = 167;
  ASSERT_FALSE(sclient == NULL);

  superuserAuthorize();
  getBasicCfg(expectedChannelID);

  ASSERT_EQ(strncmp(channelBasicCfg.Caption, "Szlaban",
                    SUPLA_CHANNEL_CAPTION_MAXSIZE),
            0);

  snprintf(expectedCaption, SUPLA_CHANNEL_CAPTION_MAXSIZE, "ABCDF");
  ASSERT_GT(supla_client_set_channel_caption(sclient, expectedChannelID,
                                             expectedCaption),
            0);

  iterateUntilDefaultTimeout();
  reconnect();
  getBasicCfg(expectedChannelID);

  ASSERT_EQ(strncmp(channelBasicCfg.Caption, expectedCaption,
                    SUPLA_CHANNEL_CAPTION_MAXSIZE),
            0);

  // Empty -----------------

  reconnect();
  superuserAuthorize();
  expectedCaption[0] = 0;
  ASSERT_GT(supla_client_set_channel_caption(sclient, expectedChannelID,
                                             expectedCaption),
            0);

  iterateUntilDefaultTimeout();
  reconnect();
  getBasicCfg(expectedChannelID);

  ASSERT_EQ(strncmp(channelBasicCfg.Caption, expectedCaption,
                    SUPLA_CHANNEL_CAPTION_MAXSIZE),
            0);

  // Emoji -----------------
  reconnect();
  superuserAuthorize();
  snprintf(expectedCaption, SUPLA_CHANNEL_CAPTION_MAXSIZE,
           "Caption with emoji 🤓");
  ASSERT_GT(supla_client_set_channel_caption(sclient, expectedChannelID,
                                             expectedCaption),
            0);

  iterateUntilDefaultTimeout();
  reconnect();
  getBasicCfg(expectedChannelID);

  ASSERT_EQ(strncmp(channelBasicCfg.Caption, expectedCaption,
                    SUPLA_CHANNEL_CAPTION_MAXSIZE),
            0);

  // NULL -----------------

  reconnect();
  superuserAuthorize();
  expectedCaption[0] = 0;
  ASSERT_GT(supla_client_set_channel_caption(sclient, expectedChannelID, NULL),
            0);

  iterateUntilDefaultTimeout();
  reconnect();
  getBasicCfg(expectedChannelID);
  ASSERT_EQ(strncmp(channelBasicCfg.Caption, expectedCaption,
                    SUPLA_CHANNEL_CAPTION_MAXSIZE),
            0);

  // FullSize -----------------
  // Database field is limited to 100 characters

  reconnect();
  superuserAuthorize();

  const char LongCaption[] =
      "FullSizeCaption........................................................."
      "......................123456789";

  snprintf(expectedCaption, SUPLA_CHANNEL_CAPTION_MAXSIZE, "%s", LongCaption);

  ASSERT_GT(supla_client_set_channel_caption(sclient, expectedChannelID,
                                             expectedCaption),
            0);
  expectedCaption[100] = 0;

  iterateUntilDefaultTimeout();
  reconnect();
  getBasicCfg(expectedChannelID);
  ASSERT_EQ(strncmp(channelBasicCfg.Caption, expectedCaption,
                    SUPLA_CHANNEL_CAPTION_MAXSIZE),
            0);
}

} /* namespace testing */
