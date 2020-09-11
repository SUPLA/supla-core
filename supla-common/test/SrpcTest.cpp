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
#define DECLARE_WITH_RANDOM(TYPE, VAR) \
  TYPE VAR;                            \
  set_random(&VAR, sizeof(TYPE));

#define SRPC_CALL_WITH_NO_DATA(spc_function, call_id) \
  TEST_F(SrpcTest, spc_function) {                    \
    data_read_result = -1;                            \
    srpc = srpcInit();                                \
    ASSERT_FALSE(srpc == NULL);                       \
    ASSERT_GT(spc_function(srpc), 0);                 \
    SendAndReceive(call_id, 23);                      \
    ASSERT_TRUE(cr_rd.data.dcs_ping == NULL);         \
    srpc_free(srpc);                                  \
    srpc = NULL;                                      \
  }

#define SRPC_CALL_BASIC_TEST(spc_function, structure_name, call_id, \
                             expected_data_size, rd_data_variable)  \
  TEST_F(SrpcTest, spc_function) {                                  \
    data_read_result = -1;                                          \
    srpc = srpcInit();                                              \
    ASSERT_FALSE(srpc == NULL);                                     \
    DECLARE_WITH_RANDOM(structure_name, param);                     \
    ASSERT_GT(spc_function(srpc, &param), 0);                       \
    SendAndReceive(call_id, expected_data_size);                    \
    ASSERT_FALSE(cr_rd.data.rd_data_variable == NULL);              \
    ASSERT_EQ(0, memcmp(cr_rd.data.rd_data_variable, &param,        \
                        sizeof(structure_name)));                   \
    free(cr_rd.data.rd_data_variable);                              \
    srpc_free(srpc);                                                \
    srpc = NULL;                                                    \
  }

#define SRPC_CALL_BASIC_TEST_WITH_SIZE_PARAM(                                \
    spc_function, structure_name, call_id, expected_data_size1,              \
    expected_data_size2, rd_data_variable, max_size, subparam_name,          \
    size_subparam_name)                                                      \
  TEST_F(SrpcTest, spc_function##_with_over_size) {                          \
    data_read_result = -1;                                                   \
    srpc = srpcInit();                                                       \
    ASSERT_FALSE(expected_data_size1 == expected_data_size2);                \
    ASSERT_FALSE(srpc == NULL);                                              \
    DECLARE_WITH_RANDOM(structure_name, param);                              \
    param.size_subparam_name = max_size + 1;                                 \
    ASSERT_EQ(spc_function(srpc, &param), 0);                                \
    srpc_free(srpc);                                                         \
    srpc = NULL;                                                             \
  }                                                                          \
                                                                             \
  TEST_F(SrpcTest, spc_function##_with_minimum_size) {                       \
    data_read_result = -1;                                                   \
    srpc = srpcInit();                                                       \
    ASSERT_FALSE(srpc == NULL);                                              \
    DECLARE_WITH_RANDOM(structure_name, param);                              \
    param.size_subparam_name = 0;                                            \
    ASSERT_GT(spc_function(srpc, &param), 0);                                \
    SendAndReceive(call_id, expected_data_size1);                            \
    ASSERT_FALSE(cr_rd.data.rd_data_variable == NULL);                       \
    ASSERT_EQ(0,                                                             \
              memcmp(cr_rd.data.rd_data_variable, &param,                    \
                     sizeof(structure_name) - sizeof(param.subparam_name))); \
    free(cr_rd.data.rd_data_variable);                                       \
    srpc_free(srpc);                                                         \
    srpc = NULL;                                                             \
  }                                                                          \
                                                                             \
  TEST_F(SrpcTest, spc_function##_with_full_size) {                          \
    data_read_result = -1;                                                   \
    srpc = srpcInit();                                                       \
    ASSERT_FALSE(srpc == NULL);                                              \
    DECLARE_WITH_RANDOM(structure_name, param);                              \
    param.size_subparam_name = max_size;                                     \
    ASSERT_GT(spc_function(srpc, &param), 0);                                \
    SendAndReceive(call_id, expected_data_size2);                            \
    ASSERT_FALSE(cr_rd.data.rd_data_variable == NULL);                       \
    ASSERT_EQ(0, memcmp(cr_rd.data.rd_data_variable, &param,                 \
                        sizeof(structure_name)));                            \
    free(cr_rd.data.rd_data_variable);                                       \
    srpc_free(srpc);                                                         \
    srpc = NULL;                                                             \
  }

#define SRPC_CALL_BASIC_TEST_WITH_CAPTION(                        \
    spc_function, structure_name, call_id, expected_data_size1,   \
    expected_data_size2, rd_data_variable, max_size)              \
  SRPC_CALL_BASIC_TEST_WITH_SIZE_PARAM(                           \
      spc_function, structure_name, call_id, expected_data_size1, \
      expected_data_size2, rd_data_variable, max_size, Caption, CaptionSize)

int *all_calls = NULL;  // Possible memory leak
int all_calls_size = 0;

static char sproto_tag[SUPLA_TAG_SIZE] = {'S', 'U', 'P', 'L', 'A'};

class SrpcTest : public ::testing::Test {
 protected:
  _supla_int_t data_read_result;
  _supla_int_t data_write_result;
  unsigned char remote_version;

  unsigned int seed;
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

  void set_random(void *ptr, unsigned int size);

 public:
  virtual void SetUp();
  virtual void TearDown();
  _supla_int_t DataRead(void *buf, _supla_int_t count);
  _supla_int_t DataWrite(void *buf, _supla_int_t count);
  void SendAndReceive(unsigned int ExpectedCallType, int ExpectedSize);
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

  struct timeval tv;
  gettimeofday(&tv, NULL);
  seed = tv.tv_sec + tv.tv_usec;
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

void SrpcTest::set_random(void *ptr, unsigned int size) {
  for (unsigned int a = 0; a < size; a++) {
    ((char *)ptr)[a] = rand_r(&seed);
  }
}

//---------------------------------------------------------
// CALL ALLOWED
//---------------------------------------------------------
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
                 SUPLA_CS_CALL_SUPERUSER_AUTHORIZATION_REQUEST,
                 SUPLA_SC_CALL_SUPERUSER_AUTHORIZATION_RESULT,
                 SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST,
                 SUPLA_SC_CALL_DEVICE_CALCFG_RESULT,
                 SUPLA_SD_CALL_DEVICE_CALCFG_REQUEST,
                 SUPLA_DS_CALL_DEVICE_CALCFG_RESULT,
                 SUPLA_SC_CALL_CHANNELPACK_UPDATE_C,
                 SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE_B,
                 SUPLA_SC_CALL_CHANNEL_UPDATE_C,
                 0};

  srpcCallAllowed(10, calls);
}

TEST_F(SrpcTest, call_allowed_v11) {
  int calls[] = {SUPLA_DCS_CALL_GET_USER_LOCALTIME,
                 SUPLA_DCS_CALL_GET_USER_LOCALTIME_RESULT,
                 SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST_B, 0};

  srpcCallAllowed(11, calls);
}

TEST_F(SrpcTest, call_allowed_v12) {
  int calls[] = {SUPLA_CS_CALL_REGISTER_CLIENT_D,
                 SUPLA_CSD_CALL_GET_CHANNEL_STATE,
                 SUPLA_DSC_CALL_CHANNEL_STATE_RESULT,
                 SUPLA_CS_CALL_GET_CHANNEL_BASIC_CFG,
                 SUPLA_SC_CALL_CHANNEL_BASIC_CFG_RESULT,
                 SUPLA_CS_CALL_SET_CHANNEL_FUNCTION,
                 SUPLA_SC_CALL_SET_CHANNEL_FUNCTION_RESULT,
                 SUPLA_CS_CALL_SET_CHANNEL_CAPTION,
                 SUPLA_SC_CALL_SET_CHANNEL_CAPTION_RESULT,
                 SUPLA_CS_CALL_CLIENTS_RECONNECT_REQUEST,
                 SUPLA_SC_CALL_CLIENTS_RECONNECT_REQUEST_RESULT,
                 SUPLA_CS_CALL_SET_REGISTRATION_ENABLED,
                 SUPLA_SC_CALL_SET_REGISTRATION_ENABLED_RESULT,
                 SUPLA_CS_CALL_DEVICE_RECONNECT_REQUEST,
                 SUPLA_SC_CALL_DEVICE_RECONNECT_REQUEST_RESULT,
                 SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_B,
                 SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_C,
                 SUPLA_DS_CALL_GET_CHANNEL_FUNCTIONS,
                 SUPLA_SD_CALL_GET_CHANNEL_FUNCTIONS_RESULT,
                 SUPLA_CS_CALL_GET_SUPERUSER_AUTHORIZATION_RESULT,
                 0};

  srpcCallAllowed(12, calls);
}

