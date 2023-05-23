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

#ifndef GET_PN_LIMIT_COMMMAND_MOCK_H_
#define GET_PN_LIMIT_COMMMAND_MOCK_H_

#include <gmock/gmock.h>

#include "ipc/abstract_get_pn_limit_command.h"

namespace testing {

class GetPnLimitCommandMock : public supla_abstract_get_pn_limit_command {
 public:
  explicit GetPnLimitCommandMock(
      supla_abstract_ipc_socket_adapter *socket_adapter);

  MOCK_METHOD2(get_pn_limit, unsigned int(int user_id, long *left));
};

} /* namespace testing */

#endif /* GET_PN_LIMIT_COMMMAND_MOCK_H_ */
