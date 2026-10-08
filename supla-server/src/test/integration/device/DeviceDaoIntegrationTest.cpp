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

#include <cstdio>
#include <memory>
#include <string>
#include <thread>

#include "device/extended_value/channel_em_extended_value.h"
#include "device/extended_value/channel_extended_value.h"
#include "device/extended_value/channel_state_extended_value.h"
#include "device/value/channel_em_value.h"
#include "jsonconfig/channel/hvac_config.h"
#include "ocpp/ocpp_device.h"
#include "ocpp/ocpp_gateway.h"
#include "user/user.h"
#include "user/userdevices.h"

using std::string;

namespace testing {

DeviceDaoIntegrationTest::DeviceDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

DeviceDaoIntegrationTest::~DeviceDaoIntegrationTest() {}

void DeviceDaoIntegrationTest::SetUp() {
  dba = new supla_mariadb_access_provider();
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
  EXPECT_EQ(properties_md5sum, "c74065d79f3dbcf05899b6109fca2e2a");

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
                 "{\"1\":2,\"homeScreenContentAvailable\":[\"NONE\","
                 "\"TEMPERATURE\",\"MAIN_AND_AUX_TEMPERATURE\"]}");

    free(str);
  }

  delete config;
}

TEST_F(DeviceDaoIntegrationTest, setDeviceConfig) {
  device_json_config cfg1;
  cfg1.set_user_config(
      "{\"buttonVolume\":100,\"homeScreen\":{\"content\":\"NONE\"}}");
  cfg1.set_properties("{\"homeScreenContentAvailable\":[\"NONE\"]}");

  EXPECT_TRUE(dao->set_device_config(
      2, 73, &cfg1, true,
      SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
          SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME |
          SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT));

  EXPECT_TRUE(dao->set_device_config(
      2, 73, &cfg1, true,
      SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS |
          SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME |
          SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT));

  device_json_config *cfg2 = dao->get_device_config(73, nullptr, nullptr);
  ASSERT_NE(cfg2, nullptr);

  char *str = cfg2->get_user_config();
  EXPECT_NE(str, nullptr);
  if (str) {
    EXPECT_STREQ(str,
                 "{\"a\":1,\"b\":\"abcd\",\"c\":true,\"screenBrightness\":98,"
                 "\"buttonVolume\":100,\"homeScreen\":{\"content\":\"NONE\"}}");

    free(str);
  }

  str = cfg2->get_properties();
  EXPECT_NE(str, nullptr);
  if (str) {
    EXPECT_STREQ(str, "{\"1\":2,\"homeScreenContentAvailable\":[\"NONE\"]}");

    free(str);
  }

  delete cfg2;

  EXPECT_TRUE(dao->set_device_config(2, 73, &cfg1, true,
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
        "\"auxMinMaxSetpointEnabled\":false,\"useSeparateHeatCoolOutputs\":"
        "false,\"temperatures\":{},\"masterThermostatChannelNo\":null,"
        "\"heatOrColdSourceSwitchChannelNo\":null,\"pumpSwitchChannelNo\":null,"
        "\"temperatureControlType\":\"NOT_SUPPORTED\",\"localUILock\":[],"
        "\"minAllowedTemperatureSetpointFromLocalUI\":0,"
        "\"maxAllowedTemperatureSetpointFromLocalUI\":0}");
    free(str);
  }

  str = cfg2->get_properties();
  EXPECT_NE(str, nullptr);
  if (str) {
    EXPECT_STREQ(str,
                 "{\"availableAlgorithms\":[],\"temperatures\":{},"
                 "\"hiddenConfigFields\":[],\"readOnlyConfigFields\":[],"
                 "\"hiddenTemperatureConfigFields\":[],"
                 "\"readOnlyTemperatureConfigFields\":[],"
                 "\"localUILockingCapabilities\":[]}");
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

TEST_F(DeviceDaoIntegrationTest,
       touchVirtualValuesPreservesDataAndScopesOwner) {
  ASSERT_TRUE(dba->connect());
  ASSERT_EQ(
      0,
      dba->query("UPDATE supla_dev_channel SET is_virtual=1 WHERE id=2", true));
  int device_id = dba->get_int(
      2, 0,
      "SELECT iodevice_id FROM supla_dev_channel WHERE id=? AND user_id=2");
  ASSERT_GT(device_id, 0);
  ASSERT_EQ(0,
            dba->query("DELETE FROM supla_dev_channel_value WHERE channel_id=2",
                       true));
  EXPECT_TRUE(dao->touch_channel_values(device_id, 2, 90));
  string result;
  sqlQuery("SELECT COUNT(*) n FROM supla_dev_channel_value WHERE channel_id=2",
           &result);
  EXPECT_EQ("n\n0\n", result);
  EXPECT_TRUE(dao->touch_channel_values(device_id, 2, 0));
  result.clear();
  sqlQuery("SELECT COUNT(*) n FROM supla_dev_channel_value WHERE channel_id=2",
           &result);
  EXPECT_EQ("n\n0\n", result);

  const char value[SUPLA_CHANNELVALUE_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8};
  dao->update_channel_value(2, 2, value, 90);
  EXPECT_TRUE(dao->touch_channel_values(device_id, 1, 0));
  result.clear();
  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_value "
      "WHERE channel_id=2 AND user_id=2 AND valid_to>UTC_TIMESTAMP()",
      &result);
  EXPECT_EQ("v\n0102030405060708\n", result);

  EXPECT_TRUE(dao->touch_channel_values(device_id, 2, 0));
  result.clear();
  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_value "
      "WHERE channel_id=2 AND user_id=2 AND valid_to<=UTC_TIMESTAMP()",
      &result);
  EXPECT_EQ("v\n0102030405060708\n", result);

  EXPECT_TRUE(dao->touch_channel_values(device_id, 2, 90));
  result.clear();
  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_value "
      "WHERE channel_id=2 AND user_id=2 AND valid_to>UTC_TIMESTAMP()",
      &result);
  EXPECT_EQ("v\n0102030405060708\n", result);
}

