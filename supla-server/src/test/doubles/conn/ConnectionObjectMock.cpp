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

supla_abstract_srpc_call_handler_collection *
ConnectionObjectMock::get_srpc_call_handler_collection(void) {
  return NULL;
}

void ConnectionObjectMock::set_id(int id) {
  supla_abstract_connection_object::set_id(id);
}

void ConnectionObjectMock::set_guid(const char guid[SUPLA_GUID_SIZE]) {
  supla_abstract_connection_object::set_guid(guid);
}

void ConnectionObjectMock::set_authkey(const char authkey[SUPLA_AUTHKEY_SIZE]) {
  supla_abstract_connection_object::set_authkey(authkey);
}

bool ConnectionObjectMock::is_sleeping_object(void) { return false; }

unsigned int ConnectionObjectMock::get_time_to_wakeup_msec(void) { return 0; }
