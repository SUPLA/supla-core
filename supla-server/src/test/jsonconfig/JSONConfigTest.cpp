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

#include "JSONConfigTest.h"

#include "doubles/jsonconfig/JSONConfigStub.h"

namespace testing {

JSONConfigTest::JSONConfigTest(void) {}

JSONConfigTest::~JSONConfigTest(void) {}

TEST_F(JSONConfigTest, setItemValue_NullOrEmptyName) {
  JSONConfigStub config;
  EXPECT_TRUE(nullptr == config.set_item_value(nullptr, "abcd", cJSON_Number,
                                               true, nullptr, 0));

  EXPECT_TRUE(nullptr == config.set_item_value(config.get_user_root(), "",
                                               cJSON_Number, true, nullptr, 0));
  char *json = config.get_user_config();
  ASSERT_TRUE(json != nullptr);
  EXPECT_STREQ(json, "{}");
  free(json);
}

TEST_F(JSONConfigTest, setItemValue_ItemNotExists_NoForce) {
  JSONConfigStub config;
  EXPECT_TRUE(nullptr == config.set_item_value(config.get_user_root(), "abcd",
                                               cJSON_Number, false, nullptr,
                                               0));

  char *json = config.get_user_config();
  ASSERT_TRUE(json != nullptr);
  EXPECT_STREQ(json, "{}");
  free(json);
}

TEST_F(JSONConfigTest, setItemValue_ItemNotExists_WithForce) {
  JSONConfigStub config;
  cJSON *item1 = config.set_item_value(config.get_user_root(), "1",
                                       cJSON_Number, true, nullptr, 15);

  cJSON *item2 = config.set_item_value(config.get_user_root(), "2",
                                       cJSON_String, true, "xyz", 0);

  cJSON *item3 = config.set_item_value(config.get_user_root(), "3", cJSON_True,
                                       true, nullptr, 0);

  cJSON *item4 = config.set_item_value(config.get_user_root(), "4", cJSON_False,
                                       true, nullptr, 0);

  cJSON *item5 = config.set_item_value(config.get_user_root(), "5", cJSON_NULL,
                                       true, nullptr, 0);

  EXPECT_TRUE(item1 != nullptr);
  EXPECT_TRUE(item2 != nullptr);
  EXPECT_TRUE(item3 != nullptr);
  EXPECT_TRUE(item4 != nullptr);
  EXPECT_TRUE(item5 != nullptr);

  char *json = config.get_user_config();
  ASSERT_TRUE(json != nullptr);
  EXPECT_STREQ(json,
               "{\"1\":15,\"2\":\"xyz\",\"3\":true,\"4\":false,\"5\":null}");
  free(json);
}

TEST_F(JSONConfigTest, setItemValue_ItemExistsWithDifferentType_NoForce) {
  JSONConfigStub config;
  config.set_user_config("{\"a\":123}");
  EXPECT_TRUE(nullptr == config.set_item_value(config.get_user_root(), "a",
                                               cJSON_String, false, "qwerty",
                                               0));
}

TEST_F(JSONConfigTest, setItemValue_ItemExistsWithDifferentType_Force) {
  JSONConfigStub config;
  config.set_user_config("{\"a\":123}");
  EXPECT_TRUE(nullptr != config.set_item_value(config.get_user_root(), "a",
                                               cJSON_String, true, "qwerty",
                                               0));

  char *json = config.get_user_config();
  ASSERT_TRUE(json != nullptr);
  EXPECT_STREQ(json, "{\"a\":\"qwerty\"}");
  free(json);
}

TEST_F(JSONConfigTest, setItemValue_ItemExistsWithSameType_NoForce) {
  JSONConfigStub config;

  config.set_user_config("{\"a\":123}");
  EXPECT_TRUE(nullptr != config.set_item_value(config.get_user_root(), "a",
                                               cJSON_Number, false, nullptr,
                                               55));

  char *json = config.get_user_config();
  ASSERT_TRUE(json != nullptr);
  EXPECT_STREQ(json, "{\"a\":55}");
  free(json);

  config.set_user_config("{\"a\":\"xyz\"}");

  EXPECT_TRUE(nullptr != config.set_item_value(config.get_user_root(), "a",
                                               cJSON_String, false, "uiop",
                                               55));
  json = config.get_user_config();
  ASSERT_TRUE(json != nullptr);
  EXPECT_STREQ(json, "{\"a\":\"uiop\"}");
  free(json);

  config.set_user_config("{\"a\":true}");

  EXPECT_TRUE(nullptr != config.set_item_value(config.get_user_root(), "a",
                                               cJSON_False, false, nullptr,
                                               55));

  json = config.get_user_config();
  ASSERT_TRUE(json != nullptr);
  EXPECT_STREQ(json, "{\"a\":false}");
  free(json);

  EXPECT_TRUE(nullptr != config.set_item_value(config.get_user_root(), "a",
                                               cJSON_True, false, nullptr, 55));

  json = config.get_user_config();
  ASSERT_TRUE(json != nullptr);
  EXPECT_STREQ(json, "{\"a\":true}");
  free(json);
}

} /* namespace testing */
