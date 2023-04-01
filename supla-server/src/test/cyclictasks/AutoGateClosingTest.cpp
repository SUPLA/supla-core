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

#include "AutoGateClosingTest.h"

#include "device/value/channel_gate_value.h"
#include "doubles/cyclictasks/AutoGateClosingDaoMock.h"

namespace testing {

using std::vector;

void AutoGateClosingTest::SetUp() { dao = new AutoGateClosingDaoMock(nullptr); }

void AutoGateClosingTest::TearDown() { delete dao; }

TEST_F(AutoGateClosingTest, unknownGateState) {
  EXPECT_CALL(agc, get_dao).Times(1).WillOnce(Return(dao));
  EXPECT_CALL(agc, get_property_getter).Times(1).WillOnce(Return(&valueGetter));
  EXPECT_CALL(*dao, get_all_active).Times(1).WillOnce([](void) {
    supla_abstract_auto_gate_closing_dao::item_t i;
    i.user_id = 1;
    i.device_id = 2;
    i.channel_id = 3;
    i.seconds_since_last_attempt = ATTEMPT_RETRY_TIME_SEC;
    i.seen_open = false;
    vector<supla_abstract_auto_gate_closing_dao::item_t> result;
    result.push_back(i);
    return result;
  });

  EXPECT_CALL(valueGetter, _get_value(1, 2, 3, IsNull(), IsNull()))
      .Times(1)
      .WillOnce([](int user_id, int device_id, int channel_id,
                   supla_channel_fragment *fragment, bool *online) {
        return new supla_channel_gate_value(gsl_unknown, gsl_unknown);
      });

  EXPECT_CALL(agc, close_the_gate).Times(0);
  EXPECT_CALL(*dao, mark_gate_open).Times(0);

  EXPECT_CALL(*dao, mark_gate_closed).Times(0);
  EXPECT_CALL(*dao, set_closing_attempt).Times(0);

  agc.run(nullptr, nullptr);
}

TEST_F(AutoGateClosingTest, stillClosed) {
  EXPECT_CALL(agc, get_dao).Times(1).WillOnce(Return(dao));
  EXPECT_CALL(agc, get_property_getter).Times(1).WillOnce(Return(&valueGetter));
  EXPECT_CALL(*dao, get_all_active).Times(1).WillOnce([](void) {
    supla_abstract_auto_gate_closing_dao::item_t i;
    i.user_id = 1;
    i.device_id = 2;
    i.channel_id = 3;
    i.seconds_since_last_attempt = ATTEMPT_RETRY_TIME_SEC;
    i.seen_open = false;
    vector<supla_abstract_auto_gate_closing_dao::item_t> result;
    result.push_back(i);
    return result;
  });

  EXPECT_CALL(valueGetter, _get_value(1, 2, 3, IsNull(), IsNull()))
      .Times(1)
      .WillOnce([](int user_id, int device_id, int channel_id,
                   supla_channel_fragment *fragment, bool *online) {
        return new supla_channel_gate_value(gsl_closed, gsl_unknown);
      });

  EXPECT_CALL(agc, close_the_gate).Times(0);
  EXPECT_CALL(*dao, mark_gate_open).Times(0);

  EXPECT_CALL(*dao, mark_gate_closed).Times(0);
  EXPECT_CALL(*dao, set_closing_attempt).Times(0);

  agc.run(nullptr, nullptr);
}

TEST_F(AutoGateClosingTest, closeSuccessfully) {
  EXPECT_CALL(agc, get_dao).Times(1).WillOnce(Return(dao));
  EXPECT_CALL(agc, get_property_getter).Times(1).WillOnce(Return(&valueGetter));
  EXPECT_CALL(*dao, get_all_active).Times(1).WillOnce([](void) {
    supla_abstract_auto_gate_closing_dao::item_t i;
    i.user_id = 1;
    i.device_id = 2;
    i.channel_id = 3;
    i.seconds_since_last_attempt = 20;
    i.seen_open = true;
    vector<supla_abstract_auto_gate_closing_dao::item_t> result;
    result.push_back(i);
    return result;
  });

  EXPECT_CALL(valueGetter, _get_value(1, 2, 3, IsNull(), IsNull()))
      .Times(1)
      .WillOnce([](int user_id, int device_id, int channel_id,
                   supla_channel_fragment *fragemtn, bool *online) {
        return new supla_channel_gate_value(gsl_closed, gsl_unknown);
      });

  EXPECT_CALL(agc, close_the_gate).Times(0);
  EXPECT_CALL(*dao, mark_gate_open).Times(0);

  EXPECT_CALL(*dao, mark_gate_closed(3)).Times(1);
  EXPECT_CALL(*dao, set_closing_attempt).Times(0);

  agc.run(nullptr, nullptr);
}

TEST_F(AutoGateClosingTest, closeAttempt) {
  EXPECT_CALL(agc, get_dao).Times(3).WillRepeatedly(Return(dao));
  EXPECT_CALL(agc, get_property_getter)
      .Times(3)
      .WillRepeatedly(Return(&valueGetter));

  Sequence s1;

  EXPECT_CALL(*dao, get_all_active)
      .Times(2)
      .InSequence(s1)
      .WillRepeatedly([](void) {
        supla_abstract_auto_gate_closing_dao::item_t i;
        i.user_id = 1;
        i.device_id = 2;
        i.channel_id = 3;
        i.seconds_since_last_attempt = ATTEMPT_RETRY_TIME_SEC;
        i.seen_open = true;
        vector<supla_abstract_auto_gate_closing_dao::item_t> result;
        result.push_back(i);
        return result;
      });

  EXPECT_CALL(*dao, get_all_active).Times(1).InSequence(s1).WillOnce([](void) {
    supla_abstract_auto_gate_closing_dao::item_t i;
    i.user_id = 1;
    i.device_id = 2;
    i.channel_id = 3;
    i.seconds_since_last_attempt = 0;
    i.seen_open = true;
    vector<supla_abstract_auto_gate_closing_dao::item_t> result;
    result.push_back(i);
    return result;
  });

  EXPECT_CALL(valueGetter, _get_value(1, 2, 3, IsNull(), IsNull()))
      .Times(3)
      .WillRepeatedly([](int user_id, int device_id, int channel_id,
                         supla_channel_fragment *fragment, bool *online) {
        return new supla_channel_gate_value(gsl_open, gsl_unknown);
      });

  EXPECT_CALL(agc, close_the_gate(1, 2, 3)).Times(2);
  EXPECT_CALL(*dao, mark_gate_open(3)).Times(3);

  EXPECT_CALL(*dao, mark_gate_closed).Times(0);
  EXPECT_CALL(*dao, set_closing_attempt(3)).Times(2);

  agc.run(nullptr, nullptr);
  agc.run(nullptr, nullptr);
  agc.run(nullptr, nullptr);
}

}  // namespace testing
