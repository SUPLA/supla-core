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

#ifndef SET_RGBW_COMMMAND_MOCK_H_
#define SET_RGBW_COMMMAND_MOCK_H_

#include <gmock/gmock.h>

#include "ipc/abstract_set_rgbw_command.h"

namespace testing {

class SetRGBWCommandMock : public supla_abstract_set_rgbw_command {
 public:
  explicit SetRGBWCommandMock(supla_abstract_ipc_socket_adapter *socket_adapter,
                              bool random_color);

  MOCK_METHOD9(set_channel_rgbw_value,
               bool(supla_user *user, int device_id, int channel_id, int color,
                    char color_brightness, char brightness, char on_off,
                    const char *alexa_correlation_token,
                    const char *google_request_id));
};

} /* namespace testing */

#endif /* SET_RGBW_COMMMAND_MOCK_H_ */