TEST_F(SrpcTest, call_allowed_v13) {
  int calls[] = {SUPLA_SD_CALL_CHANNELGROUP_SET_VALUE, 0};
  srpcCallAllowed(13, calls);
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

//---------------------------------------------------------
// ITERATE
//---------------------------------------------------------

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

void SrpcTest::SendAndReceive(unsigned int ExpectedCallType, int ExpectedSize) {
  ASSERT_EQ(SUPLA_RESULT_TRUE, srpc_iterate(srpc));
  ASSERT_FALSE(data_write == NULL);
  ASSERT_EQ(ExpectedSize, data_write_size);
  ASSERT_TRUE(data_read == NULL);
  data_read = (char *)malloc(data_write_size);
  ASSERT_FALSE(data_read == NULL);
  data_read_result = data_write_size;

  if (ExpectedCallType == (unsigned int)-1) {
    ((TSuplaDataPacket *)data_write)->call_type = (unsigned int)-1;
  }

  memcpy(data_read, data_write, data_write_size);

  data_write_size = 0;
  free(data_write);
  data_write = NULL;

  ASSERT_EQ(SUPLA_RESULT_TRUE, srpc_iterate(srpc));

  ASSERT_EQ((unsigned int)1, (unsigned int)cr_rr_id);
  ASSERT_TRUE(ExpectedCallType == cr_call_type);
  ASSERT_EQ(SUPLA_PROTO_VERSION, cr_proto_version);

  if (ExpectedCallType == (unsigned int)-1) {
    ASSERT_EQ(SUPLA_RESULT_DATA_ERROR, srpc_getdata(srpc, &cr_rd, cr_rr_id));
  } else {
    ASSERT_EQ(SUPLA_RESULT_TRUE, srpc_getdata(srpc, &cr_rd, cr_rr_id));
  }

  ASSERT_EQ(ExpectedCallType, cr_rd.call_type);
  ASSERT_EQ(cr_rr_id, cr_rd.rr_id);
}

//---------------------------------------------------------
// NO DATA
//---------------------------------------------------------

TEST_F(SrpcTest, getdata_when_no_data) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_EQ(SUPLA_RESULT_FALSE, srpc_getdata(srpc, &cr_rd, cr_rr_id));

  // No Data
  ASSERT_TRUE(cr_rd.data.dcs_ping == NULL);

  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// DATA_ERROR
//---------------------------------------------------------

TEST_F(SrpcTest, call_with_dataerror) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_GT(srpc_dcs_async_getversion(srpc), 0);

  SendAndReceive(-1, 23);

  // No Data
  ASSERT_TRUE(cr_rd.data.dcs_ping == NULL);

  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// GET VERSION
//---------------------------------------------------------

SRPC_CALL_WITH_NO_DATA(srpc_dcs_async_getversion, SUPLA_DCS_CALL_GETVERSION);

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

  free(cr_rd.data.sdc_getversion_result);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// PING
//---------------------------------------------------------

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
  ASSERT_TRUE(cr_rd.data.dcs_ping->now.tv_sec >= now.tv_sec);

  free(cr_rd.data.dcs_ping);
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
  ASSERT_TRUE(cr_rd.data.sdc_ping_result->now.tv_sec >= now.tv_sec);

  free(cr_rd.data.sdc_ping_result);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// ACTIVITY TIMEOUT
//---------------------------------------------------------

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

  free(cr_rd.data.dcs_set_activity_timeout);
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

  free(cr_rd.data.sdc_set_activity_timeout_result);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// IS REGISTRATION ENABLED
//---------------------------------------------------------

SRPC_CALL_WITH_NO_DATA(srpc_dcs_async_get_registration_enabled,
                       SUPLA_DCS_CALL_GET_REGISTRATION_ENABLED);

TEST_F(SrpcTest, call_get_registration_enabled_result) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  TSDC_RegistrationEnabled result;
  result.client_timestamp = 10;
  result.iodevice_timestamp = 20;

  ASSERT_GT(srpc_sdc_async_get_registration_enabled_result(srpc, &result), 0);
  SendAndReceive(SUPLA_SDC_CALL_GET_REGISTRATION_ENABLED_RESULT, 31);

  ASSERT_FALSE(cr_rd.data.sdc_reg_enabled == NULL);
  ASSERT_EQ(cr_rd.data.sdc_reg_enabled->client_timestamp,
            result.client_timestamp);
  ASSERT_EQ(cr_rd.data.sdc_reg_enabled->iodevice_timestamp,
            result.iodevice_timestamp);

  free(cr_rd.data.sdc_reg_enabled);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// DEVICE REGISTRATION
//---------------------------------------------------------

TEST_F(SrpcTest, call_registerdevice) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  int a;
  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice, registerdevice);

  for (a = 0; a < SUPLA_CHANNELMAXCOUNT; a++) {
    registerdevice.channels[a].Number = a;
    registerdevice.channels[a].Type = a;
  }

  registerdevice.channel_count = SUPLA_CHANNELMAXCOUNT;

  ASSERT_GT(srpc_ds_async_registerdevice(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE, 1963);

  ASSERT_FALSE(cr_rd.data.ds_register_device == NULL);
  ASSERT_EQ(memcmp(cr_rd.data.ds_register_device, &registerdevice,
                   sizeof(TDS_SuplaRegisterDevice)),
            0);

  free(cr_rd.data.ds_register_device);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_one_channel) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice, registerdevice);

  registerdevice.LocationID = 1;

  registerdevice.channels[0].Number = 1;
  registerdevice.channels[0].Type = 1;

  registerdevice.channel_count = 1;

  ASSERT_GT(srpc_ds_async_registerdevice(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE, 312);

  ASSERT_FALSE(cr_rd.data.ds_register_device == NULL);
  ASSERT_EQ(
      memcmp(cr_rd.data.ds_register_device, &registerdevice,
             sizeof(TDS_SuplaRegisterDevice) - (sizeof(TDS_SuplaDeviceChannel) *
                                                (SUPLA_CHANNELMAXCOUNT - 1))),
      0);

  free(cr_rd.data.ds_register_device);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_without_channels) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice, registerdevice);

  registerdevice.LocationID = 1;
  registerdevice.channel_count = 0;

  ASSERT_GT(srpc_ds_async_registerdevice(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE, 299);

  ASSERT_FALSE(cr_rd.data.ds_register_device == NULL);
  ASSERT_EQ(
      memcmp(cr_rd.data.ds_register_device, &registerdevice,
             sizeof(TDS_SuplaRegisterDevice) -
                 (sizeof(TDS_SuplaDeviceChannel) * (SUPLA_CHANNELMAXCOUNT))),
      0);

  free(cr_rd.data.ds_register_device);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_wrong_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice, registerdevice);

  registerdevice.channel_count = SUPLA_CHANNELMAXCOUNT + 1;

  ASSERT_EQ(srpc_ds_async_registerdevice(srpc, &registerdevice), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_b) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_B, registerdevice);

  registerdevice.channel_count = SUPLA_CHANNELMAXCOUNT;

  ASSERT_GT(srpc_ds_async_registerdevice_b(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE_B, 2987);

  ASSERT_FALSE(cr_rd.data.ds_register_device_b == NULL);
  ASSERT_EQ(memcmp(cr_rd.data.ds_register_device_b, &registerdevice,
                   sizeof(TDS_SuplaRegisterDevice)),
            0);

  free(cr_rd.data.ds_register_device_b);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_b_one_channel) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_B, registerdevice);

  registerdevice.LocationID = 1;
  registerdevice.channels[0].Number = 1;
  registerdevice.channels[0].Type = 1;
  registerdevice.channel_count = 1;

  ASSERT_GT(srpc_ds_async_registerdevice_b(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE_B, 320);

  ASSERT_FALSE(cr_rd.data.ds_register_device_b == NULL);
  ASSERT_EQ(memcmp(cr_rd.data.ds_register_device_b, &registerdevice,
                   sizeof(TDS_SuplaRegisterDevice_B) -
                       (sizeof(TDS_SuplaDeviceChannel_B) *
                        (SUPLA_CHANNELMAXCOUNT - 1))),
            0);

  free(cr_rd.data.ds_register_device_b);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_b_without_channels) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_B, registerdevice);

  registerdevice.LocationID = 1;
  registerdevice.channel_count = 0;

  ASSERT_GT(srpc_ds_async_registerdevice_b(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE_B, 299);

  ASSERT_FALSE(cr_rd.data.ds_register_device_b == NULL);
  ASSERT_EQ(
      memcmp(cr_rd.data.ds_register_device_b, &registerdevice,
             sizeof(TDS_SuplaRegisterDevice_B) -
                 (sizeof(TDS_SuplaDeviceChannel_B) * (SUPLA_CHANNELMAXCOUNT))),
      0);

  free(cr_rd.data.ds_register_device_b);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_b_wrong_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_B, registerdevice);

  registerdevice.channel_count = SUPLA_CHANNELMAXCOUNT + 1;

  ASSERT_EQ(srpc_ds_async_registerdevice_b(srpc, &registerdevice), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_c) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_C, registerdevice);
  registerdevice.LocationID = 1;

  registerdevice.channel_count = SUPLA_CHANNELMAXCOUNT;

  ASSERT_GT(srpc_ds_async_registerdevice_c(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE_C, 3052);

  ASSERT_FALSE(cr_rd.data.ds_register_device_c == NULL);
  ASSERT_EQ(memcmp(cr_rd.data.ds_register_device_c, &registerdevice,
                   sizeof(TDS_SuplaRegisterDevice_C)),
            0);

  free(cr_rd.data.ds_register_device_c);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_c_one_channel) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_C, registerdevice);

  registerdevice.LocationID = 1;
  registerdevice.channels[0].Number = 1;
  registerdevice.channels[0].Type = 1;
  registerdevice.channel_count = 1;

  ASSERT_GT(srpc_ds_async_registerdevice_c(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE_C, 385);

  ASSERT_FALSE(cr_rd.data.ds_register_device_c == NULL);
  ASSERT_EQ(memcmp(cr_rd.data.ds_register_device_c, &registerdevice,
                   sizeof(TDS_SuplaRegisterDevice_C) -
                       (sizeof(TDS_SuplaDeviceChannel_B) *
                        (SUPLA_CHANNELMAXCOUNT - 1))),
            0);

  free(cr_rd.data.ds_register_device_c);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_c_without_channels) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_C, registerdevice);

  registerdevice.LocationID = 1;
  registerdevice.channel_count = 0;

  ASSERT_GT(srpc_ds_async_registerdevice_c(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE_C, 364);

  ASSERT_FALSE(cr_rd.data.ds_register_device_c == NULL);
  ASSERT_EQ(
      memcmp(cr_rd.data.ds_register_device_c, &registerdevice,
             sizeof(TDS_SuplaRegisterDevice_C) -
                 (sizeof(TDS_SuplaDeviceChannel_B) * (SUPLA_CHANNELMAXCOUNT))),
      0);

  free(cr_rd.data.ds_register_device_c);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_c_wrong_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_C, registerdevice);

  registerdevice.channel_count = SUPLA_CHANNELMAXCOUNT + 1;

  ASSERT_EQ(srpc_ds_async_registerdevice_c(srpc, &registerdevice), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_d) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_D, registerdevice);

  registerdevice.channel_count = SUPLA_CHANNELMAXCOUNT;

  ASSERT_GT(srpc_ds_async_registerdevice_d(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE_D, 3287);

  ASSERT_FALSE(cr_rd.data.ds_register_device_d == NULL);
  ASSERT_EQ(memcmp(cr_rd.data.ds_register_device_d, &registerdevice,
                   sizeof(TDS_SuplaRegisterDevice_D)),
            0);

  free(cr_rd.data.ds_register_device_d);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_d_one_channel) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_D, registerdevice);

  registerdevice.channels[0].Number = 1;
  registerdevice.channels[0].Type = 1;
  registerdevice.channel_count = 1;

  ASSERT_GT(srpc_ds_async_registerdevice_d(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE_D, 620);

  ASSERT_FALSE(cr_rd.data.ds_register_device_d == NULL);
  ASSERT_EQ(memcmp(cr_rd.data.ds_register_device_d, &registerdevice,
                   sizeof(TDS_SuplaRegisterDevice_D) -
                       (sizeof(TDS_SuplaDeviceChannel_B) *
                        (SUPLA_CHANNELMAXCOUNT - 1))),
            0);

  free(cr_rd.data.ds_register_device_d);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_d_without_channels) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_D, registerdevice);
  registerdevice.channel_count = 0;

  ASSERT_GT(srpc_ds_async_registerdevice_d(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE_D, 599);

  ASSERT_FALSE(cr_rd.data.ds_register_device_d == NULL);
  ASSERT_EQ(
      memcmp(cr_rd.data.ds_register_device_d, &registerdevice,
             sizeof(TDS_SuplaRegisterDevice_D) -
                 (sizeof(TDS_SuplaDeviceChannel_B) * (SUPLA_CHANNELMAXCOUNT))),
      0);

  free(cr_rd.data.ds_register_device_d);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_d_wrong_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_D, registerdevice);

  registerdevice.channel_count = SUPLA_CHANNELMAXCOUNT + 1;

  ASSERT_EQ(srpc_ds_async_registerdevice_d(srpc, &registerdevice), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_e) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_E, registerdevice);

  registerdevice.channel_count = SUPLA_CHANNELMAXCOUNT;

  ASSERT_GT(srpc_ds_async_registerdevice_e(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE_E, 3807);

  ASSERT_FALSE(cr_rd.data.ds_register_device_e == NULL);
  ASSERT_EQ(memcmp(cr_rd.data.ds_register_device_e, &registerdevice,
                   sizeof(TDS_SuplaRegisterDevice_E)),
            0);

  free(cr_rd.data.ds_register_device_e);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_e_one_channel) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_E, registerdevice);

  registerdevice.channels[0].Number = 1;
  registerdevice.channels[0].Type = 1;
  registerdevice.channel_count = 1;

  ASSERT_GT(srpc_ds_async_registerdevice_e(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE_E, 632);

  ASSERT_FALSE(cr_rd.data.ds_register_device_e == NULL);
  ASSERT_EQ(memcmp(cr_rd.data.ds_register_device_e, &registerdevice,
                   sizeof(TDS_SuplaRegisterDevice_E) -
                       (sizeof(TDS_SuplaDeviceChannel_C) *
                        (SUPLA_CHANNELMAXCOUNT - 1))),
            0);

  free(cr_rd.data.ds_register_device_e);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_e_without_channels) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_E, registerdevice);

  registerdevice.channel_count = 0;

  ASSERT_GT(srpc_ds_async_registerdevice_e(srpc, &registerdevice), 0);
  SendAndReceive(SUPLA_DS_CALL_REGISTER_DEVICE_E, 607);

  ASSERT_FALSE(cr_rd.data.ds_register_device_e == NULL);
  ASSERT_EQ(
      memcmp(cr_rd.data.ds_register_device_e, &registerdevice,
             sizeof(TDS_SuplaRegisterDevice_E) -
                 (sizeof(TDS_SuplaDeviceChannel_C) * (SUPLA_CHANNELMAXCOUNT))),
      0);

  free(cr_rd.data.ds_register_device_e);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_e_wrong_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_SuplaRegisterDevice_E, registerdevice);

  registerdevice.channel_count = SUPLA_CHANNELMAXCOUNT + 1;

  ASSERT_EQ(srpc_ds_async_registerdevice_e(srpc, &registerdevice), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerdevice_result) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSD_SuplaRegisterDeviceResult, result);

  result.result_code = 1;
  result.activity_timeout = 60;
  result.version = SUPLA_PROTO_VERSION;
  result.version_min = SUPLA_PROTO_VERSION_MIN;

  ASSERT_GT(srpc_sd_async_registerdevice_result(srpc, &result), 0);
  SendAndReceive(SUPLA_SD_CALL_REGISTER_DEVICE_RESULT, 30);

  ASSERT_FALSE(cr_rd.data.sd_register_device_result == NULL);

  ASSERT_EQ(1, cr_rd.data.sd_register_device_result->result_code);
  ASSERT_EQ(60, cr_rd.data.sd_register_device_result->activity_timeout);
  ASSERT_EQ(SUPLA_PROTO_VERSION, cr_rd.data.sd_register_device_result->version);
  ASSERT_EQ(SUPLA_PROTO_VERSION_MIN,
            cr_rd.data.sd_register_device_result->version_min);

  free(cr_rd.data.sd_register_device_result);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// DS CHANNEL VALUE CHANGED
