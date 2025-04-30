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

#include <string.h>

#include "TestHelper.h"
#include "jsonconfig/device/device_json_config.h"
#include "log.h"
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
  ((TDeviceConfig_ScreenBrightness *)sds_cfg.Config)->AdjustmentForAutomatic =
      2;

  device_json_config cfg;
  cfg.set_config(&sds_cfg);
  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"screenBrightness\":{\"level\":\"auto\",\"adjustment\":2}}");
  free(str);

  cfg.set_user_config(
      "{\"screenBrightness\":{\"level\":\"auto\",\"adjustment\":12}}");
  cfg.get_config(&sds_cfg, nullptr);

  EXPECT_EQ(((TDeviceConfig_ScreenBrightness *)sds_cfg.Config)->Automatic, 1);
  EXPECT_EQ(((TDeviceConfig_ScreenBrightness *)sds_cfg.Config)
                ->AdjustmentForAutomatic,
            12);
  EXPECT_EQ(
      ((TDeviceConfig_ScreenBrightness *)sds_cfg.Config)->ScreenBrightness, 0);

  ((TDeviceConfig_ScreenBrightness *)sds_cfg.Config)->Automatic = 0;
  ((TDeviceConfig_ScreenBrightness *)sds_cfg.Config)->ScreenBrightness = 98;
  cfg.set_config(&sds_cfg);

  str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"screenBrightness\":{\"level\":98}}");
  free(str);
}

TEST_F(DeviceConfigTest, allFields) {
  TSDS_SetDeviceConfig sds_cfg = {};
  sds_cfg.Fields = SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED |
                   SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED |
                   SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
                   SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME |
                   SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE |
                   SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC |
                   SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY |
                   SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT |
                   SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE |
                   SUPLA_DEVICE_CONFIG_FIELD_MODBUS;

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

  sds_cfg.ConfigSize += sizeof(TDeviceConfig_DisableUserInterface);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_AutomaticTimeSync *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->AutomaticTimeSync = 1;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_AutomaticTimeSync);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_HomeScreenOffDelay *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->HomeScreenOffDelayS = 123;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_HomeScreenOffDelay);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  size_t home_screen_content_offset = sds_cfg.ConfigSize;

  ((TDeviceConfig_HomeScreenContent *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->HomeScreenContent = SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TIME_DATE;
  ((TDeviceConfig_HomeScreenContent *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->ContentAvailable = 0xFFFFFFFFFFFFFFFF;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_HomeScreenContent);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  sds_cfg.ConfigSize += sizeof(TDeviceConfig_HomeScreenOffDelayType);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_PowerStatusLed *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->Disabled = 0;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_PowerStatusLed);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  TDeviceConfig_Modbus *modbus =
      (TDeviceConfig_Modbus *)&sds_cfg.Config[sds_cfg.ConfigSize];
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_Modbus);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  modbus->ModbusAddress = 123;
  modbus->Network.Mode = MODBUS_NETWORK_MODE_TCP;
  modbus->Network.Port = 88;
  modbus->Serial.Baudrate = 9600;
  modbus->Serial.Mode = MODBUS_SERIAL_MODE_RTU;
  modbus->Serial.StopBits = MODBUS_SERIAL_STOP_BITS_ONE;
  modbus->Role = MODBUS_ROLE_SLAVE;

  memset(&modbus->Properties, 0xFF, sizeof(modbus->Properties));
  memset(&modbus->Properties.Reserved, 0, sizeof(modbus->Properties.Reserved));
  modbus->Properties.Baudrate.Reserved = 0;
  modbus->Properties.Baudrate.Reserved2 = 0;
  modbus->Properties.Protocol.Reserved = 0;
  modbus->Properties.Protocol.Reserved2 = 0;
  modbus->Properties.StopBits.Reserved = 0;

  device_json_config cfg;
  cfg.set_config(&sds_cfg);
  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"statusLed\":\"ALWAYS_OFF\",\"screenBrightness\":{\"level\":"
               "24},\"buttonVolume\":100,\"userInterface\":{\"disabled\":false}"
               ",\"automaticTimeSync\":true,\"homeScreen\":{\"offDelay\":123,"
               "\"content\":\"TIME_DATE\",\"offDelayType\":\"ALWAYS_ENABLED\"},"
               "\"powerStatusLed\":\"ENABLED\",\"modbus\":{\"role\":\"SLAVE\","
               "\"modbusAddress\":123,\"slaveTimeoutMs\":0,\"serialConfig\":{"
               "\"mode\":\"RTU\",\"baudRate\":9600,\"stopBits\":\"ONE\"},"
               "\"networkConfig\":{\"mode\":\"TCP\",\"port\":88}}}");

  device_json_config cfg2;
  cfg2.set_user_config(str);

  free(str);

  str = cfg.get_properties();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(
      str,
      "{\"homeScreenContentAvailable\":[\"NONE\",\"TEMPERATURE\",\"TEMPERATURE_"
      "AND_HUMIDITY\",\"TIME\",\"TIME_DATE\",\"TEMPERATURE_TIME\",\"MAIN_AND_"
      "AUX_TEMPERATURE\",\"MODE_OR_TEMPERATURE\"],\"modbus\":{"
      "\"availableProtocols\":[\"MASTER\",\"SLAVE\",\"RTU\",\"ASCII\",\"TCP\","
      "\"UDP\"],\"availableBaudrates\":[4800,9600,19200,38400,57600,115200],"
      "\"availableStopbits\":[\"ONE\",\"TWO\",\"ONE_AND_HALF\"]}}");

  cfg2.set_properties(str);

  free(str);

  TSDS_SetDeviceConfig sds_cfg2 = {};
  cfg2.get_config(&sds_cfg2, nullptr);

  EXPECT_EQ(((TDeviceConfig_HomeScreenContent *)&sds_cfg2
                 .Config[home_screen_content_offset])
                ->ContentAvailable,
            SUPLA_DEVCFG_HOME_SCREEN_CONTENT_MODE_OR_TEMPERATURE |
                (SUPLA_DEVCFG_HOME_SCREEN_CONTENT_MODE_OR_TEMPERATURE - 1));

  ((TDeviceConfig_HomeScreenContent *)&sds_cfg2
       .Config[home_screen_content_offset])
      ->ContentAvailable = 0xFFFFFFFFFFFFFFFF;

  EXPECT_EQ(sds_cfg.Fields, sds_cfg2.Fields);
  EXPECT_EQ(memcmp(sds_cfg.Config, sds_cfg2.Config, sds_cfg.ConfigSize), 0);
}

