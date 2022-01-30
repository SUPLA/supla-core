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

#include "ValueGetterStub.h"

#include <string.h>

namespace testing {
ValueGetterStub::ValueGetterStub(void) : supla_abstract_value_getter() {
  channelFunc = 0;
  memset(value, 0, SUPLA_CHANNELVALUE_SIZE);
  result = false;
}

ValueGetterStub::~ValueGetterStub(void) {}

bool ValueGetterStub::_get_value(int user_id, int device_id, int channel_id,
                                 char value[SUPLA_CHANNELVALUE_SIZE],
                                 int *channelFunc) {
  memcpy(value, this->value, SUPLA_CHANNELVALUE_SIZE);
  if (channelFunc) {
    *channelFunc = this->channelFunc;
  }

  return result;
}

void ValueGetterStub::setValue(char value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);
}

void ValueGetterStub::setFunction(int channelFunc) {
  this->channelFunc = channelFunc;
}

void ValueGetterStub::setResult(bool result) { this->result = result; }

}  // namespace testing
