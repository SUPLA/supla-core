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

#ifndef ACTION_HVAC_SETPOINT_TEMPERATURE_H_
#define ACTION_HVAC_SETPOINT_TEMPERATURE_H_

#include "actions/abstract_action_parameters.h"
#include "device/value/channel_hvac_value.h"
#include "proto.h"

class supla_action_hvac_setpoint_temperature
    : public supla_abstract_action_parameters {
 private:
  short temperature;

 public:
  supla_action_hvac_setpoint_temperature(void);
  explicit supla_action_hvac_setpoint_temperature(const short &temperature);
  virtual ~supla_action_hvac_setpoint_temperature(void);
  virtual bool equal(supla_abstract_action_parameters *params) const;
  virtual supla_abstract_action_parameters *copy(void) const;  // NOLINT

  short get_temperature(void) const;
};

#endif /* ACTION_HVAC_SETPOINT_TEMPERATURE_H_ */
