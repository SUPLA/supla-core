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

#ifndef CLIENT_H_
#define CLIENT_H_

#include <conn/abstract_connection_object.h>

#include <memory>

#include "client_scene_dao.h"
#include "client_scene_remote_updater.h"
#include "client_scenes.h"
#include "clientchannelgroups.h"
#include "clientchannels.h"
#include "clientlocation.h"

class supla_client_call_handler_collection;
class supla_user;
class supla_connection;
class supla_client : public supla_abstract_connection_object {
 private:
  char name[SUPLA_CLIENT_NAME_MAXSIZE];
  bool superuser_authorized;
  int access_id;
  static supla_client_call_handler_collection call_handler_collection;

 protected:
  supla_client_locations *locations;
  supla_client_channels *channels;
  supla_client_channelgroups *cgroups;
  supla_client_scenes *scenes;

  supla_client_scene_remote_updater *scene_remote_updater;
  supla_client_scene_dao scene_dao;

  void loadIODevices(void);
  void loadConfig(void);

  void remote_update_lists(void);
  void setName(const char *name);
  void setAccessID(int AccessID);
  bool db_authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                       const char Email[SUPLA_EMAIL_MAXSIZE],
                       const char AuthKey[SUPLA_AUTHKEY_SIZE], int *UserID,
                       database *db);

 public:
  explicit supla_client(supla_connection *connection);
  virtual supla_abstract_srpc_call_handler_collection *
  get_srpc_call_handler_collection(void);
  std::shared_ptr<supla_client> get_shared_ptr(void);
  supla_client_channels *get_channels(void);
  supla_client_channelgroups *get_cgroups(void);

  void iterate();
  unsigned _supla_int64_t wait_time_usec();

  void superuser_authorize(int UserID, const char Email[SUPLA_EMAIL_MAXSIZE],
                           const char Password[SUPLA_PASSWORD_MAXSIZE],
                           bool *connection_failed);
  void revoke_superuser_authorization(void);
  bool is_superuser_authorized(void);
  char register_client(TCS_SuplaRegisterClient_B *register_client_b,
                       TCS_SuplaRegisterClient_D *register_client_d,
                       unsigned char proto_version);
  void update_device_channels(int LocationID, int DeviceID);
  void on_channel_value_changed(int DeviceId, int ChannelId = 0,
                                bool Extended = false);
  void get_next(void);
  int getName(char *buffer, int size);
  int getAccessID(void);

  void call_event(TSC_SuplaEvent *event);

  void on_device_calcfg_result(int ChannelID, TDS_DeviceCalCfgResult *result);
  void on_device_channel_state_result(int ChannelID, TDSC_ChannelState *state);

  void set_channel_function(int ChannelId, int Func);
  void set_channel_function_result(TSC_SetChannelFunctionResult *result);
  void set_channel_caption(int ChannelId, char *Caption);
  void set_location_caption(int LocationId, char *Caption);
  void set_caption_result(TSC_SetCaptionResult *result, bool channel);

  virtual ~supla_client();
};

#endif /* CLIENT_H_ */
