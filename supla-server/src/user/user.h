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

#include "commontypes.h"
#include "proto.h"

class supla_device;
class supla_client;
class supla_user_channelgroups;
class supla_amazon_alexa;
class supla_google_home;

class supla_user {
 private:
  char *short_unique_id;
  char *long_unique_id;
  void *lck;

 protected:
  static void *user_arr;

  void *device_arr;
  void *client_arr;
  supla_user_channelgroups *cgroups;
  supla_amazon_alexa *amazon_alexa;
  supla_google_home *google_home;
  int UserID;
  bool connections_allowed;

  supla_device *find_device(int DeviceID);
  supla_device *find_device_by_channelid(int ChannelID);
  supla_client *find_client(int ClientID);
  supla_device *device_by_channel_id(supla_device *suspect, int ChannelID);
  supla_device *channel_master_device(supla_device *suspect, int ChannelID);

  static char find_user_byid(void *ptr, void *UserID);
  static char find_device_byid(void *ptr, void *ID);
  static char find_device_by_channelid(void *ptr, void *ID);
  static char find_device_byguid(void *ptr, void *GUID);
  static char find_client_byid(void *ptr, void *ID);
  static char find_client_byguid(void *ptr, void *GUID);
  static bool get_channel_double_value(int UserID, int DeviceID, int ChannelID,
                                       double *Value, char Type);

  bool get_channel_double_value(int DeviceID, int ChannelID, double *Value,
                                char Type);

  void reconnect();
  bool client_reconnect(int ClientID);
  void loadUniqueIDs(void);

 public:
  static void init(void);
  static void user_free(void);
  static supla_user *add_device(supla_device *device, int UserID);
  static supla_user *add_client(supla_client *client, int UserID);
  static supla_user *find(int UserID, bool create);
  static bool reconnect(int UserID);
  static bool client_reconnect(int UserID, int ClientID);
  static bool is_client_online(int UserID, int ClientID);
  static bool is_device_online(int UserID, int DeviceID);
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
  static int user_count(void);
  static supla_user *get_user(int idx);
  static bool set_device_channel_char_value(int UserID, int SenderID,
                                            int DeviceID, int ChannelID,
                                            const char value,
                                            event_source_type eventSourceType,
                                            char *AlexaCorrelationToken,
                                            char *GoogleRequestId);
  static bool set_device_channel_rgbw_value(int UserID, int SenderID,
                                            int DeviceID, int ChannelID,
                                            int color, char color_brightness,
                                            char brightness, char on_off,
                                            event_source_type eventSourceType,
                                            char *AlexaCorrelationToken, char *GoogleRequestId);
  static bool set_channelgroup_char_value(int UserID, int GroupID,
                                          const char value);
  static bool set_channelgroup_rgbw_value(int UserID, int GroupID, int color,
                                          char color_brightness,
                                          char brightness, char on_off);
  static void on_amazon_alexa_credentials_changed(int UserID);
  static void on_google_home_credentials_changed(int UserID);
  static void on_device_deleted(int UserID, event_source_type eventSourceType);
  void on_device_added(int DeviceID, event_source_type eventSourceType);

  void remove_device(supla_device *device);
  void remove_client(supla_client *client);

  int getUserID(void);
  char *getShortUniqueID(void);
  char *getLongUniqueID(void);
  bool getClientName(int ClientID, char *buffer, int size);
  bool isSuperUserAuthorized(int ClientID);

  bool get_channel_double_value(int DeviceID, int ChannelID, double *Value);
  bool get_channel_temperature_value(int DeviceID, int ChannelID,
                                     double *Value);
  bool get_channel_humidity_value(int DeviceID, int ChannelID, double *Value);
  bool get_channel_char_value(int DeviceID, int ChannelID, char *Value);
  bool get_channel_rgbw_value(int DeviceID, int ChannelID, int *color,
                              char *color_brightness, char *brightness,
                              char *on_off);

  bool is_client_online(int DeviceID);
  bool is_device_online(int DeviceID);
  bool get_channel_value(int DeviceID, int ChannelID, TSuplaChannelValue *value,
                         char *online);
  bool get_channel_extendedvalue(int DeviceID, int ChannelID,
                                 TSuplaChannelExtendedValue *value);

  bool set_device_channel_value(event_source_type eventSourceType, int SenderID,
                                int DeviceID, int ChannelID,
                                const char value[SUPLA_CHANNELVALUE_SIZE]);
  bool set_device_channel_char_value(int SenderID, int DeviceID, int ChannelID,
                                     const char value);
  bool set_device_channel_rgbw_value(int SenderID, int DeviceID, int ChannelID,
                                     int color, char color_brightness,
                                     char brightness, char on_off);

  bool set_channelgroup_char_value(int GroupID, const char value);
  bool set_channelgroup_rgbw_value(int GroupID, int color,
                                   char color_brightness, char brightness,
                                   char on_off);

  void update_client_device_channels(int LocationID, int DeviceID);
  void on_channel_value_changed(event_source_type eventSourceType, int DeviceId,
                                int ChannelId = 0, bool Extended = false);
  void on_channel_become_online(int DeviceId, int ChannelId);

  void call_event(TSC_SuplaEvent *event);
  void get_temp_and_humidity(void *tarr);
  void get_electricity_measurement(void *emarr);
  void get_ic_measurement(void *icarr);

  bool device_calcfg_request(int SenderID, int DeviceId,
                             TCS_DeviceCalCfgRequest *request);
  void on_device_calcfg_result(int ChannelID, TDS_DeviceCalCfgResult *result);

  channel_complex_value get_channel_complex_value(int DeviceId, int ChannelID);

  supla_amazon_alexa *amazonAlexa(void);
  supla_google_home *googleHome(void);

  explicit supla_user(int UserID);
  virtual ~supla_user();
};

#endif /* USER_H_ */
