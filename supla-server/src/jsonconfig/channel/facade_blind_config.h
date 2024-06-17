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

#ifndef FACADE_BLIND_CONFIG_H_
#define FACADE_BLIND_CONFIG_H_

#include <map>
#include <string>

#include "jsonconfig/channel/roller_shutter_config.h"

class facade_blind_config : public roller_shutter_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;
  std::string type_to_string(unsigned char type);
  unsigned char string_to_type(const std::string &type);

 public:
  explicit facade_blind_config(supla_json_config *root);
  facade_blind_config(void);
  virtual void merge(supla_json_config *dst);
  void set_config(TChannelConfig_FacadeBlind *config);
  bool get_config(TChannelConfig_FacadeBlind *config);
};

#endif /* FACADE_BLIND_CONFIG_H_ */
