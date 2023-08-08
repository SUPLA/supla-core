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

#include "DeviceConfigTest.h"

#include "jsonconfig/device/device_json_config.h"
#include "proto.h"

namespace testing {

DeviceConfigTest::DeviceConfigTest(void) {}

DeviceConfigTest::~DeviceConfigTest(void) {}

TEST_F(DeviceConfigTest, buttonVolume) {
  TSDS_SetDeviceConfig sds_cfg = {};
  sds_cfg.Fields = SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME;
  sds_cfg.ConfigSize = sizeof(TDeviceConfig_ButtonVolume);
  ((TDeviceConfig_ButtonVolume *)sds_cfg.Config)->Volume = 25;

  device_json_config cfg;
  cfg.set_config(&sds_cfg);
  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"buttonVolume\":25}");
  free(str);
}

TEST_F(DeviceConfigTest, screenBrightness) {
  TSDS_SetDeviceConfig sds_cfg = {};
  sds_cfg.Fields = SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS;
  sds_cfg.ConfigSize = sizeof(TDeviceConfig_ScreenBrightness);
  ((TDeviceConfig_ScreenBrightness *)sds_cfg.Config)->Automatic = 1;

  device_json_config cfg;
  cfg.set_config(&sds_cfg);
  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"screenBrightness\":\"auto\"}");
  free(str);

  ((TDeviceConfig_ScreenBrightness *)sds_cfg.Config)->Automatic = 0;
  ((TDeviceConfig_ScreenBrightness *)sds_cfg.Config)->ScreenBrightness = 98;
  cfg.set_config(&sds_cfg);

  str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"screenBrightness\":98}");
  free(str);
}

TEST_F(DeviceConfigTest, allFields) {
  TSDS_SetDeviceConfig sds_cfg = {};
  sds_cfg.Fields = SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED |
                   SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
                   SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME |
                   SUPLA_DEVICE_CONFIG_FIELD_DISABLE_LOCAL_CONFIG |
                   SUPLA_DEVICE_CONFIG_FIELD_TIMEZONE_OFFSET |
                   SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC |
                   SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY |
                   SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE;

  ((TDeviceConfig_StatusLed *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->StatusLedType = SUPLA_DEVCFG_STATUS_LED_ALWAYS_OFF;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_StatusLed);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_ScreenBrightness *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->ScreenBrightness = 24;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_ScreenBrightness);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_ButtonVolume *)&sds_cfg.Config[sds_cfg.ConfigSize])->Volume =
      100;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_ButtonVolume);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  sds_cfg.ConfigSize += sizeof(TDeviceConfig_DisableLocalConfig);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_TimezoneOffset *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->TimezoneOffsetMinutes = 555;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_TimezoneOffset);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_AutomaticTimeSync *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->AutomaticTimeSync = 1;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_AutomaticTimeSync);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_ScreensaverDelay *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->ScreensaverDelayMs = 123;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_ScreensaverDelay);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_ScreensaverMode *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->ScreensaverMode = SUPLA_DEVCFG_SCREENSAVER_MODE_MEASUREMENT;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_ScreensaverMode);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  device_json_config cfg;
  cfg.set_config(&sds_cfg);
  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"statusLed\":\"AlwaysOff\",\"screenBrightness\":24,"
               "\"buttonVolume\":100,\"localConfigDisabled\":false,"
               "\"timezoneOffset\":555,\"automaticTimeSync\":true,"
               "\"screenSaverDelay\":123,\"screenSaverMode\":\"Measurement\"}");
  free(str);
}

TEST_F(DeviceConfigTest, twoFieldsSlightlyApart) {
  TSDS_SetDeviceConfig sds_cfg = {};
  sds_cfg.Fields = SUPLA_DEVICE_CONFIG_FIELD_DISABLE_LOCAL_CONFIG |
                   SUPLA_DEVICE_CONFIG_FIELD_TIMEZONE_OFFSET |
                   SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY;

  ((TDeviceConfig_DisableLocalConfig *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->DisableLocalConfig = true;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_DisableLocalConfig);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_TimezoneOffset *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->TimezoneOffsetMinutes = 453;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_TimezoneOffset);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_ScreensaverDelay *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->ScreensaverDelayMs = 678;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_ScreensaverDelay);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  device_json_config cfg;
  cfg.set_config(&sds_cfg);
  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"localConfigDisabled\":true,\"timezoneOffset\":453,"
               "\"screenSaverDelay\":678}");
  free(str);
}

