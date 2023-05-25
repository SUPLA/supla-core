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

#ifndef OPENINGSENSORCONFIG_H_
#define OPENINGSENSORCONFIG_H_

#include <string>

#include "jsonconfig/channel/channel_json_config.h"
#include "proto.h"

class opening_sensor_config : public channel_json_config {
 private:
  static const char related_channel_id_key[];
  static const char related_secondary_channel_id_key[];
  static const char inverted_logic_key[];

 public:
  explicit opening_sensor_config(channel_json_config *root);
  opening_sensor_config(void);
  int get_related_channel_id(void);
  int get_related_secondary_channel_id(void);
  bool is_inverted_logic(void);
};

#endif /* OPENINGSENSORCONFIG_H_ */
