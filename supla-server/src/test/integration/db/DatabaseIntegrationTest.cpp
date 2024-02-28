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

#include "DatabaseIntegrationTest.h"

#include <string>

using std::string;

namespace testing {

DatabaseIntegrationTest::DatabaseIntegrationTest()
    : IntegrationTest(), Test() {}

DatabaseIntegrationTest::~DatabaseIntegrationTest() {}

void DatabaseIntegrationTest::SetUp() {
  initTestDatabase();
  EXPECT_TRUE(db.connect());
  Test::SetUp();
}

void DatabaseIntegrationTest::TearDown() {
  Test::TearDown();
  db.disconnect();
}

TEST_F(DatabaseIntegrationTest, setDeviceChannelCaption) {
  string result;
  char caption[20] = {};

  sqlQuery("SELECT caption FROM supla_dev_channel WHERE id = 167", &result);
  EXPECT_EQ(result, "caption\nSzlaban\n");

  snprintf(caption, sizeof(caption), "ABCD");

  EXPECT_TRUE(db.set_caption(2, 167, caption,
                             SUPLA_DCS_CALL_SET_CHANNEL_CAPTION, false));

  result = "";
  sqlQuery("SELECT caption FROM supla_dev_channel WHERE id = 167", &result);
  EXPECT_EQ(result, "caption\nABCD\n");

  snprintf(caption, sizeof(caption), "%s", "");

  result = "";
  EXPECT_TRUE(db.set_caption(2, 167, caption,
                             SUPLA_DCS_CALL_SET_CHANNEL_CAPTION, false));
  EXPECT_FALSE(db.set_caption(2, 167, caption,
                              SUPLA_DCS_CALL_SET_CHANNEL_CAPTION, false));

  sqlQuery("SELECT caption FROM supla_dev_channel WHERE id = 167", &result);

  // Make sure the empty caption is not set to NULL
  EXPECT_EQ(result, "caption\n\n");
}

TEST_F(DatabaseIntegrationTest, setDeviceChannelCaptionOnlyWhenNull) {
  string result;
  char caption[20] = {};

  sqlQuery("SELECT caption FROM supla_dev_channel WHERE id = 140", &result);
  EXPECT_EQ(result, "caption\nNULL\n");

  snprintf(caption, sizeof(caption), "ABCD");
  EXPECT_TRUE(db.set_caption(2, 140, caption,
                             SUPLA_DCS_CALL_SET_CHANNEL_CAPTION, true));
  EXPECT_FALSE(db.set_caption(2, 140, caption,
                              SUPLA_DCS_CALL_SET_CHANNEL_CAPTION, true));

  result = "";
  sqlQuery("SELECT caption FROM supla_dev_channel WHERE id = 140", &result);
  EXPECT_EQ(result, "caption\nABCD\n");

  snprintf(caption, sizeof(caption), "XYZ");

  result = "";
  EXPECT_FALSE(db.set_caption(2, 140, caption,
                              SUPLA_DCS_CALL_SET_CHANNEL_CAPTION, true));

  sqlQuery("SELECT caption FROM supla_dev_channel WHERE id = 140", &result);
  EXPECT_EQ(result, "caption\nABCD\n");

  result = "";
  EXPECT_TRUE(db.set_caption(2, 140, caption,
                             SUPLA_DCS_CALL_SET_CHANNEL_CAPTION, false));

  sqlQuery("SELECT caption FROM supla_dev_channel WHERE id = 140", &result);
  EXPECT_EQ(result, "caption\nXYZ\n");
}

} /* namespace testing */
