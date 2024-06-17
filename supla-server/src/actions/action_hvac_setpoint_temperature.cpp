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

#include "action_hvac_setpoint_temperature.h"

supla_action_hvac_setpoint_temperature::supla_action_hvac_setpoint_temperature(
    void)
    : supla_abstract_action_parameters() {
  temperature = 0;
}

supla_action_hvac_setpoint_temperature::supla_action_hvac_setpoint_temperature(
    const short &temperature)
    : supla_abstract_action_parameters() {
  this->temperature = temperature;
}

supla_action_hvac_setpoint_temperature::~supla_action_hvac_setpoint_temperature(
    void) {}

bool supla_action_hvac_setpoint_temperature::equal(
    supla_abstract_action_parameters *params) const {
  supla_action_hvac_setpoint_temperature *t =
      dynamic_cast<supla_action_hvac_setpoint_temperature *>(params);

  return t && t->temperature == this->temperature;
}

supla_abstract_action_parameters *
supla_action_hvac_setpoint_temperature::copy(  // NOLINT
    void) const {                              // NOLINT
  return new supla_action_hvac_setpoint_temperature(temperature);
}

short supla_action_hvac_setpoint_temperature::get_temperature(void) const {
  return temperature;
}
