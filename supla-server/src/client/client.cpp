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

#include "client.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <vector>

#include "client/call_handler/call_handler_collection.h"
#include "clientlocation.h"
#include "conn/authkey_cache.h"
#include "db/database.h"
#include "device/device_dao.h"
#include "lck.h"
#include "log.h"
#include "safearray.h"
#include "scene/scene_asynctask.h"
#include "srpc/srpc.h"
#include "user.h"
#include "user/userchannelgroups.h"

using std::dynamic_pointer_cast;
using std::shared_ptr;
using std::string;

// static
supla_client_call_handler_collection supla_client::call_handler_collection;

supla_client::supla_client(supla_connection *connection)
    : supla_abstract_connection_object(connection) {
  this->locations = new supla_client_locations();
  this->channels = new supla_client_channels(this);
  this->cgroups = new supla_client_channelgroups(this);
  this->client_dao = new supla_client_dao(&dba);
  this->scene_dao = new supla_client_scene_dao(&dba);
  this->scene_remote_updater =
      new supla_client_scene_remote_updater(connection->get_srpc_adapter());
  this->scenes = new supla_client_scenes(scene_remote_updater, scene_dao,
                                         supla_scene_asynctask::get_queue());
  this->name[0] = 0;
  this->superuser_authorized = false;
  this->access_id = 0;
  this->channel_relation_remote_updater =
      new supla_client_channel_relation_remote_updater(
          connection->get_srpc_adapter());
  this->channel_relations =
      new supla_client_channel_reactions(channel_relation_remote_updater);

  this->channel_state_remote_updater =
      new supla_client_channel_state_remote_updater(
          connection->get_srpc_adapter());
  this->channels_state =
      new supla_client_channels_state(channel_state_remote_updater, client_dao);
}

supla_client::~supla_client() {
  delete cgroups;
  delete channels;
  delete locations;
  delete scenes;
  delete scene_remote_updater;
  delete scene_dao;
  delete client_dao;
  delete channel_relations;
  delete channel_relation_remote_updater;
  delete channels_state;
  delete channel_state_remote_updater;
}

supla_abstract_srpc_call_handler_collection *
supla_client::get_srpc_call_handler_collection(void) {
  return &supla_client::call_handler_collection;
}

shared_ptr<supla_client> supla_client::get_shared_ptr(void) {
  return dynamic_pointer_cast<supla_client>(
      supla_abstract_connection_object::get_shared_ptr());
}

supla_client_locations *supla_client::get_locations(void) { return locations; }

supla_client_channels *supla_client::get_channels(void) { return channels; }

supla_client_channelgroups *supla_client::get_cgroups(void) { return cgroups; }

supla_client_scenes *supla_client::get_scenes(void) { return scenes; }

bool supla_client::is_sleeping_object(void) { return false; }

unsigned int supla_client::get_time_to_wakeup_msec(void) { return 0; }

void supla_client::set_name(const char *name) {
  lock();
  snprintf(this->name, SUPLA_CLIENT_NAME_MAXSIZE, "%s", name);
  this->name[SUPLA_CLIENT_NAME_MAXSIZE - 1] = 0;
  unlock();
}

int supla_client::get_name(char *buffer, int size) {
  if (size < 1) return 0;

  buffer[0] = 0;

  lock();
  snprintf(buffer, size, "%s", this->name);
  unlock();

  buffer[size - 1] = 0;
  return strnlen(buffer, size - 1);
}

string supla_client::get_name(void) { return name; }

void supla_client::set_access_id(int access_id) { this->access_id = access_id; }

int supla_client::getAccessID(void) { return access_id; }

void supla_client::revoke_superuser_authorization(void) {
  lock();
  superuser_authorized = false;
  unlock();
}

bool supla_client::is_superuser_authorized(void) {
  bool result = false;
  lock();
  result = superuser_authorized;
  unlock();

  return result;
}

void supla_client::update_device_channels(int LocationID, int DeviceID,
                                          bool IncludeChannelRelations) {
  if (locations->location_exists(LocationID)) {
    channels->update_device_channels(DeviceID);

    if (IncludeChannelRelations) {
      channel_relations->load(channels);
    }

    remote_update_lists();
  }
}