TEST_F(DeviceDaoIntegrationTest, ocppMissingValuesRequireFirstMeasurement) {
  ASSERT_TRUE(dba->connect());
  ASSERT_EQ(
      0, dba->query("INSERT INTO supla_ocpp_charging_station VALUES (1,2,73)",
                    true));
  string sql = "UPDATE supla_dev_channel SET is_virtual=1,type=" +
               std::to_string(SUPLA_CHANNELTYPE_RELAY) +
               ",func=" + std::to_string(SUPLA_CHANNELFNC_POWERSWITCH) +
               " WHERE id=140";
  ASSERT_EQ(0, dba->query(sql.c_str(), true));
  sql = "UPDATE supla_dev_channel SET is_virtual=1,type=" +
        std::to_string(SUPLA_CHANNELTYPE_ELECTRICITY_METER) +
        ",func=" + std::to_string(SUPLA_CHANNELFNC_ELECTRICITY_METER) +
        " WHERE id=141";
  ASSERT_EQ(0, dba->query(sql.c_str(), true));
  ASSERT_EQ(
      0,
      dba->query(
          "DELETE FROM supla_dev_channel_value WHERE channel_id IN (140,141)",
          true));
  ASSERT_EQ(0, dba->query("DELETE FROM supla_dev_channel_extended_value WHERE "
                          "channel_id IN (140,141)",
                          true));

  supla_user::user_free();
  supla_user::init();
  const char *suid = "ocpp-touch-test";
  supla_user user(2, suid, nullptr);
  supla_ocpp_gateway gateway;
  gateway.on_connected(suid, 73, 90);
  gateway.on_alive(suid, 73, 90);
  gateway.wait_until_idle();
  for (int channel_id : {140, 141}) {
    auto channel = user.get_devices()->get_ocpp_channel(channel_id);
    EXPECT_EQ(
        0,
        user.get_devices()->get_virtual_channel(channel_id).get_channel_id());
    EXPECT_EQ(channel_id, channel.get_channel_id());
    EXPECT_EQ(nullptr, channel.get_value());
    EXPECT_TRUE(channel.get_availability_status().is_offline());
  }
  string result;
  sqlQuery(
      "SELECT COUNT(*) n FROM supla_dev_channel_value "
      "WHERE channel_id IN (140,141)",
      &result);
  EXPECT_EQ("n\n0\n", result);

  // A real OFF initializes only the switch, not the meter.
  gateway.on_state(suid, 73, 1, false, 90);
  gateway.wait_until_idle();
  auto power_switch = user.get_devices()->get_ocpp_channel(140);
  char raw[SUPLA_CHANNELVALUE_SIZE] = {};
  ASSERT_TRUE(power_switch.get_value(raw));
  EXPECT_EQ(0, raw[0]);
  EXPECT_TRUE(power_switch.get_availability_status().is_online());
  EXPECT_EQ(nullptr, user.get_devices()->get_ocpp_channel(141).get_value());

  // A voltage sample is stored, but cannot invent an energy counter of zero.
  gateway.on_meter(suid, 73, 1, nlohmann::json{{"voltage", {230000}}}, 90);
  gateway.on_alive(suid, 73, 90);
  gateway.wait_until_idle();
  user.get_devices()->reload_ocpp_devices(73);
  auto meter = user.get_devices()->get_ocpp_channel(141);
  EXPECT_EQ(nullptr, meter.get_value());
  EXPECT_TRUE(meter.get_availability_status().is_offline());
  std::unique_ptr<supla_abstract_channel_extended_value> extended(
      meter.get_extended_value());
  auto em = dynamic_cast<supla_channel_em_extended_value *>(extended.get());
  ASSERT_NE(nullptr, em);
  EXPECT_DOUBLE_EQ(230, em->get_voltage(1));
  result.clear();
  sqlQuery(
      "SELECT COUNT(*) n FROM supla_dev_channel_value WHERE channel_id=141",
      &result);
  EXPECT_EQ("n\n0\n", result);

  // A reported zero is valid, unlike the previously absent reading.
  gateway.on_meter(suid, 73, 1, nlohmann::json{{"energy", 0}}, 90);
  gateway.wait_until_idle();
  meter = user.get_devices()->get_ocpp_channel(141);
  ASSERT_TRUE(meter.get_raw_value(raw));
  EXPECT_EQ(
      0U,
      supla_channel_em_value(raw).get_em_value()->total_forward_active_energy);
  EXPECT_TRUE(meter.get_availability_status().is_online());
  result.clear();
  sqlQuery(
      "SELECT COUNT(*) n FROM supla_dev_channel_value "
      "WHERE channel_id IN (140,141) AND valid_to>UTC_TIMESTAMP()",
      &result);
  EXPECT_EQ("n\n2\n", result);

  gateway.on_meter(suid, 73, 1, nlohmann::json{{"energy", 12000}}, 90);
  gateway.on_disconnected(suid, 73);
  gateway.wait_until_idle();
  user.get_devices()->reload_ocpp_devices(73);
  meter = user.get_devices()->get_ocpp_channel(141);
  ASSERT_TRUE(meter.get_raw_value(raw));
  EXPECT_EQ(
      1200U,
      supla_channel_em_value(raw).get_em_value()->total_forward_active_energy);
  EXPECT_TRUE(meter.get_availability_status().is_offline());
  gateway.on_connected(suid, 73, 90);
  gateway.wait_until_idle();
  meter = user.get_devices()->get_ocpp_channel(141);
  EXPECT_TRUE(meter.get_availability_status().is_online());
  ASSERT_TRUE(meter.get_raw_value(raw));
  EXPECT_EQ(
      1200U,
      supla_channel_em_value(raw).get_em_value()->total_forward_active_energy);
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
  EXPECT_EQ(dao->get_device_limit_left(2), 89);
}

