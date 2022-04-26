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

#ifndef SUPLA_DISTRIBUTED_OBJECT_REPOSITORY_H_
#define SUPLA_DISTRIBUTED_OBJECT_REPOSITORY_H_

#include <distributedobjects/dobject.h>

class supla_dobject_repository {
 private:
  int user_id;

 protected:
  int get_user_id(void);

 public:
  explicit supla_dobject_repository(int user_id);
  virtual ~supla_dobject_repository();
  virtual supla_dobject *get_first(void) = 0;
  virtual supla_dobject *get_next(void) = 0;
  virtual supla_dobject *get_object(int id) = 0;
};

#endif /* SUPLA_DISTRIBUTED_OBJECT_REPOSITORY_H_ */