void supla_client::on_channel_value_changed(int DeviceId, int ChannelId,
                                            bool Extended) {
  channels->on_channel_value_changed(
      get_connection()->get_srpc_adapter()->get_srpc(), DeviceId, ChannelId,
      Extended);
  if (!Extended) {
    cgroups->on_channel_value_changed(
        get_connection()->get_srpc_adapter()->get_srpc(), DeviceId, ChannelId);
  }
}

void supla_client::remote_update_lists(void) {
  if (locations->remote_update(
          get_connection()->get_srpc_adapter()->get_srpc()))
    return;

  if (channels->remote_update(get_connection()->get_srpc_adapter()->get_srpc()))
    return;

  if (cgroups->remote_update(get_connection()->get_srpc_adapter()->get_srpc()))
    return;

  if (scenes->update_remote()) return;

  if (channel_relations->update_remote()) return;

  if (channels_state->update_remote()) return;
}

void supla_client::load_config(void) {
  locations->load(get_id());
  channels->load();
  cgroups->load();
  scenes->load(get_user_id(), get_id());
  channel_relations->load(channels);
  channels_state->load(get_protocol_version(), get_user_id(), get_id());
}

void supla_client::get_next(void) { remote_update_lists(); }

void supla_client::call_event(TSC_SuplaEvent *event) {
  if (event != NULL && (event->Event != SUPLA_EVENT_SET_BRIDGE_VALUE_FAILED ||
                        event->SenderID == get_id())) {
    srpc_sc_async_event(get_connection()->get_srpc_adapter()->get_srpc(),
                        event);
  }
}

void supla_client::superuser_authorize(
    int UserID, const char Email[SUPLA_EMAIL_MAXSIZE],
    const char Password[SUPLA_PASSWORD_MAXSIZE], bool *connection_failed) {
  revoke_superuser_authorization();

  if (UserID == 0 || Email == NULL || Password == NULL ||
      strnlen(Email, SUPLA_EMAIL_MAXSIZE) == 0 ||
      strnlen(Password, SUPLA_PASSWORD_MAXSIZE) == 0) {
    return;
  }

  database *db = new database();

  if (db->connect() == true) {
    if (db->superuser_authorization(UserID, Email, Password)) {
      lock();
      superuser_authorized = true;
      unlock();
    }
  } else if (connection_failed) {
    *connection_failed = true;
  }

  delete db;
}

void supla_client::on_device_calcfg_result(int ChannelID,
                                           TDS_DeviceCalCfgResult *result) {
  if (result == NULL) return;

  TSC_DeviceCalCfgResult cresult;
  memset(&cresult, 0, sizeof(TSC_DeviceCalCfgResult));

  cresult.ChannelID = ChannelID;
  cresult.Command = result->Command;
  cresult.Result = result->Result;
  cresult.DataSize = result->DataSize > SUPLA_CALCFG_DATA_MAXSIZE
                         ? SUPLA_CALCFG_DATA_MAXSIZE
                         : result->DataSize;
  memcpy(cresult.Data, result->Data, SUPLA_CALCFG_DATA_MAXSIZE);

  srpc_sc_async_device_calcfg_result(
      get_connection()->get_srpc_adapter()->get_srpc(), &cresult);
}

void supla_client::on_device_channel_state_result(int ChannelID,
                                                  TDSC_ChannelState *state) {
  if (state == NULL) return;

  TDSC_ChannelState cstate;
  memcpy(&cstate, state, sizeof(TDSC_ChannelState));

  cstate.ChannelID = ChannelID;
  cstate.ReceiverID = get_id();

  srpc_csd_async_channel_state_result(
      get_connection()->get_srpc_adapter()->get_srpc(), &cstate);
}