TEST_F(DeviceConfigTest, twoFieldsSlightlyApart) {
  TSDS_SetDeviceConfig sds_cfg = {};
  sds_cfg.Fields = SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE |
                   SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC |
                   SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY;

  ((TDeviceConfig_DisableUserInterface *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->DisableUserInterface = 1;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_DisableUserInterface);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_AutomaticTimeSync *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->AutomaticTimeSync = 1;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_AutomaticTimeSync);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  ((TDeviceConfig_HomeScreenOffDelay *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->HomeScreenOffDelayS = 678;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_HomeScreenOffDelay);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  device_json_config cfg;
  cfg.set_config(&sds_cfg);
  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"userInterface\":{\"disabled\":true},\"automaticTimeSync\":"
               "true,\"homeScreen\":{\"offDelay\":678}}");
  free(str);
}

TEST_F(DeviceConfigTest, userInterfaceDisabled) {
  TSDS_SetDeviceConfig sds_cfg = {};
  sds_cfg.Fields = SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_DisableUserInterface);

  device_json_config cfg1, cfg2;
  cfg1.set_config(&sds_cfg);
  char *str = cfg1.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"userInterface\":{\"disabled\":false}}");
  free(str);

  sds_cfg = {};
  cfg1.get_config(&sds_cfg, nullptr);
  cfg2.set_config(&sds_cfg);
  str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"userInterface\":{\"disabled\":false}}");
  free(str);

  ((TDeviceConfig_DisableUserInterface *)&sds_cfg.Config)
      ->DisableUserInterface = 2;
  ((TDeviceConfig_DisableUserInterface *)&sds_cfg.Config)
      ->minAllowedTemperatureSetpointFromLocalUI = 123;
  ((TDeviceConfig_DisableUserInterface *)&sds_cfg.Config)
      ->maxAllowedTemperatureSetpointFromLocalUI = 345;

  cfg1.set_config(&sds_cfg);
  str = cfg1.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"userInterface\":{\"disabled\":\"partial\","
               "\"minAllowedTemperatureSetpointFromLocalUI\":123,"
               "\"maxAllowedTemperatureSetpointFromLocalUI\":345}}");
  free(str);

  sds_cfg = {};
  cfg1.get_config(&sds_cfg, nullptr);
  cfg2.set_config(&sds_cfg);
  str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"userInterface\":{\"disabled\":\"partial\","
               "\"minAllowedTemperatureSetpointFromLocalUI\":123,"
               "\"maxAllowedTemperatureSetpointFromLocalUI\":345}}");
  free(str);

  ((TDeviceConfig_DisableUserInterface *)&sds_cfg.Config)
      ->DisableUserInterface = 1;

  cfg1.set_config(&sds_cfg);
  str = cfg1.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"userInterface\":{\"disabled\":true}}");
  free(str);

  sds_cfg = {};
  cfg1.get_config(&sds_cfg, nullptr);
  cfg2.set_config(&sds_cfg);
  str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"userInterface\":{\"disabled\":true}}");
  free(str);
}

