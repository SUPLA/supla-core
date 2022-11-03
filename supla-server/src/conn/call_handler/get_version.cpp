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

#include "conn/call_handler/get_version.h"

#include "svrcfg.h"

using std::shared_ptr;

supla_ch_get_version::supla_ch_get_version(void)
    : supla_abstract_srpc_call_handler() {}

supla_ch_get_version::~supla_ch_get_version() {}

bool supla_ch_get_version::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_DCS_CALL_GETVERSION;
}

bool supla_ch_get_version::is_registration_required(void) { return false; }

void supla_ch_get_version::handle_call(
    shared_ptr<supla_abstract_connection_object> object,
    supla_abstract_srpc_adapter* srpc_adapter, TsrpcReceivedData* rd,
    unsigned int call_id, unsigned char proto_version) {
  char softver[SUPLA_SOFTVER_MAXSIZE] = {};

  snprintf(softver, SUPLA_SOFTVER_MAXSIZE, SERVER_VERSION);
  srpc_adapter->sdc_async_getversion_result(softver);
}
