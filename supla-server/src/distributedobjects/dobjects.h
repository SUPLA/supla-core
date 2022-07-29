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

#include <distributedobjects/abstract_dobject_remote_updater.h>

#include <functional>
#include <list>
#include <vector>

#include "distributedobjects/dobject.h"

class supla_dobjects {
 private:
  void *lck;
  std::vector<supla_dobject *> objects;
  supla_abstract_dobject_remote_updater *updater;

 protected:
  void lock(void);
  void unlock(void);

  void clear(void);
  void add(supla_dobject *object);
  void remove(int id);

  void access_object(int id,
                     std::function<void(supla_dobject *object)> on_access);

 public:
  explicit supla_dobjects(supla_abstract_dobject_remote_updater *updater);
  virtual ~supla_dobjects();
  int count(void);
  bool update_remote(void);
  bool object_exists(int id);
};

#endif /* SUPLA_DISTRIBUTED_OBJECTS_H_ */
