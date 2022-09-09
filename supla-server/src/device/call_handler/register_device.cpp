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

supla_ch_register_device::supla_ch_register_device(void)
    : supla_ch_abstract_register_device() {}

supla_ch_register_device::~supla_ch_register_device() {}

supla_authkey_cache *supla_ch_register_device::get_authkey_cache(void) {
  return nullptr;
}

int supla_ch_register_device::get_user_id_by_email(
    const char email[SUPLA_EMAIL_MAXSIZE]) {
  return 0;
}

bool supla_ch_register_device::get_object_id(int user_id,
                                             const char guid[SUPLA_GUID_SIZE],
                                             int *id) {
  return false;
}

bool supla_ch_register_device::get_authkey_hash(
    int id, char authkey_hash[BCRYPT_HASH_MAXSIZE], bool *is_null) {
  return false;
}

void supla_ch_register_device::on_registraction_success(int device_id,
                                                        bool channels_added) {}
