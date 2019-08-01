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

#ifndef SUPLA_CLIENT_H_
#define SUPLA_CLIENT_H_

#include "proto.h"

typedef struct {
  int Id;
  int Function;
  char Online;
  TSuplaChannelValue value;
  char *Caption;
} TSuplaClientDeviceChannel;

typedef void (*_suplaclient_cb_on_versionerror)(void *_suplaclient,
                                                void *user_data, int version,
                                                int remote_version_min,
                                                int remote_version);
typedef void (*_suplaclient_cb_on_action)(void *_suplaclient, void *user_data);
typedef void (*_suplaclient_cb_on_registered)(
    void *_suplaclient, void *user_data,
    TSC_SuplaRegisterClientResult_B *result);
typedef void (*_suplaclient_cb_on_error)(void *_suplaclient, void *user_data,
                                         int code);
typedef void (*_suplaclient_cb_location_update)(void *_suplaclient,
                                                void *user_data,
                                                TSC_SuplaLocation *location);
typedef void (*_suplaclient_cb_channel_update)(void *_suplaclient,
                                               void *user_data,
                                               TSC_SuplaChannel_C *channel);
typedef void (*_suplaclient_cb_channelgroup_update)(
    void *_suplaclient, void *user_data,
    TSC_SuplaChannelGroup_B *channel_group);
typedef void (*_suplaclient_cb_channelgroup_relation_update)(
    void *_suplaclient, void *user_data,
    TSC_SuplaChannelGroupRelation *channelgroup_relation);
typedef void (*_suplaclient_cb_channel_value_update)(
    void *_suplaclient, void *user_data, TSC_SuplaChannelValue *channel_value);
typedef void (*_suplaclient_cb_channel_extendedvalue_update)(
    void *_suplaclient, void *user_data,
    TSC_SuplaChannelExtendedValue *channel_extendedvalue);
typedef void (*_suplaclient_cb_on_event)(void *_suplaclient, void *user_data,
                                         TSC_SuplaEvent *event);
typedef void (*_suplaclient_cb_on_registration_enabled)(
    void *_suplaclient, void *user_data, TSDC_RegistrationEnabled *reg_enabled);
typedef void (*_suplaclient_cb_on_min_version_required)(
    void *_suplaclient, void *user_data, unsigned int call_type,
    unsigned char min_version);
typedef void (*_suplaclient_cb_on_oauth_token_request_result)(
    void *_suplaclient, void *user_data, TSC_OAuthTokenRequestResult *result);
typedef void (*_suplaclient_cb_on_superuser_authorization_result)(
    void *_suplaclient, void *user_data, char authorized, _supla_int_t code);
typedef void (*_suplaclient_cb_on_device_calcfg_result)(
    void *_suplaclient, void *user_data, TSC_DeviceCalCfgResult *result);

typedef struct {
  char clientGUID[SUPLA_GUID_SIZE];
  char Name[SUPLA_CLIENT_NAME_MAXSIZE];  // UTF8

  char Email[SUPLA_EMAIL_MAXSIZE];  // UTF8
  char AuthKey[SUPLA_AUTHKEY_SIZE];

  int AccessID;
  char AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE];  // UTF8

  char SoftVer[SUPLA_SOFTVER_MAXSIZE];

  char *host;
  int tcp_port;
  int ssl_port;
  char ssl_enabled;

  void *user_data;
  int iterate_wait_usec;

  unsigned char protocol_version;

  _suplaclient_cb_on_versionerror cb_on_versionerror;
  _suplaclient_cb_on_error cb_on_connerror;
  _suplaclient_cb_on_action cb_on_connected;
  _suplaclient_cb_on_action cb_on_disconnected;
  _suplaclient_cb_on_action cb_on_registering;
  _suplaclient_cb_on_registered cb_on_registered;
  _suplaclient_cb_on_error cb_on_registererror;

  _suplaclient_cb_location_update cb_location_update;
  _suplaclient_cb_channel_update cb_channel_update;
  _suplaclient_cb_channel_value_update cb_channel_value_update;
  _suplaclient_cb_channel_extendedvalue_update cb_channel_extendedvalue_update;

  _suplaclient_cb_channelgroup_update cb_channelgroup_update;
  _suplaclient_cb_channelgroup_relation_update cb_channelgroup_relation_update;

  _suplaclient_cb_on_event cb_on_event;
  _suplaclient_cb_on_registration_enabled cb_on_registration_enabled;

  _suplaclient_cb_on_min_version_required cb_on_min_version_required;

  _suplaclient_cb_on_oauth_token_request_result
      cb_on_oauth_token_request_result;
  _suplaclient_cb_on_superuser_authorization_result
      cb_on_superuser_authorization_result;

  _suplaclient_cb_on_device_calcfg_result cb_on_device_calcfg_result;
} TSuplaClientCfg;

#ifdef __cplusplus
extern "C" {
#endif

void supla_client_cfginit(TSuplaClientCfg *sclient_cfg);

void *supla_client_init(TSuplaClientCfg *sclient_cfg);
void supla_client_free(void *_scd);
int supla_client_get_id(void *_suplaclient);
char supla_client_connect(void *_suplaclient);
char supla_client_connected(void *_suplaclient);
char supla_client_registered(void *_suplaclient);
void supla_client_disconnect(void *_suplaclient);

// For _WIN32 wait_usec mean wait_msec
char supla_client_iterate(void *_suplaclient, int wait_usec);
void supla_client_raise_event(void *_suplaclient);
void *supla_client_get_userdata(void *_suplaclient);

char supla_client_send_raw_value(void *_suplaclient, int ID,
                                 char value[SUPLA_CHANNELVALUE_SIZE],
                                 char Target);
char supla_client_open(void *_suplaclient, int ID, char group, char open);
char supla_client_set_rgbw(void *_suplaclient, int ID, char group, int color,
                           char color_brightness, char brightness,
                           char turn_onoff);
char supla_client_set_dimmer(void *_suplaclient, int ID, char group,
                             char brightness, char turn_onoff);
char supla_client_get_registration_enabled(void *_suplaclient);
unsigned char supla_client_get_proto_version(void *_suplaclient);
char supla_client_oauth_token_request(void *_suplaclient);
char supla_client_superuser_authorization_request(void *_suplaclient,
                                                  char *email, char *password);
char supla_client_device_calcfg_request(void *_suplaclient,
                                        TCS_DeviceCalCfgRequest_B *request);

#ifdef __cplusplus
}
#endif

#endif /* SUPLA_CLIENT_H_ */
