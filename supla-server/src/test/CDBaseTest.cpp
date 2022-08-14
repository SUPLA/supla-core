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

#include "CDBaseTest.h"

#include "doubles/CDBaseMock.h"
#include "gtest/gtest.h"  // NOLINT

namespace {

class CDBaseTest : public ::testing::Test {};

TEST_F(CDBaseTest, retainReleaseTest) {
  CDBaseMock *cd = new CDBaseMock(NULL);

  ASSERT_FALSE(cd == NULL);
  ASSERT_FALSE(cd->ptr_is_used());

  ASSERT_EQ((unsigned long)0, cd->get_ptr_counter());

  cd->retain_ptr();
  ASSERT_TRUE(cd->ptr_is_used());
  ASSERT_EQ((unsigned long)1, cd->get_ptr_counter());

  cd->retain_ptr();
  ASSERT_TRUE(cd->ptr_is_used());
  ASSERT_EQ((unsigned long)2, cd->get_ptr_counter());

  cd->release_ptr();
  ASSERT_TRUE(cd->ptr_is_used());
  ASSERT_EQ((unsigned long)1, cd->get_ptr_counter());

  cd->release_ptr();
  ASSERT_FALSE(cd->ptr_is_used());
  ASSERT_EQ((unsigned long)0, cd->get_ptr_counter());

  delete cd;
}

TEST_F(CDBaseTest, authkey_cache) {
  cdbase::init();

  CDBaseMock *cd = new CDBaseMock(NULL);

  char GUID[SUPLA_GUID_SIZE];
  char Email[SUPLA_EMAIL_MAXSIZE];
  char AuthKey[SUPLA_AUTHKEY_SIZE];

  memset(GUID, 0, SUPLA_GUID_SIZE);
  memset(Email, 0, SUPLA_EMAIL_MAXSIZE);
  memset(AuthKey, 0, SUPLA_AUTHKEY_SIZE);

  ASSERT_EQ(0, cd->getDbAuthCount());
  ASSERT_EQ(0, cdbase::get_authkey_cache_size());

  cd->setCacheSizeLimit(0);

  ASSERT_FALSE(cd->authkey_auth(GUID, Email, AuthKey));
  ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));

  ASSERT_EQ(2, cd->getDbAuthCount());
  ASSERT_EQ(0, cdbase::get_authkey_cache_size());

  GUID[0] = 1;
  ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));

  ASSERT_EQ(3, cd->getDbAuthCount());
  ASSERT_EQ(0, cdbase::get_authkey_cache_size());

  cd->setCacheSizeLimit(5);

  ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));
  ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));
  ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));

  ASSERT_EQ(4, cd->getDbAuthCount());
  ASSERT_EQ(1, cdbase::get_authkey_cache_size());

  GUID[0] = 2;

  ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));
  ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));
  ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));

  ASSERT_EQ(5, cd->getDbAuthCount());
  ASSERT_EQ(2, cdbase::get_authkey_cache_size());

  Email[0] = '@';

  ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));
  ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));
  ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));

  ASSERT_EQ(6, cd->getDbAuthCount());
  ASSERT_EQ(3, cdbase::get_authkey_cache_size());

  for (int b = 0; b < 10; b++) {
    Email[b] = '@';
    for (int a = 0; a < 200; a++) {
      AuthKey[0] = 1 + a;

      ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));
      ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));
      ASSERT_TRUE(cd->authkey_auth(GUID, Email, AuthKey));

      ASSERT_EQ(7 + b * 200 + a, cd->getDbAuthCount());
      ASSERT_EQ(a > 1 || b > 0 ? 5 : 4 + a, cdbase::get_authkey_cache_size());
    }
  }

  delete cd;

  cdbase::cdbase_free();
}

TEST_F(CDBaseTest, authkey_cache_null_test) {
  cdbase::init();

  CDBaseMock *cd = new CDBaseMock(NULL);

  char GUID[SUPLA_GUID_SIZE];
  char Email[SUPLA_EMAIL_MAXSIZE];
  char AuthKey[SUPLA_AUTHKEY_SIZE];

  memset(GUID, 0, SUPLA_GUID_SIZE);
  memset(Email, 0, SUPLA_EMAIL_MAXSIZE);
  memset(AuthKey, 0, SUPLA_AUTHKEY_SIZE);

  cd->setCacheSizeLimit(5);

  ASSERT_FALSE(cd->authkey_auth(NULL, Email, AuthKey));

  ASSERT_EQ(0, cd->getDbAuthCount());
  ASSERT_EQ(0, cdbase::get_authkey_cache_size());

  ASSERT_FALSE(cd->authkey_auth(GUID, NULL, AuthKey));

  ASSERT_EQ(0, cd->getDbAuthCount());
  ASSERT_EQ(0, cdbase::get_authkey_cache_size());

  ASSERT_FALSE(cd->authkey_auth(GUID, Email, NULL));

  ASSERT_EQ(0, cd->getDbAuthCount());
  ASSERT_EQ(0, cdbase::get_authkey_cache_size());

  ASSERT_FALSE(cd->authkey_auth(GUID, Email, AuthKey));

  ASSERT_EQ(1, cd->getDbAuthCount());
  ASSERT_EQ(1, cdbase::get_authkey_cache_size());

  delete cd;

  cdbase::cdbase_free();
}

TEST_F(CDBaseTest, guid_setter_getter) {
  char v[SUPLA_GUID_SIZE];
  char v0[SUPLA_GUID_SIZE];
  char v1[SUPLA_GUID_SIZE];

  memset(v, 0, SUPLA_GUID_SIZE);
  memset(v0, 0, SUPLA_GUID_SIZE);
  memset(v1, 1, SUPLA_GUID_SIZE);

  cdbase::init();
  CDBaseMock *cd = new CDBaseMock(NULL);

  cd->get_guid(v);
  ASSERT_EQ(0, memcmp(v, v0, SUPLA_GUID_SIZE));

  cd->set_guid(v1);
  cd->get_guid(v);
  ASSERT_EQ(0, memcmp(v, v1, SUPLA_GUID_SIZE));

  cd->get_authkey(v);
  ASSERT_EQ(0, memcmp(v, v0, SUPLA_AUTHKEY_SIZE));

  delete cd;
  cdbase::cdbase_free();
}

TEST_F(CDBaseTest, authkey_setter_getter) {
  char v[SUPLA_AUTHKEY_SIZE];
  char v0[SUPLA_AUTHKEY_SIZE];
  char v1[SUPLA_AUTHKEY_SIZE];

  memset(v, 0, SUPLA_AUTHKEY_SIZE);
  memset(v0, 0, SUPLA_AUTHKEY_SIZE);
  memset(v1, 1, SUPLA_AUTHKEY_SIZE);

  cdbase::init();
  CDBaseMock *cd = new CDBaseMock(NULL);

  cd->get_authkey(v);
  ASSERT_EQ(0, memcmp(v, v0, SUPLA_AUTHKEY_SIZE));

  cd->set_authkey(v1);
  cd->get_authkey(v);
  ASSERT_EQ(0, memcmp(v, v1, SUPLA_AUTHKEY_SIZE));

  cd->get_guid(v);
  ASSERT_EQ(0, memcmp(v, v0, SUPLA_GUID_SIZE));

  delete cd;
  cdbase::cdbase_free();
}

}  // namespace
