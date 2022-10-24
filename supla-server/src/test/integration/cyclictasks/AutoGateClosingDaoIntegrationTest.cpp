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

#include "integration/cyclictasks/AutoGateClosingDaoIntegrationTest.h"

#include <string>

#include "srpc/srpc.h"

using std::string;
using std::vector;

namespace testing {

AutoGateClosingDaoIntegrationTest::AutoGateClosingDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

AutoGateClosingDaoIntegrationTest::~AutoGateClosingDaoIntegrationTest() {}

void AutoGateClosingDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  ASSERT_TRUE(dba != nullptr);
  dao = new supla_auto_gate_closing_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void AutoGateClosingDaoIntegrationTest::TearDown() {
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

TEST_F(AutoGateClosingDaoIntegrationTest, emptyList) {
  runSqlScript("AddAutoGateClosingConfig.sql");
  ASSERT_TRUE(dba->connect());

  vector<supla_abstract_auto_gate_closing_dao::item_t> result =
      dao->get_all_active();

  EXPECT_EQ(result.size(), 3);

  supla_abstract_auto_gate_closing_dao::item_t i3 = result.back();
  result.pop_back();
  supla_abstract_auto_gate_closing_dao::item_t i2 = result.back();
  result.pop_back();
  supla_abstract_auto_gate_closing_dao::item_t i1 = result.back();

  EXPECT_EQ(i1.user_id, 2);
  EXPECT_EQ(i1.device_id, 83);
  EXPECT_EQ(i1.channel_id, 167);
  EXPECT_GE(i1.seconds_since_last_attempt, ATTEMPT_RETRY_TIME_SEC);
  EXPECT_LE(i1.seconds_since_last_attempt, ATTEMPT_RETRY_TIME_SEC + 2);
  EXPECT_FALSE(i1.seen_open);

  EXPECT_EQ(i2.user_id, 2);
  EXPECT_EQ(i2.device_id, 83);
  EXPECT_EQ(i2.channel_id, 174);
  EXPECT_GE(i2.seconds_since_last_attempt, ATTEMPT_RETRY_TIME_SEC);
  EXPECT_LE(i2.seconds_since_last_attempt, ATTEMPT_RETRY_TIME_SEC + 2);
  EXPECT_TRUE(i2.seen_open);

  EXPECT_EQ(i3.user_id, 2);
  EXPECT_EQ(i3.device_id, 146);
  EXPECT_EQ(i3.channel_id, 308);
  EXPECT_GE(i3.seconds_since_last_attempt, 2);
  EXPECT_LE(i3.seconds_since_last_attempt, 3);
  EXPECT_TRUE(i3.seen_open);
}

TEST_F(AutoGateClosingDaoIntegrationTest, markGateOpen) {
  runSqlScript("AddAutoGateClosingConfig.sql");
  ASSERT_TRUE(dba->connect());

  string qresult;
  sqlQuery(
      "UPDATE supla_auto_gate_closing SET last_seen_open = "
      "DATE_ADD(UTC_TIMESTAMP(), INTERVAL -240 SECOND) WHERE channel_id = 308",
      &qresult);

  int open_time = dao->mark_gate_open(308);
  EXPECT_GE(open_time, 399);
  EXPECT_LE(open_time, 400);

  usleep(2000000);

  open_time = dao->mark_gate_open(308);
  EXPECT_GE(open_time, 397);
  EXPECT_LE(open_time, 398);

  usleep(1000000);

  open_time = dao->mark_gate_open(308);
  open_time = dao->mark_gate_open(308);
  open_time = dao->mark_gate_open(308);

  EXPECT_GE(open_time, 396);
  EXPECT_LE(open_time, 397);
}

TEST_F(AutoGateClosingDaoIntegrationTest, markGateClosed) {
  runSqlScript("AddAutoGateClosingConfig.sql");
  ASSERT_TRUE(dba->connect());

  vector<supla_abstract_auto_gate_closing_dao::item_t> result =
      dao->get_all_active();

  EXPECT_EQ(result.size(), 3);

  supla_abstract_auto_gate_closing_dao::item_t i = result.back();

  EXPECT_EQ(i.user_id, 2);
  EXPECT_EQ(i.device_id, 146);
  EXPECT_EQ(i.channel_id, 308);
  EXPECT_GE(i.seconds_since_last_attempt, 2);
  EXPECT_LE(i.seconds_since_last_attempt, 3);
  EXPECT_TRUE(i.seen_open);

  dao->mark_gate_closed(308);

  result = dao->get_all_active();

  EXPECT_EQ(result.size(), 3);

  i = result.back();

  EXPECT_EQ(i.user_id, 2);
  EXPECT_EQ(i.device_id, 146);
  EXPECT_EQ(i.channel_id, 308);
  EXPECT_GE(i.seconds_since_last_attempt, ATTEMPT_RETRY_TIME_SEC);
  EXPECT_LE(i.seconds_since_last_attempt, ATTEMPT_RETRY_TIME_SEC + 2);
  EXPECT_FALSE(i.seen_open);
}

TEST_F(AutoGateClosingDaoIntegrationTest, setClosingAttempt) {
  runSqlScript("AddAutoGateClosingConfig.sql");
  ASSERT_TRUE(dba->connect());

  vector<supla_abstract_auto_gate_closing_dao::item_t> result =
      dao->get_all_active();

  EXPECT_EQ(result.size(), 3);

  supla_abstract_auto_gate_closing_dao::item_t i = result.back();

  EXPECT_EQ(i.user_id, 2);
  EXPECT_EQ(i.device_id, 146);
  EXPECT_EQ(i.channel_id, 308);
  EXPECT_GE(i.seconds_since_last_attempt, 2);
  EXPECT_LE(i.seconds_since_last_attempt, 3);
  EXPECT_TRUE(i.seen_open);

  dao->set_closing_attempt(308);

  result = dao->get_all_active();

  EXPECT_EQ(result.size(), 3);

  i = result.back();

  EXPECT_EQ(i.user_id, 2);
  EXPECT_EQ(i.device_id, 146);
  EXPECT_EQ(i.channel_id, 308);
  EXPECT_GE(i.seconds_since_last_attempt, 0);
  EXPECT_LE(i.seconds_since_last_attempt, 1);
  EXPECT_TRUE(i.seen_open);
}

} /* namespace testing */
