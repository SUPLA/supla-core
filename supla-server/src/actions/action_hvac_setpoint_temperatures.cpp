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

#include "action_hvac_setpoint_temperatures.h"

supla_action_hvac_setpoint_temperatures::
    supla_action_hvac_setpoint_temperatures(void)
    : supla_abstract_action_parameters() {
  heating = 0;
  heating_is_set = false;
  cooling = 0;
  cooling_is_set = false;
}

supla_action_hvac_setpoint_temperatures::
    supla_action_hvac_setpoint_temperatures(short *heating, short *cooling)
    : supla_abstract_action_parameters() {
  if (heating) {
    this->heating = *heating;
    this->heating_is_set = true;
  } else {
    this->heating = 0;
    this->heating_is_set = false;
  }

  if (cooling) {
    this->cooling = *cooling;
    this->cooling_is_set = true;
  } else {
    this->cooling = 0;
    this->cooling_is_set = false;
  }
}

supla_action_hvac_setpoint_temperatures::
    ~supla_action_hvac_setpoint_temperatures(void) {}

bool supla_action_hvac_setpoint_temperatures::equal(
    supla_abstract_action_parameters *params) const {
  supla_action_hvac_setpoint_temperatures *t =
      dynamic_cast<supla_action_hvac_setpoint_temperatures *>(params);

  return t && t->heating_is_set == heating_is_set && t->heating == heating &&
         t->cooling_is_set == cooling_is_set && t->cooling == cooling;
}

supla_abstract_action_parameters *
supla_action_hvac_setpoint_temperatures::copy(  // NOLINT
    void) const {                               // NOLINT
  short heating = this->heating;
  short cooling = this->cooling;

  return new supla_action_hvac_setpoint_temperatures(
      heating_is_set ? &heating : nullptr, cooling_is_set ? &cooling : nullptr);
}

bool supla_action_hvac_setpoint_temperatures::get_heating_temperature(
    short *temperature) const {
  if (temperature && heating_is_set) {
    *temperature = heating;
    return true;
  }

  return false;
}

bool supla_action_hvac_setpoint_temperatures::get_cooling_temperature(
    short *temperature) const {
  if (temperature && cooling_is_set) {
    *temperature = cooling;
    return true;
  }

  return false;
}

void supla_action_hvac_setpoint_temperatures::set_heating_temperature(
    short *temperature) {
  if (temperature) {
    heating = *temperature;
    heating_is_set = true;
  } else {
    heating_is_set = false;
  }
}

void supla_action_hvac_setpoint_temperatures::set_cooling_temperature(
    short *temperature) {
  if (temperature) {
    cooling = *temperature;
    cooling_is_set = true;
  } else {
    cooling_is_set = false;
  }
}

void supla_action_hvac_setpoint_temperatures::set_heating_temperature(
    short temperature) {
  heating = temperature;
  heating_is_set = true;
}

void supla_action_hvac_setpoint_temperatures::set_cooling_temperature(
    short temperature) {
  cooling = temperature;
  cooling_is_set = true;
}

bool supla_action_hvac_setpoint_temperatures::is_any_temperature_set(
    void) const {
  return heating_is_set || cooling_is_set;
}
