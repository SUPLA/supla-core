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
#include "device.h"
#include "proto.h"
#include "svrdb.h"
#include "user.h"

class database : public svrdb {
 public:
  char *get_user_email(int UserID);

  bool get_user_uniqueid(int UserID, char *id, bool longid);

  int get_user_id_by_suid(const char *suid);

  void get_client_locations(int ClientID, supla_client_locations *locs);
  void get_client_channels(int ClientID, int *DeviceID,
                           supla_client_channels *channels);

  void get_user_channel_groups(int UserID, supla_user_channelgroups *cgroups);

  void get_client_channel_groups(int ClientID,
                                 supla_client_channelgroups *cgroups);

  void get_client_channel_group_relations(int ClientID,
                                          supla_client_channelgroups *cgroups);

  bool superuser_authorization(int UserID,
                               const char email[SUPLA_EMAIL_MAXSIZE],
                               const char password[SUPLA_PASSWORD_MAXSIZE]);

  bool get_user_localtime(int UserID, TSDC_UserLocalTimeResult *time);
  bool get_channel_basic_cfg(int ChannelID, TSC_ChannelBasicCfg *cfg);
  bool set_channel_function(int UserID, int ChannelID, int Func);
  bool get_channel_type_funclist_and_device_id(int UserID, int ChannelID,
                                               int *Type,
                                               unsigned int *FuncList,
                                               int *DeviceID);
  bool set_caption(int UserID, int ID, char *Caption, int call_id);
  bool channel_belong_to_group(int channel_id);
  bool channel_has_schedule(int channel_id);
  bool channel_is_associated_with_scene(int channel_id);
  bool channel_is_associated_with_action_trigger(int UserID, int ChannelID);
  void update_channel_value(int channel_id, int user_id,
                            const char value[SUPLA_CHANNELVALUE_SIZE],
                            unsigned _supla_int_t validity_time_sec);
  bool get_channel_value(int channel_id, int user_id,
                         char value[SUPLA_CHANNELVALUE_SIZE],
                         unsigned _supla_int_t *validity_time_sec);

  void update_channel_properties(int channel_id, int user_id,
                                 const char *properties);

  void update_channel_params(int channel_id, int user_id, int param1,
                             int param2, int param3, int param4);
  void update_channel_flags(int channel_id, int user_id, unsigned int flags);
};

#endif /* DATABASE_H_ */
