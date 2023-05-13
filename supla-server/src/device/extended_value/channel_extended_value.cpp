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

#include "channel_extended_value.h"

#include <stdlib.h>
#include <string.h>

supla_channel_extended_value::supla_channel_extended_value(void) {
  this->real_size = 0;
  this->value = nullptr;
}

supla_channel_extended_value::supla_channel_extended_value(
    TSuplaChannelExtendedValue *value) {
  this->real_size = 0;
  this->value = nullptr;

  if (value) {
    get_value_ptr(value->size);
    if (this->value) {
      memcpy(this->value, value, real_size);
    }
  }
}

supla_channel_extended_value::supla_channel_extended_value(
    supla_channel_extended_value *value) {
  this->real_size = 0;
  this->value = nullptr;

  if (value->value && value->real_size) {
    this->value = (TSuplaChannelExtendedValue *)malloc(value->real_size);
    this->real_size = value->real_size;
    memcpy(this->value, value->value, value->real_size);
  }
}

supla_channel_extended_value::~supla_channel_extended_value(void) {
  if (value) {
    free(value);
  }
}

TSuplaChannelExtendedValue *supla_channel_extended_value::get_value_ptr(
    size_t required_data_size) {
  if (required_data_size > SUPLA_CHANNELEXTENDEDVALUE_SIZE) {
    return nullptr;
  }

  size_t size = sizeof(TSuplaChannelExtendedValue) -
                SUPLA_CHANNELEXTENDEDVALUE_SIZE + required_data_size;

  if (size != real_size) {
    free(value);
    value = nullptr;
  }

  if (value == nullptr) {
    value = (TSuplaChannelExtendedValue *)calloc(1, size);
    real_size = size;
  }

  return value;
}

const TSuplaChannelExtendedValue *supla_channel_extended_value::get_value_ptr(
    void) {
  return value;
}

void supla_channel_extended_value::get_raw_value(
    TSuplaChannelExtendedValue *value) {
  if (value) {
    memset(value, 0, sizeof(TSuplaChannelExtendedValue));
    if (this->value) {
      memcpy(value, &this->value, real_size);
    }
  }
}

size_t supla_channel_extended_value::get_real_size(void) { return real_size; }

bool supla_channel_extended_value::is_differ(
    supla_channel_extended_value *value) {
  if (!value) {
    return true;
  }

  if (!value->value && !this->value) {
    return false;
  }

  return (!value->value && this->value) || (value->value && !this->value) ||
         (value->value->size != this->value->size) ||
         value->value->size > SUPLA_CHANNELEXTENDEDVALUE_SIZE ||
         (this->value->size != 0 &&
          memcmp(value->value->value, this->value->value, this->value->size) !=
              0);
}