//---------------------------------------------------------

TEST_F(SrpcTest, call_channel_value_changed) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  char value[SUPLA_CHANNELVALUE_SIZE];
  set_random(value, SUPLA_CHANNELVALUE_SIZE);

  ASSERT_GT(srpc_ds_async_channel_value_changed(srpc, 1, value), 0);
  SendAndReceive(SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED, 32);

  ASSERT_FALSE(cr_rd.data.ds_device_channel_value == NULL);

  ASSERT_EQ(cr_rd.data.ds_device_channel_value->ChannelNumber, 1);
  ASSERT_EQ(memcmp(cr_rd.data.ds_device_channel_value->value, value,
                   SUPLA_CHANNELVALUE_SIZE),
            0);

  free(cr_rd.data.ds_device_channel_value);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// DS CHANNEL VALUE CHANGED (B)
//---------------------------------------------------------

TEST_F(SrpcTest, call_channel_value_changed_b) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  char value[SUPLA_CHANNELVALUE_SIZE];
  set_random(value, SUPLA_CHANNELVALUE_SIZE);

  ASSERT_GT(srpc_ds_async_channel_value_changed_b(srpc, 1, value, 1), 0);
  SendAndReceive(SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_B, 33);

  ASSERT_FALSE(cr_rd.data.ds_device_channel_value_b == NULL);

  ASSERT_EQ(cr_rd.data.ds_device_channel_value_b->ChannelNumber, 1);
  ASSERT_EQ(cr_rd.data.ds_device_channel_value_b->Offline, 1);
  ASSERT_EQ(memcmp(cr_rd.data.ds_device_channel_value_b->value, value,
                   SUPLA_CHANNELVALUE_SIZE),
            0);

  free(cr_rd.data.ds_device_channel_value_b);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// DS CHANNEL VALUE CHANGED (C)
//---------------------------------------------------------

TEST_F(SrpcTest, call_channel_value_changed_c) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  char value[SUPLA_CHANNELVALUE_SIZE];
  set_random(value, SUPLA_CHANNELVALUE_SIZE);

  ASSERT_GT(srpc_ds_async_channel_value_changed_c(srpc, 2, value, 1, 777), 0);
  SendAndReceive(SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_C, 37);

  ASSERT_FALSE(cr_rd.data.ds_device_channel_value_c == NULL);

  ASSERT_EQ(cr_rd.data.ds_device_channel_value_c->ChannelNumber, 2);
  ASSERT_EQ(cr_rd.data.ds_device_channel_value_c->Offline, 1);
  ASSERT_EQ(cr_rd.data.ds_device_channel_value_c->ValidityTimeSec,
            (unsigned _supla_int_t)777);
  ASSERT_EQ(memcmp(cr_rd.data.ds_device_channel_value_c->value, value,
                   SUPLA_CHANNELVALUE_SIZE),
            0);

  free(cr_rd.data.ds_device_channel_value_c);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// DS CHANNEL EXTENDED-VALUE CHANGED
//---------------------------------------------------------

TEST_F(SrpcTest, call_channel_extendedvalue_changed_with_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSuplaChannelExtendedValue, ev);

  ASSERT_EQ(srpc_ds_async_channel_extendedvalue_changed(srpc, 1, &ev), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channel_extendedvalue_changed_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSuplaChannelExtendedValue, ev);

  ev.size = SUPLA_CHANNELEXTENDEDVALUE_SIZE + 1;

  ASSERT_EQ(srpc_ds_async_channel_extendedvalue_changed(srpc, 1, &ev), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channel_extendedvalue_changed_with_small_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSuplaChannelExtendedValue, ev);

  ev.size = 1;

  ASSERT_GT(srpc_ds_async_channel_extendedvalue_changed(srpc, 1, &ev), 0);
  SendAndReceive(SUPLA_DS_CALL_DEVICE_CHANNEL_EXTENDEDVALUE_CHANGED, 30);

  ASSERT_FALSE(cr_rd.data.ds_device_channel_extendedvalue == NULL);

  ASSERT_EQ(1, cr_rd.data.ds_device_channel_extendedvalue->ChannelNumber);
  ASSERT_EQ(0, memcmp(&cr_rd.data.ds_device_channel_extendedvalue->value, &ev,
                      sizeof(TSuplaChannelExtendedValue) -
                          SUPLA_CHANNELEXTENDEDVALUE_SIZE + 1));

  free(cr_rd.data.ds_device_channel_extendedvalue);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channel_extendedvalue_changed_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSuplaChannelExtendedValue, ev);

  ev.size = SUPLA_CHANNELEXTENDEDVALUE_SIZE;

  ASSERT_GT(srpc_ds_async_channel_extendedvalue_changed(srpc, 1, &ev), 0);
  SendAndReceive(SUPLA_DS_CALL_DEVICE_CHANNEL_EXTENDEDVALUE_CHANGED, 1053);

  ASSERT_FALSE(cr_rd.data.ds_device_channel_extendedvalue == NULL);

  ASSERT_EQ(1, cr_rd.data.ds_device_channel_extendedvalue->ChannelNumber);
  ASSERT_EQ(0, memcmp(&cr_rd.data.ds_device_channel_extendedvalue->value, &ev,
                      sizeof(TSuplaChannelExtendedValue)));

  free(cr_rd.data.ds_device_channel_extendedvalue);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// SET CHANNEL NEW VALUE
//---------------------------------------------------------

SRPC_CALL_BASIC_TEST(srpc_sd_async_set_channel_value, TSD_SuplaChannelNewValue,
                     SUPLA_SD_CALL_CHANNEL_SET_VALUE, 40, sd_channel_new_value);

TEST_F(SrpcTest, call_ds_set_channel_value_result) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSD_SuplaChannelNewValue, value);

  ASSERT_GT(srpc_ds_async_set_channel_result(srpc, 3, 2, 1), 0);
  SendAndReceive(SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT, 29);

  ASSERT_FALSE(cr_rd.data.ds_channel_new_value_result == NULL);

  ASSERT_EQ(3, cr_rd.data.ds_channel_new_value_result->ChannelNumber);
  ASSERT_EQ(2, cr_rd.data.ds_channel_new_value_result->SenderID);
  ASSERT_EQ(1, cr_rd.data.ds_channel_new_value_result->Success);

  free(cr_rd.data.ds_channel_new_value_result);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// SET CHANNEL NEW VALUE B
//---------------------------------------------------------

SRPC_CALL_BASIC_TEST(srpc_sd_async_set_channelgroup_value,
                     TSD_SuplaChannelGroupNewValue,
                     SUPLA_SD_CALL_CHANNELGROUP_SET_VALUE, 45,
                     sd_channelgroup_new_value);

//---------------------------------------------------------
// GET FIRMWARE UPDATE URL
//---------------------------------------------------------

TEST_F(SrpcTest, call_get_firmware_update_url) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_FirmwareUpdateParams, params);

  ASSERT_GT(srpc_sd_async_get_firmware_update_url(srpc, &params), 0);
  SendAndReceive(SUPLA_DS_CALL_GET_FIRMWARE_UPDATE_URL, 40);

  ASSERT_FALSE(cr_rd.data.ds_firmware_update_params == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.ds_firmware_update_params, &params,
                      sizeof(TDS_FirmwareUpdateParams)));

  free(cr_rd.data.ds_firmware_update_params);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_get_firmware_update_url_result_exists) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSD_FirmwareUpdate_UrlResult, result);

  result.exists = 1;

  ASSERT_GT(srpc_sd_async_get_firmware_update_url_result(srpc, &result), 0);
  SendAndReceive(SUPLA_SD_CALL_GET_FIRMWARE_UPDATE_URL_RESULT, 231);

  ASSERT_FALSE(cr_rd.data.sc_firmware_update_url_result == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_firmware_update_url_result, &result,
                      sizeof(TSD_FirmwareUpdate_UrlResult)));

  free(cr_rd.data.sc_firmware_update_url_result);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_get_firmware_update_url_result_not_exists) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSD_FirmwareUpdate_UrlResult, result);

  result.exists = 0;

  ASSERT_GT(srpc_sd_async_get_firmware_update_url_result(srpc, &result), 0);
  SendAndReceive(SUPLA_SD_CALL_GET_FIRMWARE_UPDATE_URL_RESULT, 24);

  ASSERT_FALSE(cr_rd.data.sc_firmware_update_url_result == NULL);

  ASSERT_EQ(0, cr_rd.data.sc_firmware_update_url_result->exists);

  free(cr_rd.data.sc_firmware_update_url_result);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// DEVICE CALIBRATION / CONFIG
//---------------------------------------------------------

TEST_F(SrpcTest, call_sd_device_calcfg_request_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSD_DeviceCalCfgRequest, request);

  request.DataSize = SUPLA_CALCFG_DATA_MAXSIZE + 1;

  ASSERT_EQ(srpc_sd_async_device_calcfg_request(srpc, &request), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_sd_device_calcfg_request_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSD_DeviceCalCfgRequest, request);

  request.DataSize = 0;

  ASSERT_GT(srpc_sd_async_device_calcfg_request(srpc, &request), 0);

  SendAndReceive(SUPLA_SD_CALL_DEVICE_CALCFG_REQUEST, 44);

  ASSERT_FALSE(cr_rd.data.sd_device_calcfg_request == NULL);

  ASSERT_EQ(
      0, memcmp(cr_rd.data.sd_device_calcfg_request, &request,
                sizeof(TSD_DeviceCalCfgRequest) - SUPLA_CALCFG_DATA_MAXSIZE));

  free(cr_rd.data.sd_device_calcfg_request);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_sd_device_calcfg_request_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSD_DeviceCalCfgRequest, request);

  request.DataSize = SUPLA_CALCFG_DATA_MAXSIZE;

  ASSERT_GT(srpc_sd_async_device_calcfg_request(srpc, &request), 0);

  SendAndReceive(SUPLA_SD_CALL_DEVICE_CALCFG_REQUEST, 172);

  ASSERT_FALSE(cr_rd.data.sd_device_calcfg_request == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sd_device_calcfg_request, &request,
                      sizeof(TSD_DeviceCalCfgRequest)));

  free(cr_rd.data.sd_device_calcfg_request);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_ds_device_calcfg_result_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_DeviceCalCfgResult, result);

  result.DataSize = SUPLA_CALCFG_DATA_MAXSIZE + 1;

  ASSERT_EQ(srpc_ds_async_device_calcfg_result(srpc, &result), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_ds_device_calcfg_result_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_DeviceCalCfgResult, result);

  result.DataSize = 0;

  ASSERT_GT(srpc_ds_async_device_calcfg_result(srpc, &result), 0);

  SendAndReceive(SUPLA_DS_CALL_DEVICE_CALCFG_RESULT, 43);

  ASSERT_FALSE(cr_rd.data.ds_device_calcfg_result == NULL);

  ASSERT_EQ(0,
            memcmp(cr_rd.data.ds_device_calcfg_result, &result,
                   sizeof(TDS_DeviceCalCfgResult) - SUPLA_CALCFG_DATA_MAXSIZE));

  free(cr_rd.data.ds_device_calcfg_result);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_ds_device_calcfg_result_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TDS_DeviceCalCfgResult, result);

  result.DataSize = SUPLA_CALCFG_DATA_MAXSIZE;

  ASSERT_GT(srpc_ds_async_device_calcfg_result(srpc, &result), 0);

  SendAndReceive(SUPLA_DS_CALL_DEVICE_CALCFG_RESULT, 171);

  ASSERT_FALSE(cr_rd.data.ds_device_calcfg_result == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.ds_device_calcfg_result, &result,
                      sizeof(TDS_DeviceCalCfgResult)));

  free(cr_rd.data.ds_device_calcfg_result);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// REGISTER CLIENT
