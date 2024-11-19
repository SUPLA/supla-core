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

using std::map;
using std::string;

supla_channel_value::supla_channel_value(void) : supla_vbt_value() {
  memset(this->raw_value, 0, SUPLA_CHANNELVALUE_SIZE);
}

supla_channel_value::supla_channel_value(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_vbt_value() {
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

bool supla_channel_value::is_differ(supla_channel_value *value,
                                    bool *significant_change) {
  bool result = !value || memcmp(this->raw_value, value->raw_value,
                                 SUPLA_CHANNELVALUE_SIZE) != 0;
  if (significant_change) {
    *significant_change = result;
  }
  return result;
}

void supla_channel_value::apply_channel_properties(
    int type, unsigned char protocol_version, int param1, int param2,
    int param3, int param4, supla_json_config *json_config) {}

map<string, string> supla_channel_value::get_replacement_map(void) {
  map<string, string> result;
  return result;
}

supla_channel_extended_value *supla_channel_value::convert2extended(
    supla_json_config *json_config, int func,
    supla_channel_extended_value **data_logger_purpose) {
  return nullptr;
}
