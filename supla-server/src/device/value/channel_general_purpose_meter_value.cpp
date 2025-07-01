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

#include "channel_general_purpose_meter_value.h"

supla_channel_general_purpose_meter_value::
    supla_channel_general_purpose_meter_value()
    : supla_channel_general_purpose_base_value() {
  original_value = 0;
}

supla_channel_general_purpose_meter_value::
    supla_channel_general_purpose_meter_value(
        const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_general_purpose_base_value(raw_value) {
  original_value = 0;
}

supla_abstract_channel_value *
supla_channel_general_purpose_meter_value::copy(  // NOLINT
    void) const {                                 // NOLINT
  supla_channel_general_purpose_meter_value *result =
      new supla_channel_general_purpose_meter_value(raw_value);
  result->original_value = original_value;
  return result;
}

// static
bool supla_channel_general_purpose_meter_value::is_function_supported(
    int func) {
  return func == SUPLA_CHANNELFNC_GENERAL_PURPOSE_METER;
}