TEST_F(DeviceConfigTest, leaveOnly) {
  TSDS_SetDeviceConfig sds_cfg = {};
  sds_cfg.Fields = SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED |
                   SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
                   SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME;

  ((TDeviceConfig_StatusLed *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->StatusLedType = SUPLA_DEVCFG_STATUS_LED_ON_WHEN_CONNECTED;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_StatusLed);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_ScreenBrightness *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->ScreenBrightness = 100;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_ScreenBrightness);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_ButtonVolume *)&sds_cfg.Config[sds_cfg.ConfigSize])->Volume =
      0;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_ButtonVolume);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  device_json_config cfg;
  cfg.set_config(&sds_cfg);
  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"statusLed\":\"OnWhenConnected\",\"screenBrightness\":100,"
               "\"buttonVolume\":0}");
  free(str);

  cfg.leave_only_thise_fields(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS);

  str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"screenBrightness\":100}");
  free(str);

  cfg.set_user_config("{\"screenBrightness\":24,\"a\":false}");
  cfg.leave_only_thise_fields(0);

  str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"a\":false}");
  free(str);
}

TEST_F(DeviceConfigTest, removeFields) {
  TSDS_SetDeviceConfig sds_cfg = {};
  sds_cfg.Fields = SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED |
                   SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
                   SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME;

  ((TDeviceConfig_StatusLed *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->StatusLedType = SUPLA_DEVCFG_STATUS_LED_ON_WHEN_CONNECTED;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_StatusLed);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_ScreenBrightness *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->ScreenBrightness = 100;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_ScreenBrightness);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_ButtonVolume *)&sds_cfg.Config[sds_cfg.ConfigSize])->Volume =
      0;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_ButtonVolume);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  device_json_config cfg;
  cfg.set_config(&sds_cfg);
  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"statusLed\":\"OnWhenConnected\",\"screenBrightness\":100,"
               "\"buttonVolume\":0}");
  free(str);

  cfg.remove_fields(SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED |
                    SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS);

  str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"buttonVolume\":0}");
  free(str);
}

TEST_F(DeviceConfigTest, merge) {
  device_json_config cfg1, cfg2;
  cfg1.set_user_config("{\"statusLed\":2}");
  cfg2.set_user_config(
      "{\"a\":true,\"b\":123,\"statusLed\":1,\"buttonVolume\":100}");

  cfg1.merge(&cfg2);

  char *user_config = cfg2.get_user_config();
  ASSERT_NE(user_config, nullptr);

  EXPECT_STREQ(user_config,
               "{\"a\":true,\"b\":123,\"statusLed\":2,\"buttonVolume\":100}");

  free(user_config);
}

TEST_F(DeviceConfigTest, getConfig_AllFields) {
  TSDS_SetDeviceConfig sds_cfg = {};
  device_json_config cfg1, cfg2;

  const char user_config[] =
      "{\"statusLed\":\"AlwaysOff\",\"screenBrightness\":24,\"buttonVolume\":"
      "100,\"localConfigDisabled\":false,\"timezoneOffset\":555,"
      "\"automaticTimeSync\":true,\"screenSaverDelay\":123,\"screenSaverMode\":"
      "\"Measurement\"}";
  cfg1.set_user_config(user_config);

  unsigned _supla_int64_t fields_left = 0xFFFFFFFFFFFFFFFF;
  cfg1.get_config(&sds_cfg, &fields_left);

  EXPECT_EQ(fields_left, 0);
  EXPECT_EQ(sds_cfg.Fields, SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED |
                                SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
                                SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME |
                                SUPLA_DEVICE_CONFIG_FIELD_DISABLE_LOCAL_CONFIG |
                                SUPLA_DEVICE_CONFIG_FIELD_TIMEZONE_OFFSET |
                                SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC |
                                SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY |
                                SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE);

  cfg2.set_config(&sds_cfg);
  char *user_config2 = cfg2.get_user_config();
  ASSERT_NE(user_config2, nullptr);

  EXPECT_STREQ(user_config, user_config2);

  free(user_config2);
}

TEST_F(DeviceConfigTest, getConfig_AutoBrightness) {
  TSDS_SetDeviceConfig sds_cfg = {};
  device_json_config cfg1, cfg2;

  const char user_config[] = "{\"screenBrightness\":\"auto\"}";
  cfg1.set_user_config(user_config);

  unsigned _supla_int64_t fields_left = 0xFFFFFFFFFFFFFFFF;
  cfg1.get_config(&sds_cfg, &fields_left);

  EXPECT_EQ(fields_left, 0);

  EXPECT_EQ(sds_cfg.Fields, SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS);

  cfg2.set_config(&sds_cfg);
  char *user_config2 = cfg2.get_user_config();
  ASSERT_NE(user_config2, nullptr);

  EXPECT_STREQ(user_config, user_config2);

  free(user_config2);
}

