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

#include "client/RegisterClientProtocolVersionsTest.h"

namespace testing {

RegisterClientProtocolVersionsTest::RegisterClientProtocolVersionsTest()
    : RegisterClientTest() {}

RegisterClientProtocolVersionsTest::~RegisterClientProtocolVersionsTest() {}

TEST_F(RegisterClientProtocolVersionsTest, version1) {
  TCS_SuplaRegisterClient_B register_client_b = {};

  EXPECT_CALL(srpcAdapter, get_proto_version).WillRepeatedly(Return(1));

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result)
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_GUID_ERROR, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(&register_client_b, nullptr, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientProtocolVersionsTest, version9) {
  TCS_SuplaRegisterClient_B register_client_b = {};

  EXPECT_CALL(srpcAdapter, get_proto_version).WillRepeatedly(Return(9));

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_b)
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_B *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_GUID_ERROR, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(&register_client_b, nullptr, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientProtocolVersionsTest, version17) {
  TCS_SuplaRegisterClient_B register_client_b = {};

  EXPECT_CALL(srpcAdapter, get_proto_version).WillRepeatedly(Return(17));

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c)
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_GUID_ERROR, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(&register_client_b, nullptr, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientProtocolVersionsTest, version19) {
  TCS_SuplaRegisterClient_B register_client_b = {};

  EXPECT_CALL(srpcAdapter, get_proto_version).WillRepeatedly(Return(19));

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_d)
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_D *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_GUID_ERROR, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(&register_client_b, nullptr, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

} /* namespace testing */
