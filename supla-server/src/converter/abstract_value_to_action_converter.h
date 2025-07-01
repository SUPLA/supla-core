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

#ifndef VALUE_TO_ACTION_CONVERTER_H_
#define VALUE_TO_ACTION_CONVERTER_H_

#include "actions/abstract_action_executor.h"
#include "device/abstract_channel_property_getter.h"

class abstract_value_to_action_converter {
 public:
  abstract_value_to_action_converter(void);

  virtual bool convert(supla_abstract_channel_value *value,
                       supla_abstract_action_executor *action_executor) = 0;
  virtual ~abstract_value_to_action_converter();
};

#endif /* VALUE_TO_ACTION_CONVERTER_H_ */