TEST_F(DeviceConfigTest, leaveOnly) {
  TSDS_SetDeviceConfig sds_cfg = {};
  sds_cfg.Fields = SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED |
                   SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
                   SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME |
                   SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY;

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

  ((TDeviceConfig_HomeScreenOffDelay *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->HomeScreenOffDelayS = 10;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_HomeScreenOffDelay);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  device_json_config cfg;
  cfg.set_config(&sds_cfg);
  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(
      str,
      "{\"statusLed\":\"ON_WHEN_CONNECTED\",\"screenBrightness\":{\"level\":"
      "100},\"buttonVolume\":0,\"homeScreen\":{\"offDelay\":10}}");
  free(str);

  cfg.leave_only_thise_fields(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS);

  str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"screenBrightness\":{\"level\":100}}");
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
                   SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME |
                   SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY;

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

  ((TDeviceConfig_HomeScreenOffDelay *)&sds_cfg.Config[sds_cfg.ConfigSize])
      ->HomeScreenOffDelayS = 15;
  sds_cfg.ConfigSize += sizeof(TDeviceConfig_HomeScreenOffDelay);
  ASSERT_LE(sds_cfg.ConfigSize, SUPLA_DEVICE_CONFIG_MAXSIZE);

  device_json_config cfg;
  cfg.set_config(&sds_cfg);
  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(
      str,
      "{\"statusLed\":\"ON_WHEN_CONNECTED\",\"screenBrightness\":{\"level\":"
      "100},\"buttonVolume\":0,\"homeScreen\":{\"offDelay\":15}}");
  free(str);

  cfg.remove_fields(SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED |
                    SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
                    SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY);

  str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"buttonVolume\":0}");
  free(str);
}

TEST_F(DeviceConfigTest, merge) {
  device_json_config cfg1, cfg2;
  cfg1.set_user_config("{\"statusLed\":2,\"homeScreen\":{\"offDelay\":15}}");
  cfg2.set_user_config(
      "{\"a\":true,\"b\":123,\"statusLed\":1,\"buttonVolume\":100}");

  cfg1.merge(&cfg2);

  char *user_config = cfg2.get_user_config();
  ASSERT_NE(user_config, nullptr);

  EXPECT_STREQ(user_config,
               "{\"a\":true,\"b\":123,\"statusLed\":2,\"buttonVolume\":100,"
               "\"homeScreen\":{\"offDelay\":15}}");

  free(user_config);
}

TEST_F(DeviceConfigTest, getConfig_AllFields) {
  TSDS_SetDeviceConfig sds_cfg = {};
  device_json_config cfg1, cfg2;

  const char user_config[] =
      "{\"statusLed\":\"ALWAYS_OFF\",\"screenBrightness\":{\"level\":24},"
      "\"buttonVolume\":100,\"userInterface\":{\"disabled\":true},"
      "\"automaticTimeSync\":true,\"homeScreen\":{\"offDelay\":123,\"content\":"
      "\"TIME_DATE\"},\"powerStatusLed\":\"ENABLED\"}";
  cfg1.set_user_config(user_config);

  const char properties[] =
      "{\"homeScreenContentAvailable\":[\"NONE\",\"TEMPERATURE\",\"MAIN_AND_"
      "AUX_TEMPERATURE\"]}";
  cfg1.set_user_config(user_config);
  cfg1.set_properties(properties);

  unsigned _supla_int64_t fields_left = 0xFFFFFFFFFFFFFFFF;
  cfg1.get_config(&sds_cfg, &fields_left);

  EXPECT_EQ(fields_left, 0);
  EXPECT_EQ(sds_cfg.Fields,
            SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED |
                SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED |
                SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
                SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME |
                SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE |
                SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC |
                SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY |
                SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT);

  cfg2.set_config(&sds_cfg);
  char *user_config2 = cfg2.get_user_config();
  ASSERT_NE(user_config2, nullptr);

  char *properties2 = cfg2.get_properties();
  ASSERT_NE(properties2, nullptr);

  EXPECT_STREQ(user_config, user_config2);
  EXPECT_STREQ(properties, properties2);

  free(user_config2);
  free(properties2);
}

