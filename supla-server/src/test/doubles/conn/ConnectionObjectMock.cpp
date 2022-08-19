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

#include "doubles/conn/ConnectionObjectMock.h"  // NOLINT

#include "gtest/gtest.h"  // NOLINT

ConnectionObjectMock::ConnectionObjectMock(supla_connection *connection)
    : supla_abstract_connection_object(connection) {}

void ConnectionObjectMock::set_cache_size_limit(int size) {
  authkey_auth_cache_size = size;
}

supla_abstract_srpc_call_handler_collection *
ConnectionObjectMock::get_srpc_call_handler_collection(void) {
  return NULL;
}

bool ConnectionObjectMock::authkey_auth(
    const char GUID[SUPLA_GUID_SIZE], const char Email[SUPLA_EMAIL_MAXSIZE],
    const char AuthKey[SUPLA_AUTHKEY_SIZE]) {
  int UserID = 0;
  return supla_abstract_connection_object::authkey_auth(GUID, Email, AuthKey,
                                                        &UserID, NULL);
}

void ConnectionObjectMock::set_id(int id) {
  supla_abstract_connection_object::set_id(id);
}

bool ConnectionObjectMock::set_guid(const char guid[SUPLA_GUID_SIZE]) {
  return supla_abstract_connection_object::set_guid(guid);
}

bool ConnectionObjectMock::set_authkey(const char authkey[SUPLA_AUTHKEY_SIZE]) {
  return supla_abstract_connection_object::set_authkey(authkey);
}
