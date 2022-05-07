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

#ifndef SUPLA_SCENE_OPERATION_H_
#define SUPLA_SCENE_OPERATION_H_

#include "actions/action_config.h"

class supla_scene_operation {
 private:
  int delay_ms;
  supla_action_config action_config;

 public:
  supla_scene_operation(void);
  virtual ~supla_scene_operation();

  void set_action_config(const supla_action_config &action);
  supla_action_config *get_action_config(void);
  void set_delay_ms(int delay_ms);
  int get_delay_ms(void);
  supla_scene_operation *clone(void);
};

#endif /* SUPLA_SCENE_OPERATION_H_ */
