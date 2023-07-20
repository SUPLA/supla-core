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

#include "DeviceDaoIntegrationTest.h"

#include <string>

using std::string;

namespace testing {

DeviceDaoIntegrationTest::DeviceDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

DeviceDaoIntegrationTest::~DeviceDaoIntegrationTest() {}

void DeviceDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  ASSERT_TRUE(dba != nullptr);
  dao = new supla_device_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void DeviceDaoIntegrationTest::TearDown() {
  if (dao) {
    delete dao;
    dao = nullptr;
  }

  if (dba) {
    delete dba;
    dba = nullptr;
  }

  Test::TearDown();
}

TEST_F(DeviceDaoIntegrationTest, setAndUpdateChanneValue) {
  char value[SUPLA_CHANNELVALUE_SIZE] = {0x01, 0x11, 0x22, 0x33,
                                         0x23, 0x24, 0x00, 0x25};
  dao->update_channel_value(2, 2, value, 10);

  string result = "";

  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_value WHERE channel_id = 2 "
      "AND "
      "user_id = 2 AND TIMESTAMPDIFF(SECOND, update_time, UTC_TIMESTAMP) >= 0 "
      "AND TIMESTAMPDIFF(SECOND, update_time, UTC_TIMESTAMP) <= 1 AND "
      "TIMESTAMPDIFF(SECOND, UTC_TIMESTAMP, valid_to) >= 9 AND "
      "TIMESTAMPDIFF(SECOND, UTC_TIMESTAMP, valid_to) <= 10",
      &result);

  EXPECT_EQ(result, "v\n0111223323240025\n");

  value[0] = 0x02;
  dao->update_channel_value(2, 2, value, 0);

  result = "";

  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_value WHERE "
      "channel_id = 2 AND user_id = 2 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) >= 0 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) <= 1 AND valid_to IS NULL",
      &result);

  EXPECT_EQ(result, "v\n0211223323240025\n");
}

TEST_F(DeviceDaoIntegrationTest, setAndUpdateExtendedValue) {
  TSuplaChannelExtendedValue ev_struct = {};

  ev_struct.type = 10;

  for (int a = 0; a < 15; a++) {
    ev_struct.value[a] = a;
  }

  {
    ev_struct.size = 10;
    supla_channel_extended_value ev(&ev_struct);
    dao->update_channel_extended_value(2, 2, &ev);
  }

  string result = "";

  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_extended_value WHERE "
      "channel_id = 2 AND user_id = 2 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) >= 0 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) <= 1",
      &result);

  EXPECT_EQ(result, "v\n00010203040506070809\n");

  {
    ev_struct.size = 5;
    supla_channel_extended_value ev(&ev_struct);
    dao->update_channel_extended_value(2, 2, &ev);
  }

  result = "";

  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_extended_value WHERE "
      "channel_id = 2 AND user_id = 2 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) >= 0 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) <= 1",
      &result);

  EXPECT_EQ(result, "v\n0001020304\n");

  {
    ev_struct.size = 0;
    supla_channel_extended_value ev(&ev_struct);
    dao->update_channel_extended_value(2, 2, &ev);
  }

  result = "";

  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_extended_value WHERE "
      "channel_id = 2 AND user_id = 2 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) >= 0 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) <= 1",
      &result);

  EXPECT_EQ(result, "v\nNULL\n");

  {
    ev_struct.size = 15;
    supla_channel_extended_value ev(&ev_struct);
    dao->update_channel_extended_value(2, 2, &ev);
  }

  result = "";

  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_extended_value WHERE "
      "channel_id = 2 AND user_id = 2 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) >= 0 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) <= 1",
      &result);

  EXPECT_EQ(result, "v\n000102030405060708090A0B0C0D0E\n");
}

} /* namespace testing */