//---------------------------------------------------------

TEST_F(SrpcTest, call_registerclient) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TCS_SuplaRegisterClient, registerclient);

  ASSERT_GT(srpc_cs_async_registerclient(srpc, &registerclient), 0);
  SendAndReceive(SUPLA_CS_CALL_REGISTER_CLIENT, 298);

  ASSERT_FALSE(cr_rd.data.cs_register_client == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.cs_register_client, &registerclient,
                      sizeof(TCS_SuplaRegisterClient)));

  free(cr_rd.data.cs_register_client);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerclient_b) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TCS_SuplaRegisterClient_B, registerclient_b);

  ASSERT_GT(srpc_cs_async_registerclient_b(srpc, &registerclient_b), 0);
  SendAndReceive(SUPLA_CS_CALL_REGISTER_CLIENT_B, 363);

  ASSERT_FALSE(cr_rd.data.cs_register_client_b == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.cs_register_client_b, &registerclient_b,
                      sizeof(TCS_SuplaRegisterClient_B)));

  free(cr_rd.data.cs_register_client_b);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerclient_c) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TCS_SuplaRegisterClient_C, registerclient_c);

  ASSERT_GT(srpc_cs_async_registerclient_c(srpc, &registerclient_c), 0);
  SendAndReceive(SUPLA_CS_CALL_REGISTER_CLIENT_C, 598);

  ASSERT_FALSE(cr_rd.data.cs_register_client_c == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.cs_register_client_c, &registerclient_c,
                      sizeof(TCS_SuplaRegisterClient_C)));

  free(cr_rd.data.cs_register_client_c);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerclient_d) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TCS_SuplaRegisterClient_D, registerclient_d);

  ASSERT_GT(srpc_cs_async_registerclient_d(srpc, &registerclient_d), 0);
  SendAndReceive(SUPLA_CS_CALL_REGISTER_CLIENT_D, 662);

  ASSERT_FALSE(cr_rd.data.cs_register_client_d == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.cs_register_client_d, &registerclient_d,
                      sizeof(TCS_SuplaRegisterClient_D)));

  free(cr_rd.data.cs_register_client_d);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerclient_result) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaRegisterClientResult, result);

  ASSERT_GT(srpc_sc_async_registerclient_result(srpc, &result), 0);
  SendAndReceive(SUPLA_SC_CALL_REGISTER_CLIENT_RESULT, 42);

  ASSERT_FALSE(cr_rd.data.sc_register_client_result == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_register_client_result, &result,
                      sizeof(TSC_SuplaRegisterClientResult)));

  free(cr_rd.data.sc_register_client_result);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_registerclient_result_b) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaRegisterClientResult_B, result);

  ASSERT_GT(srpc_sc_async_registerclient_result_b(srpc, &result), 0);
  SendAndReceive(SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_B, 50);

  ASSERT_FALSE(cr_rd.data.sc_register_client_result_b == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_register_client_result_b, &result,
                      sizeof(TSC_SuplaRegisterClientResult_B)));

  free(cr_rd.data.sc_register_client_result_b);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// LOCATION UPDATE
//---------------------------------------------------------

TEST_F(SrpcTest, call_location_update_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaLocation, location);

  location.CaptionSize = SUPLA_LOCATION_CAPTION_MAXSIZE + 1;

  ASSERT_EQ(srpc_sc_async_location_update(srpc, &location), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_location_update_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaLocation, location);

  location.CaptionSize = SUPLA_LOCATION_CAPTION_MAXSIZE;

  ASSERT_GT(srpc_sc_async_location_update(srpc, &location), 0);
  SendAndReceive(SUPLA_SC_CALL_LOCATION_UPDATE, 433);

  ASSERT_FALSE(cr_rd.data.sc_location == NULL);

  ASSERT_EQ(
      0, memcmp(cr_rd.data.sc_location, &location, sizeof(TSC_SuplaLocation)));

  free(cr_rd.data.sc_location);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_location_update_without_caption) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaLocation, location);

  location.CaptionSize = 0;

  ASSERT_GT(srpc_sc_async_location_update(srpc, &location), 0);
  SendAndReceive(SUPLA_SC_CALL_LOCATION_UPDATE, 32);

  ASSERT_FALSE(cr_rd.data.sc_location == NULL);

  ASSERT_EQ(0,
            memcmp(cr_rd.data.sc_location, &location,
                   sizeof(TSC_SuplaLocation) - SUPLA_LOCATION_CAPTION_MAXSIZE));

  free(cr_rd.data.sc_location);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_locationpack_update_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaLocationPack, location_pack);

  location_pack.count = SUPLA_LOCATIONPACK_MAXCOUNT + 1;

  ASSERT_EQ(srpc_sc_async_locationpack_update(srpc, &location_pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_locationpack_update_with_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaLocationPack, location_pack);

  location_pack.count = 0;

  ASSERT_EQ(srpc_sc_async_locationpack_update(srpc, &location_pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_locationpack_update_with_caption_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaLocationPack, location_pack);

  location_pack.count = SUPLA_LOCATIONPACK_MAXCOUNT;

  location_pack.items[0].CaptionSize = SUPLA_LOCATION_CAPTION_MAXSIZE + 1;

  for (int a = 1; a < SUPLA_LOCATIONPACK_MAXCOUNT; a++) {
    location_pack.items[a].CaptionSize = SUPLA_LOCATION_CAPTION_MAXSIZE;
  }

  ASSERT_GT(srpc_sc_async_locationpack_update(srpc, &location_pack), 0);
  SendAndReceive(SUPLA_SC_CALL_LOCATIONPACK_UPDATE, 7821);

  ASSERT_FALSE(cr_rd.data.sc_location_pack == NULL);
  ASSERT_EQ(cr_rd.data.sc_location_pack->count,
            SUPLA_LOCATIONPACK_MAXCOUNT - 1);
  ASSERT_EQ(cr_rd.data.sc_location_pack->total_left, location_pack.total_left);

  for (int a = 0; a < SUPLA_LOCATIONPACK_MAXCOUNT - 1; a++) {
    ASSERT_EQ(
        memcmp(&cr_rd.data.sc_location_pack->items[a],
               &location_pack.items[a + 1],
               sizeof(TSC_SuplaLocation) - SUPLA_LOCATION_CAPTION_MAXSIZE +
                   location_pack.items[a + 1].CaptionSize),
        0);
  }

  free(cr_rd.data.sc_location_pack);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_locationpack_update_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaLocationPack, location_pack);

  location_pack.count = SUPLA_LOCATIONPACK_MAXCOUNT;

  for (int a = 0; a < SUPLA_LOCATIONPACK_MAXCOUNT; a++) {
    location_pack.items[a].CaptionSize = SUPLA_LOCATION_CAPTION_MAXSIZE;
  }

  ASSERT_GT(srpc_sc_async_locationpack_update(srpc, &location_pack), 0);
  SendAndReceive(SUPLA_SC_CALL_LOCATIONPACK_UPDATE, 8231);

  ASSERT_FALSE(cr_rd.data.sc_location_pack == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_location_pack, &location_pack,
                      sizeof(TSC_SuplaLocationPack)));

  free(cr_rd.data.sc_location_pack);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// CHANNEL UPDATE
//---------------------------------------------------------
SRPC_CALL_BASIC_TEST_WITH_CAPTION(srpc_sc_async_channel_update,
                                  TSC_SuplaChannel,
                                  SUPLA_SC_CALL_CHANNEL_UPDATE, 57, 458,
                                  sc_channel, SUPLA_CHANNEL_CAPTION_MAXSIZE);

SRPC_CALL_BASIC_TEST_WITH_CAPTION(srpc_sc_async_channel_update_b,
                                  TSC_SuplaChannel_B,
                                  SUPLA_SC_CALL_CHANNEL_UPDATE_B, 66, 467,
                                  sc_channel, SUPLA_CHANNEL_CAPTION_MAXSIZE);

SRPC_CALL_BASIC_TEST_WITH_CAPTION(srpc_sc_async_channel_update_c,
                                  TSC_SuplaChannel_C,
                                  SUPLA_SC_CALL_CHANNEL_UPDATE_C, 82, 483,
                                  sc_channel, SUPLA_CHANNEL_CAPTION_MAXSIZE);

//---------------------------------------------------------
// CHANNEL PACK UPDATE
//---------------------------------------------------------

