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

#ifndef CHANNEL_ONOFF_VALUE_H_
#define CHANNEL_ONOFF_VALUE_H_

#include "device/value/channel_value.h"

class supla_channel_onoff_value : public supla_channel_value {
 public:
  supla_channel_onoff_value(void);
  explicit supla_channel_onoff_value(char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  explicit supla_channel_onoff_value(bool on);
  void set_on(bool on);
  void set_overcurrent_relay_off(bool active);
  bool is_on(void);
  bool is_overcurrent_relay_off(void);
  virtual void apply_channel_properties(int type,
                                        unsigned char protocol_version,
                                        int param1, int param2, int param3,
                                        int param4,
                                        channel_json_config *json_config);
  static bool is_function_supported(int func);
};

#endif /*CHANNEL_ONOFF_VALUE_H_*/
