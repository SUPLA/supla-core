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

#ifndef ACTION_HVAC_SETPOINT_TEMPERATURES_H_
#define ACTION_HVAC_SETPOINT_TEMPERATURES_H_

#include "actions/abstract_action_parameters.h"
#include "device/value/channel_hvac_value.h"
#include "proto.h"

class supla_action_hvac_setpoint_temperatures
    : public supla_abstract_action_parameters {
 private:
  short heating;
  bool heating_is_set;
  short cooling;
  bool cooling_is_set;

 public:
  supla_action_hvac_setpoint_temperatures(void);
  supla_action_hvac_setpoint_temperatures(short *heating, short *cooling);
  virtual ~supla_action_hvac_setpoint_temperatures(void);
  virtual bool equal(supla_abstract_action_parameters *params) const;
  virtual supla_abstract_action_parameters *copy(void) const;  // NOLINT

  bool get_heating_temperature(short *temperature) const;
  bool get_cooling_temperature(short *temperature) const;
  void set_heating_temperature(short *temperature);
  void set_cooling_temperature(short *temperature);
  void set_heating_temperature(short temperature);
  void set_cooling_temperature(short temperature);

  bool is_any_temperature_set(void) const;
};

#endif /* ACTION_HVAC_SETPOINT_TEMPERATURES_H_ */
