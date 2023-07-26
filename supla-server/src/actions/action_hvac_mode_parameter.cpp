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

#include "action_hvac_mode_parameter.h"

#include "tools.h"

supla_action_hvac_mode_parameter::supla_action_hvac_mode_parameter(
    unsigned char mode)
    : supla_abstract_action_parameters() {
  this->mode = mode;
}

supla_action_hvac_mode_parameter::supla_action_hvac_mode_parameter(void)
    : supla_abstract_action_parameters() {
  mode = SUPLA_HVAC_MODE_NOT_SET;
}

supla_action_hvac_mode_parameter::supla_action_hvac_mode_parameter(
    const TAction_HVACMode_Parameter &mode)
    : supla_abstract_action_parameters() {
  this->mode = mode.Mode;
}

supla_action_hvac_mode_parameter::supla_action_hvac_mode_parameter(
    const TAction_HVACMode_Parameter *mode) {
  if (mode) {
    this->mode = mode->Mode;
  } else {
    this->mode = SUPLA_HVAC_MODE_NOT_SET;
  }
}

supla_action_hvac_mode_parameter::~supla_action_hvac_mode_parameter(void) {}

bool supla_action_hvac_mode_parameter::equal(
    supla_abstract_action_parameters *params) const {
  supla_action_hvac_mode_parameter *p =
      dynamic_cast<supla_action_hvac_mode_parameter *>(params);

  return p && p->mode == mode;
}

supla_abstract_action_parameters *
supla_action_hvac_mode_parameter::copy(  // NOLINT
    void) {                              // NOLINT
  return new supla_action_hvac_mode_parameter(mode);
}

unsigned char supla_action_hvac_mode_parameter::get_mode(void) { return mode; }

void supla_action_hvac_mode_parameter::set_mode(unsigned char mode) {
  this->mode = mode;
}