TEST_F(DeviceConfigTest, getConfig_AutoBrightness) {
  TSDS_SetDeviceConfig sds_cfg = {};
  device_json_config cfg1, cfg2;

  const char user_config[] =
      "{\"screenBrightness\":{\"level\":\"auto\",\"adjustment\":33}}";
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
      "\"userInterfaceDisabled\":false,\"automaticTimeSync\":true,"
      "\"homeScreen\":{\"offDelay\":123,\"content\":\"TimeDate\"}}");

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

TEST_F(DeviceConfigTest, statusLED) {
  TDeviceConfig_StatusLed status_led = {};
  device_json_config cfg1;
  device_json_config cfg2;
  cfg1.set_user_config("{\"statusLed\": \"ALWAYS_OFF\"}");

  EXPECT_TRUE(cfg1.get_status_led(&status_led));
  EXPECT_EQ(status_led.StatusLedType, SUPLA_DEVCFG_STATUS_LED_ALWAYS_OFF);

  TSDS_SetDeviceConfig sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  char *str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"statusLed\":\"ALWAYS_OFF\"}");
  free(str);

  cfg1.set_user_config("{\"statusLed\": \"ON_WHEN_CONNECTED\"}");

  EXPECT_TRUE(cfg1.get_status_led(&status_led));
  EXPECT_EQ(status_led.StatusLedType,
            SUPLA_DEVCFG_STATUS_LED_ON_WHEN_CONNECTED);

  sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"statusLed\":\"ON_WHEN_CONNECTED\"}");
  free(str);

  cfg1.set_user_config("{\"statusLed\": \"OFF_WHEN_CONNECTED\"}");

  EXPECT_TRUE(cfg1.get_status_led(&status_led));
  EXPECT_EQ(status_led.StatusLedType,
            SUPLA_DEVCFG_STATUS_LED_OFF_WHEN_CONNECTED);

  sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"statusLed\":\"OFF_WHEN_CONNECTED\"}");
  free(str);
}

TEST_F(DeviceConfigTest, powerStatusLED) {
  TDeviceConfig_PowerStatusLed status_led = {};
  device_json_config cfg1;
  device_json_config cfg2;
  cfg1.set_user_config("{\"powerStatusLed\": \"DISABLED\"}");

  EXPECT_TRUE(cfg1.get_power_status_led(&status_led));
  EXPECT_EQ(status_led.Disabled, 1);

  TSDS_SetDeviceConfig sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  char *str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"powerStatusLed\":\"DISABLED\"}");
  free(str);

  cfg1.set_user_config("{\"powerStatusLed\": \"ENABLED\"}");

  EXPECT_TRUE(cfg1.get_power_status_led(&status_led));
  EXPECT_EQ(status_led.Disabled, 0);

  sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"powerStatusLed\":\"ENABLED\"}");
  free(str);
}

TEST_F(DeviceConfigTest, homeScreenMode) {
  TDeviceConfig_HomeScreenContent content = {};
  device_json_config cfg1;
  device_json_config cfg2;
  cfg1.set_user_config("{\"homeScreen\": {\"content\": \"TIME_DATE\"}}");

  EXPECT_TRUE(cfg1.get_home_screen_content(&content));
  EXPECT_EQ(content.HomeScreenContent,
            SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TIME_DATE);
  EXPECT_EQ(content.ContentAvailable, 0);

  TSDS_SetDeviceConfig sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  char *str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"homeScreen\":{\"content\":\"TIME_DATE\"}}");
  free(str);

  str = cfg2.get_properties();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"homeScreenContentAvailable\":[]}");
  free(str);

  cfg1.set_user_config(
      "{\"homeScreen\": {\"content\": \"TEMPERATURE_AND_HUMIDITY\"}}");

  EXPECT_TRUE(cfg1.get_home_screen_content(&content));
  EXPECT_EQ(content.HomeScreenContent,
            SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE_AND_HUMIDITY);

  sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"homeScreen\":{\"content\":\"TEMPERATURE_AND_HUMIDITY\"}}");
  free(str);

  cfg1.set_user_config(
      "{\"homeScreen\": {\"content\": \"MAIN_AND_AUX_TEMPERATURE\"}}");
  cfg1.set_properties(
      "{\"homeScreenContentAvailable\":[\"MAIN_AND_AUX_TEMPERATURE\", "
      "\"TEMPERATURE_AND_HUMIDITY\"]}");

  EXPECT_TRUE(cfg1.get_home_screen_content(&content));
  EXPECT_EQ(content.HomeScreenContent,
            SUPLA_DEVCFG_HOME_SCREEN_CONTENT_MAIN_AND_AUX_TEMPERATURE);
  EXPECT_EQ(content.ContentAvailable,
            SUPLA_DEVCFG_HOME_SCREEN_CONTENT_MAIN_AND_AUX_TEMPERATURE |
                SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE_AND_HUMIDITY);

  sds_config = {};
  cfg1.get_config(&sds_config, nullptr);
  cfg2.set_config(&sds_config);
  str = cfg2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"homeScreen\":{\"content\":\"MAIN_AND_AUX_TEMPERATURE\"}}");
  free(str);

  str = cfg2.get_properties();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"homeScreenContentAvailable\":[\"TEMPERATURE_AND_HUMIDITY\","
               "\"MAIN_AND_AUX_TEMPERATURE\"]}");
  free(str);
}

