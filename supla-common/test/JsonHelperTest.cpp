// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "JsonHelperTest.h"

#include <map>
#include <string>
#include <time.h>

#include "gtest/gtest.h"  // NOLINT
#include "helper/json_helper.h"  // NOLINT

namespace {

class TestJsonHelper : public supla_json_helper {
 public:
  using supla_json_helper::equal_ci;
  using supla_json_helper::merge;
  using supla_json_helper::set_item_value;
};

class JsonHelperTest : public ::testing::Test {
 protected:
  TestJsonHelper helper;
};

TEST_F(JsonHelperTest, compares_strings_case_insensitively) {
  cJSON *object = cJSON_Parse("{\"text\":\"TeSt\",\"number\":1}");
  ASSERT_NE(nullptr, object);

  EXPECT_TRUE(helper.equal_ci("TeSt", "test"));
  EXPECT_FALSE(helper.equal_ci("test", "testing"));
  EXPECT_FALSE(
      helper.equal_ci(static_cast<const char *>(nullptr), "test"));
  EXPECT_TRUE(helper.equal_ci(cJSON_GetObjectItem(object, "text"), "test"));
  EXPECT_FALSE(
      helper.equal_ci(cJSON_GetObjectItem(object, "number"), "test"));
  EXPECT_FALSE(helper.equal_ci(static_cast<cJSON *>(nullptr), "test"));

  cJSON_Delete(object);
}

TEST_F(JsonHelperTest, reads_scalar_values) {
  cJSON *object = cJSON_Parse(
      "{\"integer\":42,\"boolean\":true,\"number\":1.5,"
      "\"text\":\"value\"}");
  ASSERT_NE(nullptr, object);

  int integer = 0;
  bool boolean = false;
  double number = 0;
  std::string text;

  EXPECT_TRUE(supla_json_helper::get_int(object, "integer", &integer));
  EXPECT_EQ(42, integer);
  EXPECT_TRUE(supla_json_helper::get_bool(object, "boolean", &boolean));
  EXPECT_TRUE(boolean);
  EXPECT_TRUE(supla_json_helper::get_double(object, "number", &number));
  EXPECT_DOUBLE_EQ(1.5, number);
  EXPECT_TRUE(supla_json_helper::get_string(object, "text", &text));
  EXPECT_EQ("value", text);

  cJSON_Delete(object);
}

TEST_F(JsonHelperTest, rejects_missing_or_wrong_scalar_values) {
  cJSON *object = cJSON_Parse("{\"integer\":\"42\",\"boolean\":1}");
  ASSERT_NE(nullptr, object);

  int integer = 7;
  bool boolean = true;
  double number = 7;
  std::string text = "value";

  EXPECT_FALSE(supla_json_helper::get_int(object, "integer", &integer));
  EXPECT_EQ(0, integer);
  EXPECT_FALSE(supla_json_helper::get_bool(object, "boolean", &boolean));
  EXPECT_FALSE(boolean);
  EXPECT_FALSE(supla_json_helper::get_double(object, "missing", &number));
  EXPECT_EQ(0, number);
  EXPECT_FALSE(supla_json_helper::get_string(object, "missing", &text));
  EXPECT_EQ("value", text);

  cJSON_Delete(object);
}

TEST_F(JsonHelperTest, sets_and_updates_values) {
  cJSON *object = cJSON_CreateObject();
  ASSERT_NE(nullptr, object);

  EXPECT_EQ(nullptr, helper.set_item_value(object, "missing", cJSON_Number,
                                           false, nullptr, nullptr, 1.5));
  EXPECT_EQ(nullptr, cJSON_GetObjectItem(object, "missing"));

  cJSON *number = helper.set_item_value(object, "number", cJSON_Number, true,
                                        nullptr, nullptr, 1.5);
  ASSERT_NE(nullptr, number);
  EXPECT_DOUBLE_EQ(1.5, cJSON_GetNumberValue(number));
  EXPECT_EQ(number, helper.set_item_value(object, "number", cJSON_Number,
                                           false, nullptr, nullptr, 2.5));
  EXPECT_DOUBLE_EQ(
      2.5, cJSON_GetNumberValue(cJSON_GetObjectItem(object, "number")));

  cJSON *string = helper.set_item_value(object, "text", cJSON_String, true,
                                        nullptr, "old", 0);
  ASSERT_NE(nullptr, string);
  EXPECT_STREQ("old", cJSON_GetStringValue(string));
  EXPECT_EQ(string, helper.set_item_value(object, "text", cJSON_String,
                                           false, nullptr, "new", 0));
  EXPECT_STREQ(
      "new", cJSON_GetStringValue(cJSON_GetObjectItem(object, "text")));

  EXPECT_EQ(nullptr, helper.set_item_value(object, "number", cJSON_String,
                                           false, nullptr, "wrong", 0));
  EXPECT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(object, "number")));

  cJSON *enabled = helper.set_item_value(object, "enabled", cJSON_True, true,
                                         nullptr, nullptr, 0);
  cJSON *disabled = helper.set_item_value(object, "disabled", cJSON_False,
                                          true, nullptr, nullptr, 0);
  ASSERT_NE(nullptr, enabled);
  ASSERT_NE(nullptr, disabled);
  EXPECT_TRUE(cJSON_IsTrue(enabled));
  EXPECT_TRUE(cJSON_IsFalse(disabled));

  cJSON *empty = helper.set_item_value(object, "empty", cJSON_NULL, true,
                                       nullptr, nullptr, 0);
  ASSERT_NE(nullptr, empty);
  EXPECT_TRUE(cJSON_IsNull(empty));

  EXPECT_EQ(nullptr, helper.set_item_value(object, "invalid", 999, true,
                                           nullptr, nullptr, 0));

  cJSON_Delete(object);
}

