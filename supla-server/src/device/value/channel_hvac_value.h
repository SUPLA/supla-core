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

#include "device/value/channel_value.h"

class supla_channel_hvac_value : public supla_channel_value {
 public:
  supla_channel_hvac_value(void);
  explicit supla_channel_hvac_value(char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  static bool is_function_supported(int func);

  unsigned char get_mode(void);
  short get_temperature_min(void);
  short get_temperature_max(void);
  unsigned short get_flags(void);

  void set_mode(unsigned char mode);
  void set_temperature_min(short temperature);
  void set_temperature_max(short temperature);
  void set_temperature_min_to_null(void);
  void set_temperature_max_to_null(void);
  void set_flags(unsigned short flags);
  bool is_on(void);
  void turn_on(void);
  void turn_off(void);
  void toggle(void);
};

#endif /*CHANNEL_HVAC_VALUE_H_*/
