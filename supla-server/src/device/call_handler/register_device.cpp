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
#include "device/device.h"
#include "user/user.h"

using std::shared_ptr;

supla_ch_register_device::supla_ch_register_device(void)
    : supla_ch_abstract_register_device() {}

supla_ch_register_device::~supla_ch_register_device() {}

supla_authkey_cache *supla_ch_register_device::get_authkey_cache(void) {
  return &supla_authkey_cache::get_global_instance();
}

int supla_ch_register_device::get_user_id_by_email(
    const char email[SUPLA_EMAIL_MAXSIZE]) {
  return 0;
}

bool supla_ch_register_device::get_object_id(int user_id,
                                             const char guid[SUPLA_GUID_SIZE],
                                             int *id) {
  return get_device_dao()->get_device_id(user_id, guid, id);
}

bool supla_ch_register_device::get_authkey_hash(
    int id, char authkey_hash[BCRYPT_HASH_MAXSIZE], bool *is_null) {
  return false;
}

void supla_ch_register_device::on_registraction_success(void) {
  shared_ptr<supla_device> device = get_device().lock();

  device->set_id(get_device_id());
  device->set_user(supla_user::find(get_user_id(), true));
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
