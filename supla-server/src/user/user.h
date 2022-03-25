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

#ifndef USER_H_
#define USER_H_

#define SHORT_UNIQUEID_MAXSIZE 37
#define LONG_UNIQUEID_MAXSIZE 201

#include <cstddef>
#include <functional>

#include "amazon/alexacredentials.h"
#include "caller.h"
#include "commontypes.h"
#include "google/googlehomecredentials.h"
#include "proto.h"
#include "webhook/statewebhookcredentials.h"

class supla_device;
class supla_client;
class supla_user_channelgroups;
class supla_user_client_container;
class supla_user_device_container;
class supla_channel_electricity_measurement;
class supla_channel_ic_measurement;

class supla_user {
 private:
  char *short_unique_id;
  char *long_unique_id;
  void *lck;

  static struct timeval metric_tv;
  static unsigned int client_add_metric;
  static unsigned int client_max_metric;
  static unsigned int device_add_metric;
  static unsigned int device_max_metric;

  char *getUniqueID(char **id, bool longid);
  void user_init(int UserID, const char *short_unique_id,
                 const char *long_unique_id);

 protected:
  static void *user_arr;

  supla_user_client_container *client_container;
  supla_user_device_container *device_container;

  void *complex_value_functions_arr;

  supla_user_channelgroups *cgroups;
  supla_amazon_alexa_credentials *amazon_alexa_credentials;
  supla_google_home_credentials *google_home_credentials;
  supla_state_webhook_credentials *state_webhook_credentials;
  int UserID;
  bool connections_allowed;

  void compex_value_cache_clean(int DeviceId);
  channel_function_t compex_value_cache_get_function(
      int ChannelID, channel_function_t **_fnc = NULL);
  void compex_value_cache_update_function(int DeviceId, int ChannelID, int Type,
                                          int Function, bool channel_is_hidden);

  static char find_user_by_id(void *ptr, void *UserID);
  static char find_user_by_suid(void *ptr, void *suid);
  static bool get_channel_double_value(int UserID, int DeviceID, int ChannelID,
                                       double *Value, char Type);
  bool get_channel_double_value(int DeviceID, int ChannelID, double *Value,
                                char Type);

  bool client_reconnect(int ClientID);
  bool device_reconnect(int DeviceID);
  void loadUniqueIDs(void);
  void access_client(int ClientID,
                     std::function<void(supla_client *client)> on_client);

 public:
  static void init(void);
  static void user_free(void);
  static supla_user *add_device(supla_device *device, int UserID);
  static supla_user *add_client(supla_client *client, int UserID);
  static supla_user *find(int UserID, bool create);
  static supla_user *find_by_suid(const char *suid);
  static int suid_to_user_id(const char *suid, bool use_database);
  static bool reconnect(int UserID, const supla_caller &caller);
  static bool client_reconnect(int UserID, int ClientID);
  static bool device_reconnect(int UserID, int DeviceID);
  static bool is_client_online(int UserID, int ClientID);
  static bool is_device_online(int UserID, int DeviceID);
  static bool is_channel_online(int UserID, int DeviceID, int ChannelID);
  static bool get_channel_double_value(int UserID, int DeviceID, int ChannelID,
                                       double *Value);
  static bool get_channel_temperature_value(int UserID, int DeviceID,
                                            int ChannelID, double *Value);
  static bool get_channel_humidity_value(int UserID, int DeviceID,
                                         int ChannelID, double *Value);
  static bool get_channel_char_value(int UserID, int DeviceID, int ChannelID,
                                     char *Value);
  static bool get_channel_rgbw_value(int UserID, int DeviceID, int ChannelID,
                                     int *color, char *color_brightness,
                                     char *brightness, char *on_off);
  static bool get_channel_valve_value(int UserID, int DeviceID, int ChannelID,
                                      TValve_Value *Value);

  static int user_count(void);
  static supla_user *get_user(int idx);

  static void on_amazon_alexa_credentials_changed(int UserID);
  static void on_google_home_credentials_changed(int UserID);
  static void on_state_webhook_changed(int UserID);
  static void on_mqtt_settings_changed(int UserID);
  static void before_channel_function_change(int UserID, int ChannelID,
                                             const supla_caller &caller);
  static void before_device_delete(int UserID, int DeviceID,
                                   const supla_caller &caller);
  static void on_device_deleted(int UserID, int DeviceID,
                                const supla_caller &caller);
  static void on_device_settings_changed(int UserID, int DeviceID,
                                         const supla_caller &caller);
  static unsigned int total_cd_count(bool client);
  static void log_metrics(int min_interval_sec);

