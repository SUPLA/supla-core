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

TEST_F(ConnectionObjectTest, authkeyWithoutCache) {
  supla_connection_object::init();

  ConnectionObjectMock object(nullptr);
  EXPECT_EQ(0, supla_connection_object::get_authkey_cache_size());

  object.set_cache_size_limit(0);

  EXPECT_CALL(object, db_authkey_auth)
      .Times(2)
      .WillRepeatedly([](const char _guid[SUPLA_GUID_SIZE],
                         const char _email[SUPLA_EMAIL_MAXSIZE],
                         const char _authkey[SUPLA_AUTHKEY_SIZE], int *user_id,
                         database *db) {
        char guid[SUPLA_GUID_SIZE] = {};
        char email[SUPLA_EMAIL_MAXSIZE] = {};
        char authkey[SUPLA_AUTHKEY_SIZE] = {};

        guid[0] = 1;
        authkey[1] = 2;
        email[2] = 3;

        return memcmp(guid, _guid, SUPLA_GUID_SIZE) == 0 &&
               memcmp(email, _email, SUPLA_EMAIL_MAXSIZE) == 0 &&
               memcmp(authkey, _authkey, SUPLA_AUTHKEY_SIZE) == 0;
      });

  char guid[SUPLA_GUID_SIZE] = {};
  char email[SUPLA_EMAIL_MAXSIZE] = {};
  char authkey[SUPLA_AUTHKEY_SIZE] = {};

  EXPECT_FALSE(object.authkey_auth(guid, email, authkey));

  guid[0] = 1;
  authkey[1] = 2;
  email[2] = 3;

  EXPECT_TRUE(object.authkey_auth(guid, email, authkey));
  EXPECT_EQ(0, supla_connection_object::get_authkey_cache_size());

  supla_connection_object::release_cache();
}

TEST_F(ConnectionObjectTest, authkeyWithCache) {
  supla_connection_object::init();

  ConnectionObjectMock object(nullptr);
  EXPECT_EQ(0, supla_connection_object::get_authkey_cache_size());

  object.set_cache_size_limit(5);

  EXPECT_CALL(object, db_authkey_auth).Times(2003).WillRepeatedly(Return(true));

  char guid[SUPLA_GUID_SIZE] = {};
  char email[SUPLA_EMAIL_MAXSIZE] = {};
  char authkey[SUPLA_AUTHKEY_SIZE] = {};

  EXPECT_TRUE(object.authkey_auth(guid, email, authkey));
  EXPECT_TRUE(object.authkey_auth(guid, email, authkey));
  EXPECT_TRUE(object.authkey_auth(guid, email, authkey));

  EXPECT_EQ(1, supla_connection_object::get_authkey_cache_size());

  guid[0] = 2;

  EXPECT_TRUE(object.authkey_auth(guid, email, authkey));
  EXPECT_TRUE(object.authkey_auth(guid, email, authkey));
  EXPECT_TRUE(object.authkey_auth(guid, email, authkey));

  EXPECT_EQ(2, supla_connection_object::get_authkey_cache_size());

  email[0] = '@';

  EXPECT_TRUE(object.authkey_auth(guid, email, authkey));
  EXPECT_TRUE(object.authkey_auth(guid, email, authkey));
  EXPECT_TRUE(object.authkey_auth(guid, email, authkey));

  EXPECT_EQ(3, supla_connection_object::get_authkey_cache_size());

  for (int b = 0; b < 10; b++) {
    email[b] = '@';
    for (int a = 0; a < 200; a++) {
      authkey[0] = 1 + a;

      EXPECT_TRUE(object.authkey_auth(guid, email, authkey));
      EXPECT_TRUE(object.authkey_auth(guid, email, authkey));
      EXPECT_TRUE(object.authkey_auth(guid, email, authkey));

      EXPECT_EQ(a > 1 || b > 0 ? 5 : 4 + a,
                supla_connection_object::get_authkey_cache_size());
    }
  }

  supla_connection_object::release_cache();
}

TEST_F(ConnectionObjectTest, authkeyNullTest) {
  supla_connection_object::init();

  ConnectionObjectMock object(nullptr);
  EXPECT_EQ(0, supla_connection_object::get_authkey_cache_size());

  object.set_cache_size_limit(5);

  EXPECT_CALL(object, db_authkey_auth).Times(1).WillRepeatedly(Return(true));

  char guid[SUPLA_GUID_SIZE] = {};
  char email[SUPLA_EMAIL_MAXSIZE] = {};
  char authkey[SUPLA_AUTHKEY_SIZE] = {};

  EXPECT_FALSE(object.authkey_auth(NULL, email, authkey));
  EXPECT_EQ(0, supla_connection_object::get_authkey_cache_size());

  EXPECT_FALSE(object.authkey_auth(guid, NULL, authkey));
  EXPECT_EQ(0, supla_connection_object::get_authkey_cache_size());

  EXPECT_FALSE(object.authkey_auth(guid, email, NULL));
  EXPECT_EQ(0, supla_connection_object::get_authkey_cache_size());

  EXPECT_TRUE(object.authkey_auth(guid, email, authkey));
  EXPECT_EQ(1, supla_connection_object::get_authkey_cache_size());

  supla_connection_object::release_cache();
}

TEST_F(ConnectionObjectTest, guidSetterAndGtter) {
  char v[SUPLA_GUID_SIZE];
  char v0[SUPLA_GUID_SIZE];
  char v1[SUPLA_GUID_SIZE];

  memset(v, 0, SUPLA_GUID_SIZE);
  memset(v0, 0, SUPLA_GUID_SIZE);
  memset(v1, 1, SUPLA_GUID_SIZE);

  supla_connection_object::init();
  ConnectionObjectMock *cd = new ConnectionObjectMock(NULL);

  cd->get_guid(v);
  ASSERT_EQ(0, memcmp(v, v0, SUPLA_GUID_SIZE));

  cd->set_guid(v1);
  cd->get_guid(v);
  ASSERT_EQ(0, memcmp(v, v1, SUPLA_GUID_SIZE));

  cd->get_authkey(v);
  ASSERT_EQ(0, memcmp(v, v0, SUPLA_AUTHKEY_SIZE));

  delete cd;
  supla_connection_object::release_cache();
}

TEST_F(ConnectionObjectTest, authkeySetterAndGtter) {
  char v[SUPLA_AUTHKEY_SIZE];
  char v0[SUPLA_AUTHKEY_SIZE];
  char v1[SUPLA_AUTHKEY_SIZE];

  memset(v, 0, SUPLA_AUTHKEY_SIZE);
  memset(v0, 0, SUPLA_AUTHKEY_SIZE);
  memset(v1, 1, SUPLA_AUTHKEY_SIZE);

  supla_connection_object::init();
  ConnectionObjectMock *cd = new ConnectionObjectMock(NULL);

  cd->get_authkey(v);
  ASSERT_EQ(0, memcmp(v, v0, SUPLA_AUTHKEY_SIZE));

  cd->set_authkey(v1);
  cd->get_authkey(v);
  ASSERT_EQ(0, memcmp(v, v1, SUPLA_AUTHKEY_SIZE));

  cd->get_guid(v);
  ASSERT_EQ(0, memcmp(v, v0, SUPLA_GUID_SIZE));

  delete cd;
  supla_connection_object::release_cache();
}

}  // namespace testing
