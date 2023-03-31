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
#include <memory>
#include <vector>

#include "amazon/alexa_credentials.h"
#include "caller.h"
#include "commontypes.h"
#include "google/google_home_credentials.h"
#include "proto.h"
#include "user/userclients.h"
#include "user/userdevices.h"
#include "webhook/state_webhook_credentials.h"

class supla_device;
class supla_client;
class supla_user_channelgroups;
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

  supla_user_clients *clients;
  supla_user_devices *devices;

  supla_user_channelgroups *cgroups;
  supla_amazon_alexa_credentials *amazon_alexa_credentials;
  supla_google_home_credentials *google_home_credentials;
  supla_state_webhook_credentials *state_webhook_credentials;
  int UserID;
  bool connections_allowed;

  static char find_user_by_id(void *ptr, void *UserID);
  static char find_user_by_suid(void *ptr, void *suid);

  void loadUniqueIDs(void);

 public:
  static void init(void);
  static void user_free(void);
  supla_user_devices *get_devices();
  supla_user_clients *get_clients();

  static supla_user *add_device(std::shared_ptr<supla_device> device,
                                int user_id);
  static supla_user *add_client(std::shared_ptr<supla_client> client,
                                int user_id);

  static std::shared_ptr<supla_device> get_device(
      int user_id, int device_id, int channel_id);  // device_id or channel_id
  static std::shared_ptr<supla_client> get_client(int user_id, int client_id);

  static supla_user *find(int UserID, bool create);
  static supla_user *find_by_suid(const char *suid);
  static int suid_to_user_id(const char *suid, bool use_database);
  static bool reconnect(int UserID, const supla_caller &caller);

  static int user_count(void);
  static supla_user *get_user(int user_id);
  static std::vector<supla_user *> get_all_users(void);

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
  void reconnect(const supla_caller &caller, bool all_devices,
                 bool all_clients);

  void on_channels_added(int DeviceID, const supla_caller &caller);
  void on_device_registered(int DeviceID, const supla_caller &caller);

  int getUserID(void);
  const char *getShortUniqueID(void);
  const char *getLongUniqueID(void);

  bool get_channel_value(int device_id, int channel_id,
                         char value[SUPLA_CHANNELVALUE_SIZE],
                         char sub_value[SUPLA_CHANNELVALUE_SIZE],
                         char *sub_value_type, TSuplaChannelExtendedValue *ev,
                         int *function, char *online,
                         unsigned _supla_int_t *validity_time_sec,
                         bool for_client);

  bool set_device_channel_value(const supla_caller &caller, int device_id,
                                int channel_id, int group_id, unsigned char eol,
                                const char value[SUPLA_CHANNELVALUE_SIZE]);

  void on_channel_value_changed(const supla_caller &caller, int device_id,
                                int channel_id = 0, bool extended = false,
                                bool significant_change = true);
  void on_channel_become_online(int DeviceId, int ChannelId);

  bool device_calcfg_request(const supla_caller &caller, int device_id,
                             int channel_id,
                             TCS_DeviceCalCfgRequest_B *request);

  channel_complex_value get_channel_complex_value(int channel_id);

  void set_channel_function(std::shared_ptr<supla_client> sender,
                            TCS_SetChannelFunction *func);
  void set_caption(std::shared_ptr<supla_client> sender,
                   TCS_SetCaption *caption, int call_id);

  supla_amazon_alexa_credentials *amazonAlexaCredentials(void);
  supla_google_home_credentials *googleHomeCredentials(void);
  supla_state_webhook_credentials *stateWebhookCredentials(void);

  static void on_scene_changed(const supla_caller &caller, int user_id,
                               int scene_id);

  explicit supla_user(int UserID);
  supla_user(int UserID, const char *short_unique_id,
             const char *long_unique_id);
  supla_user_channelgroups *get_channel_groups(void);
  virtual ~supla_user();
};

#endif /* USER_H_ */
