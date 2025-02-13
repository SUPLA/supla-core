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

#include "action_turn_onoff.h"

s_worker_action_turn_onoff::s_worker_action_turn_onoff(
    s_abstract_worker *worker, bool setOn)
    : s_worker_action(worker) {
  this->setOn = setOn;
}

s_worker_action_turn_on::s_worker_action_turn_on(s_abstract_worker *worker)
    : s_worker_action_turn_onoff(worker, true) {}

s_worker_action_turn_off::s_worker_action_turn_off(s_abstract_worker *worker)
    : s_worker_action_turn_onoff(worker, false) {}

bool s_worker_action_turn_onoff::is_action_allowed(void) {
  switch (worker->get_channel_func()) {
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
      return true;
  }

  return false;
}

int s_worker_action_turn_onoff::try_limit(void) { return 2; }

int s_worker_action_turn_onoff::waiting_time_to_retry(void) { return 30; }

int s_worker_action_turn_onoff::waiting_time_to_check(void) { return 5; }

bool s_worker_action_turn_onoff::result_success(int *fail_result_code) {
  bool result = false;

  switch (worker->get_channel_func()) {
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING: {
      int color = 0;
      char color_brightness = 0;
      char brightness = 0;

      if (worker->ipcc_get_rgbw_value(&color, &color_brightness, &brightness)) {
        switch (worker->get_channel_func()) {
          case SUPLA_CHANNELFNC_DIMMER:
            result = (brightness > 0) == setOn;
            break;
          case SUPLA_CHANNELFNC_RGBLIGHTING:
            result = (color_brightness > 0) == setOn;
            break;
          case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
            result = (brightness > 0 && color_brightness > 0) == setOn;
            break;
        }
      }
    } break;
    default: {
      char value = 0;
      if (worker->ipcc_get_char_value(&value)) {
        result = (value == 1) == setOn;
      }
      break;
    }
  }

  return result;
}

bool s_worker_action_turn_onoff::do_action() {
  return worker->ipcc_set_char_value(setOn ? 1 : 0);
}

REGISTER_ACTION(s_worker_action_turn_on, ACTION_TURN_ON);
REGISTER_ACTION(s_worker_action_turn_off, ACTION_TURN_OFF);
