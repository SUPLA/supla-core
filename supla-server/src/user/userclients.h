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

#ifndef USERCLIENTS_H_
#define USERCLIENTS_H_

#include <functional>
#include <list>
#include <memory>

#include "channel_address.h"
#include "client.h"
#include "conn/connection_objects.h"

class supla_user_clients : public supla_connection_objects {
 public:
  supla_user_clients();
  virtual ~supla_user_clients();
  void for_each(std::function<void(std::shared_ptr<supla_client> client,
                                   bool *will_continue)>
                    on_client);
  bool add(std::shared_ptr<supla_client> client);
  std::shared_ptr<supla_client> get(int client_id);
  void set_channel_function(int channel_id, int func);
  void update_device_channels(int location_id, int device_id,
                              bool with_channel_relations);
  void on_channel_value_changed(std::list<channel_address> addr_list,
                                bool extended);
  void on_device_deleted(int device_id);
  void on_channel_added(int channel_id);
  void on_channel_deleted(int channel_id);
  void call_event(TSC_SuplaEvent *event);
  void set_channel_caption(int channel_id, char *caption);
  void set_channel_group_caption(int channel_group_id, char *caption);
  void set_location_caption(int location_id, char *caption);
  void set_scene_caption(int location_id, char *caption);
  void update_json_config(int channel_id, unsigned char config_type,
                          supla_json_config *json_config);
};

#endif /* USERCLIENTS_H_ */
