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
#include "user.h"

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

bool supla_user_channelgroups::for_each_channel(
    int GroupID, bool break_on_success,
    std::function<bool(supla_device *, int, char)> f) {
  bool result = false;

  std::list<dcpair> pairs = find_channels(GroupID);

  dcpair::sort_by_device_id(&pairs);

  for (std::list<dcpair>::iterator it = pairs.begin(); it != pairs.end();
       it++) {
    user->access_device(
        it->getDeviceId(), 0,
        [&result, f, &it, &pairs](supla_device *device) -> void {
          if (f(device, it->getChannelId(), dcpair::last_one(&pairs, it))) {
            result = true;
          }
        });

    if (break_on_success && result) {
      break;
    }
  }

  return result;
}

bool supla_user_channelgroups::for_each_channel(
    int GroupID, std::function<bool(supla_device *, int, char)> f) {
  return for_each_channel(GroupID, false, f);
}

bool supla_user_channelgroups::set_new_value(event_source_type eventSourceType,
                                             int SenderID,
                                             TCS_SuplaNewValue *new_value) {
  if (new_value->Target != SUPLA_TARGET_GROUP) {
    return false;
  }

  return for_each_channel(
      new_value->Id,
      [new_value, this, eventSourceType, SenderID](
          supla_device *device, int channelId, char EOL) -> bool {
        return user->set_device_channel_value(
            eventSourceType, SenderID, device->getID(), channelId,
            new_value->Id, EOL, new_value->value);
      });
}

bool supla_user_channelgroups::set_char_value(int GroupID, const char value) {
  return for_each_channel(
      GroupID,
      [GroupID, value](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->set_device_channel_char_value(
            0, channelId, GroupID, EOL, value);
      });
}

bool supla_user_channelgroups::set_on(int GroupID, bool on) {
  return for_each_channel(
      GroupID,
      [GroupID, on](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->set_on(0, channelId, GroupID, EOL, on);
      });
}

bool supla_user_channelgroups::set_color(int GroupID, unsigned int color) {
  return set_rgbw_value(GroupID, &color, NULL, NULL, NULL);
}

bool supla_user_channelgroups::set_color_brightness(int GroupID,
                                                    char color_brightness) {
  return set_rgbw_value(GroupID, NULL, &color_brightness, NULL, NULL);
}

bool supla_user_channelgroups::set_brightness(int GroupID, char brightness) {
  return set_rgbw_value(GroupID, NULL, NULL, &brightness, NULL);
}

bool supla_user_channelgroups::set_rgbw_value(int GroupID, unsigned int *color,
                                              char *color_brightness,
                                              char *brightness, char *on_off) {
  return for_each_channel(
      GroupID,
      [GroupID, color, color_brightness, brightness, on_off](
          supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->set_rgbw(0, channelId, GroupID, EOL,
                                                color, color_brightness,
                                                brightness, on_off);
      });
}

bool supla_user_channelgroups::set_rgbw_value(int GroupID, int color,
                                              char color_brightness,
                                              char brightness, char on_off) {
  return for_each_channel(
      GroupID,
      [GroupID, color, color_brightness, brightness, on_off](
          supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->set_device_channel_rgbw_value(
            0, channelId, GroupID, EOL, color, color_brightness, brightness,
            on_off);
      });
}

bool supla_user_channelgroups::calcfg_request(
    int SenderID, TCS_DeviceCalCfgRequest_B *request) {
  if (request == NULL || request->Target != SUPLA_TARGET_GROUP) {
    return false;
  }

  return for_each_channel(
      request->Id,
      [this, SenderID, request](supla_device *device, int channelId,
                                char EOL) -> bool {
        return user->device_calcfg_request(SenderID, device->getID(), channelId,
                                           request);
      });
}

bool supla_user_channelgroups::action_toggle(int GroupID) {
  bool any_on = for_each_channel(
      GroupID, true, [](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->is_on(channelId);
      });

  return for_each_channel(
      GroupID,
      [any_on, GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->set_on(0, channelId, GroupID, EOL,
                                              !any_on);
      });
}

bool supla_user_channelgroups::action_shut(int GroupID,
                                           const char *closing_percentage) {
  return for_each_channel(
      GroupID,
      [GroupID, closing_percentage](supla_device *device, int channelId,
                                    char EOL) -> bool {
        return device->get_channels()->action_shut(0, channelId, GroupID, EOL,
                                                   closing_percentage);
      });
}

bool supla_user_channelgroups::action_reveal(int GroupID) {
  return for_each_channel(
      GroupID,
      [GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_reveal(0, channelId, GroupID,
                                                     EOL);
      });
}

bool supla_user_channelgroups::action_stop(int GroupID) {
  return for_each_channel(
      GroupID,
      [GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_stop(0, channelId, GroupID, EOL);
      });
}

bool supla_user_channelgroups::action_up(int GroupID) {
  return for_each_channel(
      GroupID,
      [GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_up(0, channelId, GroupID, EOL);
      });
}

bool supla_user_channelgroups::action_down(int GroupID) {
  return for_each_channel(
      GroupID,
      [GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_down(0, channelId, GroupID, EOL);
      });
}

bool supla_user_channelgroups::action_up_or_stop(int GroupID) {
  return for_each_channel(
      GroupID,
      [GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_up_or_stop(0, channelId, GroupID,
                                                         EOL);
      });
}

bool supla_user_channelgroups::action_down_or_stop(int GroupID) {
  return for_each_channel(
      GroupID,
      [GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_down_or_stop(0, channelId,
                                                           GroupID, EOL);
      });
}

bool supla_user_channelgroups::action_step_by_step(int GroupID) {
  return for_each_channel(
      GroupID,
      [GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_step_by_step(0, channelId,
                                                           GroupID, EOL);
      });
}

bool supla_user_channelgroups::action_open(int GroupID) {
  return for_each_channel(
      GroupID,
      [GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_open(0, channelId, GroupID, EOL);
      });
}

bool supla_user_channelgroups::action_close(int GroupID) {
  return for_each_channel(
      GroupID,
      [GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_close(0, channelId, GroupID, EOL);
      });
}

bool supla_user_channelgroups::action_open_close(int GroupID) {
  return for_each_channel(
      GroupID,
      [GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_open_close(0, channelId, GroupID,
                                                         EOL);
      });
}
