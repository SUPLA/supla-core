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

#ifndef SUPLA_ABSTRACT_USER_DAO_H_
#define SUPLA_ABSTRACT_USER_DAO_H_

#include <vector>

#include "user/virtualchannel.h"

class supla_user;
class supla_abstract_user_dao {
 public:
  supla_abstract_user_dao();
  virtual ~supla_abstract_user_dao();

  virtual std::vector<supla_virtual_channel> get_virtual_channels(
      supla_user *user, long long int since_the_value_update_timestamp) = 0;
  virtual std::vector<int> get_users_with_virtual_channels_online(void) = 0;
};

#endif /*SUPLA_ABSTRACT_USER_DAO_H_*/
