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

  char *config_str = config->get_user_config();
  EXPECT_NE(config_str, nullptr);
  if (config_str) {
    EXPECT_STREQ(config_str,
                 "{\"a\":1,\"b\":\"abcd\",\"c\":true,\"screenBrightness\":98,"
                 "\"buttonVolume\":15}");
    EXPECT_EQ(md5sum, "426fe9ff7937ecc4fb1a223196965d68");
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

} /* namespace testing */
