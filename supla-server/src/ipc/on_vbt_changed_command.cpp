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

#include "on_vbt_changed_command.h"

#include "user/user.h"
#include "vbt/value_based_triggers.h"

using std::string;

supla_on_vbt_changed_command::supla_on_vbt_changed_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_on_vbt_changed_command(socket_adapter) {}

void supla_on_vbt_changed_command::supla_on_vbt_changed_command::on_vbt_changed(
    int user_id) {
  supla_user *user = supla_user::find(user_id, false);
  if (user) {
    user->get_value_based_triggers()->load();
  }
}
