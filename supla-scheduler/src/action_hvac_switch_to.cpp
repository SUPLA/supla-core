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

#include "action_hvac_switch_to.h"

s_worker_action_switch_to::s_worker_action_switch_to(s_abstract_worker *worker)
    : s_worker_action(worker) {}

s_worker_action_switch_to::~s_worker_action_switch_to(void) {}

bool s_worker_action_switch_to::is_action_allowed(void) {
  switch (worker->get_channel_func()) {
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
      return true;
  }

  return false;
}

int s_worker_action_switch_to::try_limit(void) { return 2; }

int s_worker_action_switch_to::waiting_time_to_retry(void) { return 30; }

int s_worker_action_switch_to::waiting_time_to_check(void) { return 5; }

bool s_worker_action_switch_to::result_success(int *fail_result_code) {
  THVACValue value = {};
  return worker->ipcc_get_hvac_value(&value, nullptr, nullptr) &&
         ((to_program_mode() &&
           (value.Flags & SUPLA_HVAC_VALUE_FLAG_WEEKLY_SCHEDULE)) ||
          (!to_program_mode() &&
           !(value.Flags & SUPLA_HVAC_VALUE_FLAG_WEEKLY_SCHEDULE)));
}

bool s_worker_action_switch_to::do_action() {
  return to_program_mode() ? worker->ipcc_action_hvac_switch_to_program_mode()
                           : worker->ipcc_action_hvac_switch_to_manual_mode();
}
