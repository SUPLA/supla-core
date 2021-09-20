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

#ifndef GATE_STATE_GETTER_MOCK_H_
#define GATE_STATE_GETTER_MOCK_H_

#include "actions/abstract_gate_state_getter.h"

namespace testing {

class GateStateGetterMock : public supla_abstract_gate_state_getter {
 private:
  bool result;
  bool is_closed;

 protected:
 public:
  GateStateGetterMock();
  virtual ~GateStateGetterMock();
  virtual bool get_related_openingsensor_state(int user_id, int device_id,
                                               int channel_id, bool *is_closed);
  void set_result(bool result);
  void set_closed(bool closed);
};

} /* namespace testing */

#endif /* GATE_STATE_GETTER_MOCK_H_ */