TEST_F(SrpcTest, call_channelpack_update_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelPack, channel_pack);

  channel_pack.count = SUPLA_CHANNELPACK_MAXCOUNT + 1;

  ASSERT_EQ(srpc_sc_async_channelpack_update(srpc, &channel_pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelpack_update_with_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelPack, channel_pack);

  channel_pack.count = 0;

  ASSERT_EQ(srpc_sc_async_channelpack_update(srpc, &channel_pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelpack_update_with_caption_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelPack, channel_pack);

  channel_pack.count = SUPLA_CHANNELPACK_MAXCOUNT;

  channel_pack.items[0].CaptionSize = SUPLA_CHANNEL_CAPTION_MAXSIZE + 1;

  for (int a = 1; a < SUPLA_CHANNELPACK_MAXCOUNT; a++) {
    channel_pack.items[a].CaptionSize = SUPLA_CHANNEL_CAPTION_MAXSIZE;
  }

  ASSERT_GT(srpc_sc_async_channelpack_update(srpc, &channel_pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELPACK_UPDATE, 8296);

  ASSERT_FALSE(cr_rd.data.sc_channel_pack == NULL);
  ASSERT_EQ(cr_rd.data.sc_channel_pack->count, SUPLA_CHANNELPACK_MAXCOUNT - 1);
  ASSERT_EQ(cr_rd.data.sc_channel_pack->total_left, channel_pack.total_left);

  for (int a = 0; a < SUPLA_CHANNELPACK_MAXCOUNT - 1; a++) {
    ASSERT_EQ(memcmp(&cr_rd.data.sc_channel_pack->items[a],
                     &channel_pack.items[a + 1],
                     sizeof(TSC_SuplaChannel) - SUPLA_CHANNEL_CAPTION_MAXSIZE +
                         channel_pack.items[a + 1].CaptionSize),
              0);
  }

  free(cr_rd.data.sc_channel_pack);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelpack_update_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelPack, channel_pack);

  channel_pack.count = SUPLA_CHANNELPACK_MAXCOUNT;

  for (int a = 0; a < SUPLA_CHANNELPACK_MAXCOUNT; a++) {
    channel_pack.items[a].CaptionSize = SUPLA_CHANNEL_CAPTION_MAXSIZE;
  }

  ASSERT_GT(srpc_sc_async_channelpack_update(srpc, &channel_pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELPACK_UPDATE, 8731);

  ASSERT_FALSE(cr_rd.data.sc_channel_pack == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_channel_pack, &channel_pack,
                      sizeof(TSC_SuplaChannelPack)));

  free(cr_rd.data.sc_channel_pack);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------

TEST_F(SrpcTest, call_channelpack_update_c_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelPack_C, channel_pack);

  channel_pack.count = SUPLA_CHANNELPACK_MAXCOUNT + 1;

  ASSERT_EQ(srpc_sc_async_channelpack_update_c(srpc, &channel_pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelpack_update_c_with_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelPack_C, channel_pack);

  channel_pack.count = 0;

  ASSERT_EQ(srpc_sc_async_channelpack_update_c(srpc, &channel_pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelpack_update_c_with_caption_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelPack_C, channel_pack);

  channel_pack.count = SUPLA_CHANNELPACK_MAXCOUNT;

  channel_pack.items[0].CaptionSize = SUPLA_CHANNEL_CAPTION_MAXSIZE + 1;

  for (int a = 1; a < SUPLA_CHANNELPACK_MAXCOUNT; a++) {
    channel_pack.items[a].CaptionSize = SUPLA_CHANNEL_CAPTION_MAXSIZE;
  }

  ASSERT_GT(srpc_sc_async_channelpack_update_c(srpc, &channel_pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELPACK_UPDATE_C, 8771);

  ASSERT_FALSE(cr_rd.data.sc_channel_pack_c == NULL);
  ASSERT_EQ(cr_rd.data.sc_channel_pack_c->count,
            SUPLA_CHANNELPACK_MAXCOUNT - 1);
  ASSERT_EQ(cr_rd.data.sc_channel_pack_c->total_left, channel_pack.total_left);

  for (int a = 0; a < SUPLA_CHANNELPACK_MAXCOUNT - 1; a++) {
    ASSERT_EQ(
        memcmp(&cr_rd.data.sc_channel_pack_c->items[a],
               &channel_pack.items[a + 1],
               sizeof(TSC_SuplaChannel_C) - SUPLA_CHANNEL_CAPTION_MAXSIZE +
                   channel_pack.items[a + 1].CaptionSize),
        0);
  }

  free(cr_rd.data.sc_channel_pack_c);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelpack_update_c_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelPack_C, channel_pack);

  channel_pack.count = SUPLA_CHANNELPACK_MAXCOUNT;

  for (int a = 0; a < SUPLA_CHANNELPACK_MAXCOUNT; a++) {
    channel_pack.items[a].CaptionSize = SUPLA_CHANNEL_CAPTION_MAXSIZE;
  }

  ASSERT_GT(srpc_sc_async_channelpack_update_c(srpc, &channel_pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELPACK_UPDATE_C, 9231);

  ASSERT_FALSE(cr_rd.data.sc_channel_pack_c == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_channel_pack_c, &channel_pack,
                      sizeof(TSC_SuplaChannelPack_C)));

  free(cr_rd.data.sc_channel_pack_c);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------

TEST_F(SrpcTest, call_channelpack_update_b_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelPack_B, channel_pack);

  channel_pack.count = SUPLA_CHANNELPACK_MAXCOUNT + 1;

  ASSERT_EQ(srpc_sc_async_channelpack_update_b(srpc, &channel_pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelpack_update_b_with_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelPack_B, channel_pack);

  channel_pack.count = 0;

  ASSERT_EQ(srpc_sc_async_channelpack_update_b(srpc, &channel_pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelpack_update_b_with_caption_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelPack_B, channel_pack);

  channel_pack.count = SUPLA_CHANNELPACK_MAXCOUNT;

  channel_pack.items[0].CaptionSize = SUPLA_CHANNEL_CAPTION_MAXSIZE + 1;

  for (int a = 1; a < SUPLA_CHANNELPACK_MAXCOUNT; a++) {
    channel_pack.items[a].CaptionSize = SUPLA_CHANNEL_CAPTION_MAXSIZE;
  }

  ASSERT_GT(srpc_sc_async_channelpack_update_b(srpc, &channel_pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELPACK_UPDATE_B, 8467);

  ASSERT_FALSE(cr_rd.data.sc_channel_pack_b == NULL);
  ASSERT_EQ(cr_rd.data.sc_channel_pack_b->count,
            SUPLA_CHANNELPACK_MAXCOUNT - 1);
  ASSERT_EQ(cr_rd.data.sc_channel_pack_b->total_left, channel_pack.total_left);

  for (int a = 0; a < SUPLA_CHANNELPACK_MAXCOUNT - 1; a++) {
    ASSERT_EQ(
        memcmp(&cr_rd.data.sc_channel_pack_b->items[a],
               &channel_pack.items[a + 1],
               sizeof(TSC_SuplaChannel_B) - SUPLA_CHANNEL_CAPTION_MAXSIZE +
                   channel_pack.items[a + 1].CaptionSize),
        0);
  }

  free(cr_rd.data.sc_channel_pack_b);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelpack_update_b_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelPack_B, channel_pack);

  channel_pack.count = SUPLA_CHANNELPACK_MAXCOUNT;

  for (int a = 0; a < SUPLA_CHANNELPACK_MAXCOUNT; a++) {
    channel_pack.items[a].CaptionSize = SUPLA_CHANNEL_CAPTION_MAXSIZE;
  }

  ASSERT_GT(srpc_sc_async_channelpack_update_b(srpc, &channel_pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELPACK_UPDATE_B, 8911);

  ASSERT_FALSE(cr_rd.data.sc_channel_pack_b == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_channel_pack_b, &channel_pack,
                      sizeof(TSC_SuplaChannelPack_B)));

  free(cr_rd.data.sc_channel_pack_b);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// CHANNEL VALUE UPDATE
//---------------------------------------------------------

TEST_F(SrpcTest, call_channel_value_update) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelValue, value);

  ASSERT_GT(srpc_sc_async_channel_value_update(srpc, &value), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE, 45);

  ASSERT_FALSE(cr_rd.data.sc_channel_value == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_channel_value, &value,
                      sizeof(TSC_SuplaChannelValue)));

  free(cr_rd.data.sc_channel_value);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// CHANNEL GROUP PACK UPDATE
//---------------------------------------------------------

TEST_F(SrpcTest, call_channelgroup_pack_update_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelGroupPack, pack);

  pack.count = SUPLA_CHANNELGROUP_PACK_MAXCOUNT + 1;

  ASSERT_EQ(srpc_sc_async_channelgroup_pack_update(srpc, &pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelgroup_pack_update_with_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelGroupPack, pack);

  pack.count = 0;

  ASSERT_EQ(srpc_sc_async_channelgroup_pack_update(srpc, &pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelgroup_pack_update_with_caption_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelGroupPack, pack);

  pack.count = SUPLA_CHANNELGROUP_PACK_MAXCOUNT;

  pack.items[0].CaptionSize = SUPLA_CHANNELGROUP_CAPTION_MAXSIZE + 1;

  for (int a = 1; a < SUPLA_CHANNELPACK_MAXCOUNT; a++) {
    pack.items[a].CaptionSize = SUPLA_CHANNELGROUP_CAPTION_MAXSIZE;
  }

  ASSERT_GT(srpc_sc_async_channelgroup_pack_update(srpc, &pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE, 8125);

  ASSERT_FALSE(cr_rd.data.sc_channelgroup_pack == NULL);
  ASSERT_EQ(cr_rd.data.sc_channelgroup_pack->count,
            SUPLA_CHANNELGROUP_PACK_MAXCOUNT - 1);
  ASSERT_EQ(cr_rd.data.sc_channelgroup_pack->total_left, pack.total_left);

  for (int a = 0; a < SUPLA_CHANNELGROUP_PACK_MAXCOUNT - 1; a++) {
    ASSERT_EQ(
        memcmp(&cr_rd.data.sc_channelgroup_pack->items[a], &pack.items[a + 1],
               sizeof(TSC_SuplaChannelGroup) -
                   SUPLA_CHANNELGROUP_CAPTION_MAXSIZE +
                   pack.items[a + 1].CaptionSize),
        0);
  }

  free(cr_rd.data.sc_channelgroup_pack);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelgroup_pack_update_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelGroupPack, pack);

  pack.count = SUPLA_CHANNELGROUP_PACK_MAXCOUNT;

  for (int a = 0; a < SUPLA_CHANNELGROUP_PACK_MAXCOUNT; a++) {
    pack.items[a].CaptionSize = SUPLA_CHANNELGROUP_CAPTION_MAXSIZE;
  }

  ASSERT_GT(srpc_sc_async_channelgroup_pack_update(srpc, &pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE, 8551);

  ASSERT_FALSE(cr_rd.data.sc_channelgroup_pack == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_channelgroup_pack, &pack,
                      sizeof(TSC_SuplaChannelGroupPack)));

  free(cr_rd.data.sc_channelgroup_pack);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// CHANNEL GROUP PACK B UPDATE
//---------------------------------------------------------

TEST_F(SrpcTest, call_channelgroup_pack_update_b_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelGroupPack_B, pack);

  pack.count = SUPLA_CHANNELGROUP_PACK_MAXCOUNT + 1;

  ASSERT_EQ(srpc_sc_async_channelgroup_pack_update_b(srpc, &pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelgroup_pack_update_b_with_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelGroupPack_B, pack);

  pack.count = 0;

  ASSERT_EQ(srpc_sc_async_channelgroup_pack_update_b(srpc, &pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelgroup_pack_update_b_with_caption_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelGroupPack_B, pack);

  pack.count = SUPLA_CHANNELGROUP_PACK_MAXCOUNT;

  pack.items[0].CaptionSize = SUPLA_CHANNELGROUP_CAPTION_MAXSIZE + 1;

  for (int a = 1; a < SUPLA_CHANNELPACK_MAXCOUNT; a++) {
    pack.items[a].CaptionSize = SUPLA_CHANNELGROUP_CAPTION_MAXSIZE;
  }

  ASSERT_GT(srpc_sc_async_channelgroup_pack_update_b(srpc, &pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE_B, 8201);

  ASSERT_FALSE(cr_rd.data.sc_channelgroup_pack_b == NULL);
  ASSERT_EQ(cr_rd.data.sc_channelgroup_pack_b->count,
            SUPLA_CHANNELGROUP_PACK_MAXCOUNT - 1);
  ASSERT_EQ(cr_rd.data.sc_channelgroup_pack_b->total_left, pack.total_left);

  for (int a = 0; a < SUPLA_CHANNELGROUP_PACK_MAXCOUNT - 1; a++) {
    ASSERT_EQ(
        memcmp(&cr_rd.data.sc_channelgroup_pack_b->items[a], &pack.items[a + 1],
               sizeof(TSC_SuplaChannelGroup_B) -
                   SUPLA_CHANNELGROUP_CAPTION_MAXSIZE +
                   pack.items[a + 1].CaptionSize),
        0);
  }

  free(cr_rd.data.sc_channelgroup_pack_b);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelgroup_pack_update_b_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelGroupPack_B, pack);

  pack.count = SUPLA_CHANNELGROUP_PACK_MAXCOUNT;

  for (int a = 0; a < SUPLA_CHANNELGROUP_PACK_MAXCOUNT; a++) {
    pack.items[a].CaptionSize = SUPLA_CHANNELGROUP_CAPTION_MAXSIZE;
  }

  ASSERT_GT(srpc_sc_async_channelgroup_pack_update_b(srpc, &pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE_B, 8631);

  ASSERT_FALSE(cr_rd.data.sc_channelgroup_pack_b == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_channelgroup_pack_b, &pack,
                      sizeof(TSC_SuplaChannelGroupPack_B)));

  free(cr_rd.data.sc_channelgroup_pack_b);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// CHANNEL GROUP RELATION PACK UPDATE
//---------------------------------------------------------

TEST_F(SrpcTest, call_channelgroup_relation_pack_update_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelGroupRelationPack, pack);

  pack.count = SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT + 1;

  ASSERT_EQ(srpc_sc_async_channelgroup_relation_pack_update(srpc, &pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelgroup_relation_pack_update_with_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelGroupRelationPack, pack);

  pack.count = SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT + 1;

  ASSERT_EQ(srpc_sc_async_channelgroup_relation_pack_update(srpc, &pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelgroup_relation_pack_update_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelGroupRelationPack, pack);

  pack.count = SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT;

  ASSERT_GT(srpc_sc_async_channelgroup_relation_pack_update(srpc, &pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELGROUP_RELATION_PACK_UPDATE, 931);

  ASSERT_FALSE(cr_rd.data.sc_channelgroup_relation_pack == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_channelgroup_relation_pack, &pack,
                      sizeof(TSC_SuplaChannelGroupRelationPack)));

  free(cr_rd.data.sc_channelgroup_relation_pack);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelgroup_relation_pack_update_with_minimum_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelGroupRelationPack, pack);

  pack.count = 1;

  ASSERT_GT(srpc_sc_async_channelgroup_relation_pack_update(srpc, &pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELGROUP_RELATION_PACK_UPDATE, 40);

  ASSERT_FALSE(cr_rd.data.sc_channelgroup_relation_pack == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_channelgroup_relation_pack, &pack,
                      sizeof(TSC_SuplaChannelGroupRelationPack) -
                          ((SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT - 1) *
                           sizeof(TSC_SuplaChannelGroupRelation))));

  free(cr_rd.data.sc_channelgroup_relation_pack);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// CHANNEL VALUE PACK
//---------------------------------------------------------

TEST_F(SrpcTest, call_channelvalue_pack_update_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelValuePack, pack);

  pack.count = SUPLA_CHANNELVALUE_PACK_MAXCOUNT + 1;

  ASSERT_EQ(srpc_sc_async_channelvalue_pack_update(srpc, &pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelvalue_pack_update_with_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelValuePack, pack);

  pack.count = SUPLA_CHANNELVALUE_PACK_MAXCOUNT + 1;

  ASSERT_EQ(srpc_sc_async_channelvalue_pack_update(srpc, &pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelvalue_pack_update_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelValuePack, pack);

  pack.count = SUPLA_CHANNELVALUE_PACK_MAXCOUNT;

  ASSERT_GT(srpc_sc_async_channelvalue_pack_update(srpc, &pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE, 471);

  ASSERT_FALSE(cr_rd.data.sc_channelvalue_pack == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_channelvalue_pack, &pack,
                      sizeof(TSC_SuplaChannelValuePack)));

  free(cr_rd.data.sc_channelvalue_pack);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelvalue_pack_update_with_minimum_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelValuePack, pack);

  pack.count = 1;

  ASSERT_GT(srpc_sc_async_channelvalue_pack_update(srpc, &pack), 0);
  SendAndReceive(SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE, 53);

  ASSERT_FALSE(cr_rd.data.sc_channelvalue_pack == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_channelvalue_pack, &pack,
                      sizeof(TSC_SuplaChannelValuePack) -
                          ((SUPLA_CHANNELVALUE_PACK_MAXCOUNT - 1) *
                           sizeof(TSC_SuplaChannelValue))));

  free(cr_rd.data.sc_channelvalue_pack);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// CHANNEL EXTENDED VALUE PACK
//---------------------------------------------------------

TEST_F(SrpcTest, call_channelextendedvalue_pack_update_with_over_count) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelExtendedValuePack, pack);

  pack.count = SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXCOUNT + 1;
  pack.pack_size = 1;

  ASSERT_EQ(srpc_sc_async_channelextendedvalue_pack_update(srpc, &pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelextendedvalue_pack_update_with_zero_count) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelExtendedValuePack, pack);

  pack.count = 0;
  pack.pack_size = 1;

  ASSERT_EQ(srpc_sc_async_channelextendedvalue_pack_update(srpc, &pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelextendedvalue_pack_update_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelExtendedValuePack, pack);

  pack.count = 1;
  pack.pack_size = SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXDATASIZE + 1;

  ASSERT_EQ(srpc_sc_async_channelextendedvalue_pack_update(srpc, &pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelextendedvalue_pack_update_with_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelExtendedValuePack, pack);

  pack.count = 1;
  pack.pack_size = 0;

  ASSERT_EQ(srpc_sc_async_channelextendedvalue_pack_update(srpc, &pack), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_channelextendedvalue_pack_update_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaChannelExtendedValuePack, pack);

  pack.count = SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXCOUNT;
  pack.pack_size = SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXDATASIZE;

  ASSERT_GT(srpc_sc_async_channelextendedvalue_pack_update(srpc, &pack), 0);

  SendAndReceive(SUPLA_SC_CALL_CHANNELEXTENDEDVALUE_PACK_UPDATE, 10225);

  ASSERT_FALSE(cr_rd.data.sc_channelextendedvalue_pack == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_channelextendedvalue_pack, &pack,
                      sizeof(TSC_SuplaChannelExtendedValuePack)));

  free(cr_rd.data.sc_channelextendedvalue_pack);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// GET NEXT
//---------------------------------------------------------

SRPC_CALL_WITH_NO_DATA(srpc_cs_async_get_next, SUPLA_CS_CALL_GET_NEXT);

//---------------------------------------------------------
// EVENT
//---------------------------------------------------------

TEST_F(SrpcTest, call_event_with_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaEvent, event);

  event.SenderNameSize = 0;

  ASSERT_GT(srpc_sc_async_event(srpc, &event), 0);
  SendAndReceive(SUPLA_SC_CALL_EVENT, 43);

  ASSERT_FALSE(cr_rd.data.sc_event == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_event, &event,
                      sizeof(TSC_SuplaEvent) - SUPLA_SENDER_NAME_MAXSIZE));

  free(cr_rd.data.sc_event);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_event_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaEvent, event);

  event.SenderNameSize = SUPLA_SENDER_NAME_MAXSIZE;

  ASSERT_GT(srpc_sc_async_event(srpc, &event), 0);
  SendAndReceive(SUPLA_SC_CALL_EVENT, 244);

  ASSERT_FALSE(cr_rd.data.sc_event == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_event, &event, sizeof(TSC_SuplaEvent)));

  free(cr_rd.data.sc_event);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_event_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuplaEvent, event);

  event.SenderNameSize = SUPLA_SENDER_NAME_MAXSIZE + 1;

  ASSERT_EQ(srpc_sc_async_event(srpc, &event), 0);

  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// CS - SET CHANNEL NEW VALUE
//---------------------------------------------------------

TEST_F(SrpcTest, call_cs_set_channel_value) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TCS_SuplaChannelNewValue, value);

  ASSERT_GT(srpc_cs_async_set_channel_value(srpc, &value), 0);
  SendAndReceive(SUPLA_CS_CALL_CHANNEL_SET_VALUE, 32);

  ASSERT_FALSE(cr_rd.data.cs_channel_new_value == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.cs_channel_new_value, &value,
                      sizeof(TCS_SuplaChannelNewValue)));

  free(cr_rd.data.cs_channel_new_value);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_cs_set_channel_value_b) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TCS_SuplaChannelNewValue_B, value);

  ASSERT_GT(srpc_cs_async_set_channel_value_b(srpc, &value), 0);
  SendAndReceive(SUPLA_CS_CALL_CHANNEL_SET_VALUE_B, 35);

  ASSERT_FALSE(cr_rd.data.cs_channel_new_value_b == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.cs_channel_new_value_b, &value,
                      sizeof(TCS_SuplaChannelNewValue_B)));

  free(cr_rd.data.cs_channel_new_value_b);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// OAUTH TOKEN
