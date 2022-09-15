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

#include "client/RegisterClientEssentialTest.h"

namespace testing {

RegisterClientEssentialTest::RegisterClientEssentialTest()
    : RegisterClientTest() {}

RegisterClientEssentialTest::~RegisterClientEssentialTest() {}

TEST_F(RegisterClientEssentialTest, incorrectSuperuserPassword) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");
  snprintf(register_client_d.Password, SUPLA_PASSWORD_MAXSIZE, "%s",
           "Bushnell.xyz");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 0;
        return true;
      });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_reg_enabled(25))
      .Times(1)
      .WillOnce(Return(false));

  EXPECT_CALL(rc, superuser_authorize(25, StrEq("nolan@atari.com"),
                                      StrEq("Bushnell.xyz")))
      .Times(1);

  EXPECT_CALL(rc, is_superuser_authorized).Times(1).WillOnce(Return(false));

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_BAD_CREDENTIALS, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

} /* namespace testing */
