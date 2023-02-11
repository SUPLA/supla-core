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

#include "channel_value.h"

#include <string.h>

supla_channel_value::supla_channel_value(void) {
  memset(this->raw_value, 0, SUPLA_CHANNELVALUE_SIZE);
}

supla_channel_value::supla_channel_value(
    char raw_value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(this->raw_value, raw_value, SUPLA_CHANNELVALUE_SIZE);
}

supla_channel_value::~supla_channel_value(void) {}

void supla_channel_value::get_raw_value(
    char raw_value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(raw_value, this->raw_value, SUPLA_CHANNELVALUE_SIZE);
}

void supla_channel_value::set_raw_value(
    char raw_value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(this->raw_value, raw_value, SUPLA_CHANNELVALUE_SIZE);
}
