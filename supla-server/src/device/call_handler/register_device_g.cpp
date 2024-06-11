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

#include "register_device_g.h"

#include <stdlib.h>
#include <string.h>

#include <memory>

#include "device/call_handler/register_device.h"
#include "log.h"
#include "proto.h"

using std::shared_ptr;

supla_ch_register_device_g::supla_ch_register_device_g(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_register_device_g::~supla_ch_register_device_g() {}

bool supla_ch_register_device_g::is_registration_required(void) {
  return false;
}

bool supla_ch_register_device_g::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_DS_CALL_REGISTER_DEVICE_G;
}

void supla_ch_register_device_g::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  supla_log(LOG_DEBUG, "SUPLA_DS_CALL_REGISTER_DEVICE_G");

  if (rd->data.ds_register_device_g != nullptr) {
    rd->data.ds_register_device_g->Email[SUPLA_EMAIL_MAXSIZE - 1] = 0;
    rd->data.ds_register_device_g->Name[SUPLA_DEVICE_NAME_MAXSIZE - 1] = 0;
    rd->data.ds_register_device_g->SoftVer[SUPLA_SOFTVER_MAXSIZE - 1] = 0;
    rd->data.ds_register_device_g->ServerName[SUPLA_SERVER_NAME_MAXSIZE - 1] =
        0;

    supla_register_device regdev;

    regdev.register_device(device, nullptr, rd->data.ds_register_device_g,
                           srpc_adapter,
                           device->get_connection()->get_client_sd(),
                           device->get_connection()->get_client_ipv4(),
                           device->get_connection()->get_activity_timeout());
  }
}
