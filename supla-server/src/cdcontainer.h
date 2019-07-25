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

#ifndef CDCONTAINER_H_
#define CDCONTAINER_H_

#include "cdbase.h"
#include "safearray.h"

class cdcontainer {
 private:
  void *arr;
  void *trash_arr;

 protected:
  cdbase *find(_func_sa_cnd_param find_cnd,
                           void *user_param);
  virtual void cd_delete(cdbase *cd) = 0;
 public:
  cdcontainer();
  virtual ~cdcontainer();
  bool exists(cdbase *cd);
  void releasePtr(cdbase *cd);
  void addToList(cdbase *cd);
  void moveAllToTrash();
  void moveToTrash(cdbase *cd);
  bool emptyTrash(void);
  bool emptyTrash(unsigned char timeout_sec);
  bool deleteAll(unsigned char timeout_sec);
  int trashCount(void);
  cdbase *get(int idx);
  int count(void);
};

#endif /* CDCONTAINER_H_ */
