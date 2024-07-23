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

#include "OcrConfigTest.h"

#include "TestHelper.h"
#include "jsonconfig/channel/ocr_config.h"
#include "proto.h"
#include "svrcfg.h"

namespace testing {

OcrConfigTest::OcrConfigTest(void) {}

OcrConfigTest::~OcrConfigTest(void) {}

TEST_F(OcrConfigTest, host) {
  ocr_config cfg;
  TChannelConfig_OCR raw = {};
  cfg.get_config(&raw);

  EXPECT_STREQ(raw.Host, scfg_string(CFG_OCR_HOST));
}

TEST_F(OcrConfigTest, interval) {
  ocr_config cfg;
  TChannelConfig_OCR raw = {};
  cfg.get_config(&raw);

  EXPECT_EQ(raw.PhotoIntervalSec, 0);
  EXPECT_GE(scfg_int(CFG_OCR_MIN_INTERVAL_SEC), 1);

  raw.PhotoIntervalSec = scfg_int(CFG_OCR_MIN_INTERVAL_SEC) - 1;
  cfg.set_config(&raw);
  cfg.get_config(&raw);

  EXPECT_EQ(raw.PhotoIntervalSec, scfg_int(CFG_OCR_MIN_INTERVAL_SEC));
}

TEST_F(OcrConfigTest, getDefault) {
  ocr_config cfg;
  TChannelConfig_OCR raw1 = {};
  TChannelConfig_OCR raw2 = {};

  EXPECT_FALSE(cfg.get_config(&raw1));
  memcpy(raw2.Host, raw1.Host, sizeof(raw1.Host));

  EXPECT_EQ(memcmp(&raw1, &raw2, sizeof(TChannelConfig_OCR)), 0);
}

TEST_F(OcrConfigTest, authKey) {
  ocr_config cfg;
  TChannelConfig_OCR raw1 = {};
  TChannelConfig_OCR raw2 = {};
  TChannelConfig_OCR raw3 = {};

  cfg.set_config(&raw1);
  EXPECT_TRUE(cfg.get_config(&raw2));
  cfg.get_config(&raw3);

  EXPECT_NE(memcmp(raw1.AuthKey, raw2.AuthKey, sizeof(raw2.AuthKey)), 0);
  EXPECT_EQ(memcmp(raw2.AuthKey, raw3.AuthKey, sizeof(raw3.AuthKey)), 0);
}

TEST_F(OcrConfigTest, setAndGet) {
  ocr_config cfg;
  TChannelConfig_OCR raw1 = {};
  raw1.AvailableLightingModes = OCR_LIGHTING_MODE_OFF |
                                OCR_LIGHTING_MODE_ALWAYS_ON |
                                OCR_LIGHTING_MODE_AUTO;
  raw1.LightingMode = OCR_LIGHTING_MODE_ALWAYS_ON;
  raw1.LightingLevel = 30;
  raw1.PhotoIntervalSec = 1;
  cfg.set_config(&raw1);

  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"ocr\":{\"photoIntervalSec\":60,\"lightingMode\":\"ALWAYS_"
               "ON\",\"lightingLevel\":30}}");
  free(str);

  str = cfg.get_properties();
  ASSERT_TRUE(str != nullptr);
  cJSON *json = cJSON_Parse(str);
  EXPECT_TRUE(json != nullptr);

  if (json) {
    cJSON *ocr = cJSON_GetObjectItem(json, "ocr");
    EXPECT_TRUE(ocr != nullptr);
    if (ocr) {
      EXPECT_EQ(cJSON_GetArraySize(ocr), 2);
      cJSON *authkey = cJSON_GetObjectItem(ocr, "authKey");
      EXPECT_TRUE(authkey != nullptr);
      if (authkey) {
        char *str = cJSON_GetStringValue(authkey);
        EXPECT_EQ(strnlen(str, SUPLA_OCR_AUTHKEY_SIZE + 1),
                  SUPLA_OCR_AUTHKEY_SIZE - 1);
      }

      cJSON_DetachItemViaPointer(ocr, authkey);
      cJSON_Delete(authkey);
    }

    char *str = cJSON_PrintUnformatted(json);
    EXPECT_TRUE(str != nullptr);
    if (str) {
      EXPECT_STREQ(str,
                   "{\"ocr\":{\"availableLightingModes\":[\"OFF\",\"ALWAYS_"
                   "ON\",\"AUTO\"]}}");
      free(str);
    }

    cJSON_Delete(json);
  }
  free(str);
}

TEST_F(OcrConfigTest, merge) {
  ocr_config cfg1, cfg2;

  cfg1.set_user_config(
      "{\"yxyz\":123,\"ocr\":{\"ab\":true,\"photoIntervalSec\":90,"
      "\"lightingMode\":\"OFF\"}}");
  cfg1.set_properties(
      "{\"ocr\":{\"A\":\"B\",\"availableLightingModes\":[\"OFF\",\"ALWAYS_ON\","
      "\"AUTO\"]},\"aa\":\"bb\"}");

  cfg2.set_user_config(
      "{\"ocr\":{\"photoIntervalSec\":61,\"lightingMode\":\"ALWAYS_ON\","
      "\"lightingLevel\":15}}");
  cfg2.set_properties(
      "{\"ocr\":{\"authKey\":\"123456789\",\"availableLightingModes\":[\"OFF\","
      "\"AUTO\"]}}");

  cfg2.merge(&cfg1);

  char *str = cfg1.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"yxyz\":123,\"ocr\":{\"ab\":true,\"photoIntervalSec\":61,"
               "\"lightingMode\":\"ALWAYS_ON\",\"lightingLevel\":15}}");
  free(str);

  str = cfg1.get_properties();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"ocr\":{\"A\":\"B\",\"availableLightingModes\":[\"OFF\","
               "\"AUTO\"],\"authKey\":\"123456789\"},\"aa\":\"bb\"}");
  free(str);

  cfg2.set_properties(
      "{\"ocr\":{\"authKey\":\"abcdefg\",\"availableLightingModes\":[\"OFF\","
      "\"AUTO\",\"ALWAYS_ON\"]}}");

  cfg2.merge(&cfg1);

  str = cfg1.get_properties();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(
      str,
      "{\"ocr\":{\"A\":\"B\",\"availableLightingModes\":[\"OFF\",\"AUTO\","
      "\"ALWAYS_ON\"],\"authKey\":\"123456789\"},\"aa\":\"bb\"}");
  free(str);
}

} /* namespace testing */
