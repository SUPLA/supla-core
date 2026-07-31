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

#include "AutodiscoverStatisticsTest.h"

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "device/devicechannel.h"
#include "device/devicechannels.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "doubles/device/DeviceDaoMock.h"
#include "doubles/device/DeviceStub.h"
#include "mqtt/mqtt_client_suite.h"
#include "proto.h"
#include "user/user.h"

using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;

namespace testing {
namespace {

supla_channel_em_extended_value *em_value(double power_active,
                                          double power_reactive,
                                          double power_apparent) {
  TElectricityMeter_ExtendedValue_V3 value = {};
  value.m_count = 1;
  value.m[0].power_active[0] = static_cast<int>(power_active * 100000);
  value.m[0].power_reactive[0] = static_cast<int>(power_reactive * 100000);
  value.m[0].power_apparent[0] = static_cast<int>(power_apparent * 100000);

  return new supla_channel_em_extended_value(&value, nullptr, 0);
}

supla_device_channel *channel(supla_device *device, int id, int func,
                              bool offline, double power_active = 0,
                              double power_reactive = 0,
                              double power_apparent = 0) {
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  unsigned _supla_int64_t flags =
      offline ? SUPLA_CHANNEL_FLAG_OFFLINE_DURING_REGISTRATION : 0;

  return new supla_device_channel(
      device, id, static_cast<unsigned char>(id % 255),
      SUPLA_CHANNELTYPE_ELECTRICITY_METER, func, 0, 0, 0, 0, nullptr, nullptr,
      nullptr, false, flags, value, 0,
      em_value(power_active, power_reactive, power_apparent), nullptr, nullptr,
      nullptr);
}

void attach_channels(supla_user *user, shared_ptr<DeviceStub> device,
                     const vector<supla_device_channel *> &channels) {
  DeviceDaoMock dao;
  EXPECT_CALL(dao, get_channels(device.get()))
      .Times(1)
      .WillOnce(Return(channels));

  device->set_channels(
      new supla_device_channels(&dao, device.get(), nullptr, nullptr, 0));
  supla_user::add_device(device, user->getUserID(), nullptr);
}

}  // namespace

void AutodiscoverStatisticsTest::SetUp() {}

void AutodiscoverStatisticsTest::TearDown() {
  supla_mqtt_client_suite::globalInstanceRelease();
}

TEST_F(AutodiscoverStatisticsTest, intervalFromConfig) {
  EXPECT_CALL(task, get_cfg_interval_sec).Times(1).WillOnce(Return(123));
  EXPECT_EQ(task.task_interval_sec(), 123);
}

TEST_F(AutodiscoverStatisticsTest, payload) {
  supla_abstract_autodiscover_statistics::electricity_meter_statistics_t stats =
      {};

  stats.channel_count = 10;
  stats.power_active = 100.25;
  stats.power_reactive = 1000;
  stats.power_apparent = 999.5;

  EXPECT_CALL(task, get_generated_at)
      .Times(1)
      .WillOnce(Return("2020-01-01T00:00:00.000Z"));

  nlohmann::json payload = nlohmann::json::parse(task.get_payload(&stats));
  EXPECT_EQ(payload["generatedAt"], "2020-01-01T00:00:00.000Z");

  nlohmann::json electricity_meters =
      payload["statistics"]["electricityMeters"];

  EXPECT_EQ(electricity_meters["channelCount"], 10);
  EXPECT_DOUBLE_EQ(electricity_meters["powerActive"].get<double>(), 100.25);
  EXPECT_DOUBLE_EQ(electricity_meters["powerReactive"].get<double>(), 1000);
  EXPECT_DOUBLE_EQ(electricity_meters["powerApparent"].get<double>(), 999.5);
}

TEST_F(AutodiscoverStatisticsTest, collectStatisticsFromOnlineChannels) {
  supla_user *user = supla_user::find(678901, true);
  vector<supla_user *> users = {user};
  vector<shared_ptr<DeviceStub>> devices;
  char guid1[SUPLA_GUID_SIZE] = {1};
  char guid2[SUPLA_GUID_SIZE] = {2};

  devices.push_back(make_shared<DeviceStub>(nullptr));
  devices.back()->set_id(101);
  devices.back()->set_guid(guid1);
  devices.back()->set_user(user);
  devices.back()->set_manufacturer_id(123);
  attach_channels(user, devices.back(),
                  {channel(devices.back().get(), 1001,
                           SUPLA_CHANNELFNC_ELECTRICITY_METER, false, 10, 20,
                           30),
                   channel(devices.back().get(), 1002,
                           SUPLA_CHANNELFNC_ELECTRICITY_METER, false, 1, 2, 3),
                   channel(devices.back().get(), 1003,
                           SUPLA_CHANNELFNC_ELECTRICITY_METER, true, 100, 200,
                           300),
                   channel(devices.back().get(), 1004,
                           SUPLA_CHANNELFNC_POWERSWITCH, false, 1000, 2000,
                           3000)});

  devices.push_back(make_shared<DeviceStub>(nullptr));
  devices.back()->set_id(102);
  devices.back()->set_guid(guid2);
  devices.back()->set_user(user);
  devices.back()->set_manufacturer_id(0);
  attach_channels(user, devices.back(),
                  {channel(devices.back().get(), 2001,
                           SUPLA_CHANNELFNC_ELECTRICITY_METER, false, 100, 200,
                           300)});

  supla_abstract_autodiscover_statistics::electricity_meter_statistics_t stats =
      task.collect_statistics(&users);

  EXPECT_EQ(stats.channel_count, 2);
  EXPECT_DOUBLE_EQ(stats.power_active, 0.011);
  EXPECT_DOUBLE_EQ(stats.power_reactive, 0.022);
  EXPECT_DOUBLE_EQ(stats.power_apparent, 0.033);
}

TEST_F(AutodiscoverStatisticsTest, postStatistics) {
  supla_abstract_autodiscover_statistics::electricity_meter_statistics_t stats =
      {};

  stats.channel_count = 10;
  stats.power_active = 100;
  stats.power_reactive = 1000;
  stats.power_apparent = 1000;

  EXPECT_CALL(task, get_generated_at)
      .Times(1)
      .WillOnce(Return("2020-01-01T00:00:00.000Z"));

  EXPECT_CALL(curlAdapter, reset).Times(1);
  EXPECT_CALL(curlAdapter,
              set_opt_url(Eq(0), StrEq(AUTODISCOVER_STATISTICS_ENDPOINT_URL)))
      .Times(1);
  EXPECT_CALL(curlAdapter,
              append_header(Eq(0), StrEq("Content-Type: application/json")))
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(curlAdapter,
              append_header(Eq(0), StrEq("Authorization: Bearer token-x")))
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(curlAdapter, set_opt_post_fields(Eq(0), NotNull()))
      .Times(1)
      .WillOnce([](int instance_id, const char *fields) {
        (void)instance_id;
        nlohmann::json payload = nlohmann::json::parse(fields);
        EXPECT_EQ(payload["generatedAt"], "2020-01-01T00:00:00.000Z");

        nlohmann::json electricity_meters =
            payload["statistics"]["electricityMeters"];

        EXPECT_EQ(electricity_meters["channelCount"], 10);
        EXPECT_DOUBLE_EQ(electricity_meters["powerActive"].get<double>(), 100);
        EXPECT_DOUBLE_EQ(electricity_meters["powerReactive"].get<double>(),
                         1000);
        EXPECT_DOUBLE_EQ(electricity_meters["powerApparent"].get<double>(),
                         1000);
      });
  EXPECT_CALL(curlAdapter, set_opt_write_data(Eq(0), NotNull())).Times(1);
  EXPECT_CALL(curlAdapter, perform(Eq(0))).Times(1).WillOnce(Return(true));
  EXPECT_CALL(curlAdapter, get_response_code(Eq(0)))
      .Times(1)
      .WillOnce(Return(202));

  EXPECT_TRUE(task.post_statistics(&curlAdapter, "token-x", &stats));
}

TEST_F(AutodiscoverStatisticsTest, noToken) {
  EXPECT_CALL(task, get_target_token(IsNull())).Times(1).WillOnce(Return(""));
  EXPECT_CALL(task, get_curl_adapter).Times(0);

  task.run(nullptr, nullptr);
}

TEST_F(AutodiscoverStatisticsTest, runWithNoOnlineChannels) {
  EXPECT_CALL(task, get_target_token(IsNull()))
      .Times(1)
      .WillOnce(Return("token-x"));
  EXPECT_CALL(task, get_curl_adapter).Times(1).WillOnce(Return(&curlAdapter));
  EXPECT_CALL(task, release_curl_adapter(&curlAdapter)).Times(1);
  EXPECT_CALL(task, get_generated_at)
      .Times(1)
      .WillOnce(Return("2020-01-01T00:00:00.000Z"));

  EXPECT_CALL(curlAdapter, reset).Times(1);
  EXPECT_CALL(curlAdapter,
              set_opt_url(Eq(0), StrEq(AUTODISCOVER_STATISTICS_ENDPOINT_URL)))
      .Times(1);
  EXPECT_CALL(curlAdapter, append_header(Eq(0), _))
      .Times(2)
      .WillRepeatedly(Return(true));
  EXPECT_CALL(curlAdapter, set_opt_post_fields(Eq(0), NotNull()))
      .Times(1)
      .WillOnce([](int instance_id, const char *fields) {
        (void)instance_id;
        nlohmann::json payload = nlohmann::json::parse(fields);
        EXPECT_EQ(payload["generatedAt"], "2020-01-01T00:00:00.000Z");

        nlohmann::json electricity_meters =
            payload["statistics"]["electricityMeters"];

        EXPECT_EQ(electricity_meters["channelCount"], 0);
        EXPECT_DOUBLE_EQ(electricity_meters["powerActive"].get<double>(), 0);
        EXPECT_DOUBLE_EQ(electricity_meters["powerReactive"].get<double>(), 0);
        EXPECT_DOUBLE_EQ(electricity_meters["powerApparent"].get<double>(), 0);
      });
  EXPECT_CALL(curlAdapter, set_opt_write_data(Eq(0), NotNull())).Times(1);
  EXPECT_CALL(curlAdapter, perform(Eq(0))).Times(1).WillOnce(Return(true));
  EXPECT_CALL(curlAdapter, get_response_code(Eq(0)))
      .Times(1)
      .WillOnce(Return(200));

  task.run(nullptr, nullptr);
}

}  // namespace testing
