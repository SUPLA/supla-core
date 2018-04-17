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

#ifndef OBJCONTAINER_H_
#define OBJCONTAINER_H_

#include "objcontaineritem.h"

typedef struct {
  int DeviceId;
  int ChannelId;
} t_dc_pair;

enum e_objc_scope { master = 0, detail1 = 1, detail2 = 2 };

#define OBJC_SCOPE_COUNT 3

typedef struct {
  int id;
  int extra_id;
  bool use_both;
} _t_objc_search_fields;

class database;
class supla_objcontainer_item;
class supla_objcontainer {
 private:
  void *arr[OBJC_SCOPE_COUNT];

 protected:
  bool id_cmp_use_both[OBJC_SCOPE_COUNT];
  static char arr_delcnd(void *ptr);
  static char arr_findcmp(void *ptr, void *_f);

  void arr_clean(void *arr);
  void *getArr(e_objc_scope scope);
  void *getArr(void);
  supla_objcontainer_item *find(_t_objc_search_fields *f, e_objc_scope scope);
  supla_objcontainer_item *find(int id, e_objc_scope scope);

  virtual void _load(database *db, e_objc_scope scope) = 0;

 public:
  supla_objcontainer(void);
  virtual ~supla_objcontainer();

  int count(e_objc_scope scope);
  int count(void);
  void load(e_objc_scope scope);
  virtual void load(void);
  virtual bool add(supla_objcontainer_item *obj, e_objc_scope scope);
  virtual bool add(supla_objcontainer_item *obj);
};

#endif /* OBJCONTAINER_H_ */
