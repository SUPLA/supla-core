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

#include "channel_value_envelope.h"

supla_abstract_channel_value_envelope::supla_abstract_channel_value_envelope(
    int channel_id, supla_abstract_channel_value *value) {
  this->channel_id = channel_id;
  this->value = value;
}

supla_abstract_channel_value_envelope::~supla_abstract_channel_value_envelope(
    void) {
  if (value) {
    delete value;
  }
}

int supla_abstract_channel_value_envelope::get_channel_id(void) {
  return channel_id;
}

supla_abstract_channel_value *supla_abstract_channel_value_envelope::get_value(
    void) {
  return value;
}
