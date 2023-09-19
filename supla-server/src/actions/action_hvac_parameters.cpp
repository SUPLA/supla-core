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

#include "action_hvac_parameters.h"

#include <string.h>

supla_action_hvac_parameters::supla_action_hvac_parameters(void)
    : supla_abstract_action_parameters() {
  params = {};
}

supla_action_hvac_parameters::supla_action_hvac_parameters(
    const TAction_HVAC_Parameters &params)
    : supla_abstract_action_parameters() {
  this->params = params;
}

supla_action_hvac_parameters::supla_action_hvac_parameters(
    const TAction_HVAC_Parameters *params) {
  if (params) {
    this->params = *params;
  } else {
    this->params = {};
  }
}

supla_action_hvac_parameters::~supla_action_hvac_parameters(void) {}

bool supla_action_hvac_parameters::equal(
    supla_abstract_action_parameters *params) const {
  supla_action_hvac_parameters *p =
      dynamic_cast<supla_action_hvac_parameters *>(params);

  return p && memcmp(&p->params, &this->params,
                     sizeof(TAction_HVAC_Parameters)) == 0;
}

supla_abstract_action_parameters *supla_action_hvac_parameters::copy(  // NOLINT
    void) {                                                            // NOLINT
  return new supla_action_hvac_parameters(params);
}

void supla_action_hvac_parameters::apply_on(supla_channel_hvac_value *value) {
  if (params.Mode != SUPLA_HVAC_MODE_NOT_SET) {
    value->set_mode(params.Mode);
  }

  if (params.Flags & SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET) {
    value->set_temperature_heat(params.SetpointTemperatureHeat);
  } else {
    value->set_temperature_heat_to_null();
  }

  if (params.Flags & SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET) {
    value->set_temperature_cool(params.SetpointTemperatureCool);
  } else {
    value->set_temperature_cool_to_null();
  }
}

unsigned int supla_action_hvac_parameters::get_duration_sec(void) {
  return params.DurationSec;
}

bool supla_action_hvac_parameters::is_any_param_set(void) {
  return params.Mode != SUPLA_HVAC_MODE_NOT_SET ||
         (params.Flags & SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET) ||
         (params.Flags & SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET);
}
