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

#include "client/call_handler/abstract_client_srpc_call_handler.h"

using std::dynamic_pointer_cast;
using std::shared_ptr;

supla_abstract_client_srpc_call_handler::
    supla_abstract_client_srpc_call_handler(void)
    : supla_abstract_srpc_call_handler() {}

supla_abstract_client_srpc_call_handler::
    ~supla_abstract_client_srpc_call_handler() {}

bool supla_abstract_client_srpc_call_handler::handle_call(
    shared_ptr<supla_abstract_connection_object> object,
    supla_abstract_srpc_adapter* srpc_adapter, TsrpcReceivedData* rd,
    unsigned int call_id, unsigned char proto_version) {
  shared_ptr<supla_client> client = dynamic_pointer_cast<supla_client>(object);

  return client != nullptr &&
                 handle_call(client, srpc_adapter, rd, call_id, proto_version)
             ? CH_HANDLED
             : CH_UNHANDLED;
}
