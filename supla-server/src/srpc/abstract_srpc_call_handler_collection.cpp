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

#include "srpc/abstract_srpc_call_hanlder_collection.h"

using std::shared_ptr;

supla_abstract_srpc_call_handler_collection::
    supla_abstract_srpc_call_handler_collection(void) {}

supla_abstract_srpc_call_handler_collection::
    ~supla_abstract_srpc_call_handler_collection() {
  for (auto it = handlers.cbegin(); it != handlers.cend(); ++it) {
    delete *it;
  }
}

void supla_abstract_srpc_call_handler_collection::add_handler(
    supla_abstract_srpc_call_handler* handler) {
  handlers.push_back(handler);
}

bool supla_abstract_srpc_call_handler_collection::handle_call(
    std::shared_ptr<supla_abstract_connection_object> object,
    supla_abstract_srpc_adapter* srpc_adapter, TsrpcReceivedData* rd,
    unsigned int call_id, unsigned char proto_version) {
  supla_abstract_srpc_call_handler* handler = nullptr;

  for (auto it = handlers.cbegin(); it != handlers.cend(); ++it) {
    if ((*it)->can_handle_call(call_id)) {
      handler = *it;
      break;
    }
  }

  if (handler) {
    if (!handler->is_registration_required() || object->is_registered()) {
      handler->handle_call(object, srpc_adapter, rd, call_id, proto_version);
    }
    return true;
  }

  return false;
}
