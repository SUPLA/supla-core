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

#ifndef USERCHANNELGROUPS_H_
#define USERCHANNELGROUPS_H_

#include <functional>
#include <list>

#include "caller.h"
#include "dcpair.h"
#include "device/device.h"
#include "objcontainer.h"
#include "user.h"
#include "userchannelgroup.h"

class supla_user;
class supla_user_channelgroups : public supla_objcontainer {
 private:
  supla_user *user;
  bool for_each_channel(int GroupID, bool break_on_success,
                        std::function<bool(supla_device *, int, char)> f);
  bool for_each_channel(int GroupID,
                        std::function<bool(supla_device *, int, char)> f);

 protected:
  void _load(database *db, e_objc_scope scope);
  std::list<dcpair> find_channels(int GroupId);
  int available_data_types_for_remote(e_objc_scope scope);

 public:
  explicit supla_user_channelgroups(supla_user *user);
  bool set_new_value(const supla_caller &caller, TCS_SuplaNewValue *new_value);
  bool set_char_value(const supla_caller &caller, int GroupID,
                      const char value);
  bool set_on(const supla_caller &caller, int GroupID, bool on);
  bool set_color(const supla_caller &caller, int GroupID, unsigned int color);
  bool set_color_brightness(const supla_caller &caller, int GroupID,
                            char color_brightness);
  bool set_brightness(const supla_caller &caller, int GroupID, char brightness);
  bool set_rgbw_value(const supla_caller &caller, int GroupID,
                      unsigned int *color, char *color_brightness,
                      char *brightness, char *on_off);
  bool set_rgbw_value(const supla_caller &caller, int GroupID, int color,
                      char color_brightness, char brightness, char on_off);
  bool calcfg_request(const supla_caller &caller,
                      TCS_DeviceCalCfgRequest_B *request);
  bool action_toggle(const supla_caller &caller, int GroupID);
  bool action_shut(const supla_caller &caller, int GroupID,
                   const char *closing_percentage, bool delta);
  bool action_reveal(const supla_caller &caller, int GroupID);
  bool action_up(const supla_caller &caller, int GroupID);
  bool action_down(const supla_caller &caller, int GroupID);
  bool action_up_or_stop(const supla_caller &caller, int GroupID);
  bool action_down_or_stop(const supla_caller &caller, int GroupID);
  bool action_step_by_step(const supla_caller &caller, int GroupID);
  bool action_stop(const supla_caller &caller, int GroupID);
  bool action_open(const supla_caller &caller, int GroupID);
  bool action_close(const supla_caller &caller, int GroupID);
  bool action_open_close(const supla_caller &caller, int GroupID);
  bool action_set_hvac_parameters(const supla_caller &caller, int group_id,
                                  supla_action_hvac_parameters *params);
};

#endif /* USERCHANNELGROUPS_H_ */
