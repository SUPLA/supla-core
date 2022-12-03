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

#include "device/call_handler/register_device.h"

#include <memory>

#include "conn/authkey_cache.h"
#include "conn/connection_dao.h"
#include "db/db_access_provider.h"
#include "device/device.h"
#include "device/device_dao.h"
#include "user/user.h"

using std::shared_ptr;

supla_register_device::supla_register_device(void)
    : supla_abstract_register_device() {}

supla_register_device::~supla_register_device() {}

supla_authkey_cache *supla_register_device::get_authkey_cache(void) {
  return &supla_authkey_cache::get_global_instance();
}

int supla_register_device::get_user_id_by_email(
    const char email[SUPLA_EMAIL_MAXSIZE]) {
  return get_conn_dao()->get_user_id_by_email(email);
}

bool supla_register_device::get_object_id(int user_id,
                                          const char guid[SUPLA_GUID_SIZE],
                                          int *id) {
  return get_device_dao()->get_device_id(user_id, guid, id);
}

bool supla_register_device::get_authkey_hash(
    int id, char authkey_hash[BCRYPT_HASH_MAXSIZE], bool *is_null) {
  return get_device_dao()->get_authkey_hash(id, authkey_hash, is_null);
}

bool supla_register_device::is_prev_entering_cfg_mode(void) {
  supla_user *user = supla_user::find(get_user_id(), true);
  if (user) {
    shared_ptr<supla_device> prev = user->get_devices()->get(get_device_id());
    return prev != nullptr && prev->entering_cfg_mode_in_progress;
  }
  return false;
}

void supla_register_device::on_registraction_success(void) {
  shared_ptr<supla_device> device = get_device().lock();

  device->set_id(get_device_id());
  device->set_guid(get_guid());
  device->set_authkey(get_authkey());
  device->set_user(supla_user::find(get_user_id(), true));
  device->set_flags(get_device_flags());
  device->set_registered(true);

  device->load_config(get_user_id());

  device->get_channels()->update_channels(get_channels_b(), get_channels_c(),
                                          get_channel_count());

  supla_user::add_device(device->get_shared_ptr(), get_user_id());
  device->get_user()->get_clients()->update_device_channels(get_location_id(),
                                                            get_device_id());

  device->get_channels()->on_device_registered(
      device->get_user(), get_device_id(), get_channels_b(), get_channels_c(),
      get_channel_count());

  if (is_channel_added()) {
    device->get_user()->on_channels_added(
        get_device_id(), supla_caller(ctDevice, get_device_id()));
  }

  device->get_user()->on_device_registered(
      get_device_id(), supla_caller(ctDevice, get_device_id()));
}

void supla_register_device::register_device(
    std::weak_ptr<supla_device> device,
    TDS_SuplaRegisterDevice_C *register_device_c,
    TDS_SuplaRegisterDevice_E *register_device_e,
    supla_abstract_srpc_adapter *srpc_adapter, int client_sd, int client_ipv4,
    unsigned char activity_timeout) {
  {
    shared_ptr<supla_device> _device = device.lock();
    if (_device && _device->is_registered()) {
      _device->terminate();
      return;
    }
  }

  supla_db_access_provider dba;
  supla_connection_dao conn_dao(&dba);
  supla_device_dao device_dao(&dba);

  supla_abstract_register_device::register_device(
      device, register_device_c, register_device_e, srpc_adapter, &dba,
      &conn_dao, &device_dao, client_sd, client_ipv4, activity_timeout);

  // Disconnect the database connection before calling
  // on_object_registration_done()
  dba.disconnect();

  {
    shared_ptr<supla_device> _device = device.lock();
    if (_device) {
      _device->get_connection()->on_object_registration_done();
    }
  }
}
