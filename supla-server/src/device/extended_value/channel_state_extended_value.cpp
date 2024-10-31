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

#include "channel_state_extended_value.h"

#include <string.h>

supla_channel_state_extended_value::supla_channel_state_extended_value(
    const TSuplaChannelExtendedValue *value)
    : supla_channel_extended_value() {
  // Don't use supla_channel_extended_value(value) here because we want to
  // overwrite the set_raw_value method

  set_raw_value(value);
}

supla_channel_state_extended_value::supla_channel_state_extended_value(void)
    : supla_channel_extended_value() {}

supla_channel_state_extended_value::~supla_channel_state_extended_value(void) {}

void supla_channel_state_extended_value::set_raw_value(
    const TSuplaChannelExtendedValue *value) {
  if (value && is_ev_type_supported(value->type) &&
      value->size == sizeof(TChannelState_ExtendedValue)) {
    supla_channel_extended_value::set_raw_value(value);
  } else {
    supla_channel_extended_value::set_raw_value(nullptr);
  }
}

void supla_channel_state_extended_value::set_raw_value(
    const TChannelState_ExtendedValue *value) {
  if (value) {
    TSuplaChannelExtendedValue *ev =
        _realloc(sizeof(TChannelState_ExtendedValue));
    if (ev) {
      ev->type = EV_TYPE_CHANNEL_STATE_V1;
      ev->size = sizeof(TChannelState_ExtendedValue);
      memcpy(ev->value, value, ev->size);
    }
  } else {
    supla_channel_extended_value::set_raw_value(nullptr);
  }
}

bool supla_channel_state_extended_value::get_raw_value(
    TChannelState_ExtendedValue *value) {
  if (value) {
    if (get_value_ptr()) {
      memcpy(value, get_value_ptr()->value,
             sizeof(TChannelState_ExtendedValue));
      return true;
    }
    memset(value, 0, sizeof(TChannelState_ExtendedValue));
  }

  return false;
}

bool supla_channel_state_extended_value::get_raw_value(
    TSuplaChannelExtendedValue *value) {
  return supla_channel_extended_value::get_raw_value(value);
}

// static
bool supla_channel_state_extended_value::is_ev_type_supported(char type) {
  return type == EV_TYPE_CHANNEL_STATE_V1;
}

supla_channel_extended_value *
supla_channel_state_extended_value::copy(  // NOLINT
    void) {                                // NOLINT
  return new supla_channel_state_extended_value(get_value_ptr());
}

void supla_channel_state_extended_value::merge_old_if_needed(
    supla_channel_extended_value *old) {
  if (dynamic_cast<supla_channel_state_extended_value *>(old)) {
    TChannelState_ExtendedValue raw, old_raw = {};
    dynamic_cast<supla_channel_state_extended_value *>(old)->get_raw_value(
        &old_raw);

    get_raw_value(&raw);

    bool merged = false;

    if (!(raw.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL) &&
        (old_raw.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL)) {
      raw.Fields |= SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL;
      raw.BatteryLevel = old_raw.BatteryLevel;
      merged = true;
    }

    if (!(raw.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED) &&
        (old_raw.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED)) {
      raw.Fields |= SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED;
      raw.BatteryPowered = old_raw.BatteryPowered;
      merged = true;
    }

    if (merged) {
      set_raw_value(&raw);
    }
  }
}