TEST_F(DeviceDaoIntegrationTest, virtualDevicesCannotRegisterOverSrpc) {
  ASSERT_TRUE(dba->connect());
  // NULL auth keys remain valid for legacy physical devices. Eligibility is
  // checked separately, including after a cached email/key authentication.
  for (int is_virtual : {0, 1}) {
    string sql =
        "UPDATE supla_iodevice SET enabled=1,auth_key=NULL,is_virtual=" +
        std::to_string(is_virtual) + " WHERE id=73";
    ASSERT_EQ(0, dba->query(sql.c_str(), true));
    bool enabled = true, location_enabled = false, addition_blocked = false;
    int original_location = 0, location = 0, flags = 0;
    EXPECT_TRUE(dao->get_device_variables(
        73, &enabled, &original_location, &location, &location_enabled, &flags,
        &addition_blocked));
    EXPECT_EQ(is_virtual == 0, enabled);
    string result;
    sqlQuery("SELECT CAST(enabled AS unsigned integer) enabled "
             "FROM supla_iodevice WHERE id=73",
             &result);
    EXPECT_EQ("enabled\n1\n", result);  // Only SRPC eligibility is disabled.
  }
}

TEST_F(DeviceDaoIntegrationTest, ocppCoalescesPersistenceAndRetriesFailures) {
  ASSERT_TRUE(dba->connect());
  ASSERT_EQ(
      0, dba->query("INSERT INTO supla_ocpp_charging_station VALUES (1,2,73)",
                    true));
  ASSERT_EQ(
      0, dba->query(
             "UPDATE supla_dev_channel SET is_virtual=1 WHERE id IN (140,141)",
             true));
  ASSERT_EQ(
      0,
      dba->query(
          "DELETE FROM supla_dev_channel_value WHERE channel_id IN (140,141)",
          true));
  ASSERT_EQ(0, dba->query("DELETE FROM supla_dev_channel_extended_value WHERE "
                          "channel_id IN (140,141)",
                          true));
  supla_ocpp_device_config config;
  config.device_id = 73;
  config.meter.id = 141;
  config.meter.type = SUPLA_CHANNELTYPE_ELECTRICITY_METER;
  config.meter.func = SUPLA_CHANNELFNC_ELECTRICITY_METER;
  config.power_switch.id = 140;
  config.power_switch.type = SUPLA_CHANNELTYPE_RELAY;
  config.power_switch.func = SUPLA_CHANNELFNC_POWERSWITCH;
  supla_user::user_free();
  supla_user::init();
  supla_user user(2, "ocpp-persistence-test", nullptr);
  auto device = std::make_shared<supla_ocpp_device>(&user, config);
  auto stored_voltage = [&]() {
    std::unique_ptr<supla_abstract_channel_extended_value> value(
        dao->get_channel_extended_value(2, 141));
    auto em = dynamic_cast<supla_channel_em_extended_value *>(value.get());
    EXPECT_NE(nullptr, em);
    return em ? em->get_voltage(1) : -1;
  };
  supla_ocpp_meter_report report;
  report.energy = 1000;
  report.voltage[0] = 230000;
  device->update_meter(report, 90);
  EXPECT_DOUBLE_EQ(230, stored_voltage());
  report.energy.reset();  // Extended-only changes must also be retained.
  for (int i = 0; i < 100; i++) {
    report.voltage[0] = 240000 + i;
    device->update_meter(report, 90);
    device->renew_validity(90);
  }
  EXPECT_DOUBLE_EQ(230, stored_voltage());
  std::this_thread::sleep_for(std::chrono::milliseconds(5100));
  device->renew_validity(90);  // Flushes without another meter report.
  EXPECT_NEAR(240.09, stored_voltage(), 0.01);

  report.voltage[0] = 250000;
  device->update_meter(report, 90);
  device->update_state(true, 90);  // A switch change is never delayed.
  EXPECT_DOUBLE_EQ(250, stored_voltage());
  string result;
  sqlQuery(
      "SELECT HEX(value) v FROM supla_dev_channel_value WHERE channel_id=140",
      &result);
  EXPECT_EQ("v\n0100000000000000\n", result);

  // A partial SQL failure must keep the latest data, without retrying on
  // every incoming heartbeat. MyISAM records attempts even on SIGNAL rollback.
  ASSERT_EQ(0,
            dba->query("CREATE TABLE ocpp_write_attempts (n INT) ENGINE=MyISAM",
                       true));
  ASSERT_EQ(0,
            dba->query("CREATE TRIGGER ocpp_fail_write BEFORE UPDATE ON "
                       "supla_dev_channel_extended_value FOR EACH ROW BEGIN "
                       "INSERT INTO ocpp_write_attempts VALUES (1); SIGNAL "
                       "SQLSTATE '45000' SET MESSAGE_TEXT='test failure'; END",
                       true));
  report.voltage[0] = 260000;
  device->update_meter(report, 90);
  device->update_state(false, 90);  // Immediate attempt, extended save fails.
  EXPECT_DOUBLE_EQ(250, stored_voltage());
  for (int i = 0; i < 100; i++) device->renew_validity(90);
  result.clear();
  sqlQuery("SELECT COUNT(*) n FROM ocpp_write_attempts", &result);
  EXPECT_EQ("n\n1\n", result);
  ASSERT_EQ(0, dba->query("DROP TRIGGER ocpp_fail_write", true));
  std::this_thread::sleep_for(std::chrono::milliseconds(5100));
  device->renew_validity(90);
  EXPECT_DOUBLE_EQ(260, stored_voltage());

  report.voltage[0] = 270000;
  device->update_meter(report, 90);
  device->disconnect();  // Includes deferred measurements and zero validity.
  EXPECT_DOUBLE_EQ(270, stored_voltage());
  result.clear();
  sqlQuery(
      "SELECT COUNT(*) n FROM supla_dev_channel_value WHERE channel_id IN "
      "(140,141) AND valid_to<=UTC_TIMESTAMP()",
      &result);
  EXPECT_EQ("n\n2\n", result);
  report.voltage[0] = 280000;
  device->update_state(true, 2);
  ASSERT_EQ(0,
            dba->query("UPDATE supla_dev_channel_value SET "
                       "valid_to=UTC_TIMESTAMP() WHERE channel_id IN (140,141)",
                       true));
  std::this_thread::sleep_for(std::chrono::milliseconds(1100));
  device->renew_validity(2);
  result.clear();
  sqlQuery(
      "SELECT COUNT(*) n FROM supla_dev_channel_value WHERE channel_id IN "
      "(140,141) AND valid_to>UTC_TIMESTAMP()",
      &result);
  EXPECT_EQ("n\n2\n", result);  // Short validity overrides normal coalescing.
  device->deactivate();
  device->update_meter(report, 90);
  device->disconnect();
  EXPECT_DOUBLE_EQ(270, stored_voltage());
}

