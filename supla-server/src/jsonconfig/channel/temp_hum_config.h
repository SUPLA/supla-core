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

#ifndef TEMP_HUM_CONFIG_H_
#define TEMP_HUM_CONFIG_H_

#include <map>
#include <string>

#include "jsonconfig/json_config.h"
#include "proto.h"

class temp_hum_config : public supla_json_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;

 public:
  explicit temp_hum_config(supla_json_config *root);
  temp_hum_config(void);
  virtual void merge(supla_json_config *dst);
  void set_config(TChannelConfig_TemperatureAndHumidity *config);
  bool get_config(TChannelConfig_TemperatureAndHumidity *config);

  _supla_int16_t get_temperature_adjustment(void);
  _supla_int16_t get_humidity_adjustment(void);

  double get_temperature_adjustment_dbl(void);
  double get_humidity_adjustment_dbl(void);

  bool is_adjustment_applied_by_device(void);
};

#endif /* TEMP_HUM_CONFIG_H_ */
