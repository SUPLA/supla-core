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

#ifndef ROLLER_SHUTTER_CONFIG_H_
#define ROLLER_SHUTTER_CONFIG_H_

#include "jsonconfig/channel/shading_system_base_config.h"

class roller_shutter_config : public shading_system_base_config {
 public:
  explicit roller_shutter_config(supla_json_config *root);
  roller_shutter_config(void);
  virtual void merge(supla_json_config *dst);
  void set_config(TChannelConfig_Rollershutter *config);
  bool get_config(TChannelConfig_Rollershutter *config);
};

#endif /* ROLLER_SHUTTER_CONFIG_H_ */