TEST_F(DeviceDaoIntegrationTest, updateChannelConflictDetails) {
  string result = "";

  sqlQuery("SELECT conflict_details FROM supla_dev_channel WHERE id = 311",
           &result);

  EXPECT_EQ(result, "conflict_details\nNULL\n");

  char details[] = "{type=\"10\"}";

  dao->update_channel_conflict_details(146, 3, details);

  result = "";
  sqlQuery("SELECT conflict_details FROM supla_dev_channel WHERE id = 311",
           &result);

  EXPECT_EQ(result, "conflict_details\n{type=\"10\"}\n");

  result = "";
  sqlQuery(
      "SELECT COUNT(*) c FROM supla_dev_channel WHERE conflict_details IS NOT "
      "NULL",
      &result);

  EXPECT_EQ(result, "c\n1\n");
}

TEST_F(DeviceDaoIntegrationTest, updateDevicePairingResult) {
  string result = "";

  sqlQuery("SELECT pairing_result FROM supla_iodevice WHERE id = 146", &result);

  EXPECT_EQ(result, "pairing_result\nNULL\n");

  char pairing_result[] = "{result=\"SUCCESS\"}";

  dao->update_device_pairing_result(146, pairing_result);

  result = "";
  sqlQuery("SELECT pairing_result FROM supla_iodevice WHERE id = 146", &result);

  EXPECT_EQ(result, "pairing_result\n{result=\"SUCCESS\"}\n");

  result = "";
  sqlQuery(
      "SELECT COUNT(*) c FROM supla_iodevice WHERE pairing_result IS NOT "
      "NULL",
      &result);

  EXPECT_EQ(result, "c\n1\n");

  dao->update_device_pairing_result(146, nullptr);

  result = "";
  sqlQuery(
      "SELECT COUNT(*) c FROM supla_iodevice WHERE pairing_result IS NOT "
      "NULL",
      &result);

  EXPECT_EQ(result, "c\n0\n");
}

