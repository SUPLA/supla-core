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

#include "conn/ConnectionObjectTest.h"

#include "doubles/conn/ConnectionObjectMock.h"

namespace testing {

TEST_F(ConnectionObjectTest, guidSetterAndGtter) {
  char v[SUPLA_GUID_SIZE];
  char v0[SUPLA_GUID_SIZE];
  char v1[SUPLA_GUID_SIZE];

  memset(v, 0, SUPLA_GUID_SIZE);
  memset(v0, 0, SUPLA_GUID_SIZE);
  memset(v1, 1, SUPLA_GUID_SIZE);

  ConnectionObjectMock *object = new ConnectionObjectMock(NULL);

  object->get_guid(v);
  ASSERT_EQ(0, memcmp(v, v0, SUPLA_GUID_SIZE));

  object->set_guid(v1);
  object->get_guid(v);
  ASSERT_EQ(0, memcmp(v, v1, SUPLA_GUID_SIZE));

  object->get_authkey(v);
  ASSERT_EQ(0, memcmp(v, v0, SUPLA_AUTHKEY_SIZE));

  delete object;
}

TEST_F(ConnectionObjectTest, authkeySetterAndGtter) {
  char v[SUPLA_AUTHKEY_SIZE];
  char v0[SUPLA_AUTHKEY_SIZE];
  char v1[SUPLA_AUTHKEY_SIZE];

  memset(v, 0, SUPLA_AUTHKEY_SIZE);
  memset(v0, 0, SUPLA_AUTHKEY_SIZE);
  memset(v1, 1, SUPLA_AUTHKEY_SIZE);

  ConnectionObjectMock *object = new ConnectionObjectMock(NULL);

  object->get_authkey(v);
  ASSERT_EQ(0, memcmp(v, v0, SUPLA_AUTHKEY_SIZE));

  object->set_authkey(v1);
  object->get_authkey(v);
  ASSERT_EQ(0, memcmp(v, v1, SUPLA_AUTHKEY_SIZE));

  object->get_guid(v);
  ASSERT_EQ(0, memcmp(v, v0, SUPLA_GUID_SIZE));

  delete object;
}

}  // namespace testing
