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

#include "client/call_handler/call_handler_collection.h"
#include "clientlocation.h"
#include "conn/authkey_cache.h"
#include "db/database.h"
#include "lck.h"
#include "log.h"
#include "safearray.h"
#include "scene/scene_asynctask.h"
#include "srpc/srpc.h"
#include "user.h"
#include "user/userchannelgroups.h"

using std::dynamic_pointer_cast;
using std::shared_ptr;

// static
supla_client_call_handler_collection supla_client::call_handler_collection;

supla_client::supla_client(supla_connection *connection)
    : supla_abstract_connection_object(connection) {
  this->locations = new supla_client_locations();
  this->channels = new supla_client_channels(this);
  this->cgroups = new supla_client_channelgroups(this);
  this->scene_dao = new supla_client_scene_dao(&dba);
  this->scene_remote_updater =
      new supla_client_scene_remote_updater(connection->get_srpc_adapter());
  this->scenes = new supla_client_scenes(scene_remote_updater, scene_dao,
                                         supla_scene_asynctask::get_queue());
  this->name[0] = 0;
  this->superuser_authorized = false;
  this->access_id = 0;
}

supla_client::~supla_client() {
  delete cgroups;
  delete channels;
  delete locations;
  delete scenes;
  delete scene_remote_updater;
  delete scene_dao;
}

supla_abstract_srpc_call_handler_collection *
supla_client::get_srpc_call_handler_collection(void) {
  return &supla_client::call_handler_collection;
}

shared_ptr<supla_client> supla_client::get_shared_ptr(void) {
  return dynamic_pointer_cast<supla_client>(
      supla_abstract_connection_object::get_shared_ptr());
}

supla_client_channels *supla_client::get_channels(void) { return channels; }
bool supla_client::is_sleeping_object(void) { return false; }

unsigned int supla_client::get_time_to_wakeup_msec(void) { return 0; }
supla_client_channelgroups *supla_client::get_cgroups(void) { return cgroups; }

void supla_client::setName(const char *name) {
  lock();
  snprintf(this->name, SUPLA_CLIENT_NAME_MAXSIZE, "%s", name);
  this->name[SUPLA_CLIENT_NAME_MAXSIZE - 1] = 0;
  unlock();
}

int supla_client::getName(char *buffer, int size) {
  if (size < 1) return 0;

  buffer[0] = 0;

  lock();
  snprintf(buffer, size, "%s", this->name);
  unlock();

  buffer[size - 1] = 0;
  return strnlen(buffer, size - 1);
}

void supla_client::setAccessID(int AccessID) { access_id = AccessID; }

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

void supla_client::update_device_channels(int LocationID, int DeviceID) {
  if (locations->location_exists(LocationID)) {
    channels->update_device_channels(DeviceID);
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
}

void supla_client::loadConfig(void) {
  locations->load(get_id());
  channels->load();
  cgroups->load();
  scenes->load(get_user_id(), get_id());
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

void supla_client::set_location_caption(int LocationId, char *Caption) {
  locations->set_caption(LocationId, Caption);
  locations->remote_update(get_connection()->get_srpc_adapter()->get_srpc());
}

void supla_client::set_caption_result(TSC_SetCaptionResult *result,
                                      bool channel) {
  if (result == NULL) {
    return;
  }
  if (channel) {
    srpc_sc_async_set_channel_caption_result(
        get_connection()->get_srpc_adapter()->get_srpc(), result);
  } else {
    srpc_sc_async_set_location_caption_result(
        get_connection()->get_srpc_adapter()->get_srpc(), result);
  }
}

void supla_client::iterate() {
  channels->update_expired(get_connection()->get_srpc_adapter()->get_srpc());
}

unsigned _supla_int64_t supla_client::wait_time_usec() {
  unsigned _supla_int64_t time = channels->value_validity_time_usec();
  if (time > 0 && time < 120000000) {
    if (time < 1000000) {
      return 1000000;
    }
    return time + 500000;
  }

  return 120000000;
}
