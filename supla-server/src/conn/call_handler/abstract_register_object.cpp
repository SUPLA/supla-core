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

#include "conn/call_handler/abstract_register_object.h"

#include <string.h>

#include "tools.h"

supla_ch_abstract_register_object::supla_ch_abstract_register_object(void) {}

supla_ch_abstract_register_object::~supla_ch_abstract_register_object() {}

bool supla_ch_abstract_register_object::authkey_auth(
    const char guid[SUPLA_GUID_SIZE], const char email[SUPLA_EMAIL_MAXSIZE],
    const char authkey[SUPLA_AUTHKEY_SIZE], int *user_id) {
  return get_authkey_cache()->authkey_auth(
      guid, email, authkey, user_id,
      [this, guid, email, authkey, user_id]() -> bool {
        int id = 0;
        int _user_id = get_user_id_by_email(email);

        if (_user_id == 0) {
          return false;
        }

        if (!get_object_id(_user_id, guid, &id)) {
          return false;
        }

        if (id == 0) {
          // Yes. When client/device not exists then is authorized
          *user_id = _user_id;
          return true;
        }

        bool is_null = false;
        char authkey_hash[BCRYPT_HASH_MAXSIZE];
        memset(authkey_hash, 0, BCRYPT_HASH_MAXSIZE);

        if (!get_authkey_hash(id, authkey_hash, BCRYPT_HASH_MAXSIZE,
                              &is_null)) {
          return false;
        }

        if (is_null) {  // Yes. When is null then is authorized
          *user_id = _user_id;
          return true;
        }

        char authkey_hex[SUPLA_AUTHKEY_HEXSIZE];
        memset(authkey_hex, 0, SUPLA_AUTHKEY_HEXSIZE);

        st_authkey2hex(authkey_hex, authkey);

        if (st_bcrypt_check(authkey_hex, authkey_hash,
                            strnlen(authkey_hash, BCRYPT_HASH_MAXSIZE))) {
          *user_id = _user_id;
          return true;
        }

        return false;
      });
}

bool supla_ch_abstract_register_object::is_valid_guid(
    char guid[SUPLA_GUID_SIZE]) {
  char _guid[SUPLA_GUID_SIZE] = {};
  return memcmp(_guid, guid, SUPLA_GUID_SIZE) == 0;
}

bool supla_ch_abstract_register_object::is_valid_authkey(
    char authkey[SUPLA_AUTHKEY_SIZE]) {
  char _authkey[SUPLA_AUTHKEY_SIZE] = {};
  return memcmp(_authkey, authkey, SUPLA_AUTHKEY_SIZE) == 0;
}
