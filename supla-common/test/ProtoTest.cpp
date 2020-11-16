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

#include "ProtoTest.h"
#include "gtest/gtest.h"  // NOLINT
#include "proto.h"

#define BUFFER_MAX_SIZE 131072

namespace {

class ProtoTest : public ::testing::Test {
 protected:
};

TEST_F(ProtoTest, check_size_of_structures_and_types) {
  ASSERT_EQ((unsigned int)2, sizeof(_supla_int16_t));
  ASSERT_EQ((unsigned int)4, sizeof(_supla_int_t));
  ASSERT_EQ((unsigned int)8, sizeof(_supla_int64_t));
  ASSERT_EQ((unsigned int)16, sizeof(struct _supla_timeval));
  ASSERT_EQ((unsigned int)10258, sizeof(TSuplaDataPacket));
  ASSERT_EQ((unsigned int)23, sizeof(TSDC_SuplaGetVersionResult));
  ASSERT_EQ((unsigned int)2, sizeof(TSDC_SuplaVersionError));
  ASSERT_EQ((unsigned int)16, sizeof(TDCS_SuplaPingServer));
  ASSERT_EQ((unsigned int)8, sizeof(TDCS_SuplaPingServer_COMPAT));
  ASSERT_EQ((unsigned int)16, sizeof(TSDC_SuplaPingServerResult));
  ASSERT_EQ((unsigned int)1, sizeof(TDCS_SuplaSetActivityTimeout));
  ASSERT_EQ((unsigned int)3, sizeof(TSDC_SuplaSetActivityTimeoutResult));
  ASSERT_EQ((unsigned int)16, sizeof(TSuplaChannelValue));
  ASSERT_EQ((unsigned int)1029, sizeof(TSuplaChannelExtendedValue));
  ASSERT_EQ((unsigned int)410, sizeof(TSC_SuplaLocation));
  ASSERT_EQ((unsigned int)8208, sizeof(TSC_SuplaLocationPack));
  ASSERT_EQ((unsigned int)13, sizeof(TDS_SuplaDeviceChannel));
  ASSERT_EQ((unsigned int)1940, sizeof(TDS_SuplaRegisterDevice));
  ASSERT_EQ((unsigned int)21, sizeof(TDS_SuplaDeviceChannel_B));
  ASSERT_EQ((unsigned int)25, sizeof(TDS_SuplaDeviceChannel_C));
  ASSERT_EQ((unsigned int)2964, sizeof(TDS_SuplaRegisterDevice_B));
  ASSERT_EQ((unsigned int)3029, sizeof(TDS_SuplaRegisterDevice_C));
  ASSERT_EQ((unsigned int)3264, sizeof(TDS_SuplaRegisterDevice_D));
  ASSERT_EQ((unsigned int)3784, sizeof(TDS_SuplaRegisterDevice_E));
  ASSERT_EQ((unsigned int)7, sizeof(TSD_SuplaRegisterDeviceResult));
  ASSERT_EQ((unsigned int)9, sizeof(TDS_SuplaDeviceChannelValue));
  ASSERT_EQ((unsigned int)10, sizeof(TDS_SuplaDeviceChannelValue_B));
  ASSERT_EQ((unsigned int)14, sizeof(TDS_SuplaDeviceChannelValue_C));
  ASSERT_EQ((unsigned int)1030, sizeof(TDS_SuplaDeviceChannelExtendedValue));
  ASSERT_EQ((unsigned int)17, sizeof(TSD_SuplaChannelNewValue));
  ASSERT_EQ((unsigned int)22, sizeof(TSD_SuplaChannelGroupNewValue));
  ASSERT_EQ((unsigned int)6, sizeof(TDS_SuplaChannelNewValueResult));
  ASSERT_EQ((unsigned int)22, sizeof(TSC_SuplaChannelValue));
  ASSERT_EQ((unsigned int)1033, sizeof(TSC_SuplaChannelExtendedValue));
  ASSERT_EQ((unsigned int)448, sizeof(TSC_SuplaChannelValuePack));
  ASSERT_EQ((unsigned int)10202, sizeof(TSC_SuplaChannelExtendedValuePack));
  ASSERT_EQ((unsigned int)435, sizeof(TSC_SuplaChannel));
  ASSERT_EQ((unsigned int)8708, sizeof(TSC_SuplaChannelPack));
  ASSERT_EQ((unsigned int)444, sizeof(TSC_SuplaChannel_B));
  ASSERT_EQ((unsigned int)8888, sizeof(TSC_SuplaChannelPack_B));
  ASSERT_EQ((unsigned int)460, sizeof(TSC_SuplaChannel_C));
  ASSERT_EQ((unsigned int)9208, sizeof(TSC_SuplaChannelPack_C));
  ASSERT_EQ((unsigned int)426, sizeof(TSC_SuplaChannelGroup));
  ASSERT_EQ((unsigned int)8528, sizeof(TSC_SuplaChannelGroupPack));
  ASSERT_EQ((unsigned int)430, sizeof(TSC_SuplaChannelGroup_B));
  ASSERT_EQ((unsigned int)8608, sizeof(TSC_SuplaChannelGroupPack_B));
  ASSERT_EQ((unsigned int)9, sizeof(TSC_SuplaChannelGroupRelation));
  ASSERT_EQ((unsigned int)908, sizeof(TSC_SuplaChannelGroupRelationPack));
  ASSERT_EQ((unsigned int)275, sizeof(TCS_SuplaRegisterClient));
  ASSERT_EQ((unsigned int)340, sizeof(TCS_SuplaRegisterClient_B));
  ASSERT_EQ((unsigned int)575, sizeof(TCS_SuplaRegisterClient_C));
  ASSERT_EQ((unsigned int)639, sizeof(TCS_SuplaRegisterClient_D));
  ASSERT_EQ((unsigned int)19, sizeof(TSC_SuplaRegisterClientResult));
  ASSERT_EQ((unsigned int)27, sizeof(TSC_SuplaRegisterClientResult_B));
  ASSERT_EQ((unsigned int)9, sizeof(TCS_SuplaChannelNewValue));
  ASSERT_EQ((unsigned int)12, sizeof(TCS_SuplaChannelNewValue_B));
  ASSERT_EQ((unsigned int)13, sizeof(TCS_SuplaNewValue));
  ASSERT_EQ((unsigned int)221, sizeof(TSC_SuplaEvent));
  ASSERT_EQ((unsigned int)17, sizeof(TDS_FirmwareUpdateParams));
  ASSERT_EQ((unsigned int)207, sizeof(TSD_FirmwareUpdate_Url));
  ASSERT_EQ((unsigned int)208, sizeof(TSD_FirmwareUpdate_UrlResult));
  ASSERT_EQ((unsigned int)8, sizeof(TSDC_RegistrationEnabled));
  ASSERT_EQ((unsigned int)264, sizeof(TSC_OAuthToken));
  ASSERT_EQ((unsigned int)265, sizeof(TSC_OAuthTokenRequestResult));
  ASSERT_EQ((unsigned int)62, sizeof(TElectricityMeter_Measurement));
  ASSERT_EQ((unsigned int)429, sizeof(TElectricityMeter_ExtendedValue));
  ASSERT_EQ((unsigned int)449, sizeof(TElectricityMeter_ExtendedValue_V2));
  ASSERT_EQ((unsigned int)5, sizeof(TElectricityMeter_Value));
  ASSERT_EQ((unsigned int)40, sizeof(TSC_ImpulseCounter_ExtendedValue));
  ASSERT_EQ((unsigned int)8, sizeof(TSC_ImpulseCounter_Value));
  ASSERT_EQ((unsigned int)320, sizeof(TCS_SuperUserAuthorizationRequest));
  ASSERT_EQ((unsigned int)144, sizeof(TCS_DeviceCalCfgRequest));
  ASSERT_EQ((unsigned int)145, sizeof(TCS_DeviceCalCfgRequest_B));
  ASSERT_EQ((unsigned int)144, sizeof(TSC_DeviceCalCfgResult));
  ASSERT_EQ((unsigned int)149, sizeof(TSD_DeviceCalCfgRequest));
  ASSERT_EQ((unsigned int)148, sizeof(TDS_DeviceCalCfgResult));

  ASSERT_LE(sizeof(TDS_ImpulseCounter_Value),
            (unsigned int)SUPLA_CHANNELVALUE_SIZE);
  ASSERT_LE(sizeof(TSC_ImpulseCounter_ExtendedValue),
            (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);

  ASSERT_LE(sizeof(TElectricityMeter_Value),
            (unsigned int)SUPLA_CHANNELVALUE_SIZE);
  ASSERT_LE(sizeof(TElectricityMeter_ExtendedValue),
            (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);
  ASSERT_LE(sizeof(TElectricityMeter_ExtendedValue_V2),
            (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);

  ASSERT_EQ((unsigned int)4, sizeof(TThermostat_Time));
  ASSERT_EQ((unsigned int)246, sizeof(TThermostat_ExtendedValue));
  ASSERT_EQ((unsigned int)6, sizeof(TThermostat_Value));
  ASSERT_EQ((unsigned int)169, sizeof(TThermostat_Schedule));
  ASSERT_EQ((unsigned int)104, sizeof(TThermostat_ScheduleCfg));
  ASSERT_EQ((unsigned int)22, sizeof(TThermostatTemperatureCfg));

  ASSERT_LE(sizeof(TThermostat_ExtendedValue),
            (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);
  ASSERT_LE(sizeof(TThermostat_Value), (unsigned int)SUPLA_CHANNELVALUE_SIZE);
  ASSERT_LE(sizeof(TThermostat_ScheduleCfg),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
  ASSERT_LE(sizeof(TThermostatTemperatureCfg),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);

  ASSERT_EQ((unsigned int)63, sizeof(TSDC_UserLocalTimeResult));

  ASSERT_EQ((unsigned int)50, sizeof(TDSC_ChannelState));
  ASSERT_EQ((unsigned int)8, sizeof(TCSD_ChannelStateRequest));
  ASSERT_EQ((unsigned int)8, sizeof(TCS_SetChannelFunction));
  ASSERT_EQ((unsigned int)9, sizeof(TSC_SetChannelFunctionResult));
  ASSERT_EQ((unsigned int)660, sizeof(TSC_ChannelBasicCfg));
  ASSERT_EQ((unsigned int)4, sizeof(TCS_ChannelBasicCfgRequest));
  ASSERT_EQ((unsigned int)1, sizeof(TSC_ClientsReconnectRequestResult));
  ASSERT_EQ((unsigned int)8, sizeof(TCS_SetRegistrationEnabled));
  ASSERT_EQ((unsigned int)1, sizeof(TSC_SetRegistrationEnabledResult));
  ASSERT_EQ((unsigned int)4, sizeof(TCS_DeviceReconnectRequest));
  ASSERT_EQ((unsigned int)5, sizeof(TSC_DeviceReconnectRequestResult));
  ASSERT_EQ((unsigned int)409, sizeof(TCS_SetChannelCaption));
  ASSERT_EQ((unsigned int)410, sizeof(TSC_SetChannelCaptionResult));
  ASSERT_EQ((unsigned int)513, sizeof(TSD_ChannelFunctions));
  ASSERT_EQ((unsigned int)58, sizeof(TCalCfg_ZWave_Node));
  ASSERT_LE(sizeof(TCalCfg_ZWave_Node),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
  ASSERT_LE(sizeof(unsigned char), (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
  ASSERT_LE(sizeof(TCalCfg_ProgressReport),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);

  ASSERT_EQ((unsigned int)221, sizeof(TTimerState_ExtendedValue));
  ASSERT_EQ((unsigned int)271, sizeof(TChannelAndTimerState_ExtendedValue));

  ASSERT_LE(sizeof(TTimerState_ExtendedValue),
            (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);
  ASSERT_LE(sizeof(TChannelAndTimerState_ExtendedValue),
            (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);

  ASSERT_EQ((unsigned int)4, sizeof(TCalCfg_LightSourceLifespan));
  ASSERT_LE(sizeof(TCalCfg_LightSourceLifespan),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);

  ASSERT_EQ((unsigned int)4, sizeof(TDigiglass_Value));

  ASSERT_EQ((unsigned int)12, sizeof(TCalCfg_ZWave_WakeupSettingsReport));
  ASSERT_LE(sizeof(TCalCfg_ZWave_WakeupSettingsReport),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);

  ASSERT_EQ((unsigned int)3, sizeof(TCalCfg_ZWave_WakeUpTime));
  ASSERT_LE(sizeof(TCalCfg_ZWave_WakeUpTime),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
}

TEST_F(ProtoTest, init) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);
  ASSERT_EQ(SUPLA_PROTO_VERSION, sproto_get_version(sproto));
  sproto_free(sproto);
}

TEST_F(ProtoTest, set_version) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  sproto_set_version(sproto, SUPLA_PROTO_VERSION_MIN - 1);
  ASSERT_EQ(SUPLA_PROTO_VERSION, sproto_get_version(sproto));

  sproto_set_version(sproto, SUPLA_PROTO_VERSION + 1);
  ASSERT_EQ(SUPLA_PROTO_VERSION, sproto_get_version(sproto));

  sproto_set_version(sproto, SUPLA_PROTO_VERSION - 1);
  ASSERT_EQ(SUPLA_PROTO_VERSION - 1, sproto_get_version(sproto));

  sproto_free(sproto);
}

TEST_F(ProtoTest, out_dataexists) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  ASSERT_EQ(SUPLA_RESULT_FALSE, sproto_out_dataexists(sproto));

  TSuplaDataPacket sdp;
  sproto_sdp_init(sproto, &sdp);
  ASSERT_EQ(SUPLA_RESULT_TRUE, sproto_out_buffer_append(sproto, &sdp));

  ASSERT_EQ(SUPLA_RESULT_TRUE, sproto_out_dataexists(sproto));

  sproto_free(sproto);
}

TEST_F(ProtoTest, in_dataexists) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  ASSERT_EQ(SUPLA_RESULT_FALSE, sproto_in_dataexists(sproto));
  char c = 'S';

  ASSERT_EQ(SUPLA_RESULT_TRUE, sproto_in_buffer_append(sproto, &c, 1));

  ASSERT_EQ(SUPLA_RESULT_TRUE, sproto_in_dataexists(sproto));

  sproto_free(sproto);
}

TEST_F(ProtoTest, in_buffer_append) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  char data[BUFFER_MAX_SIZE];
  memset(data, 123, BUFFER_MAX_SIZE);

  ASSERT_EQ(SUPLA_RESULT_BUFFER_OVERFLOW,
            sproto_in_buffer_append(sproto, data, BUFFER_MAX_SIZE));

  ASSERT_EQ(SUPLA_RESULT_FALSE, sproto_in_dataexists(sproto));

  ASSERT_EQ(SUPLA_RESULT_BUFFER_OVERFLOW,
            sproto_in_buffer_append(sproto, data, BUFFER_MAX_SIZE + 1));

  ASSERT_EQ(SUPLA_RESULT_FALSE, sproto_in_dataexists(sproto));

  ASSERT_EQ(SUPLA_RESULT_TRUE,
            sproto_in_buffer_append(sproto, data, BUFFER_MAX_SIZE - 1));

  ASSERT_EQ(SUPLA_RESULT_TRUE, sproto_in_dataexists(sproto));

  sproto_free(sproto);
}

TEST_F(ProtoTest, pop_in_sdp_test1) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  TSuplaDataPacket sdp;
  sproto_sdp_init(sproto, &sdp);

  ASSERT_EQ(
      SUPLA_RESULT_TRUE,
      sproto_in_buffer_append(sproto, (char *)&sdp,
                              sizeof(TSuplaDataPacket) - SUPLA_MAX_DATA_SIZE));

  ASSERT_EQ(SUPLA_RESULT_TRUE,
            sproto_in_buffer_append(sproto, sproto_tag, SUPLA_TAG_SIZE));

  TSuplaDataPacket sdp_rcv;
  memset(&sdp_rcv, 0, sizeof(TSuplaDataPacket));

  ASSERT_EQ(SUPLA_RESULT_TRUE, sproto_pop_in_sdp(sproto, &sdp_rcv));

  ASSERT_EQ(0, memcmp(&sdp_rcv, &sdp, sizeof(TSuplaDataPacket)));

  sproto_free(sproto);
}

TEST_F(ProtoTest, pop_in_sdp_test2) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  TSuplaDataPacket sdp;
  sproto_sdp_init(sproto, &sdp);

  sdp.tag[2] = 'X';

  ASSERT_EQ(
      SUPLA_RESULT_TRUE,
      sproto_in_buffer_append(sproto, (char *)&sdp,
                              sizeof(TSuplaDataPacket) - SUPLA_MAX_DATA_SIZE));

  ASSERT_EQ(SUPLA_RESULT_TRUE,
            sproto_in_buffer_append(sproto, sproto_tag, SUPLA_TAG_SIZE));

  TSuplaDataPacket sdp_rcv;
  ASSERT_EQ(SUPLA_RESULT_DATA_ERROR, sproto_pop_in_sdp(sproto, &sdp_rcv));

  sproto_free(sproto);
}

TEST_F(ProtoTest, pop_in_sdp_test3) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  TSuplaDataPacket sdp;
  sproto_sdp_init(sproto, &sdp);

  ASSERT_EQ(
      SUPLA_RESULT_TRUE,
      sproto_in_buffer_append(sproto, (char *)&sdp,
                              sizeof(TSuplaDataPacket) - SUPLA_MAX_DATA_SIZE));

  ASSERT_EQ(SUPLA_RESULT_TRUE,
            sproto_in_buffer_append(sproto, sproto_tag, SUPLA_TAG_SIZE - 1));

  char c[] = "x";

  ASSERT_EQ(SUPLA_RESULT_TRUE, sproto_in_buffer_append(sproto, c, 1));

  TSuplaDataPacket sdp_rcv;
  ASSERT_EQ(SUPLA_RESULT_DATA_ERROR, sproto_pop_in_sdp(sproto, &sdp_rcv));

  sproto_free(sproto);
}

TEST_F(ProtoTest, pop_in_sdp_test4) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  TSuplaDataPacket sdp;
  sproto_sdp_init(sproto, &sdp);

  sdp.data_size = 1;

  ASSERT_EQ(
      SUPLA_RESULT_TRUE,
      sproto_in_buffer_append(sproto, (char *)&sdp,
                              sizeof(TSuplaDataPacket) - SUPLA_MAX_DATA_SIZE));

  ASSERT_EQ(SUPLA_RESULT_TRUE,
            sproto_in_buffer_append(sproto, sproto_tag, SUPLA_TAG_SIZE));

  TSuplaDataPacket sdp_rcv;
  ASSERT_EQ(SUPLA_RESULT_FALSE, sproto_pop_in_sdp(sproto, &sdp_rcv));

  sproto_free(sproto);
}

TEST_F(ProtoTest, pop_in_sdp_test5) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  TSuplaDataPacket sdp;
  sproto_sdp_init(sproto, &sdp);

  ASSERT_EQ(SUPLA_RESULT_TRUE,
            sproto_in_buffer_append(
                sproto, (char *)&sdp,
                sizeof(TSuplaDataPacket) - SUPLA_MAX_DATA_SIZE + 1));

  ASSERT_EQ(SUPLA_RESULT_TRUE,
            sproto_in_buffer_append(sproto, sproto_tag, SUPLA_TAG_SIZE));

  TSuplaDataPacket sdp_rcv;
  ASSERT_EQ(SUPLA_RESULT_DATA_ERROR, sproto_pop_in_sdp(sproto, &sdp_rcv));

  sproto_free(sproto);
}

TEST_F(ProtoTest, pop_in_sdp_test6) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  TSuplaDataPacket sdp;
  sproto_sdp_init(sproto, &sdp);

