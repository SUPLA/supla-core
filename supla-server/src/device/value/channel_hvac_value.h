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

#ifndef CHANNEL_HVAC_VALUE_H_
#define CHANNEL_HVAC_VALUE_H_

#include <map>
#include <string>

#include "device/value/channel_value.h"

class supla_channel_hvac_value : public supla_channel_value {
 public:
  supla_channel_hvac_value(void);
  explicit supla_channel_hvac_value(char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  static bool is_function_supported(int func);

  void clear(void);
  unsigned char get_mode(void);
  std::string mode_to_string(void);
  short get_setpoint_temperature_heat(void);
  double get_setpoint_temperature_heat_dbl(void);
  std::string get_setpoint_temperature_heat_str(void);
  short get_setpoint_temperature_cool(void);
  double get_setpoint_temperature_cool_dbl(void);
  std::string get_setpoint_temperature_cool_str(void);
  short get_setpoint_temperature(void);
  double get_setpoint_temperature_dbl(void);
  std::string get_setpoint_temperature_str(void);
  std::string get_home_assistant_mode(void);
  std::string get_home_assistant_action(void);
  std::string get_google_home_mode(void);
  std::string get_alexa_mode(void);
  unsigned short get_flags(void);
  bool is_any_error_set(void);
  bool is_battery_cover_open(void);
  bool clock_error(void);
  bool thermometer_error(void);

  void set_mode(unsigned char mode);
  void set_setpoint_temperature_heat(short temperature);
  void set_setpoint_temperature_cool(short temperature);
  void set_setpoint_temperature_heat_to_null(void);
  void set_setpoint_temperature_cool_to_null(void);
  void set_flags(unsigned short flags);
  void set_on(unsigned char on);
  unsigned char is_on(void);
  bool is_heating(void);
  bool is_cooling(void);
  void turn_on(void);
  void turn_off(void);
  void toggle(void);
  void switch_to_manual(void);
  void switch_to_program(void);

  virtual std::map<std::string, std::string> get_replacement_map(void);
};

#endif /*CHANNEL_HVAC_VALUE_H_*/
