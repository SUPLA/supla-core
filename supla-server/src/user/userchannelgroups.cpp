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

#include "userchannelgroups.h"
#include "database.h"
#include "safearray.h"

supla_user_channelgroups::supla_user_channelgroups(supla_user *user) {
  this->user = user;
  id_cmp_use_both[master] = true;
  load();
}

void supla_user_channelgroups::_load(database *db, e_objc_scope scope) {
  db->get_user_channel_groups(user->getUserID(), this);
}

std::list<dcpair> supla_user_channelgroups::find_channels(int GroupId) {
  std::list<dcpair> pairs;
  void *arr = getArr(master);

  safe_array_lock(arr);

  for (int a = 0; a < safe_array_count(arr); a++) {
    supla_user_channelgroup *g =
        static_cast<supla_user_channelgroup *>(safe_array_get(arr, a));
    if (g && g->getGroupId() == GroupId) {
      dcpair p(g->getDeviceId(), g->getChannelId());
      pairs.push_back(p);
    }
  }

  safe_array_unlock(arr);

  return pairs;
}

int supla_user_channelgroups::available_data_types_for_remote(
    e_objc_scope scope) {
  return 0;
}

bool supla_user_channelgroups::set_char_value(int GroupID, const char value) {
  bool result = false;

  std::list<dcpair> pairs = find_channels(GroupID);

  dcpair::sort_by_device_id(&pairs);

  for (std::list<dcpair>::iterator it = pairs.begin(); it != pairs.end();
       it++) {
    if (user->set_device_channel_char_value(
            0, it->getDeviceId(), it->getChannelId(), GroupID,
            dcpair::last_one(&pairs, it), value)) {
      result = true;
    }
  }

  return result;
}

bool supla_user_channelgroups::set_rgbw_value(int GroupID, int color,
                                              char color_brightness,
                                              char brightness, char on_off) {
  bool result = false;

  std::list<dcpair> pairs = find_channels(GroupID);

  dcpair::sort_by_device_id(&pairs);

  for (std::list<dcpair>::iterator it = pairs.begin(); it != pairs.end();
       it++) {
    if (user->set_device_channel_rgbw_value(
            0, it->getDeviceId(), it->getChannelId(), GroupID,
            dcpair::last_one(&pairs, it), color, color_brightness, brightness,
            on_off)) {
      result = true;
    }
  }

  return result;
}