TEST_F(DeviceConfigTest, availableFields) {
  device_json_config cfg;
  cfg.set_user_config(
      "{\"homeScreen\": {\"content\": \"TEMPERATURE_AND_HUMIDITY\"}}");

  EXPECT_EQ(cfg.get_available_fields(),
            SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT);

  cfg.set_user_config(
      "{\"statusLed\":2,\"screenBrightness\":{\"level\":24},\"buttonVolume\":"
      "100,\"userInterface\":{\"disabled\":false},\"automaticTimeSync\":true,"
      "\"homeScreen\": {\"offDelay\":123,\"content\": "
      "\"TEMPERATURE_AND_HUMIDITY\"}}");

  EXPECT_EQ(cfg.get_available_fields(),
            SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED |
                SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
                SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME |
                SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE |
                SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC |
                SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY |
                SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT);
}

TEST_F(DeviceConfigTest, modbus_1) {
  TSDS_SetDeviceConfig sds_cfg1 = {};
  TSDS_SetDeviceConfig sds_cfg2 = {};
  sds_cfg1.Fields = SUPLA_DEVICE_CONFIG_FIELD_MODBUS;
  sds_cfg1.ConfigSize = sizeof(TDeviceConfig_Modbus);

  TDeviceConfig_Modbus *modbus = (TDeviceConfig_Modbus *)sds_cfg1.Config;

  modbus->ModbusAddress = 253;
  modbus->Role = MODBUS_ROLE_SLAVE;
  modbus->SlaveTimeoutMs = 543;
  modbus->Network.Mode = MODBUS_NETWORK_MODE_TCP;
  modbus->Network.Port = 23;
  modbus->Serial.Mode = MODBUS_SERIAL_MODE_ASCII;
  modbus->Serial.Baudrate = 115200;

  modbus->Properties.Baudrate.B9600 = 1;
  modbus->Properties.Baudrate.B38400 = 1;
  modbus->Properties.Baudrate.B115200 = 1;

  modbus->Properties.Protocol.Master = 1;
  modbus->Properties.Protocol.Rtu = 1;
  modbus->Properties.Protocol.Tcp = 1;

  modbus->Properties.StopBits.OneAndHalf = 1;

  device_json_config cfg1, cfg2;
  cfg1.set_config(&sds_cfg1);
  char *str = cfg1.get_user_config();
  ASSERT_TRUE(str != nullptr);

  cfg2.set_user_config(str);

  EXPECT_STREQ(str,
               "{\"modbus\":{\"role\":\"SLAVE\",\"modbusAddress\":253,"
               "\"slaveTimeoutMs\":543,\"serialConfig\":{\"mode\":\"ASCII\","
               "\"baudRate\":115200,\"stopBits\":\"ONE\"},\"networkConfig\":{"
               "\"mode\":\"TCP\",\"port\":23}}}");

  free(str);
  str = cfg1.get_properties();

  ASSERT_TRUE(str != nullptr);

  cfg2.set_properties(str);

  EXPECT_STREQ(str,
               "{\"modbus\":{\"availableProtocols\":[\"MASTER\",\"RTU\","
               "\"TCP\"],\"availableBaudrates\":[9600,38400,115200],"
               "\"availableStopbits\":[\"ONE_AND_HALF\"]}}");

  cfg2.get_config(&sds_cfg2, nullptr);

  free(str);

  EXPECT_EQ(sds_cfg1.Fields, sds_cfg2.Fields);
  EXPECT_EQ(memcmp(sds_cfg1.Config, sds_cfg2.Config, sds_cfg1.ConfigSize), 0);
}

