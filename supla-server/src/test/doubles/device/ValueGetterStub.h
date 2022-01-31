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

#ifndef VALUE_GETTER_STUB_H_
#define VALUE_GETTER_STUB_H_

#include <abstract_value_getter.h>

namespace testing {

class ValueGetterStub : public supla_abstract_value_getter {
 protected:
  supla_channel_value* result;
  supla_channel_value* _get_value(int user_id, int device_id, int channel_id);

 public:
  ValueGetterStub(void);
  virtual ~ValueGetterStub(void);
  void setResult(supla_channel_value* result);
};

} /* namespace testing */

#endif /* VALUE_GETTER_STUB_H_ */