//---------------------------------------------------------

SRPC_CALL_WITH_NO_DATA(srpc_cs_async_oauth_token_request,
                       SUPLA_CS_CALL_OAUTH_TOKEN_REQUEST);

TEST_F(SrpcTest, call_oauth_token_request_result_with_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_OAuthTokenRequestResult, result);

  result.Token.TokenSize = SUPLA_OAUTH_TOKEN_MAXSIZE + 1;

  ASSERT_EQ(srpc_cs_async_oauth_token_request_result(srpc, &result), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_oauth_token_request_result_with_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_OAuthTokenRequestResult, result);

  result.Token.TokenSize = SUPLA_OAUTH_TOKEN_MAXSIZE;

  ASSERT_GT(srpc_cs_async_oauth_token_request_result(srpc, &result), 0);

  SendAndReceive(SUPLA_SC_CALL_OAUTH_TOKEN_REQUEST_RESULT, 288);

  ASSERT_FALSE(cr_rd.data.sc_oauth_tokenrequest_result == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_oauth_tokenrequest_result, &result,
                      sizeof(TSC_OAuthTokenRequestResult)));

  free(cr_rd.data.sc_oauth_tokenrequest_result);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// SUPER USER AUTHORIZATION
//---------------------------------------------------------

TEST_F(SrpcTest, call_superuser_authorization_request) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TCS_SuperUserAuthorizationRequest, request);

  ASSERT_GT(srpc_cs_async_superuser_authorization_request(srpc, &request), 0);
  SendAndReceive(SUPLA_CS_CALL_SUPERUSER_AUTHORIZATION_REQUEST, 343);

  ASSERT_FALSE(cr_rd.data.cs_superuser_authorization_request == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.cs_superuser_authorization_request, &request,
                      sizeof(TCS_SuperUserAuthorizationRequest)));

  free(cr_rd.data.cs_superuser_authorization_request);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_superuser_authorization_result) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_SuperUserAuthorizationResult, result);

  ASSERT_GT(srpc_sc_async_superuser_authorization_result(srpc, &result), 0);
  SendAndReceive(SUPLA_SC_CALL_SUPERUSER_AUTHORIZATION_RESULT, 27);

  ASSERT_FALSE(cr_rd.data.sc_superuser_authorization_result == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_superuser_authorization_result, &result,
                      sizeof(TSC_SuperUserAuthorizationResult)));

  free(cr_rd.data.sc_superuser_authorization_result);
  srpc_free(srpc);
  srpc = NULL;
}

SRPC_CALL_WITH_NO_DATA(srpc_cs_async_get_superuser_authorization_result,
                       SUPLA_CS_CALL_GET_SUPERUSER_AUTHORIZATION_RESULT);

//---------------------------------------------------------
// DEVICE CALIBRATION / CONFIG
//---------------------------------------------------------

TEST_F(SrpcTest, call_cs_device_calcfg_request_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TCS_DeviceCalCfgRequest, request);

  request.DataSize = SUPLA_CALCFG_DATA_MAXSIZE + 1;

  ASSERT_EQ(srpc_cs_async_device_calcfg_request(srpc, &request), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_cs_device_calcfg_request_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TCS_DeviceCalCfgRequest, request);

  request.DataSize = 0;

  ASSERT_GT(srpc_cs_async_device_calcfg_request(srpc, &request), 0);

  SendAndReceive(SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST, 39);

  ASSERT_FALSE(cr_rd.data.cs_device_calcfg_request == NULL);

  ASSERT_EQ(
      0, memcmp(cr_rd.data.cs_device_calcfg_request, &request,
                sizeof(TCS_DeviceCalCfgRequest) - SUPLA_CALCFG_DATA_MAXSIZE));

  free(cr_rd.data.cs_device_calcfg_request);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_cs_device_calcfg_request_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TCS_DeviceCalCfgRequest, request);

  request.DataSize = SUPLA_CALCFG_DATA_MAXSIZE;

  ASSERT_GT(srpc_cs_async_device_calcfg_request(srpc, &request), 0);

  SendAndReceive(SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST, 167);

  ASSERT_FALSE(cr_rd.data.cs_device_calcfg_request == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sd_device_calcfg_request, &request,
                      sizeof(TCS_DeviceCalCfgRequest)));

  free(cr_rd.data.cs_device_calcfg_request);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_sc_device_calcfg_result_over_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_DeviceCalCfgResult, result);

  result.DataSize = SUPLA_CALCFG_DATA_MAXSIZE + 1;

  ASSERT_EQ(srpc_sc_async_device_calcfg_result(srpc, &result), 0);

  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_sc_device_calcfg_result_zero_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_DeviceCalCfgResult, result);

  result.DataSize = 0;

  ASSERT_GT(srpc_sc_async_device_calcfg_result(srpc, &result), 0);

  SendAndReceive(SUPLA_SC_CALL_DEVICE_CALCFG_RESULT, 39);

  ASSERT_FALSE(cr_rd.data.sc_device_calcfg_result == NULL);

  ASSERT_EQ(0,
            memcmp(cr_rd.data.sc_device_calcfg_result, &result,
                   sizeof(TSC_DeviceCalCfgResult) - SUPLA_CALCFG_DATA_MAXSIZE));

  free(cr_rd.data.sc_device_calcfg_result);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_sc_device_calcfg_result_full_size) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  DECLARE_WITH_RANDOM(TSC_DeviceCalCfgResult, result);

  result.DataSize = SUPLA_CALCFG_DATA_MAXSIZE;

  ASSERT_GT(srpc_sc_async_device_calcfg_result(srpc, &result), 0);

  SendAndReceive(SUPLA_SC_CALL_DEVICE_CALCFG_RESULT, 167);

  ASSERT_FALSE(cr_rd.data.sc_device_calcfg_result == NULL);

  ASSERT_EQ(0, memcmp(cr_rd.data.sc_device_calcfg_result, &result,
                      sizeof(TSC_DeviceCalCfgResult)));

  free(cr_rd.data.sc_device_calcfg_result);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// EXTENDED VALUE TOOLS
