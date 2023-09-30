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

#include "DeviceDaoIntegrationTest.h"

#include <string>

#include "jsonconfig/channel/hvac_config.h"

using std::string;

namespace testing {

DeviceDaoIntegrationTest::DeviceDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

DeviceDaoIntegrationTest::~DeviceDaoIntegrationTest() {}

void DeviceDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  ASSERT_TRUE(dba != nullptr);
  dao = new supla_device_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  runSqlScript("SetDeviceJsonConfig.sql");
  Test::SetUp();
}

void DeviceDaoIntegrationTest::TearDown() {
  if (dao) {
    delete dao;
    dao = nullptr;
  }

  if (dba) {
    delete dba;
    dba = nullptr;
  }

  Test::TearDown();
}

TEST_F(DeviceDaoIntegrationTest, getDeviceConfig) {
  string user_config_md5sum, properties_md5sum;
  device_json_config *config =
      dao->get_device_config(73, &user_config_md5sum, &properties_md5sum);
  ASSERT_NE(config, nullptr);

  EXPECT_EQ(user_config_md5sum, "426fe9ff7937ecc4fb1a223196965d68");
  EXPECT_EQ(properties_md5sum, "651e6c58d52796442056a2500b32daab");

  char *str = config->get_user_config();
  EXPECT_NE(str, nullptr);
  if (str) {
    EXPECT_STREQ(str,
                 "{\"a\":1,\"b\":\"abcd\",\"c\":true,\"screenBrightness\":98,"
                 "\"buttonVolume\":15}");

    free(str);
  }

  str = config->get_properties();
  EXPECT_NE(str, nullptr);
  if (str) {
    EXPECT_STREQ(str,
                 "{\"1\":2,\"screenSaverModesAvailable\":[\"OFF\","
                 "\"TEMPERATURE\",\"MAIN_AND_AUX_TEMPERATURE\"]}");

    free(str);
  }

  delete config;
}

TEST_F(DeviceDaoIntegrationTest, setDeviceConfig) {
  device_json_config cfg1;
  cfg1.set_user_config(
      "{\"buttonVolume\":100,\"screenSaver\":{\"mode\":\"Off\"}}");
  cfg1.set_properties("{\"screenSaverModesAvailable\":[\"OFF\"]}");

  EXPECT_TRUE(
      dao->set_device_config(2, 73, &cfg1,
                             SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
                                 SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME |
                                 SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE));

  EXPECT_TRUE(
      dao->set_device_config(2, 73, &cfg1,
                             SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
                                 SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME |
                                 SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE));

  device_json_config *cfg2 = dao->get_device_config(73, nullptr, nullptr);
  ASSERT_NE(cfg2, nullptr);

  char *str = cfg2->get_user_config();
  EXPECT_NE(str, nullptr);
  if (str) {
    EXPECT_STREQ(str,
                 "{\"a\":1,\"b\":\"abcd\",\"c\":true,\"screenBrightness\":98,"
                 "\"buttonVolume\":100,\"screenSaver\":{\"mode\":\"Off\"}}");

    free(str);
  }

  str = cfg2->get_properties();
  EXPECT_NE(str, nullptr);
  if (str) {
    EXPECT_STREQ(str, "{\"1\":2,\"screenSaverModesAvailable\":[\"OFF\"]}");

    free(str);
  }

  delete cfg2;

  EXPECT_TRUE(dao->set_device_config(2, 73, &cfg1,
                                     SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME));

  cfg2 = dao->get_device_config(73, nullptr, nullptr);
  ASSERT_NE(cfg2, nullptr);

  str = cfg2->get_user_config();
  EXPECT_NE(str, nullptr);
  if (str) {
    EXPECT_STREQ(str,
                 "{\"a\":1,\"b\":\"abcd\",\"c\":true,\"buttonVolume\":100}");

    free(str);
  }

  str = cfg2->get_properties();
  EXPECT_NE(str, nullptr);
  if (str) {
    EXPECT_STREQ(str, "{\"1\":2}");

    free(str);
  }

  delete cfg2;
}

TEST_F(DeviceDaoIntegrationTest, getChannelConfig) {
  runSqlScript("SetChannelProperties.sql");

  string user_config_md5sum, properties_md5sum;
  supla_json_config *config =
      dao->get_channel_config(144, &user_config_md5sum, &properties_md5sum);
  ASSERT_NE(config, nullptr);

  EXPECT_EQ(user_config_md5sum, "c4903ea1c9fe8f29c2031b5f08563d2c");
  EXPECT_EQ(properties_md5sum, "b96c07e038d9463be22371342b40d0c3");

  char *config_str = config->get_user_config();
  EXPECT_NE(config_str, nullptr);
  if (config_str) {
    EXPECT_STREQ(config_str, "{\"pricePerUnit\":0.56,\"currency\":\"PLN\"}");
    free(config_str);
  }

  char *properties_str = config->get_properties();
  EXPECT_NE(properties_str, nullptr);
  if (properties_str) {
    EXPECT_STREQ(properties_str,
                 "{\"countersAvailable\":[\"forwardActiveEnergy\","
                 "\"reverseActiveEnergy\",\"forwardReactiveEnergy\","
                 "\"reverseReactiveEnergy\",\"forwardActiveEnergyBalanced\","
                 "\"reverseActiveEnergyBalanced\"]}");
    free(properties_str);
  }

  delete config;
}

