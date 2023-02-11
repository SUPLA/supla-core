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

#ifndef CHANNEL_GATE_VALUE_H_
#define CHANNEL_GATE_VALUE_H_

#include "device/channel_value.h"
#include "user/user.h"

enum _gate_sensor_level_enum { gsl_unknown, gsl_closed, gsl_open };

class supla_channel_gate_value : public supla_channel_value {
 private:
  _gate_sensor_level_enum opening_sensor_level;
  _gate_sensor_level_enum partial_opening_sensor_level;
  _gate_sensor_level_enum get_sensor_state(supla_user *user, int channel_id);

 public:
  supla_channel_gate_value(void);
  explicit supla_channel_gate_value(char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  explicit supla_channel_gate_value(
      _gate_sensor_level_enum opening_sensor_level,
      _gate_sensor_level_enum partial_opening_sensor_level);

  _gate_sensor_level_enum get_opening_sensor_level(void);
  void set_opening_sensor_level(_gate_sensor_level_enum level);

  _gate_sensor_level_enum get_partial_opening_sensor_level(void);
  void set_partial_opening_sensor_level(_gate_sensor_level_enum level);

  void update_sensors(supla_user *user, int opening_sensor_channel_id,
                      int partial_opening_sensor_channel_id);
};

#endif /*CHANNEL_GATE_VALUE_H_*/