//---------------------------------------------------------

TEST_F(SrpcTest, evtool_electricity_meter_value_to_extended) {
  TSuplaChannelExtendedValue ev;
  DECLARE_WITH_RANDOM(TElectricityMeter_ExtendedValue, em_ev_src);
  TElectricityMeter_ExtendedValue em_ev_dst;

  memset(&em_ev_dst, 0, sizeof(TElectricityMeter_ExtendedValue));
  em_ev_src.m_count = EM_MEASUREMENT_COUNT;

  ASSERT_EQ(1, srpc_evtool_v1_emextended2extended(&em_ev_src, &ev));
  ASSERT_EQ(1, srpc_evtool_v1_extended2emextended(&ev, &em_ev_dst));

  ASSERT_EQ(0, memcmp(&em_ev_src, &em_ev_dst,
                      sizeof(TElectricityMeter_ExtendedValue)));

  ASSERT_EQ(1, srpc_evtool_v1_emextended2extended(&em_ev_src, &ev));
  ASSERT_EQ(0, srpc_evtool_v1_emextended2extended(NULL, &ev));
  ASSERT_EQ(0, srpc_evtool_v1_emextended2extended(&em_ev_src, NULL));

  em_ev_src.m_count = -1;
  ASSERT_EQ(0, srpc_evtool_v1_emextended2extended(&em_ev_src, &ev));

  em_ev_src.m_count = EM_MEASUREMENT_COUNT + 1;
  ASSERT_EQ(0, srpc_evtool_v1_emextended2extended(&em_ev_src, &ev));

  em_ev_src.m_count = EM_MEASUREMENT_COUNT;
  ASSERT_EQ(1, srpc_evtool_v1_emextended2extended(&em_ev_src, &ev));
  ASSERT_EQ(0, srpc_evtool_v1_extended2emextended(NULL, &em_ev_dst));
  ASSERT_EQ(0, srpc_evtool_v1_extended2emextended(&ev, NULL));

  ev.type = 0;
  ASSERT_EQ(0, srpc_evtool_v1_extended2emextended(&ev, &em_ev_dst));

  ev.type = EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1;
  ev.size = 0;
  ASSERT_EQ(0, srpc_evtool_v1_extended2emextended(&ev, &em_ev_dst));

  ev.size = sizeof(TElectricityMeter_ExtendedValue) + 1;
  ASSERT_EQ(0, srpc_evtool_v1_extended2emextended(&ev, &em_ev_dst));

  ev.size = sizeof(TElectricityMeter_ExtendedValue) - 1;
  ASSERT_EQ(0, srpc_evtool_v1_extended2emextended(&ev, &em_ev_dst));
}

TEST_F(SrpcTest, evtool_electricity_meter_value_v2_to_extended) {
  TSuplaChannelExtendedValue ev;
  DECLARE_WITH_RANDOM(TElectricityMeter_ExtendedValue_V2, em_ev_src);
  TElectricityMeter_ExtendedValue_V2 em_ev_dst;

  memset(&em_ev_dst, 0, sizeof(TElectricityMeter_ExtendedValue_V2));
  em_ev_src.m_count = EM_MEASUREMENT_COUNT;

  ASSERT_EQ(1, srpc_evtool_v2_emextended2extended(&em_ev_src, &ev));
  ASSERT_EQ(1, srpc_evtool_v2_extended2emextended(&ev, &em_ev_dst));

  ASSERT_EQ(0, memcmp(&em_ev_src, &em_ev_dst,
                      sizeof(TElectricityMeter_ExtendedValue_V2)));

  ASSERT_EQ(1, srpc_evtool_v2_emextended2extended(&em_ev_src, &ev));
  ASSERT_EQ(0, srpc_evtool_v2_emextended2extended(NULL, &ev));
  ASSERT_EQ(0, srpc_evtool_v2_emextended2extended(&em_ev_src, NULL));

  em_ev_src.m_count = -1;
  ASSERT_EQ(0, srpc_evtool_v2_emextended2extended(&em_ev_src, &ev));

  em_ev_src.m_count = EM_MEASUREMENT_COUNT + 1;
  ASSERT_EQ(0, srpc_evtool_v2_emextended2extended(&em_ev_src, &ev));

  em_ev_src.m_count = EM_MEASUREMENT_COUNT;
  ASSERT_EQ(1, srpc_evtool_v2_emextended2extended(&em_ev_src, &ev));
  ASSERT_EQ(0, srpc_evtool_v2_extended2emextended(NULL, &em_ev_dst));
  ASSERT_EQ(0, srpc_evtool_v2_extended2emextended(&ev, NULL));

  ev.type = 0;
  ASSERT_EQ(0, srpc_evtool_v2_extended2emextended(&ev, &em_ev_dst));

  ev.type = EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2;
  ev.size = 0;
  ASSERT_EQ(0, srpc_evtool_v2_extended2emextended(&ev, &em_ev_dst));

  ev.size = sizeof(TElectricityMeter_ExtendedValue_V2) + 1;
  ASSERT_EQ(0, srpc_evtool_v2_extended2emextended(&ev, &em_ev_dst));

  ev.size = sizeof(TElectricityMeter_ExtendedValue_V2) - 1;
  ASSERT_EQ(0, srpc_evtool_v2_extended2emextended(&ev, &em_ev_dst));
}

TEST_F(SrpcTest, srpc_evtool_emev_v1to2) {
  DECLARE_WITH_RANDOM(TElectricityMeter_ExtendedValue, emev_v1);
  TElectricityMeter_ExtendedValue_V2 emev_v2;
  memset(&emev_v2, 0, sizeof(TElectricityMeter_ExtendedValue_V2));

  ASSERT_EQ(srpc_evtool_emev_v1to2(NULL, &emev_v2), 0);
  ASSERT_EQ(srpc_evtool_emev_v1to2(&emev_v1, NULL), 0);
  ASSERT_EQ(srpc_evtool_emev_v1to2(&emev_v1, &emev_v2), 1);

  for (int a = 0; a < 3; a++) {
    ASSERT_EQ(emev_v1.total_forward_active_energy[a],
              emev_v2.total_forward_active_energy[a]);
    ASSERT_EQ(emev_v1.total_reverse_active_energy[a],
              emev_v2.total_reverse_active_energy[a]);
    ASSERT_EQ(emev_v1.total_forward_reactive_energy[a],
              emev_v2.total_forward_reactive_energy[a]);
    ASSERT_EQ(emev_v1.total_reverse_reactive_energy[a],
              emev_v2.total_reverse_reactive_energy[a]);
  }

  emev_v1.measured_values ^=
      emev_v1.measured_values & EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED;
  emev_v1.measured_values ^=
      emev_v1.measured_values & EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED;

  ASSERT_EQ(emev_v2.total_forward_active_energy_balanced, (unsigned long)0);
  ASSERT_EQ(emev_v2.total_reverse_active_energy_balanced, (unsigned long)0);

  ASSERT_EQ(emev_v1.total_cost, emev_v2.total_cost);
  ASSERT_EQ(emev_v2.total_cost_balanced, 0);
  ASSERT_EQ(emev_v1.price_per_unit, emev_v2.price_per_unit);
  ASSERT_EQ(
      memcmp(emev_v1.currency, emev_v2.currency, sizeof(emev_v1.currency)), 0);
  ASSERT_EQ(emev_v1.measured_values, emev_v2.measured_values);
  ASSERT_EQ(emev_v1.period, emev_v2.period);
  ASSERT_EQ(emev_v1.m_count, emev_v2.m_count);
  ASSERT_EQ(memcmp(emev_v1.m, emev_v2.m, sizeof(emev_v1.m)), 0);
}

TEST_F(SrpcTest, srpc_evtool_emev_v2to1) {
  DECLARE_WITH_RANDOM(TElectricityMeter_ExtendedValue_V2, emev_v2);
  TElectricityMeter_ExtendedValue emev_v1;
  memset(&emev_v1, 0, sizeof(TElectricityMeter_ExtendedValue));

  emev_v2.measured_values |= EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED;
  emev_v2.measured_values |= EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED;

  ASSERT_EQ(srpc_evtool_emev_v2to1(NULL, &emev_v1), 0);
  ASSERT_EQ(srpc_evtool_emev_v2to1(&emev_v2, NULL), 0);
  ASSERT_EQ(srpc_evtool_emev_v2to1(&emev_v2, &emev_v1), 1);

  for (int a = 0; a < 3; a++) {
    ASSERT_EQ(emev_v1.total_forward_active_energy[a],
              emev_v2.total_forward_active_energy[a]);
    ASSERT_EQ(emev_v1.total_reverse_active_energy[a],
              emev_v2.total_reverse_active_energy[a]);
    ASSERT_EQ(emev_v1.total_forward_reactive_energy[a],
              emev_v2.total_forward_reactive_energy[a]);
    ASSERT_EQ(emev_v1.total_reverse_reactive_energy[a],
              emev_v2.total_reverse_reactive_energy[a]);
  }

  emev_v2.measured_values ^= EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED;
  emev_v2.measured_values ^= EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED;

  ASSERT_EQ(emev_v1.total_cost, emev_v2.total_cost);
  ASSERT_EQ(emev_v1.price_per_unit, emev_v2.price_per_unit);
  ASSERT_EQ(
      memcmp(emev_v1.currency, emev_v2.currency, sizeof(emev_v1.currency)), 0);
  ASSERT_EQ(emev_v1.measured_values, emev_v2.measured_values);
  ASSERT_EQ(emev_v1.period, emev_v2.period);
  ASSERT_EQ(emev_v1.m_count, emev_v2.m_count);
  ASSERT_EQ(memcmp(emev_v1.m, emev_v2.m, sizeof(emev_v1.m)), 0);
}

