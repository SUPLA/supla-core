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

#ifndef ACTIVE_PERIOD_MOCK_H_
#define ACTIVE_PERIOD_MOCK_H_

#include <gmock/gmock.h>

#include "active_period.h"
#include "doubles/ActivePeriodMock.h"

namespace testing {

class ActivePeriodMock : public supla_active_period {
 public:
  ActivePeriodMock(void);
  virtual ~ActivePeriodMock(void);
  MOCK_METHOD0(get_current_point_in_time,
               std::chrono::time_point<std::chrono::system_clock>(void));
};

} /* namespace testing */

#endif /* ACTIVE_PERIOD_MOCK_H_ */
