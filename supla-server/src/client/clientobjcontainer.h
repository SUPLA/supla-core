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

#ifndef CLIENTOBJCONTAINER_H_
#define CLIENTOBJCONTAINER_H_

enum e_objc_scope { master = 0, detail1 = 1, detail2 = 2 };

#define OBJC_SCOPE_COUNT 3

typedef struct {
  int id;
  int extra_id;
  bool use_both;
} _t_objc_search_fields;

class supla_client;
class database;
class supla_client_objcontainer_item;
class supla_client_objcontainer {
 private:
  supla_client *client;
  void *arr[OBJC_SCOPE_COUNT];
  supla_client_objcontainer_item *get_marked(e_objc_scope scope);
  bool do_remote_update(void *srpc, bool full, e_objc_scope scope);

 protected:
  bool id_cmp_use_both[OBJC_SCOPE_COUNT];
  static char arr_delcnd(void *ptr);
  static char arr_findcmp(void *ptr, void *_f);

  void arr_clean(void *arr);
  void *getArr(e_objc_scope scope);
  void *getArr(void);
  supla_client_objcontainer_item *find(_t_objc_search_fields *f,
                                       e_objc_scope scope);
  supla_client_objcontainer_item *find(int id, e_objc_scope scope);

  virtual void _load(database *db, e_objc_scope scope) = 0;
  virtual void _update(supla_client_objcontainer_item *obj,
                       supla_client_objcontainer_item *source,
                       e_objc_scope scope) = 0;
  virtual supla_client_objcontainer_item *new_item(
      supla_client_objcontainer_item *obj, e_objc_scope scope) = 0;
  virtual bool get_data_for_remote(supla_client_objcontainer_item *obj,
                                   void **data, bool full, bool EOL,
                                   bool *check_more) = 0;
  virtual void send_data_to_remote_and_free(void *srpc, void *data,
                                            bool full) = 0;

 public:
  explicit supla_client_objcontainer(supla_client *client);
  virtual ~supla_client_objcontainer();

  supla_client *getClient();
  int count(e_objc_scope scope);
  int count(void);
  void load(e_objc_scope scope);
  virtual void load(void);
  void update(supla_client_objcontainer_item *_obj, e_objc_scope scope);
  void update(supla_client_objcontainer_item *_obj);
  bool remote_update(void *srpc);
  void on_value_changed(void *srpc, int objId, int extraId);
  void on_value_changed(void *srpc, int objId);
};

#endif /* CLIENTOBJCONTAINER_H_ */
