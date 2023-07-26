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

#include "action_hvac_temperature_parameters.h"

supla_action_hvac_temperature_parameters::
    supla_action_hvac_temperature_parameters(void)
    : supla_abstract_action_parameters() {
  params = {};
}

supla_action_hvac_temperature_parameters::
    supla_action_hvac_temperature_parameters(
        const TAction_HVACTemperature_Parameters &params)
    : supla_abstract_action_parameters() {
  this->params = params;
}

supla_action_hvac_temperature_parameters::
    supla_action_hvac_temperature_parameters(
        const TAction_HVACTemperature_Parameters *params) {
  if (params) {
    this->params = *params;
  } else {
    this->params = {};
  }
}

supla_action_hvac_temperature_parameters::
    ~supla_action_hvac_temperature_parameters(void) {}

bool supla_action_hvac_temperature_parameters::equal(
    supla_abstract_action_parameters *params) const {
  supla_action_hvac_temperature_parameters *p =
      dynamic_cast<supla_action_hvac_temperature_parameters *>(params);

  return p && p->params.Flags == this->params.Flags &&
         p->params.SetpointTemperatureMax ==
             this->params.SetpointTemperatureMax &&
         p->params.SetpointTemperatureMin ==
             this->params.SetpointTemperatureMin;
}

supla_abstract_action_parameters *
supla_action_hvac_temperature_parameters::copy(  // NOLINT
    void) {                                      // NOLINT
  return new supla_action_hvac_temperature_parameters(params);
}
