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

#ifndef CHANNEL_FB_VALUE_H_
#define CHANNEL_FB_VALUE_H_

#include "device/value/channel_value.h"
#include "user/user.h"

class supla_channel_fb_value : public supla_channel_value {
 public:
  explicit supla_channel_fb_value(
      const char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  explicit supla_channel_fb_value(const TDSC_FacadeBlindValue *value);
  const TDSC_FacadeBlindValue *get_fb_value(void);
  void set_fb_value(TDSC_FacadeBlindValue *value);
  virtual void apply_channel_properties(int type,
                                        unsigned char protocol_version,
                                        int param1, int param2, int param3,
                                        int param4,
                                        supla_json_config *json_config);
  static bool is_function_supported(int func);
  char get_position(void);
  char get_tilt(void);
  double get_tilt_angle(supla_json_config *config);
  virtual bool get_vbt_value(_vbt_var_name_e var_name, double *value);
};

#endif /*CHANNEL_FB_VALUE_H_*/
