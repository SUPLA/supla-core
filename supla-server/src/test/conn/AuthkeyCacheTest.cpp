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

#include <conn/AuthkeyCacheTest.h>

#include "doubles/conn/AuthkeyCacheStub.h"

namespace testing {

TEST_F(AuthkeyCacheTest, authkeyWithoutCache) {
  AuthkeyCacheStub cache;

  EXPECT_EQ(0, cache.get_cache_size_limit());
  EXPECT_EQ(0, cache.get_size());

  cache.set_cache_size_limit(0);

  bool db_auth = false;

  char guid[SUPLA_GUID_SIZE] = {};
  char email[SUPLA_EMAIL_MAXSIZE] = {};
  char authkey[SUPLA_AUTHKEY_SIZE] = {};
  int user_id = 10;

  guid[0] = 1;
  email[0] = 'e';
  authkey[0] = 2;

  EXPECT_FALSE(
      cache.authkey_auth(guid, email, authkey, &user_id, [&db_auth]() -> bool {
        db_auth = true;
        return false;
      }));

  EXPECT_TRUE(db_auth);
  db_auth = false;

  EXPECT_TRUE(
      cache.authkey_auth(guid, email, authkey, &user_id, [&db_auth]() -> bool {
        db_auth = true;
        return true;
      }));

  EXPECT_TRUE(db_auth);

  EXPECT_EQ(0, cache.get_size());
}

TEST_F(AuthkeyCacheTest, authkeyWithCache) {
  AuthkeyCacheStub cache;
  EXPECT_EQ(0, cache.get_cache_size_limit());

  cache.set_cache_size_limit(5);
  EXPECT_EQ(5, cache.get_cache_size_limit());

  char guid[SUPLA_GUID_SIZE] = {};
  char email[SUPLA_EMAIL_MAXSIZE] = {};
  char authkey[SUPLA_AUTHKEY_SIZE] = {};
  int user_id = 10;

  int db_auth_counter = 0;

  EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                 [&db_auth_counter]() -> bool {
                                   db_auth_counter++;
                                   return true;
                                 }));

  EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                 [&db_auth_counter]() -> bool {
                                   db_auth_counter++;
                                   return true;
                                 }));

  EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                 [&db_auth_counter]() -> bool {
                                   db_auth_counter++;
                                   return true;
                                 }));

  EXPECT_EQ(1, cache.get_size());

  guid[0] = 2;

  EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                 [&db_auth_counter]() -> bool {
                                   db_auth_counter++;
                                   return true;
                                 }));

  EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                 [&db_auth_counter]() -> bool {
                                   db_auth_counter++;
                                   return true;
                                 }));

  EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                 [&db_auth_counter]() -> bool {
                                   db_auth_counter++;
                                   return true;
                                 }));

  EXPECT_EQ(2, cache.get_size());

  email[0] = '@';

  EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                 [&db_auth_counter]() -> bool {
                                   db_auth_counter++;
                                   return true;
                                 }));

  EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                 [&db_auth_counter]() -> bool {
                                   db_auth_counter++;
                                   return true;
                                 }));

  EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                 [&db_auth_counter]() -> bool {
                                   db_auth_counter++;
                                   return true;
                                 }));

  EXPECT_EQ(3, cache.get_size());

  user_id = 11;

  EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                 [&db_auth_counter]() -> bool {
                                   db_auth_counter++;
                                   return true;
                                 }));

  EXPECT_EQ(3, cache.get_size());
  EXPECT_EQ(10, user_id);

  for (int b = 0; b < 10; b++) {
    email[b] = '@';
    for (int a = 0; a < 200; a++) {
      authkey[0] = 1 + a;

      EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                     [&db_auth_counter]() -> bool {
                                       db_auth_counter++;
                                       return true;
                                     }));

      EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                     [&db_auth_counter]() -> bool {
                                       db_auth_counter++;
                                       return true;
                                     }));

      EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                     [&db_auth_counter]() -> bool {
                                       db_auth_counter++;
                                       return true;
                                     }));

      EXPECT_EQ(a > 1 || b > 0 ? 5 : 4 + a, cache.get_size());
    }
  }
}

TEST_F(AuthkeyCacheTest, authkeyNullTest) {
  AuthkeyCacheStub cache;
  EXPECT_EQ(0, cache.get_size());

  cache.set_cache_size_limit(5);

  char guid[SUPLA_GUID_SIZE] = {};
  char email[SUPLA_EMAIL_MAXSIZE] = {};
  char authkey[SUPLA_AUTHKEY_SIZE] = {};
  int user_id = 15;

  EXPECT_FALSE(cache.authkey_auth(NULL, email, authkey, &user_id,
                                  []() -> bool { return true; }));
  EXPECT_EQ(0, cache.get_size());

  EXPECT_FALSE(cache.authkey_auth(guid, NULL, authkey, &user_id,
                                  []() -> bool { return true; }));
  EXPECT_EQ(0, cache.get_size());

  EXPECT_FALSE(cache.authkey_auth(guid, email, NULL, &user_id,
                                  []() -> bool { return true; }));
  EXPECT_EQ(0, cache.get_size());

  EXPECT_FALSE(cache.authkey_auth(guid, email, authkey, NULL,
                                  []() -> bool { return true; }));
  EXPECT_EQ(0, cache.get_size());

  EXPECT_TRUE(cache.authkey_auth(guid, email, authkey, &user_id,
                                  []() -> bool { return true; }));
  EXPECT_EQ(1, cache.get_size());
}

}  // namespace testing
