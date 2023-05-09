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

#include "register_push_notification.h"

#include <memory>
#include <string>

#include "db/db_access_provider.h"
#include "device.h"
#include "push/pn_dao.h"
#include "push/pn_delivery_task.h"

using std::shared_ptr;
using std::string;

supla_ch_register_push_notification::supla_ch_register_push_notification(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_register_push_notification::~supla_ch_register_push_notification() {}

bool supla_ch_register_push_notification::can_handle_call(
    unsigned int call_id) {
  return call_id == SUPLA_DS_CALL_REGISTER_PUSH_NOTIFICATION;
}

void supla_ch_register_push_notification::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (!rd->data.ds_register_push_notification) {
    return;
  }

  TDS_RegisterPushNotification* reg = rd->data.ds_register_push_notification;

  supla_db_access_provider dba;
  supla_pn_dao dao(&dba);

  int channel_id = reg->Context >= 0
                       ? device->get_channels()->get_channel_id(reg->Context)
                       : 0;

  dao.add_or_update_device_managed_push(
      device->get_user_id(), device->get_id(), channel_id,
      !(reg->ServerManagedFields & PN_SERVER_MANAGED_TITLE),
      !(reg->ServerManagedFields & PN_SERVER_MANAGED_BODY),
      !(reg->ServerManagedFields & PN_SERVER_MANAGED_SOUND));
}
