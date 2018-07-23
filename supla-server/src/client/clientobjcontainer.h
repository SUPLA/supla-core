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

#include "objcontainer.h"
#include "objcontaineritem.h"

class supla_client;
class supla_client_objcontainer_item;
class supla_client_objcontainer : public supla_objcontainer {
 private:
  supla_client *client;
  bool do_remote_update(void *srpc, int data_type, e_objc_scope scope);

 protected:
  virtual bool get_data_for_remote(supla_client_objcontainer_item *obj,
                                   void **data, int data_type, bool *check_more,
                                   e_objc_scope scope) = 0;
  virtual void send_data_to_remote_and_free(void *srpc, void *data,
                                            int data_type,
                                            e_objc_scope scope) = 0;

  void on_value_changed(void *srpc, int Id, int ExtraId, e_objc_scope scope,
                        int data_type);

 public:
  explicit supla_client_objcontainer(supla_client *client);
  virtual ~supla_client_objcontainer(void);

  supla_client *getClient();
  bool remote_update(void *srpc);
};

#endif /* CLIENTOBJCONTAINER_H_ */
