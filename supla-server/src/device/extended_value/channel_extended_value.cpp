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

using std::map;
using std::string;

supla_channel_extended_value::supla_channel_extended_value(void) {
  this->real_size = 0;
  this->value = nullptr;
}

supla_channel_extended_value::supla_channel_extended_value(
    const TSuplaChannelExtendedValue *value) {
  this->real_size = 0;
  this->value = nullptr;

  set_raw_value(value);
}

supla_channel_extended_value::supla_channel_extended_value(
    const supla_channel_extended_value *value) {
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

char supla_channel_extended_value::get_type() {
  return get_value_ptr() ? get_value_ptr()->type : 0;
}

supla_channel_extended_value *supla_channel_extended_value::copy(  // NOLINT
    void) {                                                        // NOLINT
  return new supla_channel_extended_value(get_value_ptr());
}

TSuplaChannelExtendedValue *supla_channel_extended_value::_realloc(
    size_t required_data_size) {
  if (required_data_size > SUPLA_CHANNELEXTENDEDVALUE_SIZE) {
    if (value) {
      free(value);
    }
    value = nullptr;
    real_size = 0;
    return nullptr;
  }

  size_t new_size = sizeof(TSuplaChannelExtendedValue) -
                    SUPLA_CHANNELEXTENDEDVALUE_SIZE + required_data_size;

  if (new_size != real_size) {
    value = (TSuplaChannelExtendedValue *)realloc(value, new_size);

    if (value) {
      value->size = required_data_size;

      if (new_size > real_size) {
        char *dst = (char *)value;
        memset(&dst[real_size], 0, new_size - real_size);
      }
    }

    real_size = new_size;
  }

  return value;
}

TSuplaChannelExtendedValue *supla_channel_extended_value::get_value_ptr(void) {
  return value;
}

bool supla_channel_extended_value::get_raw_value(
    TSuplaChannelExtendedValue *value) {
  if (value) {
    memset(value, 0, sizeof(TSuplaChannelExtendedValue));
    if (this->value) {
      memcpy(value, this->value, real_size);
      return true;
    }
  }

  return false;
}

void supla_channel_extended_value::set_raw_value(
    const TSuplaChannelExtendedValue *value) {
  if (value) {
    _realloc(value->size);
    if (this->value) {
      memcpy(this->value, value, real_size);
    }
  } else if (this->value) {
    free(this->value);
    this->value = nullptr;
    this->real_size = 0;
  }
}

size_t supla_channel_extended_value::get_real_size(void) { return real_size; }

size_t supla_channel_extended_value::get_value_size(void) {
  return value ? value->size : 0;
}

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

void supla_channel_extended_value::get_value(char *buffer) {
  if (get_value_size()) {
    memcpy(buffer, get_value_ptr()->value, get_value_size());
  }
}

map<string, string> supla_channel_extended_value::get_replacement_map(void) {
  map<string, string> result;
  return result;
}
