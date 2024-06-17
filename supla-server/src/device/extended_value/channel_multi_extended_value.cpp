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

#include "channel_multi_extended_value.h"

#include <stdlib.h>
#include <string.h>

supla_channel_multi_extended_value::supla_channel_multi_extended_value(
    const TSuplaChannelExtendedValue *value)
    : supla_channel_extended_value() {
  set_raw_value(value);
}

supla_channel_multi_extended_value::~supla_channel_multi_extended_value(void) {}

// static
bool supla_channel_multi_extended_value::is_ev_type_supported(char type) {
  return type == EV_TYPE_MULTI_VALUE;
}

void supla_channel_multi_extended_value::set_raw_value(
    const TSuplaChannelExtendedValue *value) {
  if (value && is_ev_type_supported(value->type)) {
    supla_channel_extended_value::set_raw_value(value);
  }
}
