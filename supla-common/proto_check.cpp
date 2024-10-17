/*
   Copyright (C) AC SOFTWARE SP. Z O.O

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

#include "proto.h"

static_assert(sizeof(_supla_int16_t) == 2);
static_assert(sizeof(_supla_int_t) == 4);
static_assert(sizeof(_supla_int64_t) == 8);

static_assert(sizeof(unsigned _supla_int16_t) == 2);
static_assert(sizeof(unsigned _supla_int_t) == 4);
static_assert(sizeof(unsigned _supla_int64_t) == 8);

static_assert(sizeof(HvacParameterFlags) == 8);

static_assert((unsigned int)16 == sizeof(struct _supla_timeval));
static_assert((unsigned int)23 == sizeof(TSDC_SuplaGetVersionResult));
static_assert((unsigned int)2 == sizeof(TSDC_SuplaVersionError));
static_assert((unsigned int)16 == sizeof(TDCS_SuplaPingServer));
static_assert((unsigned int)8 == sizeof(TDCS_SuplaPingServer_COMPAT));
static_assert((unsigned int)16 == sizeof(TSDC_SuplaPingServerResult));
static_assert((unsigned int)1 == sizeof(TDCS_SuplaSetActivityTimeout));
static_assert((unsigned int)3 == sizeof(TSDC_SuplaSetActivityTimeoutResult));
static_assert((unsigned int)16 == sizeof(TSuplaChannelValue));
static_assert((unsigned int)17 == sizeof(TSuplaChannelValue_B));
static_assert((unsigned int)410 == sizeof(TSC_SuplaLocation));
static_assert((unsigned int)8208 == sizeof(TSC_SuplaLocationPack));
static_assert((unsigned int)13 == sizeof(TDS_SuplaDeviceChannel));
static_assert((unsigned int)1940 == sizeof(TDS_SuplaRegisterDevice));
static_assert((unsigned int)21 == sizeof(TDS_SuplaDeviceChannel_B));
static_assert((unsigned int)25 == sizeof(TDS_SuplaDeviceChannel_C));
static_assert((unsigned int)35 == sizeof(TDS_SuplaDeviceChannel_D));
static_assert((unsigned int)2964 == sizeof(TDS_SuplaRegisterDevice_B));
static_assert((unsigned int)3029 == sizeof(TDS_SuplaRegisterDevice_C));
static_assert((unsigned int)3264 == sizeof(TDS_SuplaRegisterDevice_D));
static_assert((unsigned int)3784 == sizeof(TDS_SuplaRegisterDevice_E));
static_assert((unsigned int)5064 == sizeof(TDS_SuplaRegisterDevice_F));
static_assert((unsigned int)5192 == sizeof(TDS_SuplaRegisterDevice_G));
static_assert((unsigned int)7 == sizeof(TSD_SuplaRegisterDeviceResult));
static_assert((unsigned int)265 == sizeof(TSD_SuplaRegisterDeviceResult_B));
static_assert((unsigned int)9 == sizeof(TDS_SuplaDeviceChannelValue));
static_assert((unsigned int)10 == sizeof(TDS_SuplaDeviceChannelValue_B));
static_assert((unsigned int)14 == sizeof(TDS_SuplaDeviceChannelValue_C));
static_assert(sizeof(TSuplaChannelExtendedValue) + 1 ==
              sizeof(TDS_SuplaDeviceChannelExtendedValue));
static_assert((unsigned int)17 == sizeof(TSD_SuplaChannelNewValue));
static_assert((unsigned int)22 == sizeof(TSD_SuplaChannelGroupNewValue));
static_assert((unsigned int)6 == sizeof(TDS_SuplaChannelNewValueResult));
static_assert((unsigned int)22 == sizeof(TSC_SuplaChannelValue));
static_assert((unsigned int)23 == sizeof(TSC_SuplaChannelValue_B));
static_assert(sizeof(TSuplaChannelExtendedValue) + 4 ==
              sizeof(TSC_SuplaChannelExtendedValue));
static_assert((unsigned int)448 == sizeof(TSC_SuplaChannelValuePack));
static_assert((unsigned int)468 == sizeof(TSC_SuplaChannelValuePack_B));

static_assert((unsigned int)435 == sizeof(TSC_SuplaChannel));
static_assert((unsigned int)8708 == sizeof(TSC_SuplaChannelPack));
static_assert((unsigned int)444 == sizeof(TSC_SuplaChannel_B));
static_assert((unsigned int)8888 == sizeof(TSC_SuplaChannelPack_B));
static_assert((unsigned int)460 == sizeof(TSC_SuplaChannel_C));
static_assert((unsigned int)461 == sizeof(TSC_SuplaChannel_D));
static_assert((unsigned int)469 == sizeof(TSC_SuplaChannel_E));
static_assert((unsigned int)9208 == sizeof(TSC_SuplaChannelPack_C));
static_assert((unsigned int)9228 == sizeof(TSC_SuplaChannelPack_D));
static_assert((unsigned int)9388 == sizeof(TSC_SuplaChannelPack_E));
static_assert((unsigned int)426 == sizeof(TSC_SuplaChannelGroup));
static_assert((unsigned int)8528 == sizeof(TSC_SuplaChannelGroupPack));
static_assert((unsigned int)430 == sizeof(TSC_SuplaChannelGroup_B));
static_assert((unsigned int)8608 == sizeof(TSC_SuplaChannelGroupPack_B));
static_assert((unsigned int)9 == sizeof(TSC_SuplaChannelGroupRelation));
static_assert((unsigned int)908 == sizeof(TSC_SuplaChannelGroupRelationPack));
static_assert((unsigned int)1108 == sizeof(TSC_SuplaChannelRelationPack));
static_assert((unsigned int)275 == sizeof(TCS_SuplaRegisterClient));
static_assert((unsigned int)340 == sizeof(TCS_SuplaRegisterClient_B));
static_assert((unsigned int)575 == sizeof(TCS_SuplaRegisterClient_C));
static_assert((unsigned int)639 == sizeof(TCS_SuplaRegisterClient_D));
static_assert((unsigned int)19 == sizeof(TSC_SuplaRegisterClientResult));
static_assert((unsigned int)27 == sizeof(TSC_SuplaRegisterClientResult_B));
static_assert((unsigned int)31 == sizeof(TSC_SuplaRegisterClientResult_C));
static_assert((unsigned int)27 == sizeof(TSC_SuplaRegisterClientResult_D));
static_assert((unsigned int)9 == sizeof(TCS_SuplaChannelNewValue));
static_assert((unsigned int)12 == sizeof(TCS_SuplaChannelNewValue_B));
static_assert((unsigned int)13 == sizeof(TCS_SuplaNewValue));
static_assert((unsigned int)221 == sizeof(TSC_SuplaEvent));
static_assert((unsigned int)17 == sizeof(TDS_FirmwareUpdateParams));
static_assert((unsigned int)207 == sizeof(TSD_FirmwareUpdate_Url));
static_assert((unsigned int)208 == sizeof(TSD_FirmwareUpdate_UrlResult));
static_assert((unsigned int)8 == sizeof(TSDC_RegistrationEnabled));
static_assert((unsigned int)264 == sizeof(TSC_OAuthToken));
static_assert((unsigned int)265 == sizeof(TSC_OAuthTokenRequestResult));
static_assert((unsigned int)62 == sizeof(TElectricityMeter_Measurement));

// deprecated
// static_assert((unsigned int)429 == sizeof(TElectricityMeter_ExtendedValue));

static_assert((139 +
               sizeof(TElectricityMeter_Measurement) * EM_MEASUREMENT_COUNT) ==
              sizeof(TElectricityMeter_ExtendedValue_V2));

static_assert((144 +
               sizeof(TElectricityMeter_Measurement) * EM_MEASUREMENT_COUNT) ==
              sizeof(TElectricityMeter_ExtendedValue_V3));

static_assert((unsigned int)5 == sizeof(TElectricityMeter_Value));
static_assert((unsigned int)40 == sizeof(TSC_ImpulseCounter_ExtendedValue));
static_assert((unsigned int)8 == sizeof(TSC_ImpulseCounter_Value));
static_assert((unsigned int)320 == sizeof(TCS_SuperUserAuthorizationRequest));
static_assert((unsigned int)144 == sizeof(TCS_DeviceCalCfgRequest));
static_assert((unsigned int)145 == sizeof(TCS_DeviceCalCfgRequest_B));
static_assert((unsigned int)144 == sizeof(TSC_DeviceCalCfgResult));
static_assert((unsigned int)149 == sizeof(TSD_DeviceCalCfgRequest));
static_assert((unsigned int)148 == sizeof(TDS_DeviceCalCfgResult));
static_assert((unsigned int)9 == sizeof(TCS_TimerArmRequest));
static_assert(sizeof(TDS_ImpulseCounter_Value) <=
              (unsigned int)SUPLA_CHANNELVALUE_SIZE);
static_assert(sizeof(TSC_ImpulseCounter_ExtendedValue) <=
              (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);
static_assert(sizeof(TElectricityMeter_Value) <=
              (unsigned int)SUPLA_CHANNELVALUE_SIZE);
// deprecated
// static_assert(sizeof(TElectricityMeter_ExtendedValue) <= (unsigned
// int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);

static_assert(sizeof(TElectricityMeter_ExtendedValue_V2) <=
              (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);
static_assert(sizeof(TElectricityMeter_ExtendedValue_V3) <=
              (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);
static_assert((unsigned int)4 == sizeof(TThermostat_Time));
static_assert((unsigned int)246 == sizeof(TThermostat_ExtendedValue));
static_assert((unsigned int)6 == sizeof(TThermostat_Value));
static_assert((unsigned int)169 == sizeof(TThermostat_Schedule));
static_assert((unsigned int)104 == sizeof(TThermostat_ScheduleCfg));
static_assert((unsigned int)22 == sizeof(TThermostatTemperatureCfg));
static_assert(sizeof(TThermostat_ExtendedValue) <=
              (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);
static_assert(sizeof(TThermostat_Value) <=
              (unsigned int)SUPLA_CHANNELVALUE_SIZE);
static_assert(sizeof(TThermostat_ScheduleCfg) <=
              (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
static_assert(sizeof(TThermostatTemperatureCfg) <=
              (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
static_assert((unsigned int)63 == sizeof(TSDC_UserLocalTimeResult));
static_assert((unsigned int)50 == sizeof(TDSC_ChannelState));
static_assert(sizeof(TDSC_ChannelState) <= SUPLA_CHANNELEXTENDEDVALUE_SIZE);
static_assert((unsigned int)8 == sizeof(TCSD_ChannelStateRequest));
static_assert((unsigned int)8 == sizeof(TCS_SetChannelFunction));
static_assert((unsigned int)9 == sizeof(TSC_SetChannelFunctionResult));
static_assert((unsigned int)660 == sizeof(TSC_ChannelBasicCfg));
static_assert((unsigned int)4 == sizeof(TCS_ChannelBasicCfgRequest));
static_assert((unsigned int)1 == sizeof(TSC_ClientsReconnectRequestResult));
static_assert((unsigned int)8 == sizeof(TCS_SetRegistrationEnabled));
static_assert((unsigned int)1 == sizeof(TSC_SetRegistrationEnabledResult));
static_assert((unsigned int)4 == sizeof(TCS_DeviceReconnectRequest));
static_assert((unsigned int)5 == sizeof(TSC_DeviceReconnectRequestResult));
static_assert((unsigned int)409 == sizeof(TDCS_SetCaption));
static_assert((unsigned int)410 == sizeof(TSCD_SetCaptionResult));
static_assert((unsigned int)513 == sizeof(TSD_ChannelFunctions));
static_assert((unsigned int)58 == sizeof(TCalCfg_ZWave_Node));
static_assert(sizeof(TCalCfg_ZWave_Node) <=
              (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
static_assert(sizeof(unsigned char) <= (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
static_assert(sizeof(TCalCfg_ProgressReport) <=
              (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
static_assert((unsigned int)221 == sizeof(TTimerState_ExtendedValue));
static_assert((unsigned int)271 == sizeof(TChannelAndTimerState_ExtendedValue));
static_assert(sizeof(TTimerState_ExtendedValue) <=
              (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);
static_assert((unsigned int)4 == sizeof(TCalCfg_LightSourceLifespan));
static_assert(sizeof(TCalCfg_LightSourceLifespan) <=
              (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
static_assert(sizeof(TDigiglass_Value) <=
              (unsigned int)SUPLA_CHANNELVALUE_SIZE);
static_assert(sizeof(TCSD_Digiglass_NewValue) <=
              (unsigned int)SUPLA_CHANNELVALUE_SIZE);
static_assert((unsigned int)12 == sizeof(TCalCfg_ZWave_WakeupSettingsReport));
static_assert(sizeof(TCalCfg_ZWave_WakeupSettingsReport) <=
              (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
static_assert((unsigned int)3 == sizeof(TCalCfg_ZWave_WakeUpTime));
static_assert(sizeof(TCalCfg_ZWave_WakeUpTime) <=
              (unsigned int)SUPLA_CALCFG_DATA_MAXSIZE);
static_assert((unsigned int)13 == sizeof(TSD_ChannelIntParams));
static_assert((unsigned int)1 == sizeof(TDS_GetChannelIntParamsRequest));
static_assert((unsigned int)3 == sizeof(TRelayChannel_Value));
static_assert(sizeof(TRelayChannel_Value) <=
              (unsigned int)SUPLA_CHANNELVALUE_SIZE);
static_assert((unsigned int)6 == sizeof(TDS_GetChannelConfigRequest));
static_assert((unsigned int)520 == sizeof(TSD_ChannelConfig));
static_assert((unsigned int)520 == sizeof(TSDS_SetChannelConfig));
static_assert((unsigned int)3 == sizeof(TSDS_SetChannelConfigResult));
static_assert(sizeof(TChannelConfig_StaircaseTimer) == 4);
static_assert(sizeof(TChannelConfig_StaircaseTimer) <=
              (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert(sizeof(TChannelConfig_RollerShutter) == 44);
static_assert(sizeof(TChannelConfig_RollerShutter) <=
              (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert(sizeof(TChannelConfig_ActionTrigger) == 4);
static_assert(sizeof(TChannelConfig_ActionTrigger) <=
              (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert((unsigned int)539 == sizeof(TSDS_SetDeviceConfig));
static_assert((unsigned int)10 == sizeof(TSDS_SetDeviceConfigResult));
static_assert(sizeof(TDeviceConfig_StatusLed) <=
              (unsigned int)SUPLA_DEVICE_CONFIG_MAXSIZE);
static_assert(sizeof(TDeviceConfig_ScreenBrightness) <=
              (unsigned int)SUPLA_DEVICE_CONFIG_MAXSIZE);
static_assert(sizeof(TDeviceConfig_ButtonVolume) <=
              (unsigned int)SUPLA_DEVICE_CONFIG_MAXSIZE);
static_assert(sizeof(TDeviceConfig_DisableUserInterface) <=
              (unsigned int)SUPLA_DEVICE_CONFIG_MAXSIZE);
static_assert(sizeof(TDeviceConfig_AutomaticTimeSync) <=
              (unsigned int)SUPLA_DEVICE_CONFIG_MAXSIZE);
static_assert(sizeof(TDeviceConfig_HomeScreenOffDelay) <=
              (unsigned int)SUPLA_DEVICE_CONFIG_MAXSIZE);
static_assert(sizeof(TDeviceConfig_HomeScreenContent) <=
              (unsigned int)SUPLA_DEVICE_CONFIG_MAXSIZE);
static_assert((unsigned int)8 == sizeof(TCalCfg_RollerShutterSettings));
static_assert(sizeof(TCalCfg_RollerShutterSettings) <=
              (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert(53 == sizeof(TChannelConfig_FacadeBlind));
static_assert(sizeof(TChannelConfig_FacadeBlind) <=
              SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert(sizeof(TDSC_RollerShutterValue) <= SUPLA_CHANNELVALUE_SIZE);
static_assert(sizeof(TCSD_RollerShutterValue) <= SUPLA_CHANNELVALUE_SIZE);
static_assert(sizeof(TDSC_FacadeBlindValue) <= SUPLA_CHANNELVALUE_SIZE);
static_assert(sizeof(TCSD_FacadeBlindValue) <= SUPLA_CHANNELVALUE_SIZE);
static_assert((unsigned int)15 == sizeof(TDS_ActionTrigger));
static_assert((unsigned int)420 == sizeof(TSC_SuplaScene));
static_assert((unsigned int)8408 == sizeof(TSC_SuplaScenePack));
static_assert((unsigned int)220 == sizeof(TSC_SuplaSceneState));
static_assert((unsigned int)4408 == sizeof(TSC_SuplaSceneStatePack));
static_assert((unsigned int)511 == sizeof(TCS_Action));
static_assert((unsigned int)390 == sizeof(TCS_ClientAuthorizationDetails));
static_assert((unsigned int)901 == sizeof(TCS_ActionWithAuth));
static_assert((unsigned int)13 == sizeof(TSC_ActionExecutionResult));
static_assert((unsigned int)16 == sizeof(TAction_ShadingSystem_Parameters));
static_assert((unsigned int)16 == sizeof(TAction_RGBW_Parameters));
static_assert(sizeof(TAction_ShadingSystem_Parameters) <=
              (unsigned int)SUPLA_ACTION_PARAM_MAXSIZE);
static_assert(sizeof(TAction_RGBW_Parameters) <=
              (unsigned int)SUPLA_ACTION_PARAM_MAXSIZE);
static_assert(sizeof(TCS_GetChannelValueWithAuth) == 394);
static_assert(sizeof(TDS_RegisterPushNotification) == 11);
static_assert(sizeof(TDS_PushNotification) == 399);
static_assert(sizeof(TCS_RegisterPnClientToken) == 710);
static_assert(sizeof(TSC_RegisterPnClientTokenResult) == 4);
static_assert(sizeof(TDS_SubdeviceDetails) == 325);
static_assert(sizeof(THVACValue) == 8);
static_assert(sizeof(THVACValue) <= SUPLA_CHANNELVALUE_SIZE);
static_assert(sizeof(TChannelConfig_HVAC) == 127);
static_assert(sizeof(TChannelConfig_HVAC) <= SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert(sizeof(TChannelConfig_WeeklySchedule) == 356);
static_assert(sizeof(TChannelConfig_WeeklySchedule) <=
              SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert((unsigned int)544 == sizeof(TSC_DeviceConfigUpdateOrResult));
static_assert((unsigned int)20 == sizeof(TCS_GetDeviceConfigRequest));
static_assert((unsigned int)9 == sizeof(TCS_GetChannelConfigRequest));
static_assert((unsigned int)523 == sizeof(TSCS_ChannelConfig));
static_assert((unsigned int)524 == sizeof(TSC_ChannelConfigUpdateOrResult));
static_assert(sizeof(TChannelConfig_BinarySensor) == 32);
static_assert(sizeof(TChannelConfig_BinarySensor) <=
              (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert(sizeof(TChannelConfig_TemperatureAndHumidity) == 32);
static_assert(sizeof(TChannelConfig_TemperatureAndHumidity) <=
              (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert(sizeof(TChannelConfig_GeneralPurposeMeasurement) == 108);
static_assert(sizeof(TChannelConfig_GeneralPurposeMeasurement) <=
              (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert(sizeof(TChannelConfig_GeneralPurposeMeter) == 111);
static_assert(sizeof(TChannelConfig_GeneralPurposeMeter) <=
              (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert(sizeof(TChannelConfig_OCR) == 291);
static_assert(sizeof(TChannelConfig_OCR) <=
              (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert(sizeof(TChannelConfig_ImpulseCounter) == 61);
static_assert(sizeof(TChannelConfig_ImpulseCounter) <=
              (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert(sizeof(TChannelConfig_ElectricityMeter) == 72);
static_assert(sizeof(TChannelConfig_ElectricityMeter) <=
              (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);
static_assert(sizeof(TChannelConfig_PowerSwitch) == 42);
static_assert(sizeof(TChannelConfig_PowerSwitch) <=
              (unsigned int)SUPLA_CHANNEL_CONFIG_MAXSIZE);

static_assert(SUPLA_CHANNEL_CAPTION_MAXSIZE == SUPLA_CAPTION_MAXSIZE);
static_assert(SUPLA_LOCATION_CAPTION_MAXSIZE == SUPLA_CAPTION_MAXSIZE);
static_assert(SUPLA_SCENE_CAPTION_MAXSIZE == SUPLA_CAPTION_MAXSIZE);

// Plaform specific checks
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO) || defined(SUPLA_DEVICE)
static_assert(SUPLA_MAX_DATA_SIZE == 600);
static_assert(618 == sizeof(TSuplaDataPacket));
#elif defined(ESP8266)
static_assert(SUPLA_MAX_DATA_SIZE == 1536);
static_assert(1554 == sizeof(TSuplaDataPacket));
#else
static_assert(SUPLA_MAX_DATA_SIZE == 10240);
static_assert((unsigned int)10258 == sizeof(TSuplaDataPacket));
#endif

#if defined(__AVR__) || defined(ESP8266) || defined(ESP32) || \
    defined(ESP_PLATFORM) || defined(ARDUINO) || defined(SUPLA_DEVICE)
static_assert(SUPLA_CHANNELEXTENDEDVALUE_SIZE == 256);
static_assert(261 == sizeof(TSuplaChannelExtendedValue));
#else
static_assert(SUPLA_CHANNELEXTENDEDVALUE_SIZE == 1024);
static_assert(1029 == sizeof(TSuplaChannelExtendedValue));
#endif

// Proto checks for structs not used in supla-device and which are failing
// on supla-device compilation
#if !defined(__AVR__) && !defined(ESP8266) && !defined(ESP32) && \
    !defined(ESP_PLATFORM) && !defined(ARDUINO) && !defined(SUPLA_DEVICE)
static_assert((unsigned int)10202 == sizeof(TSC_SuplaChannelExtendedValuePack));
static_assert(sizeof(TChannelAndTimerState_ExtendedValue) <=
              (unsigned int)SUPLA_CHANNELEXTENDEDVALUE_SIZE);
static_assert(sizeof(TSC_GetChannelValueResult) == 1055);
#endif
