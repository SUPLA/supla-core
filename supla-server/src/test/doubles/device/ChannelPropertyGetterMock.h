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

#ifndef VALUE_GETTER_MOCK_H_
#define VALUE_GETTER_MOCK_H_

#include <gmock/gmock.h>

#include "device/abstract_channel_property_getter.h"

namespace testing {

class ChannelPropertyGetterMock
    : public supla_abstract_channel_property_getter {
 protected:
 public:
  ChannelPropertyGetterMock(void);
  ChannelPropertyGetterMock(int user_id, int device_id, int channel_id);
  virtual ~ChannelPropertyGetterMock(void);

  MOCK_METHOD5(_get_value, supla_abstract_channel_value *(
                               int user_id, int device_id, int channel_id,
                               supla_channel_fragment *fragment,
                               supla_channel_availability_status *status));
  MOCK_METHOD3(_get_func, int(int user_id, int device_id, int channel_id));

  MOCK_METHOD3(_get_extended_value,
               supla_abstract_channel_extended_value *(int user_id,
                                                       int device_id,
                                                       int channel_id));
  MOCK_METHOD3(_get_detached_json_config,
               supla_json_config *(int user_id, int device_id, int channel_id));

  MOCK_METHOD3(_get_channel_id,
               int(int user_id, int device_id, unsigned char channel_number));

  MOCK_METHOD3(get_channel_availability_status,
               supla_channel_availability_status(int user_id, int device_id,
                                                 int channel_id));
};

} /* namespace testing */

#endif /* VALUE_GETTER_MOCK_H_ */
