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

#ifndef SUPLA_SCENE_H_
#define SUPLA_SCENE_H_

#include <string>

#include "distributedobjects/dobject.h"
#include "scene/scene_operations.h"

class supla_scene : public supla_dobject {
 private:
  int location_id;
  std::string caption;
  supla_scene_operations *operations;

 protected:
 public:
  explicit supla_scene(int id);
  virtual ~supla_scene(void);
  void set_operations(supla_scene_operations *operations);
  void set_location_id(int location_id);
  void set_caption(const char *caption);
  int get_location_id(void);
  const std::string get_caption(void);
  virtual void set_remote_update_indicator(supla_dobject_change_indicator *rui);
  const supla_scene_operations *get_operations();
};

#endif /* SUPLA_SCENE_H_ */
