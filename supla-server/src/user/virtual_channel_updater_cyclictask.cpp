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

#include "virtual_channel_updater_cyclictask.h"

#include <memory>
#include <string>

#include "user/user.h"
#include "user/user_dao.h"

using std::string;
using std::vector;

supla_virtual_channel_updater_cyclictask::
    supla_virtual_channel_updater_cyclictask()
    : supla_abstract_cyclictask() {}

supla_virtual_channel_updater_cyclictask::
    ~supla_virtual_channel_updater_cyclictask() {}

unsigned int supla_virtual_channel_updater_cyclictask::task_interval_sec(void) {
  return 60;
}
bool supla_virtual_channel_updater_cyclictask::user_access_needed(void) {
  return true;
}

void supla_virtual_channel_updater_cyclictask::run(
    const vector<supla_user *> *users, supla_abstract_db_access_provider *dba) {
  supla_user_dao dao(dba);

  // Load all users with virtual channels online
  vector<int> users_with_vc = dao.get_users_with_virtual_channels();
  for (auto it = users_with_vc.cbegin(); it != users_with_vc.cend(); ++it) {
    supla_user::find(*it, true)->get_devices()->update_virtual_channels();
  }
}