TEST_F(DeviceDaoIntegrationTest, setAndClearCalCfgQueue) {
  string result = "";
  time_t valid_until = 1893456000;
  char queue_json[] =
      "[{\"sender_id\":123,\"channel_number\":-1,"
      "\"command\":\"IDENTIFY_DEVICE\",\"command_code\":9300,"
      "\"super_user_authorized\":true,\"data_type\":\"NONE\","
      "\"data_type_code\":0,\"data_size\":0,"
      "\"enqueue_status\":\"WAITING_TO_SEND\","
      "\"queued_at\":\"1970-01-01T00:01:40Z\","
      "\"sent_at\":null,\"requires_response\":false}]";

  EXPECT_TRUE(dao->set_calcfg_queue(2, 146, queue_json, valid_until));

  sqlQuery(
      "SELECT queue FROM supla_calcfg_queue WHERE user_id = 2 AND "
      "iodevice_id = 146",
      &result);

  EXPECT_EQ(result, string("queue\n") + queue_json + "\n");

  char updated_queue_json[] =
      "[{\"sender_id\":123,\"channel_number\":-1,"
      "\"command\":\"SET_CFG_MODE_PASSWORD\",\"command_code\":9050,"
      "\"super_user_authorized\":true,\"data_type\":\"NONE\","
      "\"data_type_code\":0,\"data_size\":32,"
      "\"data\":"
      "\"0000000000000000000000000000000000000000000000000000000000000000\","
      "\"enqueue_status\":\"WAITING_FOR_RESULT\","
      "\"queued_at\":\"1970-01-01T00:01:40Z\","
      "\"sent_at\":\"1970-01-01T00:03:20Z\","
      "\"requires_response\":true},"
      "{\"sender_id\":456,\"channel_number\":3,"
      "\"command\":\"IDENTIFY_DEVICE\",\"command_code\":9300,"
      "\"super_user_authorized\":false,\"data_type\":\"NONE\","
      "\"data_type_code\":0,\"data_size\":0,"
      "\"enqueue_status\":\"WAITING_TO_SEND\","
      "\"queued_at\":\"1970-01-01T00:05:00Z\","
      "\"sent_at\":null,\"requires_response\":false}]";

  EXPECT_TRUE(dao->set_calcfg_queue(2, 146, updated_queue_json, valid_until));

  result = "";
  sqlQuery(
      "SELECT queue FROM supla_calcfg_queue WHERE user_id = 2 AND "
      "iodevice_id = 146",
      &result);

  EXPECT_EQ(result, string("queue\n") + updated_queue_json + "\n");

  string loaded_queue;
  time_t loaded_valid_until = 0;
  EXPECT_TRUE(
      dao->get_calcfg_queue(2, 146, &loaded_queue, &loaded_valid_until));
  EXPECT_EQ(loaded_queue, updated_queue_json);
  EXPECT_EQ(loaded_valid_until, valid_until);

  EXPECT_TRUE(dao->set_calcfg_queue(2, 146, "[]"));

  result = "";
  sqlQuery("SELECT COUNT(*) c FROM supla_calcfg_queue", &result);

  EXPECT_EQ(result, "c\n0\n");
}

