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

#include "action_shading_system_parameters.h"

supla_action_shading_system_parameters::supla_action_shading_system_parameters(
    void)
    : supla_abstract_action_parameters() {
  params = {};
  params.Percentage = -1;
  params.Tilt = -1;
}

supla_action_shading_system_parameters::supla_action_shading_system_parameters(
    char percentage, char tilt, bool delta)
    : supla_abstract_action_parameters() {
  params = {};
  set_percentage(percentage);
  set_tilt(tilt);
  params.Delta = delta ? 1 : 0;
}

supla_action_shading_system_parameters::supla_action_shading_system_parameters(
    const TAction_ShadingSystem_Parameters &params)
    : supla_abstract_action_parameters() {
  this->params = params;
}

supla_action_shading_system_parameters::supla_action_shading_system_parameters(
    const TAction_ShadingSystem_Parameters *params)
    : supla_abstract_action_parameters() {
  this->params = {};
  if (params) {
    this->params = *params;
  }
}

supla_action_shading_system_parameters::~supla_action_shading_system_parameters(
    void) {}

bool supla_action_shading_system_parameters::equal(
    supla_abstract_action_parameters *params) const {
  supla_action_shading_system_parameters *p =
      dynamic_cast<supla_action_shading_system_parameters *>(params);
  return p && p->get_percentage() == get_percentage() &&
         p->get_tilt() == get_tilt() && p->is_delta() == is_delta();
}

supla_abstract_action_parameters *supla_action_shading_system_parameters::copy(
    void) const {                                             // NOLINT
  return new supla_action_shading_system_parameters(params);  // NOLINT
}

char supla_action_shading_system_parameters::get_percentage(void) const {
  return params.Percentage;
}

char supla_action_shading_system_parameters::get_tilt(void) const {
  return params.Tilt;
}

void supla_action_shading_system_parameters::set_percentage(char percentage) {
  params.Percentage = clamp(percentage);
}

void supla_action_shading_system_parameters::set_tilt(char tilt) {
  params.Tilt = clamp(tilt);
}

bool supla_action_shading_system_parameters::is_delta(void) const {
  return params.Delta > 0;
}

TAction_ShadingSystem_Parameters
supla_action_shading_system_parameters::get_params(void) {
  return params;
}

char supla_action_shading_system_parameters::clamp(char percentage) {
  if (percentage < -1) {
    percentage = -1;
  } else if (percentage > 100) {
    percentage = 100;
  }

  return percentage;
}

char supla_action_shading_system_parameters::percentage_to_position(
    char percentage) const {
  if (percentage == -1) {
    return -1;
  } else if (percentage > 100) {
    return 110;
  } else if (percentage < 0) {
    return 10;
  }

  return percentage + 10;
}

bool supla_action_shading_system_parameters::apply_on_value(
    int action, char value[SUPLA_CHANNELVALUE_SIZE], int func,
    unsigned _supla_int64_t flags) const {
  bool result = false;

  switch (func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND:
      break;
    default:
      return result;
  }

  char position = -1;

  switch (action) {
    case ACTION_STOP:
      position = 0;
      break;
    case ACTION_DOWN:
      position = 1;
      break;
    case ACTION_UP:
      position = 2;
      break;
    case ACTION_DOWN_OR_STOP:
      if (flags & SUPLA_CHANNEL_FLAG_RS_SBS_AND_STOP_ACTIONS) {
        position = 3;
      }
      break;
    case ACTION_UP_OR_STOP:
      if (flags & SUPLA_CHANNEL_FLAG_RS_SBS_AND_STOP_ACTIONS) {
        position = 4;
      }
      break;
    case ACTION_STEP_BY_STEP:
      if (flags & SUPLA_CHANNEL_FLAG_RS_SBS_AND_STOP_ACTIONS) {
        position = 5;
      }
      break;
    case ACTION_SHUT: {
      char percentage = get_percentage();

      if (is_delta()) {
        char current = 0;

        if (func == SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND) {
          current = ((TCSD_FacadeBlindValue *)value)->position;
        } else {
          current = ((TCSD_RollerShutterValue *)value)->position;
        }

        if (current > 100) {
          current = 100;
        } else if (current < 0) {
          current = 0;
        }

        if (percentage + current > 100) {
          percentage = 100;
        } else if (percentage + current < 0) {
          percentage = 0;
        }
      }

      position = percentage_to_position(percentage);
    }

    break;
    case ACTION_REVEAL:
      position = 10;
      break;
  }

  if (position >= -1 && position <= 110) {
    if (func == SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND) {
      ((TCSD_FacadeBlindValue *)value)->position = position;
      ((TCSD_FacadeBlindValue *)value)->tilt =
          percentage_to_position(get_tilt());
    } else {
      ((TCSD_RollerShutterValue *)value)->position = position;
    }

    result = true;
  }

  return result;
}