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
using std::function;
using std::list;
using std::shared_ptr;
using std::vector;

supla_user_clients::supla_user_clients() : supla_connection_objects() {}

supla_user_clients::~supla_user_clients() {}

bool supla_user_clients::add(shared_ptr<supla_client> client) {
  return supla_connection_objects::add(client);
}

std::shared_ptr<supla_client> supla_user_clients::get(int client_id) {
  return dynamic_pointer_cast<supla_client>(
      supla_connection_objects::get(client_id));
}

void supla_user_clients::for_each(
    function<void(shared_ptr<supla_client> client, bool *will_continue)>
        on_client) {
  supla_connection_objects::for_each(
      [on_client](shared_ptr<supla_abstract_connection_object> obj,
                  bool *will_continue) -> void {
        shared_ptr<supla_client> client =
            dynamic_pointer_cast<supla_client>(obj);
        if (client) {
          on_client(client, will_continue);
        }
      });
}

void supla_user_clients::set_channel_function(int channel_id, int func) {
  for_each([channel_id, func](shared_ptr<supla_client> client,
                              bool *will_continue) -> void {
    client->set_channel_function(channel_id, func);
  });
}

void supla_user_clients::update_device_channels(int location_id, int device_id,
                                                bool with_channel_relations) {
  for_each([location_id, device_id, with_channel_relations](
               shared_ptr<supla_client> client, bool *will_continue) -> void {
    client->update_device_channels(location_id, device_id,
                                   with_channel_relations);
  });
}

void supla_user_clients::on_channel_value_changed(
    list<channel_address> addr_list, bool extended) {
  for_each([&addr_list, extended](shared_ptr<supla_client> client,
                                  bool *will_continue) -> void {
    for (auto it = addr_list.begin(); it != addr_list.end(); it++) {
      client->on_channel_value_changed(it->getDeviceId(), it->getChannelId(),
                                       extended);
    }
  });
}

void supla_user_clients::call_event(TSC_SuplaEvent *event) {
  for_each([event](shared_ptr<supla_client> client,
                   bool *will_continue) -> void { client->call_event(event); });
}

void supla_user_clients::set_channel_caption(int channel_id, char *caption) {
  for_each([channel_id, caption](shared_ptr<supla_client> client,
                                 bool *will_continue) -> void {
    client->set_channel_caption(channel_id, caption);
  });
}

void supla_user_clients::set_channel_group_caption(int channel_group_id,
                                                   char *caption) {
  for_each([channel_group_id, caption](shared_ptr<supla_client> client,
                                       bool *will_continue) -> void {
    client->set_channel_group_caption(channel_group_id, caption);
  });
}

void supla_user_clients::set_location_caption(int location_id, char *caption) {
  for_each([location_id, caption](shared_ptr<supla_client> client,
                                  bool *will_continue) -> void {
    client->set_location_caption(location_id, caption);
  });
}

void supla_user_clients::set_scene_caption(int scene_id, char *caption) {
  for_each([scene_id, caption](shared_ptr<supla_client> client,
                               bool *will_continue) -> void {
    client->set_scene_caption(scene_id, caption);
  });
}

void supla_user_clients::update_json_config(int channel_id,
                                            unsigned char config_type,
                                            supla_json_config *json_config) {
  for_each(
      [&](std::shared_ptr<supla_client> client, bool *will_continue) -> void {
        client->update_json_config(channel_id, config_type, json_config);
      });
}

void supla_user_clients::on_channel_deleted(int channel_id) {
  for_each([channel_id](std::shared_ptr<supla_client> client,
                        bool *will_continue) -> void {
    client->on_channel_deleted(channel_id);
  });
}

void supla_user_clients::on_device_deleted(int device_id) {
  for_each([device_id](std::shared_ptr<supla_client> client,
                       bool *will_continue) -> void {
    client->on_device_deleted(device_id);
  });
}
