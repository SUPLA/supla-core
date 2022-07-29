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

#include "SetLocationCaptionIntegrationTest.h"

#include "log.h"

namespace testing {

using std::string;

SetLocationCaptionIntegrationTest::SetLocationCaptionIntegrationTest() {
  expectedResultCode = 0;
  expectedLocationID = 0;
  memset(expectedCaption, 0, SUPLA_LOCATION_CAPTION_MAXSIZE);
}

SetLocationCaptionIntegrationTest::~SetLocationCaptionIntegrationTest() {}

void SetLocationCaptionIntegrationTest::locationMatch(
    TSC_SetCaptionResult *result, TSC_SuplaLocation *location) {
  if (result) {
    ASSERT_EQ(result->ResultCode, expectedResultCode);
    ASSERT_EQ(result->ID, expectedLocationID);
    ASSERT_EQ(strncmp(expectedCaption, result->Caption,
                      SUPLA_LOCATION_CAPTION_MAXSIZE),
              0);

    match |= 0x1;
  } else if (location && location->Id == expectedLocationID) {
    if (strncmp(expectedCaption, location->Caption,
                SUPLA_LOCATION_CAPTION_MAXSIZE) == 0) {
      match |= 0x2;
    }
  }

  if (match & 0x1 &&
      (match & 0x2 || expectedResultCode != SUPLA_RESULTCODE_TRUE)) {
    cancelIteration();
  }
}

void SetLocationCaptionIntegrationTest::onLocationCaptionSetResult(
    TSC_SetCaptionResult *result) {
  ASSERT_FALSE(result == NULL);
  locationMatch(result, NULL);
}

void SetLocationCaptionIntegrationTest::locationUpdate(
    TSC_SuplaLocation *location) {
  ASSERT_FALSE(location == NULL);
  locationMatch(NULL, location);
}

void SetLocationCaptionIntegrationTest::reconnect() {
  match = 0;
  IntegrationTest::reconnect();
}

string SetLocationCaptionIntegrationTest::dbGetCaption(int locationId) {
  char sql[100];
  snprintf(sql, sizeof(sql), "SELECT caption FROM supla_location WHERE id = %i",
           locationId);
  return sqlQuery(sql);
}

TEST_F(SetLocationCaptionIntegrationTest, SetCaptionWhtnClientIsNotAuthorized) {
  expectedCaption[0] = 0;
  expectedResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  expectedLocationID = 167;
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_set_location_caption(sclient, expectedLocationID,
                                              expectedCaption),
            0);

  iterateUntilDefaultTimeout();
}

TEST_F(SetLocationCaptionIntegrationTest, SetFewViarintsOfCaption) {
  expectedResultCode = SUPLA_RESULTCODE_TRUE;
  expectedLocationID = 2;
  ASSERT_FALSE(sclient == NULL);

  superuserAuthorize();

  ASSERT_EQ(dbGetCaption(expectedLocationID).compare("caption\nLocation #1\n"),
            0);

  snprintf(expectedCaption, SUPLA_LOCATION_CAPTION_MAXSIZE, "ABCDF");

  ASSERT_GT(supla_client_set_location_caption(sclient, expectedLocationID,
                                              expectedCaption),
            0);

  iterateUntilDefaultTimeout();

  ASSERT_EQ(dbGetCaption(expectedLocationID).compare("caption\nABCDF\n"), 0);

  // Empty -----------------

  reconnect();
  superuserAuthorize();
  expectedCaption[0] = 0;
  ASSERT_GT(supla_client_set_location_caption(sclient, expectedLocationID,
                                              expectedCaption),
            0);

  iterateUntilDefaultTimeout();

  ASSERT_EQ(dbGetCaption(expectedLocationID).compare("caption\nNULL\n"), 0);

  // Emoji -----------------
  reconnect();
  superuserAuthorize();
  snprintf(expectedCaption, SUPLA_LOCATION_CAPTION_MAXSIZE,
           "Caption with emoji 🤓");
  ASSERT_GT(supla_client_set_location_caption(sclient, expectedLocationID,
                                              expectedCaption),
            0);

  iterateUntilDefaultTimeout();

  // FullSize -----------------
  // Database field is limited to 100 characters

  reconnect();
  superuserAuthorize();

  const char LongCaption[] =
      "FullSizeCaption........................................................."
      "......................123456789";

  snprintf(expectedCaption, SUPLA_LOCATION_CAPTION_MAXSIZE, "%s", LongCaption);

  ASSERT_GT(supla_client_set_location_caption(sclient, expectedLocationID,
                                              expectedCaption),
            0);
  expectedCaption[100] = 0;

  iterateUntilDefaultTimeout();
  ASSERT_EQ(
      dbGetCaption(expectedLocationID)
          .compare("caption\nFullSizeCaption..................................."
                   "............................................123456\n"),
      0);
}

} /* namespace testing */
