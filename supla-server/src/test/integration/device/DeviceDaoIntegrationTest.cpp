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
  runSqlScript("SetDeviceUserConfig.sql");
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
  string md5sum;
  device_json_config *config = dao->get_device_config(73, &md5sum);
  ASSERT_NE(config, nullptr);

  EXPECT_EQ(md5sum, "426fe9ff7937ecc4fb1a223196965d68");

  char *config_str = config->get_user_config();
  EXPECT_NE(config_str, nullptr);
  if (config_str) {
    EXPECT_STREQ(config_str,
                 "{\"a\":1,\"b\":\"abcd\",\"c\":true,\"screenBrightness\":98,"
                 "\"buttonVolume\":15}");

    free(config_str);
  }

  delete config;
}

TEST_F(DeviceDaoIntegrationTest, setDeviceConfig) {
  device_json_config cfg1;
  cfg1.set_user_config("{\"buttonVolume\":100,\"screenSaverMode\":3}");
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

  device_json_config *cfg2 = dao->get_device_config(73, nullptr);
  ASSERT_NE(cfg2, nullptr);

  char *config_str = cfg2->get_user_config();
  EXPECT_NE(config_str, nullptr);
  if (config_str) {
    EXPECT_STREQ(config_str,
                 "{\"a\":1,\"b\":\"abcd\",\"c\":true,\"screenBrightness\":98,"
                 "\"buttonVolume\":100,\"screenSaverMode\":3}");

    free(config_str);
  }

  delete cfg2;

  EXPECT_TRUE(dao->set_device_config(2, 73, &cfg1,
                                     SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME));

  cfg2 = dao->get_device_config(73, nullptr);
  ASSERT_NE(cfg2, nullptr);

  config_str = cfg2->get_user_config();
  EXPECT_NE(config_str, nullptr);
  if (config_str) {
    EXPECT_STREQ(config_str,
                 "{\"a\":1,\"b\":\"abcd\",\"c\":true,\"buttonVolume\":100}");

    free(config_str);
  }

  delete cfg2;
}

TEST_F(DeviceDaoIntegrationTest, getChannelConfig) {
  runSqlScript("SetChannelProperties.sql");

  string user_config_md5sum, properties_md5sum;
  channel_json_config *config =
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
  TSD_ChannelConfig_HVAC ds_hvac = {};
  ds_hvac.MainThermometerChannelNo = 1;
  hvac_config cfg1;
  cfg1.set_config(&ds_hvac);

  EXPECT_TRUE(dao->set_channel_user_config(2, 144, &cfg1));

  channel_json_config *cfg2 = dao->get_channel_config(144, nullptr, nullptr);
  ASSERT_NE(cfg2, nullptr);

  char *config_str = cfg2->get_user_config();
  EXPECT_NE(config_str, nullptr);
  if (config_str) {
    EXPECT_STREQ(
        config_str,
        "{\"pricePerUnit\":0.56,\"currency\":\"PLN\",\"hvac\":{"
        "\"mainThermometerChannelNo\":1,\"auxThermometerChannelNo\":0,"
        "\"auxThermometerType\":\"NotSet\","
        "\"antiFreezeAndOverheatProtectionEnabled\":false,"
        "\"availableAlgorithms\":[],\"usedAlgorithm\":\"\",\"minOnTimeS\":0,"
        "\"minOffTimeS\":0,\"outputValueOnError\":0,\"temperatures\":{}}}");
    free(config_str);
  }

  delete cfg2;
}

TEST_F(DeviceDaoIntegrationTest, setChannelProperties) {
  channel_json_config cfg1;
  cfg1.set_properties("{\"props\": 123}");
  dao->set_channel_properties(2, 144, &cfg1);

  channel_json_config *cfg2 = dao->get_channel_config(144, nullptr, nullptr);
  ASSERT_NE(cfg2, nullptr);

  char *prop_str = cfg2->get_properties();
  EXPECT_NE(prop_str, nullptr);
  if (prop_str) {
    EXPECT_STREQ(prop_str, "{\"props\":123}");
    free(prop_str);
  }

  delete cfg2;
}

} /* namespace testing */