  void reconnect(const supla_caller &caller);
  void reconnect(const supla_caller &caller, bool allDevices, bool allClients);

  void on_channels_added(int DeviceID, const supla_caller &caller);
  void on_device_registered(int DeviceID, const supla_caller &caller);

  void moveDeviceToTrash(supla_device *device);
  void moveClientToTrash(supla_client *client);
  void emptyTrash(void);

  int getUserID(void);
  const char *getShortUniqueID(void);
  const char *getLongUniqueID(void);
  bool getClientName(int ClientID, char *buffer, int size);
  bool isSuperUserAuthorized(int ClientID);

  // Remember to call device->releasePtr()
  // Deprecated. Use the access_device method
  static supla_device *get_device(int UserID, int DeviceID);
  supla_device *get_device(int DeviceID);
  supla_device *device_by_channelid(int ChannelID);
  // ----

  void access_device(int DeviceId, int ChannelId,  // DeviceId or ChannelId
                     std::function<void(supla_device *device)> on_device);
  static void access_device(
      int UserID, int DeviceId, int ChannelId,  // DeviceId or ChannelId
      std::function<void(supla_device *device)> on_device);

  void for_each_device(std::function<bool(supla_device *device)> on_device);

  bool get_channel_double_value(int DeviceID, int ChannelID, double *Value);
  bool get_channel_temperature_value(int DeviceID, int ChannelID,
                                     double *Value);
  bool get_channel_humidity_value(int DeviceID, int ChannelID, double *Value);
  bool get_channel_char_value(int DeviceID, int ChannelID, char *Value);
  bool get_channel_rgbw_value(int DeviceID, int ChannelID, int *color,
                              char *color_brightness, char *brightness,
                              char *on_off);
  bool get_channel_valve_value(int DeviceID, int ChannelID,
                               TValve_Value *Value);

  bool is_client_online(int DeviceID);
  bool is_device_online(int DeviceID);
  bool is_channel_online(int DeviceID, int ChannelID);
  bool get_channel_value(int DeviceID, int ChannelID,
                         char value[SUPLA_CHANNELVALUE_SIZE],
                         char sub_value[SUPLA_CHANNELVALUE_SIZE],
                         char *sub_value_type, char *online,
                         unsigned _supla_int_t *validity_time_sec,
                         bool for_client);

  bool set_device_channel_value(const supla_caller &caller, int DeviceID,
                                int ChannelID, int GroupID, unsigned char EOL,
                                const char value[SUPLA_CHANNELVALUE_SIZE]);

  void update_client_device_channels(int LocationID, int DeviceID);
  void on_channel_value_changed(const supla_caller &caller, int DeviceId,
                                int ChannelId = 0, bool Extended = false,
                                bool SignificantChange = true);
  void on_channel_become_online(int DeviceId, int ChannelId);

  void call_event(TSC_SuplaEvent *event);

  bool device_calcfg_request(int SenderID, int DeviceId, int ChannelId,
                             TCS_DeviceCalCfgRequest_B *request);
  void on_device_calcfg_result(int ChannelID, TDS_DeviceCalCfgResult *result);
  void on_device_channel_state_result(int ChannelID, TDSC_ChannelState *state);

  bool device_get_channel_state(int SenderID, int DeviceId,
                                TCSD_ChannelStateRequest *request);

  channel_complex_value get_channel_complex_value(int ChannelID);

  void set_channel_function(supla_client *sender, TCS_SetChannelFunction *func);
  void set_caption(supla_client *sender, TCS_SetCaption *caption, bool channel);

  supla_amazon_alexa_credentials *amazonAlexaCredentials(void);
  supla_google_home_credentials *googleHomeCredentials(void);
  supla_state_webhook_credentials *stateWebhookCredentials(void);

  explicit supla_user(int UserID);
  supla_user(int UserID, const char *short_unique_id,
             const char *long_unique_id);
  supla_user_channelgroups *get_channel_groups(void);
  virtual ~supla_user();
};

#endif /* USER_H_ */
