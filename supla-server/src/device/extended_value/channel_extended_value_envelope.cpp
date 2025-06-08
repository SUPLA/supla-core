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

#include "channel_extended_value_envelope.h"

supla_abstract_channel_extended_value_envelope::
    supla_abstract_channel_extended_value_envelope(
        int channel_id, supla_abstract_channel_extended_value *extended_value) {
  this->channel_id = channel_id;
  this->extended_value = extended_value;
}

supla_abstract_channel_extended_value_envelope::
    ~supla_abstract_channel_extended_value_envelope(void) {
  if (extended_value) {
    delete extended_value;
  }
}

int supla_abstract_channel_extended_value_envelope::get_channel_id(void) {
  return channel_id;
}

supla_abstract_channel_extended_value *
supla_abstract_channel_extended_value_envelope::get_extended_value(void) {
  return extended_value;
}
