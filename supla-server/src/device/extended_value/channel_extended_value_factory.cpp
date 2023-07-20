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

#include "channel_extended_value_factory.h"

#include "device/extended_value/channel_and_timer_state_extended_value.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "device/extended_value/channel_ic_extended_value.h"
#include "device/extended_value/channel_multi_extended_value.h"
#include "device/extended_value/channel_state_extended_value.h"
#include "device/extended_value/channel_thermostat_extended_value.h"
#include "device/extended_value/timer_state_extended_value.h"

supla_channel_extended_value_factory::supla_channel_extended_value_factory(
    void) {}

// static
supla_channel_extended_value *supla_channel_extended_value_factory::new_value(
    TSuplaChannelExtendedValue *ev, const char *text_param1, int param2,
    supla_user *user) {
  if (supla_channel_em_extended_value::is_ev_type_supported(ev->type)) {
    return new supla_channel_em_extended_value(ev, text_param1, param2);
  } else if (supla_timer_state_extended_value::is_ev_type_supported(ev->type)) {
    return new supla_timer_state_extended_value(ev, user);
  } else if (supla_channel_state_extended_value::is_ev_type_supported(
                 ev->type)) {
    return new supla_channel_state_extended_value(ev);
  } else if (supla_channel_and_timer_state_extended_value::is_ev_type_supported(
                 ev->type)) {
    return new supla_channel_and_timer_state_extended_value(ev, user);
  } else if (supla_channel_thermostat_extended_value::is_ev_type_supported(
                 ev->type)) {
    return new supla_channel_thermostat_extended_value(ev);
  } else if (supla_channel_multi_extended_value::is_ev_type_supported(
                 ev->type)) {
    return new supla_channel_multi_extended_value(ev);
  }

  return nullptr;
}

// static
supla_channel_extended_value *supla_channel_extended_value_factory::new_value(
    TSuplaChannelExtendedValue *ev) {
  if (supla_channel_em_extended_value::is_ev_type_supported(ev->type)) {
    return new supla_channel_em_extended_value(ev);
  } else if (supla_channel_ic_extended_value::is_ev_type_supported(ev->type)) {
    return new supla_channel_ic_extended_value(ev);
  } else if (supla_timer_state_extended_value::is_ev_type_supported(ev->type)) {
    // return new supla_timer_state_extended_value(ev);
  } else if (supla_channel_state_extended_value::is_ev_type_supported(
                 ev->type)) {
    return new supla_channel_state_extended_value(ev);
  } else if (supla_channel_and_timer_state_extended_value::is_ev_type_supported(
                 ev->type)) {
    return new supla_channel_and_timer_state_extended_value(ev);
  } else if (supla_channel_thermostat_extended_value::is_ev_type_supported(
                 ev->type)) {
    return new supla_channel_thermostat_extended_value(ev);
  } else if (supla_channel_multi_extended_value::is_ev_type_supported(
                 ev->type)) {
    return new supla_channel_multi_extended_value(ev);
  }

  return nullptr;
}
