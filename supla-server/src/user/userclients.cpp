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

#include <userclients.h>
#include <vector>

#include "client/client.h"

using std::dynamic_pointer_cast;
using std::list;
using std::shared_ptr;
using std::vector;

supla_user_clients::supla_user_clients()
    : supla_connection_objects() {}

supla_user_clients::~supla_user_clients() {}

bool supla_user_clients::add(shared_ptr<supla_client> client) {
  return supla_user_clients::add(client);
}

shared_ptr<supla_client> supla_user_clients::find_by_id(
    int client_id) {
  return dynamic_pointer_cast<supla_client>(
      supla_connection_objects::find_by_id(client_id));
}

shared_ptr<supla_client> supla_user_clients::find_by_guid(
    const char *guid) {
  return dynamic_pointer_cast<supla_client>(
      supla_connection_objects::find_by_guid(guid));
}

void supla_user_clients::set_channel_function(int channel_id,
                                                       int func) {
  vector<shared_ptr<supla_connection_object> > objects = get_all();

  for (auto it = objects.begin(); it != objects.end(); ++it) {
    dynamic_pointer_cast<supla_client>(*it)->set_channel_function(channel_id,
                                                                  func);
  }
}

bool supla_user_clients::get_client_name(int client_id, char *buffer,
                                                  int size) {
  if (size < 1) return false;

  buffer[0] = 0;

  shared_ptr<supla_client> client = find_by_id(client_id);

  if (client != nullptr) {
    client->getName(buffer, size);
  }

  return client != NULL;
}

bool supla_user_clients::is_super_user_authorized(int client_id) {
  bool result = false;

  shared_ptr<supla_client> client = find_by_id(client_id);

  if (client != nullptr) {
    result = client->is_superuser_authorized();
  }

  return result;
}

void supla_user_clients::update_device_channels(int location_id,
                                                         int device_id) {
  vector<shared_ptr<supla_connection_object> > objects = get_all();

  for (auto it = objects.begin(); it != objects.end(); ++it) {
    dynamic_pointer_cast<supla_client>(*it)->update_device_channels(location_id,
                                                                    device_id);
  }
}

void supla_user_clients::on_channel_value_changed(
    list<channel_address> addr_list, bool extended) {
  vector<shared_ptr<supla_connection_object> > objects = get_all();

  for (auto it1 = objects.begin(); it1 != objects.end(); ++it1) {
    shared_ptr<supla_client> client = dynamic_pointer_cast<supla_client>(*it1);

    for (auto it2 = addr_list.begin(); it2 != addr_list.end(); it2++) {
      client->on_channel_value_changed(it2->getDeviceId(), it2->getChannelId(),
                                       extended);
    }
  }
}

void supla_user_clients::call_event(TSC_SuplaEvent *event) {
  vector<shared_ptr<supla_connection_object> > objects = get_all();

  for (auto it = objects.begin(); it != objects.end(); ++it) {
    dynamic_pointer_cast<supla_client>(*it)->call_event(event);
  }
}

void supla_user_clients::on_device_calcfg_result(
    int channel_id, TDS_DeviceCalCfgResult *result) {
  if (result != NULL) {
    shared_ptr<supla_client> client = find_by_id(result->ReceiverID);

    if (client != nullptr) {
      client->on_device_calcfg_result(channel_id, result);
    }
  }
}

void supla_user_clients::on_device_channel_state_result(
    int channel_id, TDSC_ChannelState *state) {
  if (state != NULL) {
    shared_ptr<supla_client> client = find_by_id(state->ReceiverID);

    if (client != nullptr) {
      client->on_device_channel_state_result(channel_id, state);
    }
  }
}

void supla_user_clients::set_channel_caption(int channel_id,
                                                      char *caption) {
  vector<shared_ptr<supla_connection_object> > objects = get_all();

  for (auto it = objects.begin(); it != objects.end(); ++it) {
    dynamic_pointer_cast<supla_client>(*it)->set_channel_caption(channel_id,
                                                                 caption);
  }
}

void supla_user_clients::set_location_caption(int location_id,
                                                       char *caption) {
  vector<shared_ptr<supla_connection_object> > objects = get_all();

  for (auto it = objects.begin(); it != objects.end(); ++it) {
    dynamic_pointer_cast<supla_client>(*it)->set_location_caption(location_id,
                                                                  caption);
  }
}
