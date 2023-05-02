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

#include "channel_openclosed_value.h"

#include <string.h>

supla_channel_openclosed_value::supla_channel_openclosed_value(void)
    : supla_channel_value() {}

supla_channel_openclosed_value::supla_channel_openclosed_value(
    char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {}

supla_channel_openclosed_value::supla_channel_openclosed_value(bool closed)
    : supla_channel_value() {
  set_closed(closed);
}

void supla_channel_openclosed_value::set_closed(bool closed) {
  raw_value[0] = closed ? 1 : 0;
}

bool supla_channel_openclosed_value::is_closed(void) {
  return raw_value[0] > 0;
}

bool supla_channel_openclosed_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      return true;
  }

  return false;
}
