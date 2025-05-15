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

#ifndef VALVE_CONFIG_H_
#define VALVE_CONFIG_H_

#include <map>
#include <string>

#include "jsonconfig/json_config.h"
#include "proto.h"

class valve_config : public supla_json_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;
  std::string close_on_flood_type_to_string(unsigned char type);
  unsigned char string_to_close_on_flood_type(const std::string &type);

 protected:
 public:
  explicit valve_config(supla_json_config *root);
  valve_config(void);
  virtual void merge(supla_json_config *dst);
  void set_config(TChannelConfig_Valve *config);
  bool get_config(TChannelConfig_Valve *config);
};

#endif /* VALVE_CONFIG_H_ */
