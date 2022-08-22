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

#include "conn/call_handler/set_activity_timeout.h"

#include <stdlib.h>
#include <string.h>

#include <memory>

#include "log.h"
#include "proto.h"

using std::shared_ptr;

#define ACTIVITY_TIMEOUT_MIN 10
#define ACTIVITY_TIMEOUT_MAX 240

supla_ch_set_activity_timeout::supla_ch_set_activity_timeout(void)
    : supla_abstract_srpc_call_handler() {}

supla_ch_set_activity_timeout::~supla_ch_set_activity_timeout() {}

bool supla_ch_set_activity_timeout::handle_call(
    shared_ptr<supla_abstract_connection_object> object,
    supla_abstract_srpc_adapter* srpc_adapter, TsrpcReceivedData* rd,
    unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_DCS_CALL_SET_ACTIVITY_TIMEOUT ||
      !object->is_registered()) {
    return false;
  }

  if (rd->data.dcs_set_activity_timeout->activity_timeout <
      ACTIVITY_TIMEOUT_MIN)
    rd->data.dcs_set_activity_timeout->activity_timeout = ACTIVITY_TIMEOUT_MIN;

  if (rd->data.dcs_set_activity_timeout->activity_timeout >
      ACTIVITY_TIMEOUT_MAX)
    rd->data.dcs_set_activity_timeout->activity_timeout = ACTIVITY_TIMEOUT_MAX;

  TSDC_SuplaSetActivityTimeoutResult result;
  result.activity_timeout = rd->data.dcs_set_activity_timeout->activity_timeout;
  result.min = ACTIVITY_TIMEOUT_MIN;
  result.max = ACTIVITY_TIMEOUT_MAX;

  object->get_connection()->set_activity_timeout(result.activity_timeout);

  srpc_adapter->dcs_async_set_activity_timeout_result(&result);

  return true;
}