TEST_F(JsonHelperTest, replaces_existing_value_or_object_when_forced) {
  cJSON *object = cJSON_Parse("{\"value\":42,\"nested\":{\"old\":true}}");
  ASSERT_NE(nullptr, object);

  cJSON *string = helper.set_item_value(object, "value", cJSON_String, true,
                                        nullptr, "replacement", 0);
  ASSERT_NE(nullptr, string);
  EXPECT_TRUE(cJSON_IsString(cJSON_GetObjectItem(object, "value")));
  EXPECT_STREQ("replacement", cJSON_GetStringValue(string));

  cJSON *new_nested = cJSON_CreateObject();
  ASSERT_NE(nullptr, new_nested);
  ASSERT_EQ(new_nested, helper.set_item_value(object, "new_nested",
                                              cJSON_Object, true, new_nested,
                                              nullptr, 0));

  cJSON *replacement = cJSON_CreateObject();
  ASSERT_NE(nullptr, replacement);
  ASSERT_NE(nullptr, helper.set_item_value(object, "nested", cJSON_Object,
                                            true, replacement, nullptr, 0));
  EXPECT_EQ(replacement, cJSON_GetObjectItem(object, "nested"));
  EXPECT_EQ(nullptr, cJSON_GetObjectItem(replacement, "old"));

  cJSON_Delete(object);
}