  sdp.data_size = SUPLA_MAX_DATA_SIZE + 1;

  ASSERT_EQ(
      SUPLA_RESULT_TRUE,
      sproto_in_buffer_append(sproto, (char *)&sdp, sizeof(TSuplaDataPacket)));

  char c[] = "x";

  ASSERT_EQ(SUPLA_RESULT_TRUE, sproto_in_buffer_append(sproto, c, 1));

  ASSERT_EQ(SUPLA_RESULT_TRUE,
            sproto_in_buffer_append(sproto, sproto_tag, SUPLA_TAG_SIZE));

  TSuplaDataPacket sdp_rcv;
  ASSERT_EQ(SUPLA_RESULT_DATA_ERROR, sproto_pop_in_sdp(sproto, &sdp_rcv));

  sproto_free(sproto);
}

TEST_F(ProtoTest, pop_in_sdp_test7) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  TSuplaDataPacket sdp;
  sproto_sdp_init(sproto, &sdp);

  sdp.version = SUPLA_PROTO_VERSION + 1;

  ASSERT_EQ(
      SUPLA_RESULT_TRUE,
      sproto_in_buffer_append(sproto, (char *)&sdp,
                              sizeof(TSuplaDataPacket) - SUPLA_MAX_DATA_SIZE));

  ASSERT_EQ(SUPLA_RESULT_TRUE,
            sproto_in_buffer_append(sproto, sproto_tag, SUPLA_TAG_SIZE));

  TSuplaDataPacket sdp_rcv;
  ASSERT_EQ(SUPLA_RESULT_VERSION_ERROR, sproto_pop_in_sdp(sproto, &sdp_rcv));

  sproto_free(sproto);
}