unsigned char supla_client::send_device_config(int device_id,
                                               unsigned _supla_int64_t fields) {
  if (get_protocol_version() < 21) {
    return SUPLA_CONFIG_RESULT_FALSE;
  }

  if (!get_channels()->get_any_channel_id_with_deviceid(device_id)) {
    return SUPLA_CONFIG_RESULT_DEVICE_NOT_FOUND;
  }

  unsigned char result_code = SUPLA_CONFIG_RESULT_FALSE;

  supla_db_access_provider dba;
  supla_device_dao dao(&dba);

  device_json_config *config =
      dao.get_device_config(device_id, nullptr, nullptr);
  if (config) {
    unsigned _supla_int64_t available_fields = config->get_available_fields();
    while (fields) {
      TSDS_SetDeviceConfig sds_config = {};
      config->get_config(&sds_config, fields, &fields);

      TSC_DeviceConfigUpdateOrResult result = {};

      result.Result = SUPLA_CONFIG_RESULT_TRUE;
      result.Config.DeviceId = device_id;
      result.Config.EndOfDataFlag = fields == 0 ? 1 : 0;
      result.Config.AvailableFields = available_fields;
      result.Config.Fields = sds_config.Fields;
      result.Config.ConfigSize = sds_config.ConfigSize;
      memcpy(result.Config.Config, sds_config.Config,
             SUPLA_DEVICE_CONFIG_MAXSIZE);

      get_connection()
          ->get_srpc_adapter()
          ->sc_async_device_config_update_or_result(&result);

      result_code = result.Result;
    }

    delete config;
  }

  return result_code;
}

void supla_client::set_channel_function(int ChannelId, int Func) {
  channels->set_channel_function(
      get_connection()->get_srpc_adapter()->get_srpc(), ChannelId, Func);
}

void supla_client::set_channel_function_result(
    TSC_SetChannelFunctionResult *result) {
  if (result == NULL) {
    return;
  }
  srpc_sc_async_set_channel_function_result(
      get_connection()->get_srpc_adapter()->get_srpc(), result);
}

void supla_client::set_channel_caption(int ChannelId, char *Caption) {
  channels->set_channel_caption(
      get_connection()->get_srpc_adapter()->get_srpc(), ChannelId, Caption);
}

void supla_client::set_channel_group_caption(int ChannelGroupId,
                                             char *Caption) {
  cgroups->set_caption(get_connection()->get_srpc_adapter()->get_srpc(),
                       ChannelGroupId, Caption);
}

void supla_client::set_location_caption(int LocationId, char *Caption) {
  locations->set_caption(LocationId, Caption);
  locations->remote_update(get_connection()->get_srpc_adapter()->get_srpc());
}

void supla_client::set_scene_caption(int scene_id, char *caption) {
  scenes->set_caption(scene_id, caption);
  scenes->update_remote();
}

void supla_client::iterate() {
  supla_abstract_connection_object::iterate();
  channels->update_expired(get_connection()->get_srpc_adapter()->get_srpc());
}

void supla_client::connection_will_close(void) {
  supla_abstract_connection_object::connection_will_close();
  scenes->connection_will_close();
}

unsigned _supla_int64_t supla_client::wait_time_usec() {
  unsigned _supla_int64_t result =
      supla_abstract_connection_object::wait_time_usec();
  unsigned _supla_int64_t validity_time = channels->value_validity_time_usec();
  if (validity_time > 0) {
    validity_time += 500000;
    if (validity_time < result) {
      return validity_time;
    }
  }

  return result;
}

void supla_client::update_json_config(int channel_id, unsigned char config_type,
                                      supla_json_config *json_config) {
  bool relations_changed = false;

  channels->channel_access(
      channel_id, [&](supla_client_channel *channel) -> void {
        std::vector<supla_channel_relation> rel_before =
            channel->get_channel_relations(relation_any);

        channel->set_json_config(
            json_config ? new supla_json_config(json_config, true) : nullptr);

        std::vector<supla_channel_relation> rel_after =
            channel->get_channel_relations(relation_any);

        if (!supla_channel_relation::equal(&rel_before, &rel_after)) {
          relations_changed = true;
        }

        TSC_ChannelConfigUpdateOrResult cfg_result = {};
        cfg_result.Result = SUPLA_CONFIG_RESULT_TRUE;
        channel->get_config(&cfg_result.Config, config_type, nullptr, 0);

        get_connection()
            ->get_srpc_adapter()
            ->sc_async_channel_config_update_or_result(&cfg_result);
      });

  if (relations_changed) {
    channel_relations->load(channels);
    remote_update_lists();
  }
}
