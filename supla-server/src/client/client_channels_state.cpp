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

#include "client/client_channels_state.h"

#include <list>

#include "client/client_scene_change_indicator.h"
#include "client/clientchannel.h"
#include "device/channel_state.h"

using std::list;

supla_client_channels_state::supla_client_channels_state(
    supla_abstract_dobject_remote_updater *updater,
    supla_abstract_client_dao *dao)
    : supla_dobjects(updater) {
  this->dao = dao;
}

supla_client_channels_state::~supla_client_channels_state() {}

void supla_client_channels_state::load(int client_protocol_version, int user_id,
                                       int client_id) {
  if (!get_updater()->is_protocol_version_allowed(client_protocol_version)) {
    return;
  }

  list<supla_channel_state *> cs = dao->get_chanels_state(user_id, client_id);

  lock();
  clear();
  for (auto it = cs.begin(); it != cs.end(); ++it) {
    (*it)->set_change_indicator(new supla_client_scene_change_indicator());
    add(*it);
  }

  unlock();
}

bool supla_client_channels_state::update_remote(void) {
  bool result = supla_dobjects::update_remote();
  if (!result) {
    clear();
  }

  return result;
}
