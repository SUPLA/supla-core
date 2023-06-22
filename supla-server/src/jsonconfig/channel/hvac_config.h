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

#ifndef HVAC_CONFIG_H_
#define HVAC_CONFIG_H_

#include <map>
#include <string>

#include "jsonconfig/channel/channel_json_config.h"
#include "proto.h"

class hvac_config : public channel_json_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;
  cJSON *get_hvac_root(void);
  cJSON *get_hvac_root(bool force);
  std::string aux_thermometer_type_to_string(unsigned char type);
  unsigned char string_to_aux_thermometer_type(const std::string &type);
  std::string alg_to_string(unsigned _supla_int16_t alg);
  unsigned _supla_int16_t string_to_alg(const std::string &alg);
  void add_algorithm_to_array(cJSON *root, cJSON *algs,
                              TSD_ChannelConfig_HVAC *config,
                              unsigned _supla_int16_t alg);

 public:
  explicit hvac_config(supla_json_config *root);
  hvac_config(void);
  virtual void merge(supla_json_config *dst);
  void set_config(TSD_ChannelConfig_HVAC *config);
  bool get_config(TSD_ChannelConfig_HVAC *config);
};

#endif /* HVAC_CONFIG_H_ */