TEST_F(SrpcTest, evtool_input_counter_value_to_extended) {
  ASSERT_GE((unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE,
            sizeof(TSC_ImpulseCounter_ExtendedValue));

  TSuplaChannelExtendedValue ev;
  DECLARE_WITH_RANDOM(TSC_ImpulseCounter_ExtendedValue, ic_ev_src);
  TSC_ImpulseCounter_ExtendedValue ic_ev_dst;

  memset(&ic_ev_dst, 0, sizeof(TSC_ImpulseCounter_ExtendedValue));

  ASSERT_EQ(1, srpc_evtool_v1_icextended2extended(&ic_ev_src, &ev));
  ASSERT_EQ(1, srpc_evtool_v1_extended2icextended(&ev, &ic_ev_dst));

  ASSERT_EQ(0, memcmp(&ic_ev_src, &ic_ev_dst,
                      sizeof(TSC_ImpulseCounter_ExtendedValue)));

  ASSERT_EQ(1, srpc_evtool_v1_icextended2extended(&ic_ev_src, &ev));
  ASSERT_EQ(0, srpc_evtool_v1_icextended2extended(NULL, &ev));
  ASSERT_EQ(0, srpc_evtool_v1_icextended2extended(&ic_ev_src, NULL));

  ev.type = 0;
  ASSERT_EQ(0, srpc_evtool_v1_extended2icextended(&ev, &ic_ev_dst));

  ev.type = EV_TYPE_IMPULSE_COUNTER_DETAILS_V1;
  ev.size = 0;
  ASSERT_EQ(0, srpc_evtool_v1_extended2icextended(&ev, &ic_ev_dst));

  ev.size = sizeof(TSC_ImpulseCounter_ExtendedValue) + 1;
  ASSERT_EQ(0, srpc_evtool_v1_extended2icextended(&ev, &ic_ev_dst));

  ev.size = sizeof(TSC_ImpulseCounter_ExtendedValue) - 1;
  ASSERT_EQ(0, srpc_evtool_v1_extended2icextended(&ev, &ic_ev_dst));
}

TEST_F(SrpcTest, evtool_v1_extended2thermostatextended) {
  TThermostat_ExtendedValue th_ev;
  DECLARE_WITH_RANDOM(TSuplaChannelExtendedValue, ev);
  ev.type = EV_TYPE_THERMOSTAT_DETAILS_V1;

  ASSERT_EQ(0, srpc_evtool_v1_extended2thermostatextended(NULL, NULL));
  ASSERT_EQ(0, srpc_evtool_v1_extended2thermostatextended(&ev, NULL));
  ASSERT_EQ(0, srpc_evtool_v1_extended2thermostatextended(NULL, &th_ev));

  ev.size = 0;
  ASSERT_EQ(0, srpc_evtool_v1_extended2thermostatextended(&ev, &th_ev));
  ev.size = sizeof(TThermostat_ExtendedValue) + 1;
  ASSERT_EQ(0, srpc_evtool_v1_extended2thermostatextended(&ev, &th_ev));

  ev.size = sizeof(TThermostat_ExtendedValue);
  ASSERT_EQ(1, srpc_evtool_v1_extended2thermostatextended(&ev, &th_ev));
  ASSERT_EQ(0, memcmp(&th_ev, ev.value, sizeof(TThermostat_ExtendedValue)));
}

TEST_F(SrpcTest, evtool_v1_thermostatextended2extended) {
  TSuplaChannelExtendedValue ev;
  DECLARE_WITH_RANDOM(TThermostat_ExtendedValue, th_ev);
  th_ev.Fields = 0;
  ASSERT_EQ(0, srpc_evtool_v1_thermostatextended2extended(NULL, NULL));
  ASSERT_EQ(0, srpc_evtool_v1_thermostatextended2extended(&th_ev, NULL));
  ASSERT_EQ(0, srpc_evtool_v1_thermostatextended2extended(NULL, &ev));

  ASSERT_EQ(1, srpc_evtool_v1_thermostatextended2extended(&th_ev, &ev));
  ASSERT_EQ((unsigned int)1, ev.size);

  th_ev.Fields = THERMOSTAT_FIELD_MeasuredTemperatures;
  ASSERT_EQ(1, srpc_evtool_v1_thermostatextended2extended(&th_ev, &ev));
  ASSERT_EQ((unsigned int)21, ev.size);

  th_ev.Fields = THERMOSTAT_FIELD_PresetTemperatures;
  ASSERT_EQ(1, srpc_evtool_v1_thermostatextended2extended(&th_ev, &ev));
  ASSERT_EQ((unsigned int)41, ev.size);

  th_ev.Fields = THERMOSTAT_FIELD_Flags;
  ASSERT_EQ(1, srpc_evtool_v1_thermostatextended2extended(&th_ev, &ev));
  ASSERT_EQ((unsigned int)57, ev.size);

  th_ev.Fields = THERMOSTAT_FIELD_Values;
  ASSERT_EQ(1, srpc_evtool_v1_thermostatextended2extended(&th_ev, &ev));
  ASSERT_EQ((unsigned int)73, ev.size);

  th_ev.Fields = THERMOSTAT_FIELD_Time;
  ASSERT_EQ(1, srpc_evtool_v1_thermostatextended2extended(&th_ev, &ev));
  ASSERT_EQ((unsigned int)77, ev.size);

  th_ev.Fields = THERMOSTAT_FIELD_Schedule;
  ASSERT_EQ(1, srpc_evtool_v1_thermostatextended2extended(&th_ev, &ev));
  ASSERT_EQ((unsigned int)246, ev.size);

  ASSERT_EQ(0, memcmp(ev.value, &th_ev, sizeof(TThermostat_ExtendedValue)));
}

//---------------------------------------------------------
// GET USER LOCALTIME
//---------------------------------------------------------

SRPC_CALL_WITH_NO_DATA(srpc_dcs_async_get_user_localtime,
                       SUPLA_DCS_CALL_GET_USER_LOCALTIME);

TEST_F(SrpcTest, call_get_user_localtime_result) {
  DECLARE_WITH_RANDOM(TSDC_UserLocalTimeResult, result);

  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_EQ(srpc_sdc_async_get_user_localtime_result(srpc, NULL), 0);

  result.timezoneSize = -1;
  ASSERT_EQ(srpc_sdc_async_get_user_localtime_result(srpc, &result), 0);

  result.timezoneSize = SUPLA_TIMEZONE_MAXSIZE + 1;
  ASSERT_EQ(srpc_sdc_async_get_user_localtime_result(srpc, &result), 0);

  result.timezoneSize = 0;
  ASSERT_GT(srpc_sdc_async_get_user_localtime_result(srpc, &result), 0);
  SendAndReceive(SUPLA_DCS_CALL_GET_USER_LOCALTIME_RESULT, 35);

  ASSERT_FALSE(cr_rd.data.sdc_user_localtime_result == NULL);

  free(cr_rd.data.sdc_user_localtime_result);
  srpc_free(srpc);
  srpc = NULL;
}

TEST_F(SrpcTest, call_get_user_localtime_result_with_timezone_maxsize) {
  DECLARE_WITH_RANDOM(TSDC_UserLocalTimeResult, result);

  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  result.timezoneSize = SUPLA_TIMEZONE_MAXSIZE;
  ASSERT_GT(srpc_sdc_async_get_user_localtime_result(srpc, &result), 0);
  SendAndReceive(SUPLA_DCS_CALL_GET_USER_LOCALTIME_RESULT, 86);

  ASSERT_FALSE(cr_rd.data.sdc_user_localtime_result == NULL);

  ASSERT_EQ(memcmp(cr_rd.data.sdc_user_localtime_result, &result,
                   sizeof(TSDC_UserLocalTimeResult)),
            0);

  free(cr_rd.data.sdc_user_localtime_result);
  srpc_free(srpc);
  srpc = NULL;
}

//---------------------------------------------------------
// GET CHANNEL STATE
//---------------------------------------------------------

SRPC_CALL_BASIC_TEST(srpc_csd_async_get_channel_state, TCSD_ChannelStateRequest,
                     SUPLA_CSD_CALL_GET_CHANNEL_STATE, 31,
                     csd_channel_state_request);

SRPC_CALL_BASIC_TEST(srpc_csd_async_channel_state_result, TDSC_ChannelState,
                     SUPLA_DSC_CALL_CHANNEL_STATE_RESULT, 73,
                     dsc_channel_state);

//---------------------------------------------------------
// GET CHANNEL BESIC CONFIGURATION
//---------------------------------------------------------
TEST_F(SrpcTest, call_cs_async_get_channel_basic_cfg) {
  data_read_result = -1;
  srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);

  ASSERT_GT(srpc_cs_async_get_channel_basic_cfg(srpc, 55), 0);
  SendAndReceive(SUPLA_CS_CALL_GET_CHANNEL_BASIC_CFG, 27);

  ASSERT_FALSE(cr_rd.data.cs_channel_basic_cfg_request == NULL);

  ASSERT_EQ(55, cr_rd.data.cs_channel_basic_cfg_request->ChannelID);

  free(cr_rd.data.cs_channel_basic_cfg_request);
  srpc_free(srpc);
  srpc = NULL;
}

SRPC_CALL_BASIC_TEST_WITH_CAPTION(srpc_sc_async_channel_basic_cfg_result,
                                  TSC_ChannelBasicCfg,
                                  SUPLA_SC_CALL_CHANNEL_BASIC_CFG_RESULT, 282,
                                  683, sc_channel_basic_cfg,
                                  SUPLA_CHANNEL_CAPTION_MAXSIZE);

//---------------------------------------------------------
// SET CHANNEL FUNCTION
//---------------------------------------------------------

SRPC_CALL_BASIC_TEST(srpc_cs_async_set_channel_function, TCS_SetChannelFunction,
                     SUPLA_CS_CALL_SET_CHANNEL_FUNCTION, 31,
                     cs_set_channel_function);

SRPC_CALL_BASIC_TEST(srpc_sc_async_set_channel_function_result,
                     TSC_SetChannelFunctionResult,
                     SUPLA_SC_CALL_SET_CHANNEL_FUNCTION_RESULT, 32,
                     sc_set_channel_function_result);

//---------------------------------------------------------
// SET CHANNEL CAPTION
//---------------------------------------------------------

SRPC_CALL_BASIC_TEST_WITH_CAPTION(srpc_cs_async_set_channel_caption,
                                  TCS_SetChannelCaption,
                                  SUPLA_CS_CALL_SET_CHANNEL_CAPTION, 31, 432,
                                  cs_set_channel_caption,
                                  SUPLA_CHANNEL_CAPTION_MAXSIZE);

SRPC_CALL_BASIC_TEST_WITH_CAPTION(srpc_sc_async_set_channel_caption_result,
                                  TSC_SetChannelCaptionResult,
                                  SUPLA_SC_CALL_SET_CHANNEL_CAPTION_RESULT, 32,
                                  433, sc_set_channel_caption_result,
                                  SUPLA_CHANNEL_CAPTION_MAXSIZE);

//---------------------------------------------------------
// CLIENTS RECONNECT REQUEST
//---------------------------------------------------------

SRPC_CALL_WITH_NO_DATA(srpc_cs_async_clients_reconnect_request,
                       SUPLA_CS_CALL_CLIENTS_RECONNECT_REQUEST);

SRPC_CALL_BASIC_TEST(srpc_sc_async_clients_reconnect_request_result,
                     TSC_ClientsReconnectRequestResult,
                     SUPLA_SC_CALL_CLIENTS_RECONNECT_REQUEST_RESULT, 24,
                     sc_clients_reconnect_result);

//---------------------------------------------------------
// SET REGISTRATION ENABLED
//---------------------------------------------------------

SRPC_CALL_BASIC_TEST(srpc_cs_async_set_registration_enabled,
                     TCS_SetRegistrationEnabled,
                     SUPLA_CS_CALL_SET_REGISTRATION_ENABLED, 31,
                     cs_set_registration_enabled);

SRPC_CALL_BASIC_TEST(srpc_sc_async_set_registration_enabled_result,
                     TSC_SetRegistrationEnabledResult,
                     SUPLA_SC_CALL_SET_REGISTRATION_ENABLED_RESULT, 24,
                     sc_set_registration_enabled_result);

//---------------------------------------------------------
// DEVICE RECONNECT REQUEST
//---------------------------------------------------------

SRPC_CALL_BASIC_TEST(srpc_cs_async_device_reconnect_request,
                     TCS_DeviceReconnectRequest,
                     SUPLA_CS_CALL_DEVICE_RECONNECT_REQUEST, 27,
                     cs_device_reconnect_request);

SRPC_CALL_BASIC_TEST(srpc_sc_async_device_reconnect_request_result,
                     TSC_DeviceReconnectRequestResult,
                     SUPLA_SC_CALL_DEVICE_RECONNECT_REQUEST_RESULT, 28,
                     sc_device_reconnect_request_result);

//---------------------------------------------------------
// GET CHANNEL FUNCTIONS
//---------------------------------------------------------

SRPC_CALL_WITH_NO_DATA(srpc_ds_async_get_channel_functions,
                       SUPLA_DS_CALL_GET_CHANNEL_FUNCTIONS);

SRPC_CALL_BASIC_TEST_WITH_SIZE_PARAM(srpc_sd_async_get_channel_functions_result,
                                     TSD_ChannelFunctions,
                                     SUPLA_SD_CALL_GET_CHANNEL_FUNCTIONS_RESULT,
                                     24, 536, sd_channel_functions,
                                     SUPLA_CHANNELMAXCOUNT, Functions,
                                     ChannelCount);

}  // namespace
