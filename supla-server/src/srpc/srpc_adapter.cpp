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

#include "srpc_adapter.h"

#include "srpc.h"

supla_srpc_adapter::supla_srpc_adapter(void *srpc)
    : supla_abstract_srpc_adapter(srpc) {}

supla_srpc_adapter::~supla_srpc_adapter() {}

void supla_srpc_adapter::set_proto_version(unsigned char version) {
  srpc_set_proto_version(get_srpc(), version);
}

unsigned char supla_srpc_adapter::get_proto_version(void) {
  return srpc_get_proto_version(get_srpc());
}

_supla_int_t supla_srpc_adapter::sdc_async_ping_server_result(void) {
  return srpc_sdc_async_ping_server_result(get_srpc());
}

_supla_int_t supla_srpc_adapter::dcs_async_set_activity_timeout_result(
    TSDC_SuplaSetActivityTimeoutResult *result) {
  return srpc_dcs_async_set_activity_timeout_result(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sdc_async_get_registration_enabled_result(
    TSDC_RegistrationEnabled *reg_enabled) {
  return srpc_sdc_async_get_registration_enabled_result(get_srpc(),
                                                        reg_enabled);
}

_supla_int_t supla_srpc_adapter::sdc_async_get_user_localtime_result(
    TSDC_UserLocalTimeResult *localtime) {
  return srpc_sdc_async_get_user_localtime_result(get_srpc(), localtime);
}

_supla_int_t supla_srpc_adapter::sc_async_scene_pack_update(
    TSC_SuplaScenePack *scene_pack) {
  return srpc_sc_async_scene_pack_update(get_srpc(), scene_pack);
}

_supla_int_t supla_srpc_adapter::sc_async_scene_state_pack_update(
    TSC_SuplaSceneStatePack *scene_state_pack) {
  return srpc_sc_async_scene_state_pack_update(get_srpc(), scene_state_pack);
}

_supla_int_t supla_srpc_adapter::sd_async_get_firmware_update_url_result(
    TSD_FirmwareUpdate_UrlResult *result) {
  return srpc_sd_async_get_firmware_update_url_result(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sd_async_get_channel_functions_result(
    TSD_ChannelFunctions *result) {
  return srpc_sd_async_get_channel_functions_result(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sd_async_get_channel_config_result(
    TSD_ChannelConfig *config) {
  return srpc_sd_async_get_channel_config_result(get_srpc(), config);
}

_supla_int_t supla_srpc_adapter::cs_async_oauth_token_request_result(
    TSC_OAuthTokenRequestResult *result) {
  return srpc_cs_async_oauth_token_request_result(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sc_async_superuser_authorization_result(
    TSC_SuperUserAuthorizationResult *result) {
  return srpc_sc_async_superuser_authorization_result(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sc_async_channel_basic_cfg_result(
    TSC_ChannelBasicCfg *basic_cfg) {
  return srpc_sc_async_channel_basic_cfg_result(get_srpc(), basic_cfg);
}

_supla_int_t supla_srpc_adapter::sc_async_clients_reconnect_request_result(
    TSC_ClientsReconnectRequestResult *result) {
  return srpc_sc_async_clients_reconnect_request_result(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sc_async_set_registration_enabled_result(
    TSC_SetRegistrationEnabledResult *result) {
  return srpc_sc_async_set_registration_enabled_result(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sc_async_device_reconnect_request_result(
    TSC_DeviceReconnectRequestResult *result) {
  return srpc_sc_async_device_reconnect_request_result(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sd_async_registerdevice_result(
    TSD_SuplaRegisterDeviceResult *result) {
  return srpc_sd_async_registerdevice_result(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sc_async_registerclient_result(
    TSC_SuplaRegisterClientResult *result) {
  return srpc_sc_async_registerclient_result(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sc_async_registerclient_result_b(
    TSC_SuplaRegisterClientResult_B *result) {
  return srpc_sc_async_registerclient_result_b(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sc_async_registerclient_result_c(
    TSC_SuplaRegisterClientResult_C *result) {
  return srpc_sc_async_registerclient_result_c(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sc_async_registerclient_result_d(
    TSC_SuplaRegisterClientResult_D *result) {
  return srpc_sc_async_registerclient_result_d(get_srpc(), result);
}