TEST_F(DeviceConfigTest, getConfig_OneField) {
  TSDS_SetDeviceConfig sds_cfg = {};
  device_json_config cfg1, cfg2;

  cfg1.set_user_config(
      "{\"statusLed\":2,\"screenBrightness\":24,\"buttonVolume\":100,"
      "\"localConfigDisabled\":false,\"timezoneOffset\":555,"
      "\"automaticTimeSync\":true,\"screenSaverDelay\":123,"
      "\"screenSaverMode\":3}");

  unsigned _supla_int64_t fields_left = 0xFFFFFFFFFFFFFFFF;
  cfg1.get_config(&sds_cfg, SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC,
                  &fields_left);

  EXPECT_EQ(fields_left, 0);
  EXPECT_EQ(sds_cfg.Fields, SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC);

  cfg2.set_config(&sds_cfg);
  char *user_config = cfg2.get_user_config();
  ASSERT_NE(user_config, nullptr);

  EXPECT_STREQ(user_config, "{\"automaticTimeSync\":true}");

  free(user_config);
}

TEST_F(DeviceConfigTest, isLocalConfigDisabled) {
  device_json_config cfg;

  cfg.set_user_config("");
  EXPECT_FALSE(cfg.is_local_config_disabled());

  cfg.set_user_config("{\"localConfigDisabled\":false}");
  EXPECT_FALSE(cfg.is_local_config_disabled());

  cfg.set_user_config("{\"localConfigDisabled\":true}");
  EXPECT_TRUE(cfg.is_local_config_disabled());
}

TEST_F(DeviceConfigTest, statusLED) {
  TDeviceConfig_StatusLed status_led = {};
  device_json_config cfg1;
  device_json_config cfg2;
  cfg1.set_user_config("{\"statusLed\": \"AlwaysOff\"}");

  EXPECT_TRUE(cfg1.get_status_led(&status_led));
  EXPECT_EQ(status_led.StatusLedType, SUPLA_DEVCFG_STATUS_LED_ALWAYS_OFF);

  TSDS_SetDeviceConfig sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  char *str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"statusLed\":\"AlwaysOff\"}");
  free(str);

  cfg1.set_user_config("{\"statusLed\": \"OnWhenConnected\"}");

  EXPECT_TRUE(cfg1.get_status_led(&status_led));
  EXPECT_EQ(status_led.StatusLedType,
            SUPLA_DEVCFG_STATUS_LED_ON_WHEN_CONNECTED);

  sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"statusLed\":\"OnWhenConnected\"}");
  free(str);

  cfg1.set_user_config("{\"statusLed\": \"OffWhenConnected\"}");

  EXPECT_TRUE(cfg1.get_status_led(&status_led));
  EXPECT_EQ(status_led.StatusLedType,
            SUPLA_DEVCFG_STATUS_LED_OFF_WHEN_CONNECTED);

  sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"statusLed\":\"OffWhenConnected\"}");
  free(str);
}

TEST_F(DeviceConfigTest, screenSaverMode) {
  TDeviceConfig_ScreensaverMode mode = {};
  device_json_config cfg1;
  device_json_config cfg2;
  cfg1.set_user_config("{\"screenSaverMode\": \"Measurement\"}");

  EXPECT_TRUE(cfg1.get_screen_saver_mode(&mode));
  EXPECT_EQ(mode.ScreensaverMode, SUPLA_DEVCFG_SCREENSAVER_MODE_MEASUREMENT);

  TSDS_SetDeviceConfig sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  char *str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"screenSaverMode\":\"Measurement\"}");
  free(str);

  cfg1.set_user_config("{\"screenSaverMode\": \"Time\"}");

  EXPECT_TRUE(cfg1.get_screen_saver_mode(&mode));
  EXPECT_EQ(mode.ScreensaverMode, SUPLA_DEVCFG_SCREENSAVER_MODE_TIME);

  sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"screenSaverMode\":\"Time\"}");
  free(str);

  cfg1.set_user_config("{\"screenSaverMode\": \"All\"}");

  EXPECT_TRUE(cfg1.get_screen_saver_mode(&mode));
  EXPECT_EQ(mode.ScreensaverMode, SUPLA_DEVCFG_SCREENSAVER_MODE_ALL);

  sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"screenSaverMode\":\"All\"}");
  free(str);
}

TEST_F(DeviceConfigTest, availableFields) {
  device_json_config cfg;
  cfg.set_user_config("{\"screenSaverMode\": \"Measurement\"}");

  EXPECT_EQ(cfg.get_available_fields(),
            SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE);

  cfg.set_user_config(
      "{\"statusLed\":2,\"screenBrightness\":24,\"buttonVolume\":100,"
      "\"localConfigDisabled\":false,\"timezoneOffset\":555,"
      "\"automaticTimeSync\":true,\"screenSaverDelay\":123,"
      "\"screenSaverMode\":3}");

  EXPECT_EQ(cfg.get_available_fields(),
            SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED |
                SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
                SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME |
                SUPLA_DEVICE_CONFIG_FIELD_DISABLE_LOCAL_CONFIG |
                SUPLA_DEVICE_CONFIG_FIELD_TIMEZONE_OFFSET |
                SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC |
                SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY |
                SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE);
}

} /* namespace testing */