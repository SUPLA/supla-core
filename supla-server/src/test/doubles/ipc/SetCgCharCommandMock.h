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

#ifndef SET_CG_CHAR_COMMMAND_MOCK_H_
#define SET_CG_CHAR_COMMMAND_MOCK_H_

#include <gmock/gmock.h>

#include "ipc/abstract_set_cg_char_command.h"

namespace testing {

class SetCgCharCommandMock : public supla_abstract_set_cg_char_command {
 public:
  explicit SetCgCharCommandMock(
      supla_abstract_ipc_socket_adapter *socket_adapter);

  MOCK_METHOD3(set_cg_char_value,
               bool(supla_user *user, int group_id, char value));
};

} /* namespace testing */

#endif /* SET_CHAR_COMMMAND_MOCK_H_ */
