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

#include "proto.h"

class supla_device;
class supla_client;
class supla_user_channelgroups;

class supla_user {
 protected:
  static void *user_arr;

  void *device_arr;
  void *client_arr;
  supla_user_channelgroups *cgroups;
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

 public:
  static void init(void);
  static void free(void);
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
                                     char *brightness);
  static int user_count(void);
  static supla_user *get_user(int idx);
  static bool set_device_channel_char_value(int UserID, int SenderID,
                                            int DeviceID, int ChannelID,
                                            const char value);
  static bool set_device_channel_rgbw_value(int UserID, int SenderID,
                                            int DeviceID, int ChannelID,
                                            int color, char color_brightness,
                                            char brightness);
  static bool set_channelgroup_char_value(int UserID, int GroupID,
                                          const char value);
  static bool set_channelgroup_rgbw_value(int UserID, int GroupID, int color,
                                          char color_brightness,
                                          char brightness);

  void remove_device(supla_device *device);
  void remove_client(supla_client *client);

  int getUserID(void);
  bool getClientName(int ClientID, char *buffer, int size);

  bool get_channel_double_value(int DeviceID, int ChannelID, double *Value);
  bool get_channel_temperature_value(int DeviceID, int ChannelID,
                                     double *Value);
  bool get_channel_humidity_value(int DeviceID, int ChannelID, double *Value);
  bool get_channel_char_value(int DeviceID, int ChannelID, char *Value);
  bool get_channel_rgbw_value(int DeviceID, int ChannelID, int *color,
                              char *color_brightness, char *brightness);

  bool is_client_online(int DeviceID);
  bool is_device_online(int DeviceID);
  bool get_channel_value(int DeviceID, int ChannelID, TSuplaChannelValue *value,
                         char *online);

  bool set_device_channel_value(int SenderID, int DeviceID, int ChannelID,
                                const char value[SUPLA_CHANNELVALUE_SIZE]);
  bool set_device_channel_char_value(int SenderID, int DeviceID, int ChannelID,
                                     const char value);
  bool set_device_channel_rgbw_value(int SenderID, int DeviceID, int ChannelID,
                                     int color, char color_brightness,
                                     char brightness);

  bool set_channelgroup_char_value(int GroupID, const char value);
  bool set_channelgroup_rgbw_value(int GroupID, int color,
                                   char color_brightness, char brightness);

  void update_client_device_channels(int LocationID, int DeviceID);
  void on_channel_value_changed(int DeviceId, int ChannelId = 0);

  void call_event(TSC_SuplaEvent *event);
  void get_temp_and_humidity(void *tarr);

  explicit supla_user(int UserID);
  virtual ~supla_user();
};

#endif /* USER_H_ */
