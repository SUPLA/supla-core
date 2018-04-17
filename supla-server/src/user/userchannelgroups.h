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

#include <list>
#include "objcontainer.h"
#include "user.h"
#include "userchannelgroup.h"

class supla_user;
class supla_user_channelgroups : public supla_objcontainer {
 private:
  supla_user *user;

 protected:
  void _load(database *db, e_objc_scope scope);
  std::list<t_dc_pair> find_channels(int GroupId);

 public:
  supla_user_channelgroups(supla_user *user);
  bool set_char_value(int GroupID, const char value);
  bool set_rgbw_value(int GroupID, int color, char color_brightness,
                      char brightness);
};

#endif /* USERCHANNELGROUPS_H_ */
