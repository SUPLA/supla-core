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

namespace {

#define SRPC_QUEUE_SIZE 10
#define MAX_CALL_ID 10000
#define BUFFER_MAX_SIZE 131072

int *all_calls = NULL;  // Possible memory leak
int all_calls_size = 0;

static char sproto_tag[SUPLA_TAG_SIZE] = {'S', 'U', 'P', 'L', 'A'};

class SrpcTest : public ::testing::Test {
 protected:
  _supla_int_t data_read_result;
  _supla_int_t data_write_result;
  unsigned char remote_version;

  void *srpc;
  char *data_read;
  char *data_write;
  _supla_int_t data_write_size;
  void *srpcInit(void);
  void srpcCallAllowed(int min_version, int *calls);

  unsigned _supla_int_t cr_rr_id;
  unsigned _supla_int_t cr_call_type;
  unsigned char cr_proto_version;
  TsrpcReceivedData cr_rd;

 public:
  virtual void SetUp();
  virtual void TearDown();
  _supla_int_t DataRead(void *buf, _supla_int_t count);
  _supla_int_t DataWrite(void *buf, _supla_int_t count);
  void SendAndReceive(int ExpectedCallType, int ExpectedSize);
  void OnVersionError(unsigned char remote_version);
  void OnRemoteCallReceived(unsigned _supla_int_t rr_id,
                            unsigned _supla_int_t call_type,
                            unsigned char proto_version);
};

_supla_int_t srpc_data_read(void *buf, _supla_int_t count, void *user_params) {
  return static_cast<SrpcTest *>(user_params)->DataRead(buf, count);
}

_supla_int_t srpc_data_write(void *buf, _supla_int_t count, void *user_params) {
  return static_cast<SrpcTest *>(user_params)->DataWrite(buf, count);
}

void srpc_event_OnVersionError(void *_srpc, unsigned char remote_version,
                               void *user_params) {
  return static_cast<SrpcTest *>(user_params)->OnVersionError(remote_version);
}

void srpc_on_remote_call_received(void *_srpc, unsigned _supla_int_t rr_id,
                                  unsigned _supla_int_t call_type,
                                  void *user_params,
                                  unsigned char proto_version) {
  return static_cast<SrpcTest *>(user_params)
      ->OnRemoteCallReceived(rr_id, call_type, proto_version);
}

void SrpcTest::SetUp() {
  data_read = NULL;
  data_write = NULL;
  data_write_size = 0;
  srpc = NULL;
  remote_version = 0;
  data_read_result = 0;
  data_write_result = 0;
  cr_rr_id = 0;
  cr_call_type = 0;
  cr_proto_version = 0;
  memset(&cr_rd, 0, sizeof(TsrpcReceivedData));
}

void SrpcTest::TearDown() {
  if (data_read != NULL) {
    free(data_read);
    data_read = NULL;
  }

  if (data_write != NULL) {
    free(data_write);
    data_write = NULL;
  }

  if (srpc != NULL) {
    srpc_free(srpc);
    srpc = NULL;
  }
}

void *SrpcTest::srpcInit(void) {
  TsrpcParams params;
  srpc_params_init(&params);
  params.user_params = this;
  params.data_read = &srpc_data_read;
  params.data_write = &srpc_data_write;
  params.on_version_error = &srpc_event_OnVersionError;
  params.on_remote_call_received = &srpc_on_remote_call_received;

  return srpc_init(&params);
}

TEST_F(SrpcTest, init) {
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);
  ASSERT_EQ(SUPLA_PROTO_VERSION, srpc_get_proto_version(srpc));
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, set_proto) {
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);
  ASSERT_EQ(SUPLA_PROTO_VERSION, srpc_get_proto_version(srpc));
  srpc_set_proto_version(srpc, SUPLA_PROTO_VERSION_MIN);
  ASSERT_EQ(SUPLA_PROTO_VERSION_MIN, srpc_get_proto_version(srpc));
  srpc_free(srpc);
  srpc = NULL;
}

