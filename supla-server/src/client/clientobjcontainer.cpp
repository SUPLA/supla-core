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

#include "client.h"
#include <stdlib.h>  // NOLINT
#include "../database.h"
#include "../log.h"
#include "../safearray.h"
#include "clientobjcontainer.h"

supla_client_objcontainer::supla_client_objcontainer(supla_client *client)
    : supla_objcontainer() {
  this->client = client;
}

supla_client_objcontainer::~supla_client_objcontainer(void) {}

supla_client *supla_client_objcontainer::getClient() { return client; }

bool supla_client_objcontainer::do_remote_update(void *srpc, int data_type,
                                                 e_objc_scope scope) {
  void *data = NULL;
  supla_client_objcontainer_item *obj = NULL;

  safe_array_lock(getArr(scope));

  bool check_more = false;
  bool result = false;

  for (int a = 0; a < safe_array_count(getArr(scope)); a++) {
    obj = static_cast<supla_client_objcontainer_item *>(
        safe_array_get(getArr(scope), a));
    if (obj->marked_for_remote_update() & data_type) {
      if (get_data_for_remote(obj, &data, data_type, &check_more, scope)) {
        obj->unmark_for_remote_update(data_type);
        result = true;
      }

      if (!check_more) {
        break;
      }
    }
  }

  safe_array_unlock(getArr(scope));

  if (data) {
    send_data_to_remote_and_free(srpc, data, data_type, scope);
  }

  return result;
}

bool supla_client_objcontainer::remote_update(void *srpc) {
  for (int a = 0; a < OBJC_SCOPE_COUNT; a++) {
    e_objc_scope scope = static_cast<e_objc_scope>(a);
    int a_data_types = available_data_types_for_remote(scope);
    int data_type = 0x1;
    if (count(scope) > 0) {
      for (int b = 0; b < 4; b++) {
        if (a_data_types & data_type) {
          if (do_remote_update(srpc, data_type, scope)) {
            return true;
          }
        } else {
          break;
        }
        data_type <<= 1;
      }
    }
  }

  return false;
}

void supla_client_objcontainer::on_value_changed(void *srpc, int Id,
                                                 int ExtraId,
                                                 e_objc_scope scope,
                                                 int data_type) {
  supla_client_objcontainer_item *obj;
  bool r = false;

  void *arr = getArr(scope);

  safe_array_lock(arr);

  for (int a = 0; a < safe_array_count(arr); a++) {
    obj = static_cast<supla_client_objcontainer_item *>(safe_array_get(arr, a));
    if (obj && obj->getExtraId() == ExtraId &&
        (Id == 0 || obj->getId() == Id)) {
      obj->mark_for_remote_update(data_type);
      r = true;
    }
  }

  safe_array_unlock(arr);

  if (srpc && r) {
    remote_update(srpc);
  }
}
