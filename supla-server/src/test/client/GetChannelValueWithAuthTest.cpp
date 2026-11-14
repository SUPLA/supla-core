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

#include <memory>

#include "client/call_handler/get_channel_value_with_auth.h"
#include "doubles/SrpcAdapterMock.h"
#include "gtest/gtest.h"  // NOLINT

namespace testing {

class GetChannelValueWithAuthHandlerStub
    : public supla_ch_get_channel_value_with_auth {
 public:
  GetChannelValueWithAuthHandlerStub()
      : result_code(SUPLA_RESULTCODE_TRUE),
        client_id(0),
        user_id(0),
        channel_available(false),
        channel_exists_call_count(0) {}

  void handle_request(supla_abstract_srpc_adapter* srpc_adapter,
                      TCS_GetChannelValueWithAuth* request) {
    supla_ch_get_channel_value_with_auth::handle_request(
        std::shared_ptr<supla_client>(), srpc_adapter, request);
  }

  int result_code;
  int client_id;
  int user_id;
  bool channel_available;
  int channel_exists_call_count;

 protected:
  authentication_result authenticate(std::shared_ptr<supla_client>,
                                     TCS_ClientAuthorizationDetails*,
                                     supla_abstract_srpc_adapter*,
                                     supla_mariadb_access_provider*,
                                     supla_connection_dao*,
                                     supla_client_dao*) override {
    return {result_code, client_id, user_id};
  }

  bool channel_exists(supla_client_dao*, int, int) override {
    channel_exists_call_count++;
    return channel_available;
  }
};

TEST(GetChannelValueWithAuthTest, respondsOnAuthenticationFailure) {
  GetChannelValueWithAuthHandlerStub handler;
  SrpcAdapterMock srpc_adapter;
  TCS_GetChannelValueWithAuth request = {};

  request.ChannelId = 123;
  handler.result_code = SUPLA_RESULTCODE_BAD_CREDENTIALS;

  EXPECT_CALL(srpc_adapter, sc_async_get_channel_value_result(
                                Truly([](TSC_GetChannelValueResult* result) {
                                  return result && result->ChannelId == 123 &&
                                         result->ResultCode ==
                                             SUPLA_RESULTCODE_BAD_CREDENTIALS;
                                })))
      .Times(1)
      .WillOnce(Return(1));

  handler.handle_request(&srpc_adapter, &request);

  EXPECT_EQ(handler.channel_exists_call_count, 0);
}

TEST(GetChannelValueWithAuthTest, respondsWhenChannelIsNotAvailable) {
  GetChannelValueWithAuthHandlerStub handler;
  SrpcAdapterMock srpc_adapter;
  TCS_GetChannelValueWithAuth request = {};

  request.ChannelId = 456;
  handler.result_code = SUPLA_RESULTCODE_TRUE;
  handler.client_id = 12;
  handler.user_id = 34;
  handler.channel_available = false;

  EXPECT_CALL(srpc_adapter, sc_async_get_channel_value_result(
                                Truly([](TSC_GetChannelValueResult* result) {
                                  return result && result->ChannelId == 456 &&
                                         result->ResultCode ==
                                             SUPLA_RESULTCODE_SUBJECT_NOT_FOUND;
                                })))
      .Times(1)
      .WillOnce(Return(1));

  handler.handle_request(&srpc_adapter, &request);

  EXPECT_EQ(handler.channel_exists_call_count, 1);
}

TEST(GetChannelValueWithAuthTest, respondsWhenChannelIdIsZero) {
  GetChannelValueWithAuthHandlerStub handler;
  SrpcAdapterMock srpc_adapter;
  TCS_GetChannelValueWithAuth request = {};

  handler.result_code = SUPLA_RESULTCODE_TRUE;

  EXPECT_CALL(srpc_adapter, sc_async_get_channel_value_result(
                                Truly([](TSC_GetChannelValueResult* result) {
                                  return result && result->ChannelId == 0 &&
                                         result->ResultCode ==
                                             SUPLA_RESULTCODE_SUBJECT_NOT_FOUND;
                                })))
      .Times(1)
      .WillOnce(Return(1));

  handler.handle_request(&srpc_adapter, &request);

  EXPECT_EQ(handler.channel_exists_call_count, 0);
}

}  // namespace testing