void SrpcTest::srpcCallAllowed(int min_version, int *calls) {
  srpc = srpcInit();
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
  srpc = NULL;
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

  srpc = srpcInit();
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

  free(all_calls);
  all_calls = NULL;

  srpc_free(srpc);
  srpc = NULL;
}

_supla_int_t SrpcTest::DataRead(void *buf, _supla_int_t count) {
  if (data_read_result > 0 && data_read != NULL) {
    int size = data_read_result > count ? count : data_read_result;
    memcpy(buf, data_read, size);
    return size;
  }
  return data_read_result;
}

_supla_int_t SrpcTest::DataWrite(void *buf, _supla_int_t count) {
  if (count > 0) {
    data_write = (char *)realloc(data_write, count);
    if (data_write == NULL) {
      data_write_size = 0;
    } else {
      memcpy(data_write, buf, count);
      data_write_size = count;
    }
  }
  return data_write_result == 0 ? count : data_write_result;
}

void SrpcTest::OnVersionError(unsigned char remote_version) {
  this->remote_version = remote_version;
}

void SrpcTest::OnRemoteCallReceived(unsigned _supla_int_t rr_id,
                                    unsigned _supla_int_t call_type,
                                    unsigned char proto_version) {
  cr_rr_id = rr_id;
  cr_call_type = call_type;
  cr_proto_version = proto_version;
}

TEST_F(SrpcTest, iterate_read_error_when_zero) {
  data_read_result = 0;
  data_write_result = 0;

  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_EQ(SUPLA_RESULT_FALSE, srpc_iterate(srpc));

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, iterate_incorrect_data) {
  data_read_result = sizeof(TSuplaDataPacket) + sizeof(sproto_tag);
  data_write_result = 0;

  data_read = (char *)malloc(data_read_result);
  memset(data_read, 0, data_read_result);

  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_EQ(0, remote_version);
  ASSERT_EQ(SUPLA_RESULT_FALSE, srpc_iterate(srpc));
  ASSERT_EQ(0, remote_version);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, iterate_incorrect_version) {
  data_read_result = sizeof(TSuplaDataPacket) + sizeof(sproto_tag);
  data_write_result = 0;

  data_read = (char *)malloc(data_read_result);
  memset(data_read, 0, data_read_result);
  ((TSuplaDataPacket *)data_read)->version = SUPLA_PROTO_VERSION + 1;
  ((TSuplaDataPacket *)data_read)->data_size = SUPLA_MAX_DATA_SIZE;

  memcpy(((TSuplaDataPacket *)data_read)->tag, sproto_tag, SUPLA_TAG_SIZE);
  memcpy(&data_read[sizeof(TSuplaDataPacket)], sproto_tag, SUPLA_TAG_SIZE);

  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_EQ(0, remote_version);
  ASSERT_EQ(SUPLA_RESULT_FALSE, srpc_iterate(srpc));
  ASSERT_EQ(SUPLA_PROTO_VERSION + 1, remote_version);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, iterate_input_queue_size) {
  data_read_result = sizeof(TSuplaDataPacket) + sizeof(sproto_tag);
  data_write_result = 0;

  data_read = (char *)malloc(data_read_result);
  memset(data_read, 0, data_read_result);
  ((TSuplaDataPacket *)data_read)->version = SUPLA_PROTO_VERSION;
  ((TSuplaDataPacket *)data_read)->data_size = SUPLA_MAX_DATA_SIZE;

  memcpy(((TSuplaDataPacket *)data_read)->tag, sproto_tag, SUPLA_TAG_SIZE);
  memcpy(&data_read[sizeof(TSuplaDataPacket)], sproto_tag, SUPLA_TAG_SIZE);

  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  for (int a = 0; a < SRPC_QUEUE_SIZE; a++) {
    ASSERT_EQ(SUPLA_RESULT_TRUE, srpc_iterate(srpc));
  }

  ASSERT_EQ(SUPLA_RESULT_FALSE, srpc_iterate(srpc));

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, iterate_buffer_overflow) {
  data_read_result = sizeof(TSuplaDataPacket) + sizeof(sproto_tag);
  data_write_result = 0;

  data_read = (char *)malloc(data_read_result);
  memset(data_read, 0, data_read_result);
  ((TSuplaDataPacket *)data_read)->version = SUPLA_PROTO_VERSION;
  ((TSuplaDataPacket *)data_read)->data_size = SUPLA_MAX_DATA_SIZE;

  memcpy(((TSuplaDataPacket *)data_read)->tag, sproto_tag, SUPLA_TAG_SIZE);
  memcpy(&data_read[sizeof(TSuplaDataPacket)], sproto_tag, SUPLA_TAG_SIZE);
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_EQ(SUPLA_RESULT_TRUE, srpc_iterate(srpc));

  data_read_result = BUFFER_MAX_SIZE;
  free(data_read);
  data_read = NULL;

  ASSERT_EQ(SUPLA_RESULT_FALSE, srpc_iterate(srpc));

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, iterate_no_data) {
  data_read_result = sizeof(TSuplaDataPacket) + sizeof(sproto_tag);
  data_write_result = 0;

  data_read = (char *)malloc(data_read_result);
  memset(data_read, 0, data_read_result);
  ((TSuplaDataPacket *)data_read)->version = SUPLA_PROTO_VERSION;
  ((TSuplaDataPacket *)data_read)->data_size = 0;
  memcpy(((TSuplaDataPacket *)data_read)->tag, sproto_tag, SUPLA_TAG_SIZE);
  memcpy(&data_read[sizeof(TSuplaDataPacket) - SUPLA_MAX_DATA_SIZE], sproto_tag,
         SUPLA_TAG_SIZE);

  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_EQ(SUPLA_RESULT_TRUE, srpc_iterate(srpc));

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, iterate_out_queue_pop) {
  data_read_result = -1;
  data_write_result = 0;

  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_GT(srpc_dcs_async_getversion(srpc), 0);
  ASSERT_EQ(SUPLA_RESULT_TRUE, srpc_iterate(srpc));
  ASSERT_FALSE(data_write == NULL);
  ASSERT_EQ(23, data_write_size);

  srpc_free(srpc);
  srpc = NULL;
}

