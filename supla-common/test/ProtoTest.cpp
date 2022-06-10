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
  EXPECT_EQ((unsigned int)2, sizeof(_supla_int16_t));
  EXPECT_EQ((unsigned int)4, sizeof(_supla_int_t));
  EXPECT_EQ((unsigned int)8, sizeof(_supla_int64_t));
  EXPECT_EQ((unsigned int)16, sizeof(struct _supla_timeval));
  EXPECT_EQ((unsigned int)10258, sizeof(TSuplaDataPacket));
  EXPECT_EQ((unsigned int)23, sizeof(TSDC_SuplaGetVersionResult));
  EXPECT_EQ((unsigned int)2, sizeof(TSDC_SuplaVersionError));
  EXPECT_EQ((unsigned int)16, sizeof(TDCS_SuplaPingServer));
  EXPECT_EQ((unsigned int)8, sizeof(TDCS_SuplaPingServer_COMPAT));
  EXPECT_EQ((unsigned int)16, sizeof(TSDC_SuplaPingServerResult));
  EXPECT_EQ((unsigned int)1, sizeof(TDCS_SuplaSetActivityTimeout));
  EXPECT_EQ((unsigned int)3, sizeof(TSDC_SuplaSetActivityTimeoutResult));
  EXPECT_EQ((unsigned int)16, sizeof(TSuplaChannelValue));
  EXPECT_EQ((unsigned int)17, sizeof(TSuplaChannelValue_B));
  EXPECT_EQ((unsigned int)1029, sizeof(TSuplaChannelExtendedValue));
  EXPECT_EQ((unsigned int)410, sizeof(TSC_SuplaLocation));
  EXPECT_EQ((unsigned int)8208, sizeof(TSC_SuplaLocationPack));
  EXPECT_EQ((unsigned int)13, sizeof(TDS_SuplaDeviceChannel));
  EXPECT_EQ((unsigned int)1940, sizeof(TDS_SuplaRegisterDevice));
  EXPECT_EQ((unsigned int)21, sizeof(TDS_SuplaDeviceChannel_B));
  EXPECT_EQ((unsigned int)25, sizeof(TDS_SuplaDeviceChannel_C));
  EXPECT_EQ((unsigned int)2964, sizeof(TDS_SuplaRegisterDevice_B));
  EXPECT_EQ((unsigned int)3029, sizeof(TDS_SuplaRegisterDevice_C));
  EXPECT_EQ((unsigned int)3264, sizeof(TDS_SuplaRegisterDevice_D));
  EXPECT_EQ((unsigned int)3784, sizeof(TDS_SuplaRegisterDevice_E));
  EXPECT_EQ((unsigned int)7, sizeof(TSD_SuplaRegisterDeviceResult));
  EXPECT_EQ((unsigned int)9, sizeof(TDS_SuplaDeviceChannelValue));
  EXPECT_EQ((unsigned int)10, sizeof(TDS_SuplaDeviceChannelValue_B));
  EXPECT_EQ((unsigned int)14, sizeof(TDS_SuplaDeviceChannelValue_C));
  EXPECT_EQ((unsigned int)1030, sizeof(TDS_SuplaDeviceChannelExtendedValue));
  EXPECT_EQ((unsigned int)17, sizeof(TSD_SuplaChannelNewValue));
  EXPECT_EQ((unsigned int)22, sizeof(TSD_SuplaChannelGroupNewValue));
  EXPECT_EQ((unsigned int)6, sizeof(TDS_SuplaChannelNewValueResult));
  EXPECT_EQ((unsigned int)22, sizeof(TSC_SuplaChannelValue));
  EXPECT_EQ((unsigned int)23, sizeof(TSC_SuplaChannelValue_B));
  EXPECT_EQ((unsigned int)1033, sizeof(TSC_SuplaChannelExtendedValue));
  EXPECT_EQ((unsigned int)448, sizeof(TSC_SuplaChannelValuePack));
  EXPECT_EQ((unsigned int)468, sizeof(TSC_SuplaChannelValuePack_B));
  EXPECT_EQ((unsigned int)10202, sizeof(TSC_SuplaChannelExtendedValuePack));
  EXPECT_EQ((unsigned int)435, sizeof(TSC_SuplaChannel));
  EXPECT_EQ((unsigned int)8708, sizeof(TSC_SuplaChannelPack));
  EXPECT_EQ((unsigned int)444, sizeof(TSC_SuplaChannel_B));
  EXPECT_EQ((unsigned int)8888, sizeof(TSC_SuplaChannelPack_B));
  EXPECT_EQ((unsigned int)460, sizeof(TSC_SuplaChannel_C));
  EXPECT_EQ((unsigned int)9208, sizeof(TSC_SuplaChannelPack_C));
  EXPECT_EQ((unsigned int)9228, sizeof(TSC_SuplaChannelPack_D));
  EXPECT_EQ((unsigned int)426, sizeof(TSC_SuplaChannelGroup));
  EXPECT_EQ((unsigned int)8528, sizeof(TSC_SuplaChannelGroupPack));
  EXPECT_EQ((unsigned int)430, sizeof(TSC_SuplaChannelGroup_B));
  EXPECT_EQ((unsigned int)8608, sizeof(TSC_SuplaChannelGroupPack_B));
  EXPECT_EQ((unsigned int)9, sizeof(TSC_SuplaChannelGroupRelation));
  EXPECT_EQ((unsigned int)908, sizeof(TSC_SuplaChannelGroupRelationPack));
  EXPECT_EQ((unsigned int)275, sizeof(TCS_SuplaRegisterClient));
  EXPECT_EQ((unsigned int)340, sizeof(TCS_SuplaRegisterClient_B));
  EXPECT_EQ((unsigned int)575, sizeof(TCS_SuplaRegisterClient_C));
  EXPECT_EQ((unsigned int)639, sizeof(TCS_SuplaRegisterClient_D));
  EXPECT_EQ((unsigned int)19, sizeof(TSC_SuplaRegisterClientResult));
  EXPECT_EQ((unsigned int)27, sizeof(TSC_SuplaRegisterClientResult_B));
  EXPECT_EQ((unsigned int)31, sizeof(TSC_SuplaRegisterClientResult_C));
  EXPECT_EQ((unsigned int)9, sizeof(TCS_SuplaChannelNewValue));
  EXPECT_EQ((unsigned int)12, sizeof(TCS_SuplaChannelNewValue_B));
  EXPECT_EQ((unsigned int)13, sizeof(TCS_SuplaNewValue));
  EXPECT_EQ((unsigned int)221, sizeof(TSC_SuplaEvent));
  EXPECT_EQ((unsigned int)17, sizeof(TDS_FirmwareUpdateParams));
  EXPECT_EQ((unsigned int)207, sizeof(TSD_FirmwareUpdate_Url));
  EXPECT_EQ((unsigned int)208, sizeof(TSD_FirmwareUpdate_UrlResult));
  EXPECT_EQ((unsigned int)8, sizeof(TSDC_RegistrationEnabled));
  EXPECT_EQ((unsigned int)264, sizeof(TSC_OAuthToken));
  EXPECT_EQ((unsigned int)265, sizeof(TSC_OAuthTokenRequestResult));
  EXPECT_EQ((unsigned int)62, sizeof(TElectricityMeter_Measurement));
  EXPECT_EQ((unsigned int)429, sizeof(TElectricityMeter_ExtendedValue));
  EXPECT_EQ((unsigned int)449, sizeof(TElectricityMeter_ExtendedValue_V2));
  EXPECT_EQ((unsigned int)5, sizeof(TElectricityMeter_Value));
  EXPECT_EQ((unsigned int)40, sizeof(TSC_ImpulseCounter_ExtendedValue));
  EXPECT_EQ((unsigned int)8, sizeof(TSC_ImpulseCounter_Value));
  EXPECT_EQ((unsigned int)320, sizeof(TCS_SuperUserAuthorizationRequest));
  EXPECT_EQ((unsigned int)144, sizeof(TCS_DeviceCalCfgRequest));
  EXPECT_EQ((unsigned int)145, sizeof(TCS_DeviceCalCfgRequest_B));
  EXPECT_EQ((unsigned int)144, sizeof(TSC_DeviceCalCfgResult));
  EXPECT_EQ((unsigned int)149, sizeof(TSD_DeviceCalCfgRequest));
  EXPECT_EQ((unsigned int)148, sizeof(TDS_DeviceCalCfgResult));
  EXPECT_EQ((unsigned int)9, sizeof(TCS_TimerArmRequest));

  EXPECT_LE(sizeof(TDS_ImpulseCounter_Value),
            (unsigned int)SUPLA_CHANNELVALUE_SIZE);
  EXPECT_LE(sizeof(TSC_ImpulseCounter_ExtendedValue),
            (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);

  EXPECT_LE(sizeof(TElectricityMeter_Value),
            (unsigned int)SUPLA_CHANNELVALUE_SIZE);
  EXPECT_LE(sizeof(TElectricityMeter_ExtendedValue),
            (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);
  EXPECT_LE(sizeof(TElectricityMeter_ExtendedValue_V2),
            (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);

  EXPECT_EQ((unsigned int)4, sizeof(TThermostat_Time));
  EXPECT_EQ((unsigned int)246, sizeof(TThermostat_ExtendedValue));
  EXPECT_EQ((unsigned int)6, sizeof(TThermostat_Value));
  EXPECT_EQ((unsigned int)169, sizeof(TThermostat_Schedule));
  EXPECT_EQ((unsigned int)104, sizeof(TThermostat_ScheduleCfg));
  EXPECT_EQ((unsigned int)22, sizeof(TThermostatTemperatureCfg));

  EXPECT_LE(sizeof(TThermostat_ExtendedValue),
            (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);
  EXPECT_LE(sizeof(TThermostat_Value), (unsigned int)SUPLA_CHANNELVALUE_SIZE);
  EXPECT_LE(sizeof(TThermostat_ScheduleCfg),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
  EXPECT_LE(sizeof(TThermostatTemperatureCfg),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);

  EXPECT_EQ((unsigned int)63, sizeof(TSDC_UserLocalTimeResult));

  EXPECT_EQ((unsigned int)50, sizeof(TDSC_ChannelState));
  EXPECT_EQ((unsigned int)8, sizeof(TCSD_ChannelStateRequest));
  EXPECT_EQ((unsigned int)8, sizeof(TCS_SetChannelFunction));
  EXPECT_EQ((unsigned int)9, sizeof(TSC_SetChannelFunctionResult));
  EXPECT_EQ((unsigned int)660, sizeof(TSC_ChannelBasicCfg));
  EXPECT_EQ((unsigned int)4, sizeof(TCS_ChannelBasicCfgRequest));
  EXPECT_EQ((unsigned int)1, sizeof(TSC_ClientsReconnectRequestResult));
  EXPECT_EQ((unsigned int)8, sizeof(TCS_SetRegistrationEnabled));
  EXPECT_EQ((unsigned int)1, sizeof(TSC_SetRegistrationEnabledResult));
  EXPECT_EQ((unsigned int)4, sizeof(TCS_DeviceReconnectRequest));
  EXPECT_EQ((unsigned int)5, sizeof(TSC_DeviceReconnectRequestResult));
  EXPECT_EQ((unsigned int)409, sizeof(TCS_SetCaption));
  EXPECT_EQ((unsigned int)410, sizeof(TSC_SetCaptionResult));
  EXPECT_EQ((unsigned int)513, sizeof(TSD_ChannelFunctions));
  EXPECT_EQ((unsigned int)58, sizeof(TCalCfg_ZWave_Node));
  EXPECT_LE(sizeof(TCalCfg_ZWave_Node),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
  EXPECT_LE(sizeof(unsigned char), (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
  EXPECT_LE(sizeof(TCalCfg_ProgressReport),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);

  EXPECT_EQ((unsigned int)221, sizeof(TTimerState_ExtendedValue));
  EXPECT_EQ((unsigned int)271, sizeof(TChannelAndTimerState_ExtendedValue));

  EXPECT_LE(sizeof(TTimerState_ExtendedValue),
            (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);
  EXPECT_LE(sizeof(TChannelAndTimerState_ExtendedValue),
            (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);

  EXPECT_EQ((unsigned int)4, sizeof(TCalCfg_LightSourceLifespan));
  EXPECT_LE(sizeof(TCalCfg_LightSourceLifespan),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);

  EXPECT_LE(sizeof(TDigiglass_Value), (unsigned int)SUPLA_CHANNELVALUE_SIZE);

  EXPECT_LE(sizeof(TCSD_Digiglass_NewValue),
            (unsigned int)SUPLA_CHANNELVALUE_SIZE);

  EXPECT_EQ((unsigned int)12, sizeof(TCalCfg_ZWave_WakeupSettingsReport));
  EXPECT_LE(sizeof(TCalCfg_ZWave_WakeupSettingsReport),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);

  EXPECT_EQ((unsigned int)3, sizeof(TCalCfg_ZWave_WakeUpTime));
  EXPECT_LE(sizeof(TCalCfg_ZWave_WakeUpTime),
            (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);

  EXPECT_EQ((unsigned int)13, sizeof(TSD_ChannelIntParams));
  EXPECT_EQ((unsigned int)1, sizeof(TDS_GetChannelIntParamsRequest));
  EXPECT_EQ((unsigned int)3, sizeof(TRelayChannel_Value));
  EXPECT_LE(sizeof(TRelayChannel_Value), (unsigned int)SUPLA_CHANNELVALUE_SIZE);

  EXPECT_EQ((unsigned int)6, sizeof(TDS_GetChannelConfigRequest));
  EXPECT_EQ((unsigned int)136, sizeof(TSD_ChannelConfig));
  EXPECT_LE(sizeof(TSD_ChannelConfig_StaircaseTimer),
            (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
  EXPECT_LE(sizeof(TSD_ChannelConfig_Rollershutter),
            (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
  EXPECT_LE(sizeof(TSD_ChannelConfig_ActionTrigger),
            (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);

  EXPECT_EQ((unsigned int)8, sizeof(TCalCfg_RollerShutterSettings));
  EXPECT_LE(sizeof(TCalCfg_RollerShutterSettings),
            (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);

  EXPECT_EQ(static_cast<size_t>(13), sizeof(TCalCfg_FacadeBlindSettings));
  EXPECT_LE(sizeof(TCalCfg_FacadeBlindSettings),
            static_cast<size_t>(SUPLA_CHANNEL_CONFIG_MAXSIZE));

  EXPECT_EQ(static_cast<size_t>(13), sizeof(TSD_ChannelConfig_FacadeBlind));
  EXPECT_LE(sizeof(TSD_ChannelConfig_FacadeBlind),
            static_cast<size_t>(SUPLA_CHANNEL_CONFIG_MAXSIZE));

  EXPECT_LE(sizeof(TDSC_RollerShutterValue),
            static_cast<size_t>(SUPLA_CHANNELVALUE_SIZE));
  EXPECT_LE(sizeof(TCSD_RollerShutterValue),
            static_cast<size_t>(SUPLA_CHANNELVALUE_SIZE));

  EXPECT_LE(sizeof(TDSC_FacadeBlindValue),
            static_cast<size_t>(SUPLA_CHANNELVALUE_SIZE));
  EXPECT_LE(sizeof(TCSD_FacadeBlindValue),
            static_cast<size_t>(SUPLA_CHANNELVALUE_SIZE));

  EXPECT_EQ((unsigned int)15, sizeof(TDS_ActionTrigger));

  EXPECT_EQ((unsigned int)420, sizeof(TSC_SuplaScene));
  EXPECT_EQ((unsigned int)8408, sizeof(TSC_SuplaScenePack));
  EXPECT_EQ((unsigned int)220, sizeof(TSC_SuplaSceneState));
  EXPECT_EQ((unsigned int)4408, sizeof(TSC_SuplaSceneStatePack));

  EXPECT_EQ((unsigned int)514, sizeof(TCS_Action));
  EXPECT_EQ((unsigned int)390, sizeof(TCS_ClientAuthorizationDetails));
  EXPECT_EQ((unsigned int)904, sizeof(TCS_ActionWithAuth));
  EXPECT_EQ((unsigned int)16, sizeof(TSC_ActionExecutionResult));
}

TEST_F(ProtoTest, init) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);
  ASSERT_EQ(SUPLA_PROTO_VERSION, sproto_get_version(sproto));
  sproto_free(sproto);
}

TEST_F(ProtoTest, littleEndian) {
  char i[] = {1, 2, 3, 4};
  ASSERT_EQ((unsigned int)4, sizeof(int));
  ASSERT_EQ(67305985, *(int *)i);
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

#ifndef SPROTO_WITHOUT_OUT_BUFFER
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
#endif /*SPROTO_WITHOUT_OUT_BUFFER*/

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

#ifndef SPROTO_WITHOUT_OUT_BUFFER
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

TEST_F(ProtoTest, set_null_terminated_string) {
  char src[] = "abcdefghijk";
  char msk[] = "nmoprstuwz123456789";
  char dst[] = "                   ";

  snprintf(dst, sizeof(dst), "%s", msk);
  unsigned _supla_int16_t dest_size = 0;

  sproto__set_null_terminated_string(NULL, dst, &dest_size, 10);

  EXPECT_EQ(dst[0], 0);
  EXPECT_EQ(dest_size, 1U);

  snprintf(dst, sizeof(dst), "%s", msk);
  dest_size = 0;

  sproto__set_null_terminated_string(src, dst, &dest_size, 1);

  EXPECT_EQ(dst[0], 0);
  EXPECT_EQ(dest_size, 1U);

  snprintf(dst, sizeof(dst), "%s", msk);
  dest_size = 0;

  sproto__set_null_terminated_string(src, dst, &dest_size, 5);

  EXPECT_EQ(dst[0], 'a');
  EXPECT_EQ(dst[1], 'b');
  EXPECT_EQ(dst[2], 'c');
  EXPECT_EQ(dst[3], 'd');
  EXPECT_EQ(dst[4], 0);
  EXPECT_EQ(dest_size, 5U);

  snprintf(dst, sizeof(dst), "%s", msk);
  dest_size = 0;

  sproto__set_null_terminated_string(src, dst, &dest_size, sizeof(dst));

  EXPECT_EQ(strncmp(src, dst, sizeof(dst)), 0);
  EXPECT_EQ(dest_size, strnlen(src, sizeof(src)) + 1);
}

#endif /*SPROTO_WITHOUT_OUT_BUFFER*/

}  // namespace
