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

#ifndef SUPLA_SCENE_OPERATIONS_H_
#define SUPLA_SCENE_OPERATIONS_H_

#include <scene/scene_operation.h>

#include <list>

class supla_scene_operations {
 private:
  std::list<supla_scene_operation *> operations;

 public:
  supla_scene_operations();
  virtual ~supla_scene_operations();

  int count(void);
  void push(supla_scene_operation *operation);
  supla_scene_operation *pop(void);
  unsigned int get_delay_ms(void);
  unsigned int get_total_delay_ms(void);
  supla_scene_operations *clone(void) const;
};

#endif /* SUPLA_SCENE_OPERATIONS_H_ */
