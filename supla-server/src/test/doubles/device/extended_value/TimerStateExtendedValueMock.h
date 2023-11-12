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

#ifndef TIMER_STATE_EXTENDED_VALUE_MOCK_H_
#define TIMER_STATE_EXTENDED_VALUE_MOCK_H_

#include <gmock/gmock.h>

#include <string>

#include "device/extended_value/timer_state_extended_value.h"

namespace testing {

class TimerStateExtendedValueMock : public supla_timer_state_extended_value {
 protected:
 public:
  TimerStateExtendedValueMock(const TSuplaChannelExtendedValue *value,
                              supla_user *user);
  virtual ~TimerStateExtendedValueMock(void);
  void set_raw_value_with_update(const TSuplaChannelExtendedValue *value,
                                 supla_user *user);

  MOCK_METHOD2(get_sender_name, std::string(supla_user *user, int client_id));
};

} /* namespace testing */

#endif /* TIMER_STATE_EXTENDED_VALUE_MOCK_H_ */
