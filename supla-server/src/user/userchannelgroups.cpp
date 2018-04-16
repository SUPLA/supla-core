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

supla_user_channelgroups::supla_user_channelgroups(supla_user *user) {
  this->user = user;
  id_cmp_use_both[master] = true;
  load();
}

void supla_user_channelgroups::_load(database *db, e_objc_scope scope) {
  db->get_user_channel_groups(user->getUserID(), this);
}