void SrpcTest::SendAndReceive(int ExpectedCallType, int ExpectedSize) {
  ASSERT_EQ(SUPLA_RESULT_TRUE, srpc_iterate(srpc));
  ASSERT_FALSE(data_write == NULL);
  ASSERT_EQ(ExpectedSize, data_write_size);
  ASSERT_TRUE(data_read == NULL);
  data_read = (char *)malloc(data_write_size);
  ASSERT_FALSE(data_read == NULL);
  data_read_result = data_write_size;

  memcpy(data_read, data_write, data_write_size);

  data_write_size = 0;
  free(data_write);
  data_write = NULL;

  ASSERT_EQ(SUPLA_RESULT_TRUE, srpc_iterate(srpc));

  ASSERT_EQ(1, cr_rr_id);
  ASSERT_EQ(ExpectedCallType, cr_call_type);
  ASSERT_EQ(SUPLA_PROTO_VERSION, cr_proto_version);

  ASSERT_EQ(SUPLA_RESULT_TRUE, srpc_getdata(srpc, &cr_rd, cr_rr_id));
  ASSERT_EQ(ExpectedCallType, cr_rd.call_type);
  ASSERT_EQ(cr_rr_id, cr_rd.rr_id);
}

TEST_F(SrpcTest, call_getversion) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_GT(srpc_dcs_async_getversion(srpc), 0);

  SendAndReceive(SUPLA_DCS_CALL_GETVERSION, 23);

  // No Data
  ASSERT_TRUE(cr_rd.data.dcs_ping == NULL);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_getversion_reasult) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  char SoftVer[SUPLA_SOFTVER_MAXSIZE];
  memset(SoftVer, 0, SUPLA_SOFTVER_MAXSIZE);
  snprintf(SoftVer, SUPLA_SOFTVER_MAXSIZE, "UTEST");

  ASSERT_GT(srpc_sdc_async_getversion_result(srpc, SoftVer), 0);

  SendAndReceive(SUPLA_SDC_CALL_GETVERSION_RESULT, 46);

  ASSERT_FALSE(cr_rd.data.sdc_getversion_result == NULL);

  ASSERT_EQ(SUPLA_PROTO_VERSION,
            cr_rd.data.sdc_getversion_result->proto_version);
  ASSERT_EQ(SUPLA_PROTO_VERSION_MIN,
            cr_rd.data.sdc_getversion_result->proto_version_min);
  ASSERT_EQ(0, memcmp(SoftVer, cr_rd.data.sdc_getversion_result->SoftVer,
                      SUPLA_SOFTVER_MAXSIZE));

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_ping_server) {
  struct timeval now;
  gettimeofday(&now, NULL);

  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_GT(srpc_dcs_async_ping_server(srpc), 0);

  SendAndReceive(SUPLA_DCS_CALL_PING_SERVER, 39);

  ASSERT_FALSE(cr_rd.data.dcs_ping == NULL);

  ASSERT_GE(now.tv_sec, cr_rd.data.dcs_ping->now.tv_sec);
  ASSERT_TRUE(cr_rd.data.dcs_ping->now.tv_sec > now.tv_sec ||
              cr_rd.data.dcs_ping->now.tv_usec > now.tv_usec);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_ping_server_result) {
  struct timeval now;
  gettimeofday(&now, NULL);

  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_GT(srpc_sdc_async_ping_server_result(srpc), 0);

  SendAndReceive(SUPLA_SDC_CALL_PING_SERVER_RESULT, 39);

  ASSERT_FALSE(cr_rd.data.sdc_ping_result == NULL);

  ASSERT_GE(now.tv_sec, cr_rd.data.sdc_ping_result->now.tv_sec);
  ASSERT_TRUE(cr_rd.data.sdc_ping_result->now.tv_sec > now.tv_sec ||
              cr_rd.data.sdc_ping_result->now.tv_usec > now.tv_usec);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_set_activity_timeout) {
  struct timeval now;
  gettimeofday(&now, NULL);

  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  TDCS_SuplaSetActivityTimeout timeout;
  timeout.activity_timeout = 123;

  ASSERT_GT(srpc_dcs_async_set_activity_timeout(srpc, &timeout), 0);

  SendAndReceive(SUPLA_DCS_CALL_SET_ACTIVITY_TIMEOUT, 24);

  ASSERT_FALSE(cr_rd.data.dcs_set_activity_timeout == NULL);
  ASSERT_EQ(123, cr_rd.data.dcs_set_activity_timeout->activity_timeout);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_set_activity_timeout_result) {
  struct timeval now;
  gettimeofday(&now, NULL);

  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  TSDC_SuplaSetActivityTimeoutResult result;
  result.min = 5;
  result.activity_timeout = 10;
  result.max = 20;

  ASSERT_GT(srpc_dcs_async_set_activity_timeout_result(srpc, &result), 0);

  SendAndReceive(SUPLA_SDC_CALL_SET_ACTIVITY_TIMEOUT_RESULT, 26);

  ASSERT_FALSE(cr_rd.data.sdc_set_activity_timeout_result == NULL);
  ASSERT_EQ(5, cr_rd.data.sdc_set_activity_timeout_result->min);
  ASSERT_EQ(10, cr_rd.data.sdc_set_activity_timeout_result->activity_timeout);
  ASSERT_EQ(20, cr_rd.data.sdc_set_activity_timeout_result->max);

  srpc_free(srpc);
  srpc = NULL;
}

}  // namespace
