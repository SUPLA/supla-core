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

#include "ToolsTest.h"
#include "gtest/gtest.h"  // NOLINT
#include "tools.h"        // NOLINT

namespace {

class ToolsTest : public ::testing::Test {
 protected:
};

TEST_F(ToolsTest, st_file_exists) {
  ASSERT_EQ(1, st_file_exists("/dev/null"));
  ASSERT_EQ(0, st_file_exists(NULL));
}

TEST_F(ToolsTest, pid_file) {
  char file[] = "/tmp/QDZgKvbTrNh8.pid";

  st_delpidfile(file);

  ASSERT_EQ(0, st_file_exists(file));
  st_setpidfile(file);
  ASSERT_EQ(1, st_file_exists(file));

  char str1[32], str2[32];
  memset(str2, 0, 32);
  snprintf(str1, sizeof(str1), "%i\n", getpid());

  FILE *F = fopen(file, "r");
  if (F) {
    fread(str2, 1, 32, F);
    fclose(F);
  }

  size_t len = strnlen(str1, 32);

  ASSERT_EQ(len, strnlen(str2, 32));
  ASSERT_EQ(0, memcmp(str1, str2, len));

  st_delpidfile(file);
  ASSERT_EQ(0, st_file_exists(file));
}

TEST_F(ToolsTest, st_bin2hex) {
  char src[2] = {(char)255, (char)255};
  char dest[5];

  ASSERT_STREQ("FFFF", st_bin2hex(dest, src, 2));
  ASSERT_EQ(NULL, st_bin2hex(NULL, NULL, 0));
}

TEST_F(ToolsTest, st_guid2hex) {
  ASSERT_EQ(16, SUPLA_GUID_SIZE);
  ASSERT_EQ(33, SUPLA_GUID_HEXSIZE);

  const char guid[SUPLA_GUID_SIZE] = {
      (char)255, (char)255, (char)255, (char)255, (char)0,   (char)0,
      (char)0,   (char)0,   (char)255, (char)255, (char)255, (char)255,
      (char)0,   (char)0,   (char)0,   (char)0};

  char result[SUPLA_GUID_HEXSIZE];
  st_guid2hex(result, guid);

  ASSERT_STREQ("FFFFFFFF00000000FFFFFFFF00000000", result);
}

TEST_F(ToolsTest, st_authkey2hex) {
  ASSERT_EQ(16, SUPLA_AUTHKEY_SIZE);
  ASSERT_EQ(33, SUPLA_AUTHKEY_HEXSIZE);

  const char guid[SUPLA_AUTHKEY_SIZE] = {
      (char)255, (char)255, (char)255, (char)255, (char)0,   (char)0,
      (char)0,   (char)0,   (char)255, (char)255, (char)255, (char)255,
      (char)0,   (char)0,   (char)0,   (char)0};

  char result[SUPLA_AUTHKEY_HEXSIZE];
  st_authkey2hex(result, guid);

  ASSERT_STREQ("FFFFFFFF00000000FFFFFFFF00000000", result);
}

TEST_F(ToolsTest, st_str2hex) {
  const char src[] = "SUPLA";
  char dest[15];

  ASSERT_STREQ("5355504C41", st_str2hex(dest, src, 5));
}

TEST_F(ToolsTest, st_read_randkey_from_file) {
  char file[] = "/tmp/t7oosvsTZS89";
  char key1[4];
  char key2[4];

  memset(key1, 0, 4);
  memset(key2, 0, 4);

  ASSERT_EQ(1, st_read_randkey_from_file(file, key1, 4, 1));
  ASSERT_EQ(1, st_file_exists(file));
  ASSERT_NE(0, memcmp(key1, key2, 4));
  unlink(file);
}

TEST_F(ToolsTest, st_read_guid_from_file) {
  char file[] = "/tmp/t7oosvsTZS88";
  char GUID1[SUPLA_GUID_SIZE];
  char GUID2[SUPLA_GUID_SIZE];

  memset(GUID1, 0, SUPLA_GUID_SIZE);
  memset(GUID1, 0, SUPLA_GUID_SIZE);

  ASSERT_EQ(1, st_read_guid_from_file(file, GUID1, 1));
  ASSERT_EQ(1, st_file_exists(file));
  ASSERT_NE(0, memcmp(GUID1, GUID2, SUPLA_GUID_SIZE));
  unlink(file);
}

TEST_F(ToolsTest, st_read_authkey_from_file) {
  char file[] = "/tmp/t7oosvsTZS87";
  char AUTHKEY1[SUPLA_AUTHKEY_SIZE];
  char AUTHKEY2[SUPLA_AUTHKEY_SIZE];

  memset(AUTHKEY1, 0, SUPLA_AUTHKEY_SIZE);
  memset(AUTHKEY1, 0, SUPLA_AUTHKEY_SIZE);

  ASSERT_EQ(1, st_read_authkey_from_file(file, AUTHKEY1, 1));
  ASSERT_EQ(1, st_file_exists(file));
  ASSERT_NE(0, memcmp(AUTHKEY1, AUTHKEY2, SUPLA_AUTHKEY_SIZE));
  unlink(file);
}

TEST_F(ToolsTest, st_bcrypt) {
  char AuthKey[SUPLA_AUTHKEY_SIZE];
  char *AuthKeyHashHEX = st_get_authkey_hash_hex(AuthKey);
  ASSERT_TRUE(AuthKeyHashHEX != NULL);
  ASSERT_EQ((size_t)120, strnlen(AuthKeyHashHEX, BCRYPT_HASH_MAXSIZE * 2));

  char hash[BCRYPT_HASH_MAXSIZE];
  st_bcrypt_crypt(AuthKeyHashHEX, hash, BCRYPT_HASH_MAXSIZE, 8);

  ASSERT_EQ((size_t)60, strnlen(hash, BCRYPT_HASH_MAXSIZE));
  ASSERT_EQ(1, st_bcrypt_check(AuthKeyHashHEX, hash, 60));

  hash[0] = hash[0] == 'A' ? 'B' : 'A';

  ASSERT_NE(1, st_bcrypt_check(AuthKeyHashHEX, hash, 60));
  free(AuthKeyHashHEX);
}
}  // namespace