TEST_F(DeviceDaoIntegrationTest, subDevice) {
  string result = "";
  sqlQuery("SELECT COUNT(*) c FROM supla_subdevice", &result);
  EXPECT_EQ(result, "c\n0\n");

  TDS_SubdeviceDetails details = {};
  details.SubDeviceId = 15;
  snprintf(details.SerialNumber, sizeof(details.SerialNumber), "%s", "SN");
  dao->set_subdevice_details(83, &details);
  dao->set_subdevice_details(83, &details);

  result = "";
  sqlQuery(
      "SELECT name, software_version, product_code, serial_number FROM "
      "supla_subdevice WHERE id = 15 AND iodevice_id = 83 AND reg_date >= "
      "DATE_ADD(UTC_TIMESTAMP(), INTERVAL -1 SECOND) AND updated_at IS NULL",
      &result);
  EXPECT_EQ(result,
            "name\tsoftware_version\tproduct_code\tserial_"
            "number\nNULL\tNULL\tNULL\tSN\n");

  snprintf(details.Name, sizeof(details.Name), "%s", "Nnaamme");
  snprintf(details.ProductCode, sizeof(details.ProductCode), "%s", "Coode");
  snprintf(details.SoftVer, sizeof(details.SoftVer), "%s", "SV");

  dao->set_subdevice_details(83, &details);

  snprintf(details.SoftVer, sizeof(details.SoftVer), "%s", "1.0");

  dao->set_subdevice_details(84, &details);

  result = "";
  sqlQuery(
      "SELECT name, software_version, product_code, serial_number FROM "
      "supla_subdevice WHERE id = 15 AND iodevice_id = 83 AND updated_at >= "
      "DATE_ADD(UTC_TIMESTAMP(), INTERVAL -1 SECOND)",
      &result);
  EXPECT_EQ(result,
            "name\tsoftware_version\tproduct_code\tserial_"
            "number\nNnaamme\tSV\tCoode\tSN\n");

  result = "";
  sqlQuery(
      "SELECT name, software_version, product_code, serial_number FROM "
      "supla_subdevice",
      &result);
  EXPECT_EQ(result,
            "name\tsoftware_version\tproduct_code\tserial_"
            "number\nNnaamme\tSV\tCoode\tSN\nNnaamme\t1.0\tCoode\tSN\n");
}

