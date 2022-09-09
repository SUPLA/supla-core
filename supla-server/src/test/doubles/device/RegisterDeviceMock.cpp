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

#include "doubles/device/RegisterDeviceMock.h"

namespace testing {
RegisterDeviceMock::RegisterDeviceMock(void)
    : supla_ch_abstract_register_device() {}

RegisterDeviceMock::~RegisterDeviceMock(void) {}

void RegisterDeviceMock::register_device(
    TDS_SuplaRegisterDevice_C *register_device_c,
    TDS_SuplaRegisterDevice_E *register_device_e,
    supla_abstract_srpc_adapter *srpc_adapter,
    supla_abstract_db_access_provider *dba,
    supla_abstract_device_dao *device_dao, int client_sd, int client_ipv4,
    unsigned char activity_timeout) {
  std::weak_ptr<supla_device> device;
  supla_ch_abstract_register_device::register_device(
      device, register_device_c, register_device_e, srpc_adapter, dba, nullptr,
      device_dao, client_sd, client_ipv4, activity_timeout);
}

void RegisterDeviceMock::set_hold_time_on_failure_usec(__useconds_t usec) {
  hold_time_on_failure_usec = usec;
}

bool RegisterDeviceMock::is_channel_added(void) {
  return supla_ch_abstract_register_device::is_channel_added();
}

int RegisterDeviceMock::get_device_id() {
  return supla_ch_abstract_register_device::get_device_id();
}

}  // namespace testing
