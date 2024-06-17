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

#include "jsonconfig/json_config.h"
#include "proto.h"

class hvac_config : public supla_json_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;
  static const std::map<unsigned int, std::string> temperatures_map;
  static const unsigned int readonly_temperatures;
  std::string aux_thermometer_type_to_string(unsigned char type);
  unsigned char string_to_aux_thermometer_type(const std::string &type);
  std::string alg_to_string(unsigned _supla_int16_t alg);
  unsigned _supla_int16_t string_to_alg(const std::string &alg);
  std::string subfunction_to_string(unsigned char subfunction);
  unsigned char string_to_subfunction(const std::string &subfunction);
  std::string temperature_key_to_string(unsigned int temperature_key);
  void add_algorithm_to_array(cJSON *root, cJSON *algs,
                              TChannelConfig_HVAC *config,
                              unsigned _supla_int16_t alg);
  void set_channel_number(cJSON *root, int field,
                          unsigned char cfg_channel_number,
                          unsigned char channel_number);
  bool get_channel_number(cJSON *root, int field, unsigned char channel_number,
                          unsigned char *result);
  void set_temperatures(TChannelConfig_HVAC *config, cJSON *root,
                        unsigned int filter);
  bool get_temperatures(TChannelConfig_HVAC *config, cJSON *root,
                        unsigned int filter);

 public:
  explicit hvac_config(supla_json_config *root);
  hvac_config(void);
  virtual void merge(supla_json_config *dst);
  void set_config(TChannelConfig_HVAC *config, unsigned char channel_number);
  bool get_config(TChannelConfig_HVAC *config, unsigned char channel_number);
};

#endif /* HVAC_CONFIG_H_ */
