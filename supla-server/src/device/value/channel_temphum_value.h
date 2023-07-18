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

#ifndef CHANNEL_TEMPHUM_VALUE_H_
#define CHANNEL_TEMPHUM_VALUE_H_

#include <map>
#include <string>

#include "device/value/channel_value.h"

class supla_channel_temphum_value : public supla_channel_value {
 private:
  bool with_humidity;

 public:
  supla_channel_temphum_value(void);
  supla_channel_temphum_value(int channel_type, int func,
                              char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  supla_channel_temphum_value(bool with_humidity,
                              char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  supla_channel_temphum_value(bool with_humidity, double temperature,
                              double humidity);

  bool is_humidity_available(void);
  double get_temperature(void);
  double get_humidity(void);
  void set_temperature(double temperature);
  void set_humidity(double humidity);
  virtual bool is_differ(supla_channel_value *value, bool *significant_change);
  virtual void apply_channel_properties(int type,
                                        unsigned char protocol_version,
                                        int param1, int param2, int param3,
                                        int param4,
                                        channel_json_config *json_config);
  virtual std::map<std::string, std::string> get_replacement_map(void);
  static int incorrect_temperature(void);
  static int incorrect_humidity(void);
  static bool is_function_supported(int func);
};

#endif /* CHANNEL_TEMPHUM_VALUE_H_ */
