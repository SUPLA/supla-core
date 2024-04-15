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

#include <string.h>

#include "log.h"

supla_action_shading_system_parameters::supla_action_shading_system_parameters(
    void)
    : supla_abstract_action_parameters() {
  percentage = -1;
  tilt = -1;
  flags = 0;
}

supla_action_shading_system_parameters::supla_action_shading_system_parameters(
    char percentage, char tilt, unsigned char flags)
    : supla_abstract_action_parameters() {
  this->percentage = percentage;
  this->tilt = tilt;
  this->flags = flags;
}

supla_action_shading_system_parameters::supla_action_shading_system_parameters(
    const TAction_ShadingSystem_Parameters &params)
    : supla_abstract_action_parameters() {
  percentage = params.Percentage;
  tilt = params.Tilt;
  flags = params.Flags;
}

supla_action_shading_system_parameters::supla_action_shading_system_parameters(
    const TAction_ShadingSystem_Parameters *params)
    : supla_abstract_action_parameters() {
  percentage = -1;
  tilt = -1;
  flags = 0;

  if (params) {
    percentage = params->Percentage;
    tilt = params->Tilt;
    flags = params->Flags;
  }
}

supla_action_shading_system_parameters::~supla_action_shading_system_parameters(
    void) {}

bool supla_action_shading_system_parameters::equal(
    supla_abstract_action_parameters *params) const {
  supla_action_shading_system_parameters *p =
      dynamic_cast<supla_action_shading_system_parameters *>(params);
  return p && p->get_percentage() == get_percentage() &&
         p->get_tilt() == get_tilt() && p->get_flags() == get_flags();
}

supla_abstract_action_parameters *supla_action_shading_system_parameters::copy(
    void) const {  // NOLINT
  supla_action_shading_system_parameters *result =
      new supla_action_shading_system_parameters();
  result->percentage = percentage;
  result->tilt = tilt;
  result->flags = flags;
  return result;
}

char supla_action_shading_system_parameters::get_percentage(void) const {
  return clamp(percentage, flags & SSP_FLAG_PERCENTAGE_AS_DELTA);
}

char supla_action_shading_system_parameters::get_tilt(void) const {
  return clamp(tilt, flags & SSP_FLAG_TILT_AS_DELTA);
}

void supla_action_shading_system_parameters::set_percentage(char percentage) {
  this->percentage = percentage;
}

void supla_action_shading_system_parameters::set_tilt(char tilt) {
  this->tilt = tilt;
}

unsigned char supla_action_shading_system_parameters::get_flags(void) const {
  return flags;
}

void supla_action_shading_system_parameters::set_flags(unsigned char flags) {
  this->flags = flags;
}

TAction_ShadingSystem_Parameters
supla_action_shading_system_parameters::get_params(void) {
  TAction_ShadingSystem_Parameters p = {};
  p.Percentage = get_percentage();
  p.Tilt = get_tilt();
  p.Flags = get_flags();
  return p;
}

char supla_action_shading_system_parameters::clamp(char percentage,
                                                   bool delta) const {
  if (percentage < -1 && !delta) {
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

char supla_action_shading_system_parameters::add_delta(char current,
                                                       char delta) const {
  if (current > 100) {
    current = 100;
  } else if (current < 0) {
    current = 0;
  }

  if (delta + current > 100) {
    return 100;
  } else if (delta + current < 0) {
    return 0;
  }

  return delta + current;
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
  char tilt_position = -1;

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
      char tilt = get_tilt();

      if (flags & SSP_FLAG_PERCENTAGE_AS_DELTA) {
        percentage =
            add_delta(((TDSC_RollerShutterValue *)value)->position, percentage);
      }

      if (func == SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND &&
          (flags & SSP_FLAG_TILT_AS_DELTA)) {
        tilt = add_delta(((TDSC_FacadeBlindValue *)value)->tilt, tilt);
      }

      position = percentage_to_position(percentage);
      tilt_position = percentage_to_position(tilt);
    }

    break;
    case ACTION_REVEAL:
      position = 10;
      break;
  }

  memset(value, 0, SUPLA_CHANNELVALUE_SIZE);

  if (position >= -1 && position <= 110) {
    if (func == SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND) {
      if (tilt_position >= -1 && tilt_position <= 110) {
        ((TCSD_FacadeBlindValue *)value)->position = position;
        ((TCSD_FacadeBlindValue *)value)->tilt = tilt_position;
        result = true;
      }
    } else {
      ((TCSD_RollerShutterValue *)value)->position = position;
      result = true;
    }
  }

  return result;
}

// static
supla_action_shading_system_parameters *
supla_action_shading_system_parameters::create_from_json(cJSON *root) {
  supla_action_shading_system_parameters *result = nullptr;

  cJSON *item = cJSON_GetObjectItem(root, "percentage");

  bool percentageAsDelta = false;
  bool tiltAsDelta = false;

  if (!item) {
    item = cJSON_GetObjectItem(root, "percentageDelta");
    percentageAsDelta = true;
  }

  cJSON *tilt_item = cJSON_GetObjectItem(root, "tilt");
  supla_log(LOG_DEBUG, "tilt: %x", tilt_item);
  if (!tilt_item) {
    tilt_item = cJSON_GetObjectItem(root, "tiltDelta");
    tiltAsDelta = true;
  }

  if (item && !cJSON_IsNumber(item)) {
    item = nullptr;
  }

  if (tilt_item && !cJSON_IsNumber(tilt_item)) {
    tilt_item = nullptr;
  }

  if (item || tilt_item) {
    result = new supla_action_shading_system_parameters();

    if (item) {
      result->set_percentage(item->valueint);
      if (percentageAsDelta) {
        result->set_flags(result->get_flags() | SSP_FLAG_PERCENTAGE_AS_DELTA);
      }
    }

    if (tilt_item) {
      result->set_tilt(tilt_item->valueint);
      supla_log(LOG_DEBUG, "tilt: %i", tilt_item->valueint);
      if (tiltAsDelta) {
        result->set_flags(result->get_flags() | SSP_FLAG_TILT_AS_DELTA);
      }
    }
  }

  return result;
}
