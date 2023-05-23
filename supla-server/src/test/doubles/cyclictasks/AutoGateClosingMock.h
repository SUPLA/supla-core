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

#ifndef AUTO_GATE_CLOSING_MOCK_H_
#define AUTO_GATE_CLOSING_MOCK_H_

#include <gmock/gmock.h>

#include "cyclictasks/abstract_auto_gate_closing.h"

namespace testing {

class AutoGateClosingMock : public supla_abstract_auto_gate_closing {
 public:
  AutoGateClosingMock(void);
  virtual ~AutoGateClosingMock(void);

  MOCK_METHOD1(get_dao, supla_abstract_auto_gate_closing_dao *(
                            supla_abstract_db_access_provider *dba));
  MOCK_METHOD1(release_dao, void(supla_abstract_auto_gate_closing_dao *dao));
  MOCK_METHOD0(get_property_getter,
               supla_abstract_channel_property_getter *(void));
  MOCK_METHOD1(release_property_getter,
               void(supla_abstract_channel_property_getter *property_getter));
  MOCK_METHOD3(close_the_gate,
               void(int user_id, int device_id, int channel_id));

  void run(const std::vector<supla_user *> *users,
           supla_abstract_db_access_provider *dba);
};

} /* namespace testing */

#endif /* AUTO_GATE_CLOSING_MOCK_H_ */
