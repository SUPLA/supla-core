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

class supla_client;
class database;
class supla_client_objcontainer_item;
class supla_client_objcontainer {
 private:
  supla_client *client;
  void *arr;
  supla_client_objcontainer_item *get_marked(void);
  bool do_remote_update(void *srpc, bool full);

 protected:
  static char arr_delcnd(void *ptr);
  static char arr_findcmp(void *ptr, void *id);

  void arr_clean(void);
  void *getArr(void);
  supla_client_objcontainer_item *find(int Id);

  virtual void _load(database *db) = 0;
  virtual void _update(supla_client_objcontainer_item *obj,
                       supla_client_objcontainer_item *source) = 0;
  virtual supla_client_objcontainer_item *new_item(
      supla_client_objcontainer_item *obj) = 0;
  virtual bool get_data_for_remote(supla_client_objcontainer_item *obj,
                                   void **data, bool full, bool EOL,
                                   bool *check_more) = 0;
  virtual void send_data_to_remote_and_free(void *srpc, void *data,
                                            bool full) = 0;

 public:
  explicit supla_client_objcontainer(supla_client *client);
  virtual ~supla_client_objcontainer();

  supla_client *getClient();
  int count(void);
  void load(void);
  void update(supla_client_objcontainer_item *_obj);
  bool remote_update(void *srpc);
  void on_value_changed(void *srpc, int objId, int extraId);
  void on_value_changed(void *srpc, int objId);
};

#endif /* CLIENTOBJCONTAINER_H_ */
