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

#ifndef CHANNEL_RS_VALUE_H_
#define CHANNEL_RS_VALUE_H_

#include "device/value/channel_value.h"
#include "user/user.h"

enum _rs_sensor_level_enum { rsl_unknown, rsl_closed, rsl_open };

class supla_channel_rs_value : public supla_channel_value {
 private:
  _rs_sensor_level_enum opening_sensor_level;

 public:
  explicit supla_channel_rs_value(
      const char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  explicit supla_channel_rs_value(const TDSC_RollerShutterValue *value);
  const TDSC_RollerShutterValue *get_rs_value(void);
  void set_rs_value(TDSC_RollerShutterValue *value);
  _rs_sensor_level_enum get_opening_sensor_level(void);
  void set_opening_sensor_level(_rs_sensor_level_enum level);
  void update_sensor(supla_user *user, int opening_sensor_channel_id);
  virtual void apply_channel_properties(int type,
                                        unsigned char protocol_version,
                                        int param1, int param2, int param3,
                                        int param4,
                                        supla_json_config *json_config);
  static bool is_function_supported(int func);
};

#endif /*CHANNEL_RS_VALUE_H_*/
