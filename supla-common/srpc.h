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
#ifndef supladex_H_
#define supladex_H_

#include <stddef.h>
#include <stdio.h>
#include "eh.h"
#include "proto.h"

#ifdef __ANDROID__
#define SRPC_EXCLUDE_DEVICE
#endif /*__ANDROID__*/

#ifdef ESP8266
#define SRPC_EXCLUDE_CLIENT
#include <mem.h>
#include <os_type.h>
#define SRPC_ICACHE_FLASH ICACHE_FLASH_ATTR
#else
#define SRPC_ICACHE_FLASH
#endif

#ifdef __AVR__
#define SRPC_EXCLUDE_CLIENT
#endif /*__AVR__*/

#ifdef __cplusplus
extern "C" {
#endif

typedef _supla_int_t (*_func_srpc_DataRW)(void *buf, _supla_int_t count,
                                          void *user_params);
typedef void (*_func_srpc_event_OnRemoteCallReceived)(
    void *_srpc, unsigned _supla_int_t rr_id, unsigned _supla_int_t call_type,
    void *user_params, unsigned char proto_version);
typedef void (*_func_srpc_event_BeforeCall)(void *_srpc,
                                            unsigned _supla_int_t call_type,
                                            void *user_params);
typedef void (*_func_srpc_event_OnVersionError)(void *_srpc,
                                                unsigned char remote_version,
                                                void *user_params);
typedef void (*_func_srpc_event_OnMinVersionRequired)(
    void *_srpc, unsigned _supla_int_t call_type, unsigned char min_version,
    void *user_params);

typedef struct {
  _func_srpc_DataRW data_read;
  _func_srpc_DataRW data_write;
  _func_srpc_event_OnRemoteCallReceived on_remote_call_received;
  _func_srpc_event_OnVersionError on_version_error;
  _func_srpc_event_BeforeCall before_async_call;
  _func_srpc_event_OnMinVersionRequired on_min_version_required;

  TEventHandler *eh;

  void *user_params;
} TsrpcParams;

union TsrpcDataPacketData {
  TDCS_SuplaPingServer *dcs_ping;
  TSDC_SuplaPingServerResult *sdc_ping_result;
  TSDC_SuplaGetVersionResult *sdc_getversion_result;
  TSDC_SuplaVersionError *sdc_version_error;
  TDCS_SuplaSetActivityTimeout *dcs_set_activity_timeout;
  TSDC_SuplaSetActivityTimeoutResult *sdc_set_activity_timeout_result;
  TDS_SuplaRegisterDevice *ds_register_device;
  TDS_SuplaRegisterDevice_B *ds_register_device_b;
  TDS_SuplaRegisterDevice_C *ds_register_device_c;
  TDS_SuplaRegisterDevice_D *ds_register_device_d;
  TDS_SuplaRegisterDevice_E *ds_register_device_e;
  TSD_SuplaRegisterDeviceResult *sd_register_device_result;
  TCS_SuplaRegisterClient *cs_register_client;
  TCS_SuplaRegisterClient_B *cs_register_client_b;
  TCS_SuplaRegisterClient_C *cs_register_client_c;
  TSC_SuplaRegisterClientResult *sc_register_client_result;
  TSC_SuplaRegisterClientResult_B *sc_register_client_result_b;
  TDS_SuplaDeviceChannelValue *ds_device_channel_value;
  TDS_SuplaDeviceChannelExtendedValue *ds_device_channel_extendedvalue;
  TSC_SuplaLocation *sc_location;
  TSC_SuplaLocationPack *sc_location_pack;
  TSC_SuplaChannel *sc_channel;
  TSC_SuplaChannel_B *sc_channel_b;
  TSC_SuplaChannel_C *sc_channel_c;
  TSC_SuplaChannelPack *sc_channel_pack;
  TSC_SuplaChannelPack_B *sc_channel_pack_b;
  TSC_SuplaChannelPack_C *sc_channel_pack_c;
  TSC_SuplaChannelValue *sc_channel_value;
  TSC_SuplaEvent *sc_event;
  TSD_SuplaChannelNewValue *sd_channel_new_value;
  TDS_SuplaChannelNewValueResult *ds_channel_new_value_result;
  TCS_SuplaChannelNewValue *cs_channel_new_value;
  TCS_SuplaChannelNewValue_B *cs_channel_new_value_b;
  TDS_FirmwareUpdateParams *ds_firmware_update_params;
  TSD_FirmwareUpdate_UrlResult *sc_firmware_update_url_result;
  TSDC_RegistrationEnabled *sdc_reg_enabled;
  TSC_SuplaChannelGroupPack *sc_channelgroup_pack;
  TSC_SuplaChannelGroupPack_B *sc_channelgroup_pack_b;
  TSC_SuplaChannelGroupRelationPack *sc_channelgroup_relation_pack;
  TSC_SuplaChannelValuePack *sc_channelvalue_pack;
  TSC_SuplaChannelExtendedValuePack *sc_channelextendedvalue_pack;
  TCS_SuplaNewValue *cs_new_value;
  TSC_OAuthTokenRequestResult *sc_oauth_tokenrequest_result;
  TCS_SuperUserAuthorizationRequest *cs_superuser_authorization_request;
  TSC_SuperUserAuthorizationResult *sc_superuser_authorization_result;
  TCS_DeviceCalCfgRequest *cs_device_calcfg_request;
  TCS_DeviceCalCfgRequest_B *cs_device_calcfg_request_b;
  TSC_DeviceCalCfgResult *sc_device_calcfg_result;
  TSD_DeviceCalCfgRequest *sd_device_calcfg_request;
  TDS_DeviceCalCfgResult *ds_device_calcfg_result;
  TSDC_UserLocalTimeResult *sdc_user_localtime_result;
};

typedef struct {
  unsigned _supla_int_t call_type;
  unsigned _supla_int_t rr_id;

  union TsrpcDataPacketData data;
} TsrpcReceivedData;

void SRPC_ICACHE_FLASH srpc_params_init(TsrpcParams *params);

void *SRPC_ICACHE_FLASH srpc_init(TsrpcParams *params);
void SRPC_ICACHE_FLASH srpc_free(void *_srpc);

char SRPC_ICACHE_FLASH srpc_input_dataexists(void *_srpc);

char SRPC_ICACHE_FLASH srpc_iterate(void *_srpc);

char SRPC_ICACHE_FLASH srpc_getdata(void *_srpc, TsrpcReceivedData *rd,
                                    unsigned _supla_int_t rr_id);
void SRPC_ICACHE_FLASH srpc_rd_free(TsrpcReceivedData *rd);

unsigned char SRPC_ICACHE_FLASH srpc_get_proto_version(void *_srpc);
void SRPC_ICACHE_FLASH srpc_set_proto_version(void *_srpc,
                                              unsigned char version);

unsigned char SRPC_ICACHE_FLASH
srpc_call_min_version_required(void *_srpc, unsigned _supla_int_t call_type);
unsigned char SRPC_ICACHE_FLASH
srpc_call_allowed(void *_srpc, unsigned _supla_int_t call_type);

// device/client <-> server
_supla_int_t SRPC_ICACHE_FLASH srpc_dcs_async_getversion(void *_srpc);
_supla_int_t SRPC_ICACHE_FLASH srpc_sdc_async_getversion_result(
    void *_srpc, char SoftVer[SUPLA_SOFTVER_MAXSIZE]);
_supla_int_t SRPC_ICACHE_FLASH
srpc_sdc_async_versionerror(void *_srpc, unsigned char remote_version);
_supla_int_t SRPC_ICACHE_FLASH srpc_dcs_async_ping_server(void *_srpc);
_supla_int_t SRPC_ICACHE_FLASH srpc_sdc_async_ping_server_result(void *_srpc);
_supla_int_t SRPC_ICACHE_FLASH srpc_dcs_async_set_activity_timeout(
    void *_srpc, TDCS_SuplaSetActivityTimeout *dcs_set_activity_timeout);
_supla_int_t SRPC_ICACHE_FLASH srpc_dcs_async_set_activity_timeout_result(
    void *_srpc,
    TSDC_SuplaSetActivityTimeoutResult *sdc_set_activity_timeout_result);
_supla_int_t SRPC_ICACHE_FLASH
srpc_dcs_async_get_registration_enabled(void *_srpc);
_supla_int_t SRPC_ICACHE_FLASH srpc_sdc_async_get_registration_enabled_result(
    void *_srpc, TSDC_RegistrationEnabled *reg_enabled);
_supla_int_t SRPC_ICACHE_FLASH srpc_dcs_async_get_user_localtime(void *_srpc);
_supla_int_t SRPC_ICACHE_FLASH srpc_sdc_async_get_user_localtime_result(
    void *_srpc, TSDC_UserLocalTimeResult *localtime);

#ifndef SRPC_EXCLUDE_DEVICE
// device <-> server
_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice(
    void *_srpc, TDS_SuplaRegisterDevice *registerdevice);
_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice_b(
    void *_srpc, TDS_SuplaRegisterDevice_B *registerdevice);  // ver. >= 2
_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice_c(
    void *_srpc, TDS_SuplaRegisterDevice_C *registerdevice);  // ver. >= 6
_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice_d(
    void *_srpc, TDS_SuplaRegisterDevice_D *registerdevice);  // ver. >= 7
_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice_e(
    void *_srpc, TDS_SuplaRegisterDevice_E *registerdevice);  // ver. >= 10
_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_registerdevice_result(
    void *_srpc, TSD_SuplaRegisterDeviceResult *registerdevice_result);
_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_channel_value_changed(
    void *_srpc, unsigned char channel_number, char *value);
_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_channel_extendedvalue_changed(
    void *_srpc, unsigned char channel_number,
    TSuplaChannelExtendedValue *value);
_supla_int_t SRPC_ICACHE_FLASH
srpc_sd_async_set_channel_value(void *_srpc, TSD_SuplaChannelNewValue *value);
_supla_int_t SRPC_ICACHE_FLASH
srpc_ds_async_set_channel_result(void *_srpc, unsigned char ChannelNumber,
                                 _supla_int_t SenderID, char Success);
_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_get_firmware_update_url(
    void *_srpc, TDS_FirmwareUpdateParams *params);
_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_get_firmware_update_url_result(
    void *_srpc, TSD_FirmwareUpdate_UrlResult *result);
_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_device_calcfg_request(
    void *_srpc, TSD_DeviceCalCfgRequest *request);
_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_device_calcfg_request_b(
    void *_srpc, TSD_DeviceCalCfgRequest_B *request_b);
_supla_int_t SRPC_ICACHE_FLASH
srpc_ds_async_device_calcfg_result(void *_srpc, TDS_DeviceCalCfgResult *result);

#endif /*SRPC_EXCLUDE_DEVICE*/

#ifndef SRPC_EXCLUDE_CLIENT
// client <-> server
_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_registerclient(
    void *_srpc, TCS_SuplaRegisterClient *registerclient);
_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_registerclient_b(
    void *_srpc, TCS_SuplaRegisterClient_B *registerclient);  // ver. >= 6
_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_registerclient_c(
    void *_srpc, TCS_SuplaRegisterClient_C *registerclient);  // ver. >= 7
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_registerclient_result(
    void *_srpc, TSC_SuplaRegisterClientResult *registerclient_result);
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_registerclient_result_b(
    void *_srpc,
    TSC_SuplaRegisterClientResult_B *registerclient_result);  // ver. >= 9
_supla_int_t SRPC_ICACHE_FLASH
srpc_sc_async_location_update(void *_srpc, TSC_SuplaLocation *location);
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_locationpack_update(
    void *_srpc, TSC_SuplaLocationPack *location_pack);
_supla_int_t SRPC_ICACHE_FLASH
srpc_sc_async_channel_update(void *_srpc, TSC_SuplaChannel *channel);
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channel_update_b(
    void *_srpc, TSC_SuplaChannel_B *channel);  // ver. >= 8
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channel_update_c(
    void *_srpc, TSC_SuplaChannel_C *channel);  // ver. >= 10
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelpack_update(
    void *_srpc, TSC_SuplaChannelPack *channel_pack);
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelpack_update_b(
    void *_srpc, TSC_SuplaChannelPack_B *channel_pack);  // ver. >= 8
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelpack_update_c(
    void *_srpc, TSC_SuplaChannelPack_C *channel_pack);  // ver. >= 10
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channel_value_update(
    void *_srpc, TSC_SuplaChannelValue *channel_item_value);
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelgroup_pack_update(
    void *_srpc, TSC_SuplaChannelGroupPack *channelgroup_pack);  // ver. >= 9
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelgroup_pack_update_b(
    void *_srpc, TSC_SuplaChannelGroupPack_B *channelgroup_pack);  // ver. >= 10
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelgroup_relation_pack_update(
    void *_srpc, TSC_SuplaChannelGroupRelationPack
                     *channelgroup_relation_pack);  // ver. >= 9
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelvalue_pack_update(
    void *_srpc, TSC_SuplaChannelValuePack *channelvalue_pack);  // ver. >= 9
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelextendedvalue_pack_update(
    void *_srpc,
    TSC_SuplaChannelExtendedValuePack *extendedvalue_pack);  // ver. >= 10
_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_get_next(void *_srpc);
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_event(void *_srpc,
                                                   TSC_SuplaEvent *event);
_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_set_channel_value(void *_srpc, TCS_SuplaChannelNewValue *value);
_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_set_value(void *_srpc, TCS_SuplaNewValue *value);  // ver. >= 9
_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_set_channel_value_b(
    void *_srpc, TCS_SuplaChannelNewValue_B *value);
_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_oauth_token_request(void *_srpc);
_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_oauth_token_request_result(
    void *_srpc, TSC_OAuthTokenRequestResult *result);
_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_superuser_authorization_request(
    void *_srpc, TCS_SuperUserAuthorizationRequest *request);
_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_superuser_authorization_result(
    void *_srpc, TSC_SuperUserAuthorizationResult *result);
_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_device_calcfg_request(
    void *_srpc, TCS_DeviceCalCfgRequest *request);
_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_device_calcfg_request_b(
    void *_srpc, TCS_DeviceCalCfgRequest_B *request);
_supla_int_t SRPC_ICACHE_FLASH
srpc_sc_async_device_calcfg_result(void *_srpc, TSC_DeviceCalCfgResult *result);

#endif /*SRPC_EXCLUDE_CLIENT*/

#ifndef SRPC_EXCLUDE_EXTENDEDVALUE_TOOLS
_supla_int_t SRPC_ICACHE_FLASH srpc_evtool_v1_emextended2extended(
    TElectricityMeter_ExtendedValue *em_ev, TSuplaChannelExtendedValue *ev);
_supla_int_t SRPC_ICACHE_FLASH srpc_evtool_v1_extended2emextended(
    TSuplaChannelExtendedValue *ev, TElectricityMeter_ExtendedValue *em_ev);

_supla_int_t SRPC_ICACHE_FLASH srpc_evtool_v1_extended2thermostatextended(
    TSuplaChannelExtendedValue *ev, TThermostat_ExtendedValue *th_ev);
_supla_int_t SRPC_ICACHE_FLASH srpc_evtool_v1_thermostatextended2extended(
    TThermostat_ExtendedValue *th_ev, TSuplaChannelExtendedValue *ev);

#ifndef SRPC_EXCLUDE_CLIENT
_supla_int_t SRPC_ICACHE_FLASH srpc_evtool_v1_icextended2extended(
    TSC_ImpulseCounter_ExtendedValue *ic_ev, TSuplaChannelExtendedValue *ev);
_supla_int_t SRPC_ICACHE_FLASH srpc_evtool_v1_extended2icextended(
    TSuplaChannelExtendedValue *ev, TSC_ImpulseCounter_ExtendedValue *ic_ev);
#endif /*SRPC_EXCLUDE_CLIENT*/

#endif /*SRPC_EXCLUDE_EXTENDEDVALUE_TOOLS*/

#ifdef __cplusplus
}
#endif

#endif /* supladex_H_ */
