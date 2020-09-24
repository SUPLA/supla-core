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

#ifndef DATABASE_H_
#define DATABASE_H_

#include "client.h"
#include "db.h"
#include "device.h"
#include "proto.h"
#include "user.h"

class database : public dbcommon {
 private:
  virtual char *cfg_get_host(void);
  virtual char *cfg_get_user(void);
  virtual char *cfg_get_password(void);
  virtual char *cfg_get_database(void);
  virtual int cfg_get_port(void);

  bool auth(const char *query, int ID, char *PWD, int PWD_MAXXSIZE, int *UserID,
            bool *is_enabled);
  bool authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                    const char Email[SUPLA_EMAIL_MAXSIZE],
                    const char AuthKey[SUPLA_AUTHKEY_SIZE], int *UserID,
                    bool Client, const char *sql);

  bool get_authkey_hash(int ID, char *buffer, unsigned int buffer_size,
                        bool *is_null, const char *sql);

  void em_set_longlong(unsigned _supla_int64_t *v, void *pbind);
  int get_device_client_id(int UserID, const char GUID[SUPLA_GUID_SIZE],
                           bool client);

 public:
  bool location_auth(int LocationID, char *LocationPWD, int *UserID,
                     bool *is_enabled);
  bool accessid_auth(int AccessID, char *AccessIDpwd, int *UserID,
                     bool *is_enabled);

  bool get_user_uniqueid(int UserID, char shortID[SHORT_UNIQUEID_MAXSIZE],
                         char longID[LONG_UNIQUEID_MAXSIZE]);

  int get_user_id_by_email(const char Email[SUPLA_EMAIL_MAXSIZE]);

  bool client_authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                           const char Email[SUPLA_EMAIL_MAXSIZE],
                           const char AuthKey[SUPLA_AUTHKEY_SIZE], int *UserID);
  bool device_authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                           const char Email[SUPLA_EMAIL_MAXSIZE],
                           const char AuthKey[SUPLA_AUTHKEY_SIZE], int *UserID);

  int add_device(int LocationID, const char GUID[SUPLA_GUID_SIZE],
                 const char *AuthKey, const char *Name, unsigned int ipv4,
                 const char *softver, int proto_version, short ManufacturerID,
                 short ProductID, int Flags, int UserID);

  int update_device(int DeviceID, int OriginalLocationID, const char *AuthKey,
                    const char *Name, unsigned int ipv4, const char *softver,
                    int proto_version);

  int add_channel(int DeviceID, int ChannelNumber, int ChannelType);
  int add_device_channel(int DeviceID, int ChannelNumber, int Type, int Func,
                         int FList, int Flags, int UserID, bool *new_channel);

  int get_device_limit_left(int UserID);
  int get_device_count(int UserID);

  int get_location_id(int UserID, bool enabled);

  bool get_device_reg_enabled(int UserID);
  int get_device_id(int UserID, const char GUID[SUPLA_GUID_SIZE]);
  int get_device(int DeviceID, bool *device_enabled, int *original_location_id,
                 int *location_id, bool *location_enabled);

  int get_device_channel(int DeviceID, int ChannelNumber, int *Type);
  int get_device_channel_count(int DeviceID);
  int get_device_channel_type(int DeviceID, int ChannelNumber);
  void get_device_channels(int UserID, int DeviceID,
                           supla_device_channels *channels);

  bool get_device_firmware_update_url(int DeviceID,
                                      TDS_FirmwareUpdateParams *params,
                                      TSD_FirmwareUpdate_UrlResult *url);

  bool on_newdevice(int DeviceID);
  bool on_newclient(int ClientID);
  bool on_channeladded(int DeviceID, int ChannelID);

  int get_client_limit_left(int UserID);
  int get_client_count(int UserID);

  int get_access_id(int UserID, bool enabled);
  int get_client_access_id(int ClientID, bool *accessid_enabled);

  bool get_client_reg_enabled(int UserID);
  int get_client_id(int UserID, const char GUID[SUPLA_GUID_SIZE]);
  int get_client(int ClientID, bool *client_enabled, int *access_id,
                 bool *accessid_enabled);

  int add_client(int AccessID, const char *GUID, const char *AuthKey,
                 const char *Name, unsigned int ipv4, const char *softver,
                 int proto_version, int UserID);

  bool update_client(int ClientID, int AccessID, const char *AuthKey,
                     const char *Name, unsigned int ipv4, const char *softver,
                     int proto_version);

  void get_client_locations(int ClientID, supla_client_locations *locs);
  void get_client_channels(int ClientID, int *DeviceID,
                           supla_client_channels *channels);

  void get_user_channel_groups(int UserID, supla_user_channelgroups *cgroups);

  void get_client_channel_groups(int ClientID,
                                 supla_client_channelgroups *cgroups);

  void get_client_channel_group_relations(int ClientID,
                                          supla_client_channelgroups *cgroups);

  void add_temperature(int ChannelID, double temperature);
  void add_temperature_and_humidity(int ChannelID, double temperature,
                                    double humidity);

  void add_electricity_measurement(supla_channel_electricity_measurement *em);
  void add_impulses(supla_channel_ic_measurement *ic);
  void add_thermostat_measurements(supla_channel_thermostat_measurement *th);

  bool get_reg_enabled(int UserID, unsigned int *client,
                       unsigned int *iodevice);
  bool set_reg_enabled(int UserID, int deviceRegTimeSec, int clientRegTimeSec);

  int oauth_add_client_id(void);
  int oauth_get_client_id(bool create);
  bool oauth_get_token(TSC_OAuthToken *token, int user_id, int access_id);

  bool superuser_authorization(int UserID,
                               const char email[SUPLA_EMAIL_MAXSIZE],
                               const char password[SUPLA_PASSWORD_MAXSIZE]);

  bool amazon_alexa_load_credentials(supla_amazon_alexa_credentials *alexa);
  void amazon_alexa_remove_token(supla_amazon_alexa_credentials *alexa);
  void amazon_alexa_update_token(supla_amazon_alexa_credentials *alexa,
                                 const char *token, const char *refresh_token,
                                 int expires_in);

  bool google_home_load_credentials(supla_google_home_credentials *google_home);
  bool state_webhook_load_credentials(supla_state_webhook_credentials *webhook);
  void state_webhook_update_token(int UserID, const char *token,
                                  const char *refresh_token, int expires_in);
  void state_webhook_remove_token(int UserID);

  bool get_user_localtime(int UserID, TSDC_UserLocalTimeResult *time);
  bool get_channel_basic_cfg(int ChannelID, TSC_ChannelBasicCfg *cfg);
  bool set_channel_function(int UserID, int ChannelID, int Func);
  bool get_channel_type_and_funclist(int UserID, int ChannelID, int *Type,
                                     unsigned int *FuncList);
  bool set_channel_caption(int UserID, int ChannelID, char *Caption);
  bool channel_belong_to_group(int channel_id);
  bool channel_has_schedule(int channel_id);
  bool channel_is_associated_with_scene(int channel_id);
  void update_channel_value(int channel_id, int user_id,
                            const char value[SUPLA_CHANNELVALUE_SIZE],
                            unsigned _supla_int_t validity_time_sec);
  bool get_channel_value(int channel_id, int user_id,
                         char value[SUPLA_CHANNELVALUE_SIZE],
                         unsigned _supla_int_t *validity_time_sec);
  void load_temperatures_and_humidity(int UserID, void *tarr);
};

#endif /* DATABASE_H_ */
