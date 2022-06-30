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

#ifndef IS_CHANNEL_CONNECTED_COMMAND_MOCK_H_
#define IS_CHANNEL_CONNECTED_COMMAND_MOCK_H_

#include <gmock/gmock.h>

#include "ipc/abstract_is_channel_connected_command.h"

namespace testing {

class IsChannelConnectedCommandMock
    : public supla_abstract_is_channel_connected_command {
 public:
  explicit IsChannelConnectedCommandMock(
      supla_abstract_ipc_socket_adapter *socket_adapter);

  MOCK_METHOD3(is_channel_online,
               bool(int user_id, int device_id, int channel_id));
};

} /* namespace testing */

#endif /* IS_CHANNEL_CONNECTED_COMMAND_MOCK_H_ */