TEST_F(JsonHelperTest, merges_mapped_values_and_optionally_deletes_them) {
  cJSON *source = cJSON_Parse(
      "{\"first\":1,\"nested\":{\"enabled\":true}}");
  cJSON *destination = cJSON_Parse(
      "{\"first\":0,\"nested\":{\"enabled\":false},"
      "\"removed\":true,\"unmapped\":\"keep\"}");
  ASSERT_NE(nullptr, source);
  ASSERT_NE(nullptr, destination);

  std::map<unsigned _supla_int16_t, std::string> fields;
  fields[1] = "first";
  fields[2] = "nested";
  fields[3] = "removed";

  EXPECT_TRUE(helper.merge(source, destination, fields, true));
  EXPECT_EQ(1, cJSON_GetObjectItem(destination, "first")->valueint);
  EXPECT_TRUE(cJSON_IsTrue(cJSON_GetObjectItem(
      cJSON_GetObjectItem(destination, "nested"), "enabled")));
  EXPECT_EQ(nullptr, cJSON_GetObjectItem(destination, "removed"));
  EXPECT_STREQ("keep", cJSON_GetStringValue(
                           cJSON_GetObjectItem(destination, "unmapped")));
  EXPECT_FALSE(helper.merge(source, destination, fields, true));

  cJSON_Delete(source);
  cJSON_Delete(destination);
}

TEST_F(JsonHelperTest, keeps_destination_values_when_deletion_is_disabled) {
  cJSON *source = cJSON_Parse("{\"value\":1}");
  cJSON *destination = cJSON_Parse("{\"value\":0,\"other\":2}");
  ASSERT_NE(nullptr, source);
  ASSERT_NE(nullptr, destination);

  std::map<unsigned _supla_int16_t, std::string> fields;
  fields[1] = "value";
  fields[2] = "other";

  EXPECT_TRUE(helper.merge(source, destination, fields, false));
  EXPECT_EQ(1, cJSON_GetObjectItem(destination, "value")->valueint);
  EXPECT_EQ(2, cJSON_GetObjectItem(destination, "other")->valueint);

  cJSON_Delete(source);
  cJSON_Delete(destination);
}

TEST_F(JsonHelperTest, reads_strict_zulu_timestamp_and_null) {
  cJSON *object = cJSON_Parse(
      "{\"timestamp\":\"1970-01-01T00:01:40Z\",\"empty\":null}");
  ASSERT_NE(nullptr, object);

  time_t timestamp = 0;
  EXPECT_TRUE(supla_json_helper::get_zulu_time_from_object(
      object, "timestamp", &timestamp));
  EXPECT_EQ(100, timestamp);
  EXPECT_TRUE(supla_json_helper::get_zulu_time_from_object(
      object, "empty", &timestamp, true));
  EXPECT_EQ(0, timestamp);
  EXPECT_FALSE(supla_json_helper::get_zulu_time_from_object(
      object, "empty", &timestamp));

  cJSON_Delete(object);
}

TEST_F(JsonHelperTest, rejects_invalid_zulu_timestamp) {
  cJSON *object = cJSON_Parse(
      "{\"invalid_day\":\"2026-02-30T00:00:00Z\","
      "\"invalid_format\":\"2026-01-01 00:00:00Z\"}");
  ASSERT_NE(nullptr, object);

  time_t timestamp = 123;
  EXPECT_FALSE(supla_json_helper::get_zulu_time_from_object(
      object, "invalid_day", &timestamp));
  EXPECT_FALSE(supla_json_helper::get_zulu_time_from_object(
      object, "invalid_format", &timestamp));

  cJSON_Delete(object);
}

TEST_F(JsonHelperTest, writes_zulu_timestamp_and_null) {
  cJSON *object = cJSON_CreateObject();
  ASSERT_NE(nullptr, object);

  ASSERT_NE(nullptr, supla_json_helper::add_zulu_time_to_object(
                          object, "timestamp", 100));
  ASSERT_NE(nullptr, supla_json_helper::add_zulu_time_to_object(
                          object, "empty", 0));

  cJSON *timestamp = cJSON_GetObjectItem(object, "timestamp");
  cJSON *empty = cJSON_GetObjectItem(object, "empty");
  ASSERT_NE(nullptr, timestamp);
  ASSERT_NE(nullptr, empty);
  EXPECT_STREQ("1970-01-01T00:01:40Z", cJSON_GetStringValue(timestamp));
  EXPECT_TRUE(cJSON_IsNull(empty));

  cJSON_Delete(object);
}

}  // namespace
