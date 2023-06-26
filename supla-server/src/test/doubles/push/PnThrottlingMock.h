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

#ifndef PN_THROTTLING_MOCK_H_
#define PN_THROTTLING_MOCK_H_

#include <gmock/gmock.h>

#include "push/pn_throttling.h"

namespace testing {

class PnThrottlingMock : public supla_pn_throttling {
 public:
  PnThrottlingMock();
  virtual ~PnThrottlingMock();

  MOCK_METHOD3(is_delivery_possible,
               bool(int user_id, bool *first_time_exceeded,
                    unsigned int *limit));
};

} /* namespace testing */

#endif /* PN_THROTTLING_MOCK_H_ */