TEST_F(DeviceDaoIntegrationTest, setChannelHvacUserConfig) {
  TChannelConfig_HVAC ds_hvac = {};
  ds_hvac.MainThermometerChannelNo = 1;
  hvac_config cfg1;
  cfg1.set_config(&ds_hvac, 0);

  EXPECT_TRUE(dao->set_channel_config(2, 144, &cfg1));

  supla_json_config *cfg2 = dao->get_channel_config(144, nullptr, nullptr);
  ASSERT_NE(cfg2, nullptr);

  char *str = cfg2->get_user_config();
  EXPECT_NE(str, nullptr);
  if (str) {
    EXPECT_STREQ(
        str,
        "{\"pricePerUnit\":0.56,\"currency\":\"PLN\","
        "\"mainThermometerChannelNo\":1,\"auxThermometerChannelNo\":null,"
        "\"auxThermometerType\":\"NOT_SET\",\"binarySensorChannelNo\":null,"
        "\"antiFreezeAndOverheatProtectionEnabled\":false,\"usedAlgorithm\":"
        "\"\",\"minOnTimeS\":0,\"minOffTimeS\":0,\"outputValueOnError\":0,"
        "\"subfunction\":\"NOT_SET\","
        "\"temperatureSetpointChangeSwitchesToManualMode\":false,"
        "\"temperatures\":{}}");
    free(str);
  }

  str = cfg2->get_properties();
  EXPECT_NE(str, nullptr);
  if (str) {
    EXPECT_STREQ(str, "{\"availableAlgorithms\":[],\"temperatures\":{}}");
    free(str);
  }

  delete cfg2;
}

TEST_F(DeviceDaoIntegrationTest, setChannelProperties) {
  supla_json_config cfg1;
  cfg1.set_properties("{\"props\": 123}");
  dao->set_channel_properties(2, 144, &cfg1);

  supla_json_config *cfg2 = dao->get_channel_config(144, nullptr, nullptr);
  ASSERT_NE(cfg2, nullptr);

  char *prop_str = cfg2->get_properties();
  EXPECT_NE(prop_str, nullptr);
  if (prop_str) {
    EXPECT_STREQ(prop_str, "{\"props\":123}");
    free(prop_str);
  }

  delete cfg2;
}

TEST_F(DeviceDaoIntegrationTest, setAndUpdateChanneValue) {
  char value[SUPLA_CHANNELVALUE_SIZE] = {0x01, 0x11, 0x22, 0x33,
                                         0x23, 0x24, 0x00, 0x25};
  dao->update_channel_value(2, 2, value, 10);

  string result = "";

  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_value WHERE channel_id = 2 "
      "AND "
      "user_id = 2 AND TIMESTAMPDIFF(SECOND, update_time, UTC_TIMESTAMP) >= 0 "
      "AND TIMESTAMPDIFF(SECOND, update_time, UTC_TIMESTAMP) <= 1 AND "
      "TIMESTAMPDIFF(SECOND, UTC_TIMESTAMP, valid_to) >= 9 AND "
      "TIMESTAMPDIFF(SECOND, UTC_TIMESTAMP, valid_to) <= 10",
      &result);

  EXPECT_EQ(result, "v\n0111223323240025\n");

  value[0] = 0x02;
  dao->update_channel_value(2, 2, value, 0);

  result = "";

  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_value WHERE "
      "channel_id = 2 AND user_id = 2 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) >= 0 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) <= 1 AND valid_to IS NULL",
      &result);

  EXPECT_EQ(result, "v\n0211223323240025\n");
}

TEST_F(DeviceDaoIntegrationTest, setAndUpdateExtendedValue) {
  TSuplaChannelExtendedValue ev_struct = {};

  ev_struct.type = 10;

  for (int a = 0; a < 15; a++) {
    ev_struct.value[a] = a;
  }

  {
    ev_struct.size = 10;
    supla_channel_extended_value ev(&ev_struct);
    dao->update_channel_extended_value(2, 2, &ev);
  }

  string result = "";

  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_extended_value WHERE "
      "channel_id = 2 AND user_id = 2 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) >= 0 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) <= 1",
      &result);

  EXPECT_EQ(result, "v\n00010203040506070809\n");

  {
    ev_struct.size = 5;
    supla_channel_extended_value ev(&ev_struct);
    dao->update_channel_extended_value(2, 2, &ev);
  }

  result = "";

  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_extended_value WHERE "
      "channel_id = 2 AND user_id = 2 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) >= 0 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) <= 1",
      &result);

  EXPECT_EQ(result, "v\n0001020304\n");

  {
    ev_struct.size = 0;
    supla_channel_extended_value ev(&ev_struct);
    dao->update_channel_extended_value(2, 2, &ev);
  }

  result = "";

  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_extended_value WHERE "
      "channel_id = 2 AND user_id = 2 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) >= 0 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) <= 1",
      &result);

  EXPECT_EQ(result, "v\nNULL\n");

  {
    ev_struct.size = 15;
    supla_channel_extended_value ev(&ev_struct);
    dao->update_channel_extended_value(2, 2, &ev);
  }

  result = "";

  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_extended_value WHERE "
      "channel_id = 2 AND user_id = 2 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) >= 0 AND TIMESTAMPDIFF(SECOND, update_time, "
      "UTC_TIMESTAMP) <= 1",
      &result);

  EXPECT_EQ(result, "v\n000102030405060708090A0B0C0D0E\n");
}

TEST_F(DeviceDaoIntegrationTest, deviceLimit) {
  ASSERT_TRUE(dba->connect());
  EXPECT_EQ(dao->get_device_limit_left(2), 90);
}

} /* namespace testing */
