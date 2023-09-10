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

#include "channel_and_timer_state_extended_value.h"

#include <string.h>

supla_channel_and_timer_state_extended_value::
    supla_channel_and_timer_state_extended_value(
        const TSuplaChannelExtendedValue *value)
    : supla_timer_state_extended_value(nullptr, nullptr) {
  set_raw_value(value);
}

supla_channel_and_timer_state_extended_value::
    supla_channel_and_timer_state_extended_value(
        const TSuplaChannelExtendedValue *value, supla_user *user)
    : supla_timer_state_extended_value(nullptr, nullptr) {
  set_raw_value(value);
  update_sender_name(user);
  update_time();
}

supla_channel_and_timer_state_extended_value::
    ~supla_channel_and_timer_state_extended_value(void) {}

size_t supla_channel_and_timer_state_extended_value::get_real_size(void) {
  return supla_timer_state_extended_value::get_real_size() +
         channel.get_value_size();
}

size_t supla_channel_and_timer_state_extended_value::get_value_size(void) {
  return supla_timer_state_extended_value::get_value_size() +
         channel.get_value_size();
}

void supla_channel_and_timer_state_extended_value::set_raw_value(
    const TChannelAndTimerState_ExtendedValue *value) {
  if (value) {
    channel.set_raw_value(&value->Channel);
    supla_timer_state_extended_value::set_raw_value(&value->Timer);
  } else {
    channel.set_raw_value((TSuplaChannelExtendedValue *)nullptr);
    supla_timer_state_extended_value::set_raw_value(
        (TSuplaChannelExtendedValue *)nullptr);
  }
}

void supla_channel_and_timer_state_extended_value::set_raw_value(
    const TSuplaChannelExtendedValue *value) {
  if (value && is_ev_type_supported(value->type) &&
      value->size >= sizeof(TChannelState_ExtendedValue) &&
      supla_timer_state_extended_value::valid_size(
          value->size - sizeof(TChannelState_ExtendedValue))) {
    set_raw_value((TChannelAndTimerState_ExtendedValue *)value->value);
  }
}

bool supla_channel_and_timer_state_extended_value::get_raw_value(
    TChannelAndTimerState_ExtendedValue *value) {
  if (!value) {
    return false;
  }

  memset(value, 0, sizeof(TChannelAndTimerState_ExtendedValue));

  return channel.get_raw_value(&value->Channel) &&
         supla_timer_state_extended_value::get_raw_value(&value->Timer);
}

bool supla_channel_and_timer_state_extended_value::get_raw_value(
    TSuplaChannelExtendedValue *value) {
  if (!value) {
    return false;
  }

  if (get_raw_value((TChannelAndTimerState_ExtendedValue *)value->value)) {
    value->type = EV_TYPE_CHANNEL_AND_TIMER_STATE_V1;
    value->size = channel.get_value_size() +
                  supla_timer_state_extended_value::get_value_size();
    return true;
  }

  memset(value, 0, sizeof(TSuplaChannelExtendedValue));
  return false;
}

// static
bool supla_channel_and_timer_state_extended_value::is_ev_type_supported(
    char type) {
  return type == EV_TYPE_CHANNEL_AND_TIMER_STATE_V1;
}

supla_channel_extended_value *
supla_channel_and_timer_state_extended_value::copy(  // NOLINT
    void) {                                          // NOLINT

  TChannelAndTimerState_ExtendedValue val;
  get_raw_value(&val);

  supla_channel_and_timer_state_extended_value *result =
      new supla_channel_and_timer_state_extended_value(nullptr, nullptr);

  result->set_raw_value(&val);

  return result;
}
