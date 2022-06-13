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

#ifndef INITIATOR_NAME_GETTER_MOCK_H_
#define INITIATOR_NAME_GETTER_MOCK_H_

#include <gmock/gmock.h>

#include "abstract_initiator_name_getter.h"

namespace testing {

class InitiatorNameGetterrMock : public supla_abstract_initiator_name_getter {
 private:
 protected:
 public:
  InitiatorNameGetterrMock(void);
  virtual ~InitiatorNameGetterrMock(void);
  MOCK_METHOD1(get_name_with_caller, char*(const supla_caller& caller));
};

} /* namespace testing */

#endif /* #define INITIATOR_NAME_GETTER_MOCK_H_ */