TEST_F(ProtoTest, out_buffer_append_test1) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  TSuplaDataPacket sdp;
  sproto_sdp_init(sproto, &sdp);

  ASSERT_EQ(SUPLA_RESULT_TRUE, sproto_out_buffer_append(sproto, &sdp));

  sproto_free(sproto);
}

TEST_F(ProtoTest, out_buffer_append_test2) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  TSuplaDataPacket sdp;
  sproto_sdp_init(sproto, &sdp);
  sdp.data_size = SUPLA_MAX_DATA_SIZE;

  ASSERT_EQ(SUPLA_RESULT_TRUE, sproto_out_buffer_append(sproto, &sdp));

  sproto_free(sproto);
}

TEST_F(ProtoTest, out_buffer_append_test3) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  TSuplaDataPacket sdp;
  sproto_sdp_init(sproto, &sdp);
  sdp.data_size = SUPLA_MAX_DATA_SIZE + 1;

  ASSERT_EQ(SUPLA_RESULT_DATA_TOO_LARGE,
            sproto_out_buffer_append(sproto, &sdp));

  sproto_free(sproto);
}

TEST_F(ProtoTest, pop_out_data) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  TSuplaDataPacket sdp;
  sproto_sdp_init(sproto, &sdp);
  sdp.data_size = 100;

  for (int a = 0; a < 100; a++) {
    sdp.data[a] = a;
  }

  ASSERT_EQ(SUPLA_RESULT_TRUE, sproto_out_buffer_append(sproto, &sdp));

  unsigned int expected_size = sizeof(TSuplaDataPacket) - SUPLA_MAX_DATA_SIZE +
                               sdp.data_size + SUPLA_TAG_SIZE;

  char *buffer = (char *)malloc(expected_size + 10);

  ASSERT_EQ(expected_size,
            sproto_pop_out_data(sproto, buffer, expected_size + 10));

  ASSERT_EQ(0, memcmp(buffer, &sdp, expected_size - SUPLA_TAG_SIZE));
  ASSERT_EQ(0, memcmp(&buffer[expected_size - SUPLA_TAG_SIZE],
                      (void *)sproto_tag, SUPLA_TAG_SIZE));

  free(buffer);

  sproto_free(sproto);
}

}  // namespace