TEST_F(DeviceConfigTest, modbus_2) {
  TSDS_SetDeviceConfig sds_cfg1 = {};
  TSDS_SetDeviceConfig sds_cfg2 = {};
  sds_cfg1.Fields = SUPLA_DEVICE_CONFIG_FIELD_MODBUS;
  sds_cfg1.ConfigSize = sizeof(TDeviceConfig_Modbus);

  TDeviceConfig_Modbus *modbus = (TDeviceConfig_Modbus *)sds_cfg1.Config;

  modbus->ModbusAddress = 253;
  modbus->Role = MODBUS_ROLE_MASTER;
  modbus->SlaveTimeoutMs = 543;
  modbus->Network.Mode = MODBUS_NETWORK_MODE_UDP;
  modbus->Network.Port = 23;
  modbus->Serial.Mode = MODBUS_SERIAL_MODE_RTU;
  modbus->Serial.Baudrate = 115200;
  modbus->Serial.StopBits = MODBUS_SERIAL_STOP_BITS_TWO;

  modbus->Properties.Baudrate.B4800 = 1;
  modbus->Properties.Baudrate.B19200 = 1;
  modbus->Properties.Baudrate.B57600 = 1;

  modbus->Properties.Protocol.Slave = 1;
  modbus->Properties.Protocol.Ascii = 1;
  modbus->Properties.Protocol.Udp = 1;

  modbus->Properties.StopBits.One = 1;
  modbus->Properties.StopBits.Two = 1;

  device_json_config cfg1, cfg2;
  cfg1.set_config(&sds_cfg1);
  char *str = cfg1.get_user_config();
  ASSERT_TRUE(str != nullptr);

  cfg2.set_user_config(str);

  EXPECT_STREQ(str,
               "{\"modbus\":{\"role\":\"MASTER\",\"modbusAddress\":253,"
               "\"slaveTimeoutMs\":543,\"serialConfig\":{\"mode\":\"RTU\","
               "\"baudRate\":115200,\"stopBits\":\"TWO\"},\"networkConfig\":{"
               "\"mode\":\"UDP\",\"port\":23}}}");

  free(str);
  str = cfg1.get_properties();

  ASSERT_TRUE(str != nullptr);

  cfg2.set_properties(str);

  EXPECT_STREQ(str,
               "{\"modbus\":{\"availableProtocols\":[\"SLAVE\",\"ASCII\","
               "\"UDP\"],\"availableBaudrates\":[4800,19200,57600],"
               "\"availableStopbits\":[\"ONE\",\"TWO\"]}}");

  cfg2.get_config(&sds_cfg2, nullptr);

  free(str);

  EXPECT_EQ(sds_cfg1.Fields, sds_cfg2.Fields);
  EXPECT_EQ(memcmp(sds_cfg1.Config, sds_cfg2.Config, sds_cfg1.ConfigSize), 0);
}

TEST_F(DeviceConfigTest, modbus_readSampleUserConfig) {
  const char user_config[] =
      "{\"modbus\":{\"role\":\"SLAVE\",\"modbusAddress\":3,\"slaveTimeoutMs\":"
      "0,\"serialConfig\":{\"mode\":\"DISABLED\",\"baudRate\":19200,"
      "\"stopBits\":\"ONE\"},\"networkConfig\":{\"mode\":\"DISABLED\",\"port\":"
      "502}}}";
  device_json_config cfg;
  cfg.set_user_config(user_config);

  TSDS_SetDeviceConfig sds_cfg = {};
  cfg.get_config(&sds_cfg, nullptr);

  EXPECT_EQ(sds_cfg.Fields, SUPLA_DEVICE_CONFIG_FIELD_MODBUS);
  EXPECT_EQ(sds_cfg.ConfigSize, sizeof(TDeviceConfig_Modbus));
  EXPECT_EQ(((TDeviceConfig_Modbus *)sds_cfg.Config)->Serial.Baudrate, 19200);
  EXPECT_EQ(((TDeviceConfig_Modbus *)sds_cfg.Config)->ModbusAddress, 3);
  EXPECT_EQ(((TDeviceConfig_Modbus *)sds_cfg.Config)->Network.Port, 502);
}

} /* namespace testing */
