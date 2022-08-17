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

#include <memory>

#include "database.h"
#include "safearray.h"
#include "user.h"

using std::function;
using std::list;
using std::shared_ptr;

supla_user_channelgroups::supla_user_channelgroups(supla_user *user) {
  this->user = user;
  id_cmp_use_both[master] = true;
  load();
}

void supla_user_channelgroups::_load(database *db, e_objc_scope scope) {
  db->get_user_channel_groups(user->getUserID(), this);
}

list<dcpair> supla_user_channelgroups::find_channels(int GroupId) {
  list<dcpair> pairs;
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
    function<bool(supla_device *, int, char)> f) {
  bool result = false;

  list<dcpair> pairs = find_channels(GroupID);

  dcpair::sort_by_device_id(&pairs);

  for (auto it = pairs.begin(); it != pairs.end(); it++) {
    shared_ptr<supla_device> device =
        user->get_devices()->get(it->getDeviceId());
    if (device &&
        f(device.get(), it->getChannelId(), dcpair::last_one(&pairs, it))) {
      result = true;
    }

    if (break_on_success && result) {
      break;
    }
  }

  return result;
}

bool supla_user_channelgroups::for_each_channel(
    int GroupID, function<bool(supla_device *, int, char)> f) {
  return for_each_channel(GroupID, false, f);
}

bool supla_user_channelgroups::set_new_value(const supla_caller &caller,
                                             TCS_SuplaNewValue *new_value) {
  if (new_value->Target != SUPLA_TARGET_GROUP) {
    return false;
  }

  return for_each_channel(
      new_value->Id,
      [new_value, this, caller](supla_device *device, int channelId,
                                char EOL) -> bool {
        return user->set_device_channel_value(caller, device->get_id(),
                                              channelId, new_value->Id, EOL,
                                              new_value->value);
      });
}

bool supla_user_channelgroups::set_char_value(const supla_caller &caller,
                                              int GroupID, const char value) {
  return for_each_channel(
      GroupID,
      [caller, GroupID, value](supla_device *device, int channelId,
                               char EOL) -> bool {
        return device->get_channels()->set_device_channel_char_value(
            caller, channelId, GroupID, EOL, value);
      });
}

bool supla_user_channelgroups::set_on(const supla_caller &caller, int GroupID,
                                      bool on) {
  return for_each_channel(
      GroupID,
      [caller, GroupID, on](supla_device *device, int channelId,
                            char EOL) -> bool {
        return device->get_channels()->set_on(caller, channelId, GroupID, EOL,
                                              on);
      });
}

bool supla_user_channelgroups::set_color(const supla_caller &caller,
                                         int GroupID, unsigned int color) {
  return set_rgbw_value(caller, GroupID, &color, NULL, NULL, NULL);
}

bool supla_user_channelgroups::set_color_brightness(const supla_caller &caller,
                                                    int GroupID,
                                                    char color_brightness) {
  return set_rgbw_value(caller, GroupID, NULL, &color_brightness, NULL, NULL);
}

bool supla_user_channelgroups::set_brightness(const supla_caller &caller,
                                              int GroupID, char brightness) {
  return set_rgbw_value(caller, GroupID, NULL, NULL, &brightness, NULL);
}

bool supla_user_channelgroups::set_rgbw_value(const supla_caller &caller,
                                              int GroupID, unsigned int *color,
                                              char *color_brightness,
                                              char *brightness, char *on_off) {
  return for_each_channel(
      GroupID,
      [caller, GroupID, color, color_brightness, brightness, on_off](
          supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->set_rgbw(caller, channelId, GroupID, EOL,
                                                color, color_brightness,
                                                brightness, on_off);
      });
}

bool supla_user_channelgroups::set_rgbw_value(const supla_caller &caller,
                                              int GroupID, int color,
                                              char color_brightness,
                                              char brightness, char on_off) {
  return for_each_channel(
      GroupID,
      [caller, GroupID, color, color_brightness, brightness, on_off](
          supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->set_device_channel_rgbw_value(
            caller, channelId, GroupID, EOL, color, color_brightness,
            brightness, on_off);
      });
}

