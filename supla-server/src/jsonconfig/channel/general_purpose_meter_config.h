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

#ifndef GENERAL_PURPOSE_METER_CONFIG_H_
#define GENERAL_PURPOSE_METER_CONFIG_H_

#include <map>
#include <string>

#include "jsonconfig/channel/general_purpose_base_config.h"
#include "proto.h"

class general_purpose_meter_config : public general_purpose_base_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;
  std::string chart_type_to_string(unsigned char chart_type);
  unsigned char string_to_chart_type(const std::string &chart_type);
  std::string counter_type_to_string(unsigned char counter_type);
  unsigned char string_to_counter_type(const std::string &counter_type);

 public:
  explicit general_purpose_meter_config(supla_json_config *root);
  general_purpose_meter_config(void);
  virtual void merge(supla_json_config *dst);
  void set_config(TChannelConfig_GeneralPurposeMeter *config);
  bool get_config(TChannelConfig_GeneralPurposeMeter *config);
};

#endif /* GENERAL_PURPOSE_METER_CONFIG_H_ */
