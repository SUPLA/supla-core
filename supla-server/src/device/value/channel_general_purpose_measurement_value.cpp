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

#include "channel_general_purpose_measurement_value.h"

supla_channel_general_purpose_measurement_value::
    supla_channel_general_purpose_measurement_value()
    : supla_channel_value() {}

supla_channel_general_purpose_measurement_value::
    supla_channel_general_purpose_measurement_value(
        const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {}

// static
bool supla_channel_general_purpose_measurement_value::is_function_supported(
    int func) {
  return func == SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT;
}
