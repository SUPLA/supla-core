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

#ifndef CONNECTION_OBJECTS_H_
#define CONNECTION_OBJECTS_H_

#include "conn/connection_object.h"
#include "safearray.h"

class supla_connection_objects {
 private:
  void *arr;
  void *trash_arr;

 protected:
  supla_connection_object *find(_func_sa_cnd_param find_cnd, void *user_param);
  virtual void cd_delete(supla_connection_object *cd) = 0;

 public:
  supla_connection_objects();
  virtual ~supla_connection_objects();
  bool exists(supla_connection_object *cd);
  void releasePtr(supla_connection_object *cd);
  void addToList(supla_connection_object *cd);
  void moveAllToTrash();
  void moveToTrash(supla_connection_object *cd);
  bool emptyTrash(void);
  bool emptyTrash(unsigned char timeout_sec);
  bool deleteAll(unsigned char timeout_sec);
  int trashCount(void);
  supla_connection_object *get(int idx);
  int count(void);
};

#endif /* CONNECTION_OBJECTS_H_ */
