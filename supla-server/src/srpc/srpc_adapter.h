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

#ifndef SRPC_ADAPTER_H_
#define SRPC_ADAPTER_H_

#include <functional>

#include "abstract_srpc_adapter.h"

#define START_OF_LIST_BIT 0x2
#define END_OF_LIST_BIT 0x1

class supla_srpc_adapter : public supla_abstract_srpc_adapter {
 public:
  explicit supla_srpc_adapter(void *srpc);
  virtual ~supla_srpc_adapter();

  template <typename TSuplaDataPack, typename TSuplaDataPackItem>
  static bool datapack_add(TSuplaDataPack *pack, int max_count, bool SOL,
                           std::function<void(TSuplaDataPackItem *)> fill) {
    if (pack->count < max_count) {
      fill(&pack->items[pack->count]);
      pack->items[pack->count].EOL = SOL ? START_OF_LIST_BIT : 0;
      pack->count++;
      return true;
    } else {
      pack->total_left++;
    }
    return false;
  }

  template <typename TSuplaDataPack>
  static void datapack_set_eol(TSuplaDataPack *pack, bool set_bit) {
    if (pack && pack->count > 0) {
      if (pack->total_left == 0) {
        if (set_bit) {
          pack->items[pack->count - 1].EOL |= END_OF_LIST_BIT;

        } else {
          pack->items[pack->count - 1].EOL = 1;
        }
      }
    }
  }

  virtual void lock(void);

  virtual void unlock(void);

  virtual void set_proto_version(unsigned char version);
  virtual unsigned char get_proto_version(void);

  virtual _supla_int_t sdc_async_getversion_result(
      char softver[SUPLA_SOFTVER_MAXSIZE]);

  virtual _supla_int_t sdc_async_ping_server_result(void);

  virtual _supla_int_t dcs_async_set_activity_timeout_result(
      TSDC_SuplaSetActivityTimeoutResult *result);

  virtual _supla_int_t sdc_async_get_registration_enabled_result(
      TSDC_RegistrationEnabled *reg_enabled);

  virtual _supla_int_t sdc_async_get_user_localtime_result(
      TSDC_UserLocalTimeResult *localtime);

  virtual _supla_int_t sc_async_channel_relation_pack_update(
      TSC_SuplaChannelRelationPack *relation_pack);  // ver. >= 21

  virtual _supla_int_t sc_async_scene_pack_update(
      TSC_SuplaScenePack *scene_pack);  // ver. >= 18

  virtual _supla_int_t sc_async_scene_state_pack_update(
      TSC_SuplaSceneStatePack *scene_state_pack);  // ver. >= 18

  virtual _supla_int_t sc_async_channel_state_pack_update(
      TSC_SuplaChannelStatePack *state_pack);  // ver. >= 26

  virtual _supla_int_t sd_async_get_firmware_update_url_result(
      TSD_FirmwareUpdate_UrlResult *result);

  virtual _supla_int_t sd_async_get_channel_functions_result(
      TSD_ChannelFunctions *result);

  virtual _supla_int_t sd_async_get_channel_config_result(
      TSD_ChannelConfig *config);

  virtual _supla_int_t cs_async_oauth_token_request_result(
      TSC_OAuthTokenRequestResult *result);

  virtual _supla_int_t sc_async_superuser_authorization_result(
      TSC_SuperUserAuthorizationResult *result);

  virtual _supla_int_t sc_async_channel_basic_cfg_result(
      TSC_ChannelBasicCfg *basic_cfg);

  virtual _supla_int_t sc_async_clients_reconnect_request_result(
      TSC_ClientsReconnectRequestResult *result);

  virtual _supla_int_t sc_async_set_registration_enabled_result(
      TSC_SetRegistrationEnabledResult *result);

  virtual _supla_int_t sc_async_device_reconnect_request_result(
      TSC_DeviceReconnectRequestResult *result);

  virtual _supla_int_t sd_async_registerdevice_result(
      TSD_SuplaRegisterDeviceResult *result);

  virtual _supla_int_t sd_async_registerdevice_result_b(
      TSD_SuplaRegisterDeviceResult_B *result);

  virtual _supla_int_t sc_async_registerclient_result(
      TSC_SuplaRegisterClientResult *result);

  virtual _supla_int_t sc_async_registerclient_result_b(
      TSC_SuplaRegisterClientResult_B *result);  // ver. >= 9

  virtual _supla_int_t sc_async_registerclient_result_c(
      TSC_SuplaRegisterClientResult_C *result);  // ver. >= 17

  virtual _supla_int_t sc_async_registerclient_result_d(
      TSC_SuplaRegisterClientResult_D *result);  // ver. >= 19

  virtual _supla_int_t sc_async_action_execution_result(
      TSC_ActionExecutionResult *result);  // ver. >= 19

  virtual _supla_int_t cs_async_get_channel_value_with_auth(
      TCS_GetChannelValueWithAuth *vwa);  // ver. >= 19

  virtual _supla_int_t sc_async_get_channel_value_result(
      TSC_GetChannelValueResult *result);  // ver. >= 19

  virtual _supla_int_t sc_async_register_pn_client_token_result(
      TSC_RegisterPnClientTokenResult *result);  // ver. >= 20

  virtual _supla_int_t sd_async_set_device_config_result(
      TSDS_SetDeviceConfigResult *result);  // ver. >= 21

  virtual _supla_int_t sd_async_set_device_config_request(
      TSDS_SetDeviceConfig *config);  // ver. >= 21

  virtual _supla_int_t sd_async_set_channel_config_result(
      TSDS_SetChannelConfigResult *result);  // ver. >= 21

  virtual _supla_int_t sd_async_set_channel_config_request(
      TSDS_SetChannelConfig *config);  // ver. >= 21

  virtual _supla_int_t sc_async_channel_config_update_or_result(
      TSC_ChannelConfigUpdateOrResult *config);  // ver. >= 21

  virtual _supla_int_t sd_async_channel_config_finished(
      TSD_ChannelConfigFinished *fin);  // ver. >= 21

  virtual _supla_int_t sc_async_device_config_update_or_result(
      TSC_DeviceConfigUpdateOrResult *config);  // ver. >= 21

  virtual _supla_int_t sd_async_device_calcfg_request(
      TSD_DeviceCalCfgRequest *request);
};

#endif /* SRPC_ADAPTER_H_ */