bool supla_user_channelgroups::calcfg_request(
    const supla_caller &caller, TCS_DeviceCalCfgRequest_B *request) {
  if (request == NULL || request->Target != SUPLA_TARGET_GROUP) {
    return false;
  }

  return for_each_channel(
      request->Id,
      [this, caller, request](supla_device *device, int channelId,
                              char EOL) -> bool {
        /*
         *
         */
        // return user->device_calcfg_request(SenderID, device->getID(),
        // channelId,
        //                                   request);
        return false;
      });
}

bool supla_user_channelgroups::action_toggle(const supla_caller &caller,
                                             int GroupID) {
  bool any_on = for_each_channel(
      GroupID, true, [](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->is_on(channelId);
      });

  return for_each_channel(
      GroupID,
      [caller, any_on, GroupID](supla_device *device, int channelId,
                                char EOL) -> bool {
        return device->get_channels()->set_on(caller, channelId, GroupID, EOL,
                                              !any_on);
      });
}

bool supla_user_channelgroups::action_shut(const supla_caller &caller,
                                           int GroupID,
                                           const char *closing_percentage) {
  return for_each_channel(
      GroupID,
      [caller, GroupID, closing_percentage](supla_device *device, int channelId,
                                            char EOL) -> bool {
        return device->get_channels()->action_shut(caller, channelId, GroupID,
                                                   EOL, closing_percentage);
      });
}

bool supla_user_channelgroups::action_reveal(const supla_caller &caller,
                                             int GroupID) {
  return for_each_channel(
      GroupID,
      [caller, GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_reveal(caller, channelId, GroupID,
                                                     EOL);
      });
}

bool supla_user_channelgroups::action_stop(const supla_caller &caller,
                                           int GroupID) {
  return for_each_channel(
      GroupID,
      [caller, GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_stop(caller, channelId, GroupID,
                                                   EOL);
      });
}

bool supla_user_channelgroups::action_up(const supla_caller &caller,
                                         int GroupID) {
  return for_each_channel(
      GroupID,
      [caller, GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_up(caller, channelId, GroupID,
                                                 EOL);
      });
}

bool supla_user_channelgroups::action_down(const supla_caller &caller,
                                           int GroupID) {
  return for_each_channel(
      GroupID,
      [caller, GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_down(caller, channelId, GroupID,
                                                   EOL);
      });
}

bool supla_user_channelgroups::action_up_or_stop(const supla_caller &caller,
                                                 int GroupID) {
  return for_each_channel(
      GroupID,
      [caller, GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_up_or_stop(caller, channelId,
                                                         GroupID, EOL);
      });
}

bool supla_user_channelgroups::action_down_or_stop(const supla_caller &caller,
                                                   int GroupID) {
  return for_each_channel(
      GroupID,
      [caller, GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_down_or_stop(caller, channelId,
                                                           GroupID, EOL);
      });
}

bool supla_user_channelgroups::action_step_by_step(const supla_caller &caller,
                                                   int GroupID) {
  return for_each_channel(
      GroupID,
      [caller, GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_step_by_step(caller, channelId,
                                                           GroupID, EOL);
      });
}

bool supla_user_channelgroups::action_open(const supla_caller &caller,
                                           int GroupID) {
  return for_each_channel(
      GroupID,
      [caller, GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_open(caller, channelId, GroupID,
                                                   EOL);
      });
}

bool supla_user_channelgroups::action_close(const supla_caller &caller,
                                            int GroupID) {
  return for_each_channel(
      GroupID,
      [caller, GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_close(caller, channelId, GroupID,
                                                    EOL);
      });
}

bool supla_user_channelgroups::action_open_close(const supla_caller &caller,
                                                 int GroupID) {
  return for_each_channel(
      GroupID,
      [caller, GroupID](supla_device *device, int channelId, char EOL) -> bool {
        return device->get_channels()->action_open_close(caller, channelId,
                                                         GroupID, EOL);
      });
}
