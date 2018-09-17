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

#include "SrpcTest.h"
#include "gtest/gtest.h"  // NOLINT
#include "srpc.h"

namespace {

#define MAX_CALL_ID 10000

int *all_calls = NULL;  // Possible memory leak
int all_calls_size = 0;

_supla_int_t srpc_data_read(void *buf, _supla_int_t count, void *user_params) {
  return 0;
}

_supla_int_t srpc_data_write(void *buf, _supla_int_t count, void *user_params) {
  return 0;
}

class SrpcTest : public ::testing::Test {
 protected:
  void *srpcInit(void);
  void srpcCallAllowed(int min_version, int *calls);
};

void *SrpcTest::srpcInit(void) {
  TsrpcParams params;
  srpc_params_init(&params);
  params.data_read = &srpc_data_read;
  params.data_write = &srpc_data_write;

  return srpc_init(&params);
}

TEST_F(SrpcTest, init) {
  void *srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);
  ASSERT_EQ(SUPLA_PROTO_VERSION, srpc_get_proto_version(srpc));
  srpc_free(srpc);
}

TEST_F(SrpcTest, set_proto) {
  void *srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);
  ASSERT_EQ(SUPLA_PROTO_VERSION, srpc_get_proto_version(srpc));
  srpc_set_proto_version(srpc, SUPLA_PROTO_VERSION_MIN);
  ASSERT_EQ(SUPLA_PROTO_VERSION_MIN, srpc_get_proto_version(srpc));
  srpc_free(srpc);
}

void SrpcTest::srpcCallAllowed(int min_version, int *calls) {
  void *srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);
  srpc_set_proto_version(srpc, min_version);

  int n = 0;
  while (calls[n] != 0) {
    ASSERT_TRUE(calls[n] <= MAX_CALL_ID);
    ASSERT_EQ(srpc_call_allowed(srpc, calls[n]), 1);

    all_calls = (int *)realloc(all_calls, sizeof(int) * (all_calls_size + 1));
    ASSERT_TRUE(all_calls != NULL);
    all_calls[all_calls_size] = calls[n];
    all_calls_size++;
    n++;
    ASSERT_TRUE(n <= MAX_CALL_ID);
  }

  srpc_free(srpc);
}

TEST_F(SrpcTest, call_allowed_v1) {
  int calls[] = {SUPLA_DCS_CALL_GETVERSION,
                 SUPLA_SDC_CALL_GETVERSION_RESULT,
                 SUPLA_SDC_CALL_VERSIONERROR,
                 SUPLA_DCS_CALL_PING_SERVER,
                 SUPLA_SDC_CALL_PING_SERVER_RESULT,
                 SUPLA_DS_CALL_REGISTER_DEVICE,
                 SUPLA_SD_CALL_REGISTER_DEVICE_RESULT,
                 SUPLA_CS_CALL_REGISTER_CLIENT,
                 SUPLA_SC_CALL_REGISTER_CLIENT_RESULT,
                 SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED,
                 SUPLA_SD_CALL_CHANNEL_SET_VALUE,
                 SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT,
                 SUPLA_SC_CALL_LOCATION_UPDATE,
                 SUPLA_SC_CALL_LOCATIONPACK_UPDATE,
                 SUPLA_SC_CALL_CHANNEL_UPDATE,
                 SUPLA_SC_CALL_CHANNELPACK_UPDATE,
                 SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE,
                 SUPLA_CS_CALL_GET_NEXT,
                 SUPLA_SC_CALL_EVENT,
                 SUPLA_CS_CALL_CHANNEL_SET_VALUE,
                 0};
  srpcCallAllowed(1, calls);
}

TEST_F(SrpcTest, call_allowed_v2) {
  int calls[] = {SUPLA_DS_CALL_REGISTER_DEVICE_B,
                 SUPLA_DCS_CALL_SET_ACTIVITY_TIMEOUT,
                 SUPLA_SDC_CALL_SET_ACTIVITY_TIMEOUT_RESULT, 0};

  srpcCallAllowed(2, calls);
}

TEST_F(SrpcTest, call_allowed_v3) {
  int calls[] = {SUPLA_CS_CALL_CHANNEL_SET_VALUE_B, 0};

  srpcCallAllowed(3, calls);
}

TEST_F(SrpcTest, call_allowed_v5) {
  int calls[] = {SUPLA_DS_CALL_GET_FIRMWARE_UPDATE_URL,
                 SUPLA_SD_CALL_GET_FIRMWARE_UPDATE_URL_RESULT, 0};

  srpcCallAllowed(5, calls);
}

TEST_F(SrpcTest, call_allowed_v6) {
  int calls[] = {SUPLA_DS_CALL_REGISTER_DEVICE_C,
                 SUPLA_CS_CALL_REGISTER_CLIENT_B, 0};

  srpcCallAllowed(6, calls);
}

TEST_F(SrpcTest, call_allowed_v7) {
  int calls[] = {SUPLA_CS_CALL_REGISTER_CLIENT_C,
                 SUPLA_DS_CALL_REGISTER_DEVICE_D,
                 SUPLA_DCS_CALL_GET_REGISTRATION_ENABLED,
                 SUPLA_SDC_CALL_GET_REGISTRATION_ENABLED_RESULT, 0};

  srpcCallAllowed(7, calls);
}

TEST_F(SrpcTest, call_allowed_v8) {
  int calls[] = {SUPLA_SC_CALL_CHANNELPACK_UPDATE_B,
                 SUPLA_SC_CALL_CHANNEL_UPDATE_B, 0};

  srpcCallAllowed(8, calls);
}

TEST_F(SrpcTest, call_allowed_v9) {
  int calls[] = {SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_B,
                 SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE,
                 SUPLA_SC_CALL_CHANNELGROUP_RELATION_PACK_UPDATE,
                 SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE,
                 SUPLA_CS_CALL_SET_VALUE,
                 0};

  srpcCallAllowed(9, calls);
}

TEST_F(SrpcTest, call_allowed_v10) {
  int calls[] = {SUPLA_DS_CALL_DEVICE_CHANNEL_EXTENDEDVALUE_CHANGED,
                 SUPLA_SC_CALL_CHANNELEXTENDEDVALUE_PACK_UPDATE,
                 SUPLA_SD_CALL_CHANNEL_CALIBRATE,
                 SUPLA_CS_CALL_CHANNEL_CALIBRATE,
                 SUPLA_CS_CALL_OAUTH_TOKEN_REQUEST,
                 SUPLA_SC_CALL_OAUTH_TOKEN_REQUEST_RESULT,
                 SUPLA_DS_CALL_REGISTER_DEVICE_E,
                 SUPLA_SD_CALL_CHANNEL_ERASE_DATA,
                 SUPLA_DS_CALL_CHANNEL_ERASE_DATA_RESULT,
                 0};

  srpcCallAllowed(10, calls);
}

TEST_F(SrpcTest, call_not_allowed) {
  ASSERT_TRUE(all_calls != NULL);
  ASSERT_GT(all_calls_size, 0);

  void *srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  for (int a = 0; a <= MAX_CALL_ID; a++) {
    int b = 0;
    for (b = 0; b < all_calls_size; b++) {
      if (all_calls[b] == a) break;
    }

    if (b >= all_calls_size) {
      ASSERT_EQ(srpc_call_allowed(srpc, a), 0);
    }
  }

  srpc_free(srpc);
}

}  // namespace