TEST_F(DeviceDaoIntegrationTest, getExtendedValue) {
  runSqlScript("InsertExtendedValue.sql");

  EXPECT_EQ(dao->get_channel_extended_value(1, 2), nullptr);
  supla_abstract_channel_extended_value *value =
      dao->get_channel_extended_value(2, 140);

  ASSERT_NE(value, nullptr);
  EXPECT_NE(dynamic_cast<supla_channel_state_extended_value *>(value), nullptr);

  delete value;
}

TEST_F(DeviceDaoIntegrationTest, setAndUpdateChannelState) {
  string result = "";

  sqlQuery("SELECT count(*) c FROM supla_dev_channel_state", &result);

  EXPECT_EQ(result, "c\n0\n");

  TDSC_ChannelState raw = {};
  raw.defaultIconField = 1;
  supla_channel_state state1(&raw);
  dao->update_channel_state(140, 2, &state1);

  raw.defaultIconField = 0;
  supla_channel_state state2(&raw);
  dao->update_channel_state(141, 2, &state2);

  result = "";

  sqlQuery(
      "SELECT channel_id, user_id, state FROM supla_dev_channel_state WHERE "
      "TIMESTAMPDIFF(SECOND, update_time, UTC_TIMESTAMP) >= 0 AND "
      "TIMESTAMPDIFF(SECOND, update_time, UTC_TIMESTAMP) <= 1",
      &result);

  EXPECT_EQ(result,
            "channel_id\tuser_id\tstate\n140\t2\t{\"defaultIconField\":1}"
            "\n141\t2\t{\"defaultIconField\":0}\n");

  raw.Fields = SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL;
  raw.BatteryLevel = 5;

  supla_channel_state state3(&raw);
  dao->update_channel_state(140, 2, &state3);

  result = "";

  sqlQuery(
      "SELECT channel_id, user_id, state FROM supla_dev_channel_state WHERE "
      "TIMESTAMPDIFF(SECOND, update_time, UTC_TIMESTAMP) >= 0 AND "
      "TIMESTAMPDIFF(SECOND, update_time, UTC_TIMESTAMP) <= 1",
      &result);

  EXPECT_EQ(result,
            "channel_id\tuser_id\tstate\n140\t2\t{\"defaultIconField\":0,"
            "\"batteryLevel\":5}\n141\t2\t{\"defaultIconField\":0}\n");
}

} /* namespace testing */
