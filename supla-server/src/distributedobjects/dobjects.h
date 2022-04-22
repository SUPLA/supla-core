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

#ifndef SUPLA_DISTRIBUTED_OBJECTS_H_
#define SUPLA_DISTRIBUTED_OBJECTS_H_

#include <distributedobjects/dobject.h>

#include <functional>
#include <list>

class supla_dobjects {
 private:
  void *lck;
  std::list<supla_dobject *> objects;

 protected:
 public:
  supla_dobjects(void);
  virtual ~supla_dobjects();
  void load(void);
  void load(int id);
  void access_object(int id,
                     std::function<void(supla_dobject *object)> on_access);
};

#endif /* SUPLA_DISTRIBUTED_OBJECTS_H_ */
