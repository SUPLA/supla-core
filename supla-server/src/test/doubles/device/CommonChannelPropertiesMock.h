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

#ifndef COMMON_CHANNEL_PROPERTIES_MOCK_H_
#define COMMON_CHANNEL_PROPERTIES_MOCK_H_

#include <gmock/gmock.h>

#include "device/abstract_common_channel_properties.h"

namespace testing {

class CommonChannelPropertiesMock
    : public supla_abstract_common_channel_properties {
 protected:
 public:
  CommonChannelPropertiesMock(void);
  virtual ~CommonChannelPropertiesMock(void);

  MOCK_METHOD0(get_id, int());
  MOCK_METHOD0(get_device_id, int());
  MOCK_METHOD0(get_func, int());
  MOCK_METHOD0(get_param1, int());
  MOCK_METHOD0(get_param2, int());
  MOCK_METHOD0(get_param3, int());
  MOCK_METHOD0(get_param4, int());
  MOCK_METHOD0(get_json_config, channel_json_config *());
  MOCK_METHOD1(get_channel_id, int(unsigned char channel_number));
  MOCK_METHOD0(get_channel_number, unsigned char());
  MOCK_METHOD1(for_each,
               void(std::function<
                    void(supla_abstract_common_channel_properties *, bool *)>
                        on_channel_properties));
};

} /* namespace testing */

#endif /* COMMON_CHANNEL_PROPERTIES_MOCK_H_ */
