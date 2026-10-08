// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "actions/action_executor.h"
#include "analyzer/electricity_analyzer.h"
#include "datalogger/current_logger.h"
#include "datalogger/power_active_logger.h"
#include "datalogger/total_energy_logger.h"
#include "datalogger/voltage_aberration_logger.h"
#include "datalogger/voltage_logger.h"
#include "db/mariadb_access_provider.h"
#include "device/device_dao.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "device/value/channel_onoff_value.h"
#include "integration/IntegrationTest.h"
#include "jsonconfig/channel/electricity_meter_config.h"
#include "ocpp/ocpp_dao.h"
#include "ocpp/ocpp_device.h"
#include "ocpp/ocpp_gateway.h"
#include "sthread.h"
#include "user/user.h"
#include "user/user_dao.h"
#include "user/userchannelgroups.h"
#include "user/virtualchannel.h"
#include "vbt/value_based_triggers.h"

namespace testing {
namespace {
double meter_energy(supla_abstract_channel_extended_value *value) {
  std::unique_ptr<supla_abstract_channel_extended_value> owned(value);
  auto meter = dynamic_cast<supla_channel_em_extended_value *>(owned.get());
  EXPECT_NE(nullptr, meter);
  if (meter) {
    EXPECT_DOUBLE_EQ(0, meter->get_fae(2));
    EXPECT_DOUBLE_EQ(0, meter->get_fae(3));
    EXPECT_DOUBLE_EQ(0, meter->get_fae_balanced());
    EXPECT_FALSE(meter->get_measured_values() &
                 EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED);
  }
  return meter ? meter->get_fae_sum() : -1;
}
double logged_energy(const supla_ocpp_channel *channel) {
  return meter_energy(channel->get_extended_value(true));
}
double displayed_energy(const supla_ocpp_channel *channel) {
  return meter_energy(channel->get_extended_value());
}
double raw_energy(const supla_ocpp_channel *channel) {
  return meter_energy(channel->get_raw_extended_value());
}

class recording_value_based_triggers : public supla_value_based_triggers {
 public:
  std::vector<std::pair<bool, bool>> switch_changes;

  explicit recording_value_based_triggers(supla_user *user)
      : supla_value_based_triggers(user) {}

  void on_value_changed(const supla_caller &, int channel_id,
                        supla_vbt_value *old_value, supla_vbt_value *new_value,
                        supla_abstract_action_executor *,
                        supla_abstract_channel_property_getter *) override {
    double before = 0;
    double after = 0;
    if (channel_id == 140 && old_value->get_vbt_value(var_name_none, &before) &&
        new_value->get_vbt_value(var_name_none, &after)) {
      switch_changes.emplace_back(before != 0, after != 0);
    }
  }
};

class ocpp_test_user : public supla_user {
 public:
  ocpp_test_user(int user_id, const char *short_unique_id)
      : supla_user(user_id, short_unique_id, nullptr) {}

  recording_value_based_triggers *install_trigger_recorder() {
    delete value_based_triggers;
    auto recorder = new recording_value_based_triggers(this);
    value_based_triggers = recorder;
    return recorder;
  }
};

bool switch_is_on(supla_user *user) {
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  EXPECT_TRUE(user->get_devices()->get_ocpp_channel(140).get_value(value));
  return supla_channel_onoff_value(value).is_on();
}

bool receive_gateway_command(int socket_fd, nlohmann::json *message) {
  pollfd descriptor = {socket_fd, POLLIN, 0};
  if (poll(&descriptor, 1, 1000) != 1) return false;
  uint32_t size = 0;
  if (recv(socket_fd, &size, sizeof(size), MSG_WAITALL) != sizeof(size))
    return false;
  size = ntohl(size);
  if (!size || size >= 1024) return false;
  std::string payload(size, '\0');
  if (recv(socket_fd, &payload[0], size, MSG_WAITALL) != size) return false;
  *message = nlohmann::json::parse(payload, nullptr, false);
  return !message->is_discarded();
}

class ocpp_ipc_test_session {
 private:
  int peer;
  std::shared_ptr<supla_ocpp_connection> receiver;
  void *thread = nullptr;

 public:
  supla_ocpp_connection sender;

  explicit ocpp_ipc_test_session(const int sockets[2])
      : peer(sockets[1]),
        receiver(std::make_shared<supla_ocpp_connection>(sockets[0])),
        sender(sockets[1]) {
    supla_ocpp_gateway::global_instance()->configure(true, 65536);
    sthread_simple_run(
        [](void *data, void *thread) {
          static_cast<supla_ocpp_connection *>(data)->execute(thread);
        },
        receiver.get(), 0, &thread);
  }

  ~ocpp_ipc_test_session() {
    shutdown(peer, SHUT_RDWR);
    sthread_wait(thread);
    sthread_free(thread);
    supla_ocpp_gateway::global_instance()->wait_until_idle();
    supla_ocpp_gateway::global_instance()->configure(false, 65536);
  }

  bool send_frames(const std::string &frames) {
    size_t offset = 0;
    while (offset < frames.size()) {
      auto sent = send(peer, frames.data() + offset, frames.size() - offset,
                       MSG_NOSIGNAL);
      if (sent <= 0) return false;
      offset += sent;
    }
    return true;
  }

  bool wait_for_final_state(supla_user *user) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (std::chrono::steady_clock::now() < deadline) {
      char value[SUPLA_CHANNELVALUE_SIZE] = {};
      if (user->get_devices()->get_ocpp_channel(140).get_value(value) &&
          value[0])
        return true;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return false;
  }
};

class synchronous_ocpp_gateway : public supla_ocpp_gateway {
 public:
  void on_result(unsigned long long id, bool ok, const std::string &error) {
    supla_ocpp_gateway::on_result(id, ok, error);
    wait_until_idle();
  }

  void on_connected(const std::string &user_suid, int device_id,
                    unsigned int validity) {
    supla_ocpp_gateway::on_connected(user_suid, device_id, validity);
    wait_until_idle();
  }

  void on_disconnected(const std::string &user_suid, int device_id) {
    supla_ocpp_gateway::on_disconnected(user_suid, device_id);
    wait_until_idle();
  }

  void on_alive(const std::string &user_suid, int device_id,
                unsigned int validity) {
    supla_ocpp_gateway::on_alive(user_suid, device_id, validity);
    wait_until_idle();
  }

  void on_state(const std::string &user_suid, int device_id, int connector_id,
                bool on, unsigned int validity) {
    supla_ocpp_gateway::on_state(user_suid, device_id, connector_id, on,
                                 validity);
    wait_until_idle();
  }

  void on_meter(const std::string &user_suid, int device_id, int connector_id,
                const nlohmann::json &message, unsigned int validity) {
    supla_ocpp_gateway::on_meter(user_suid, device_id, connector_id, message,
                                 validity);
    wait_until_idle();
  }
};
}  // namespace

class OcppLoggerIntegrationTest : public IntegrationTest, public Test {
 protected:
  supla_mariadb_access_provider dba;
  std::unique_ptr<ocpp_test_user> user;
  synchronous_ocpp_gateway gateway;
  const char *suid = "ocpp-logger-test";

  void SetUp() override {
    initTestDatabase();
    ASSERT_TRUE(dba.connect());
    ASSERT_EQ(
        0, dba.query("INSERT INTO supla_ocpp_charging_station VALUES (1,2,73)",
                     true));
    std::string switch_sql =
        "UPDATE supla_dev_channel SET is_virtual=1,type=" +
        std::to_string(SUPLA_CHANNELTYPE_RELAY) +
        ",func=" + std::to_string(SUPLA_CHANNELFNC_POWERSWITCH) +
        " WHERE id=140";
    ASSERT_EQ(0, dba.query(switch_sql.c_str(), true));
    std::string sql =
        "UPDATE supla_dev_channel SET is_virtual=1,type=" +
        std::to_string(SUPLA_CHANNELTYPE_ELECTRICITY_METER) +
        ",func=" + std::to_string(SUPLA_CHANNELFNC_ELECTRICITY_METER) +
        R"(,user_config='{"voltageLoggerEnabled":true,"currentLoggerEnabled":true,"powerActiveLoggerEnabled":true,"lowerVoltageThreshold":210,"upperVoltageThreshold":235}',properties='{"countersAvailable":["forwardActiveEnergy"]}' WHERE id=141)";
    ASSERT_EQ(0, dba.query(sql.c_str(), true));
    ASSERT_EQ(
        0, dba.query("DELETE FROM supla_dev_channel_value WHERE channel_id=141",
                     true));
    ASSERT_EQ(0, dba.query("DELETE FROM supla_dev_channel_extended_value WHERE "
                           "channel_id=141",
                           true));
    supla_user::user_free();
    supla_user::init();
    user.reset(new ocpp_test_user(2, suid));
    gateway.on_connected(suid, 73, 90);
  }

  void run_logger(supla_abstract_cyclictask *logger) {
    std::vector<supla_user *> users = {user.get()};
    timeval now = {};
    gettimeofday(&now, nullptr);
    logger->run(&now, &users, &dba);
  }

  void expect_query(const char *query, const char *expected) {
    std::string result;
    sqlQuery(query, &result);
    EXPECT_EQ(expected, result);
  }

  void expect_meter_values(double displayed, double raw, double history) {
    auto channel = user->get_devices()->get_ocpp_channel(141);
    EXPECT_DOUBLE_EQ(displayed, displayed_energy(&channel));
    EXPECT_DOUBLE_EQ(raw, raw_energy(&channel));
    EXPECT_DOUBLE_EQ(history, logged_energy(&channel));
    supla_device_dao dao(&dba);
    EXPECT_DOUBLE_EQ(displayed,
                     meter_energy(dao.get_channel_extended_value(2, 141)));
  }
};

TEST_F(OcppLoggerIntegrationTest,
       SwitchValueBasedTriggersWaitForConfirmedState) {
  gateway.on_state(suid, 73, 1, false, 90);
  auto recorder = user->install_trigger_recorder();
  auto device = user->get_devices()->get_ocpp_device(73);
  ASSERT_NE(nullptr, device);

  ASSERT_TRUE(device->request_charging(
      1, true, false, [](bool requested) { return requested; }));
  EXPECT_TRUE(switch_is_on(user.get()));
  EXPECT_TRUE(recorder->switch_changes.empty());
  device->on_result(1, false);
  EXPECT_FALSE(switch_is_on(user.get()));
  EXPECT_TRUE(recorder->switch_changes.empty());

  ASSERT_TRUE(device->request_charging(
      2, true, false, [](bool requested) { return requested; }));
  EXPECT_TRUE(recorder->switch_changes.empty());
  device->on_result(2, true);
  EXPECT_TRUE(recorder->switch_changes.empty());

  device->update_state(true, 90);
  ASSERT_EQ(1U, recorder->switch_changes.size());
  EXPECT_EQ(std::make_pair(false, true), recorder->switch_changes[0]);

  device->update_state(true, 90);
  EXPECT_EQ(1U, recorder->switch_changes.size());
  ASSERT_TRUE(device->request_charging(
      3, false, false, [](bool requested) { return !requested; }));
  EXPECT_EQ(1U, recorder->switch_changes.size());
  device->update_state(false, 90);
  ASSERT_EQ(2U, recorder->switch_changes.size());
  EXPECT_EQ(std::make_pair(true, false), recorder->switch_changes[1]);
  device->on_result(3, true);
  EXPECT_EQ(2U, recorder->switch_changes.size());
}

TEST_F(OcppLoggerIntegrationTest, StoresAdjustedValuesButLogsRawByDefault) {
  ASSERT_EQ(
      0,
      dba.query(
          R"(UPDATE supla_dev_channel SET user_config='{"electricityMeterInitialValues":{"forwardActiveEnergy":5}}' WHERE id=141)",
          true));
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  expect_meter_values(17, 12, 12);
  expect_query(
      "SELECT JSON_UNQUOTE(JSON_EXTRACT(properties, '$.countersAvailable[0]')) "
      "counter "
      "FROM supla_dev_channel WHERE id=141",
      "counter\nforwardActiveEnergy\n");

  for (int i = 0; i < 2; i++) {
    user->get_devices()->reload_ocpp_devices(73);
    gateway.on_alive(suid, 73, 90);
    gateway.on_meter(suid, 73, 1, {{"voltage", {230000}}}, 90);
    expect_meter_values(17, 12, 12);
  }
  supla_total_energy_logger logger;
  run_logger(&logger);
  expect_query(
      "SELECT phase1_fae,phase2_fae,phase3_fae,fae_balanced FROM supla_em_log",
      "phase1_fae\tphase2_fae\tphase3_fae\tfae_"
      "balanced\n1200000\tNULL\tNULL\tNULL\n");
}

TEST_F(OcppLoggerIntegrationTest, StoresRealPhaseEnergyWithoutBalancedCounter) {
  gateway.on_meter(suid, 73, 1, {{"energy_phases", {1000, 2000, 3000}}}, 90);
  auto channel = user->get_devices()->get_ocpp_channel(141);
  std::unique_ptr<supla_abstract_channel_extended_value> extended(
      channel.get_extended_value());
  auto meter = dynamic_cast<supla_channel_em_extended_value *>(extended.get());
  ASSERT_NE(nullptr, meter);
  EXPECT_DOUBLE_EQ(1, meter->get_fae(1));
  EXPECT_DOUBLE_EQ(2, meter->get_fae(2));
  EXPECT_DOUBLE_EQ(3, meter->get_fae(3));
  EXPECT_DOUBLE_EQ(0, meter->get_fae_balanced());
  supla_total_energy_logger logger;
  run_logger(&logger);
  expect_query(
      "SELECT phase1_fae,phase2_fae,phase3_fae,fae_balanced FROM supla_em_log",
      "phase1_fae\tphase2_fae\tphase3_fae\tfae_balanced\n"
      "100000\t200000\t300000\tNULL\n");
}

TEST_F(OcppLoggerIntegrationTest, HistorySelectsRawOrAdjustedValue) {
  ASSERT_EQ(
      0,
      dba.query(
          R"(UPDATE supla_dev_channel SET user_config='{"addToHistory":true,"electricityMeterInitialValues":{"forwardActiveEnergy":5}}' WHERE id=141)",
          true));
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  expect_meter_values(17, 12, 17);
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_meter(suid, 73, 1, {{"current", {16000}}}, 90);
  expect_meter_values(17, 12, 17);
  supla_total_energy_logger logger;
  run_logger(&logger);
  expect_query(
      "SELECT phase1_fae,phase2_fae,phase3_fae,fae_balanced FROM supla_em_log",
      "phase1_fae\tphase2_fae\tphase3_fae\tfae_"
      "balanced\n1700000\tNULL\tNULL\tNULL\n");

  ASSERT_EQ(
      0,
      dba.query(
          R"(UPDATE supla_dev_channel SET user_config='{"addToHistory":false,"electricityMeterInitialValues":{"forwardActiveEnergy":8}}' WHERE id=141)",
          true));
  user->get_devices()->reload_ocpp_devices(73);
  // Reload recalculates from the preserved raw counter without a new sample.
  expect_meter_values(20, 12, 12);
  gateway.on_meter(suid, 73, 1, {{"voltage", {240000}}}, 90);
  expect_meter_values(20, 12, 12);
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  expect_meter_values(20, 12, 12);
}

TEST_F(OcppLoggerIntegrationTest, RestartMergesOnlyReceivedFields) {
  ASSERT_EQ(
      0,
      dba.query(
          R"(UPDATE supla_dev_channel SET user_config=JSON_SET(user_config,'$.electricityMeterInitialValues',JSON_OBJECT('forwardActiveEnergy',5)) WHERE id=141)",
          true));
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_meter(suid, 73, 1,
                   {{"energy", 12000},
                    {"voltage", {230000, 231000, 232000}},
                    {"current", {10000, 11000, 12000}},
                    {"power_phases", {7500}}},
                   90);
  expect_meter_values(17, 12, 12);

  // Simulate a restart: only the persisted DB values survive, not the raw
  // readings
  // or pending statistical samples. Null phases must not become zeroes.
  user->get_devices()->on_channel_deleted(73, 141);
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_meter(suid, 73, 1, {{"voltage", {nullptr, 240000}}}, 90);
  auto channel = user->get_devices()->get_ocpp_channel(141);
  EXPECT_EQ(nullptr, channel.get_raw_extended_value());
  EXPECT_EQ(nullptr, channel.get_extended_value(true));
  auto check_readings = [](supla_abstract_channel_extended_value *value,
                           double energy, double power) {
    std::unique_ptr<supla_abstract_channel_extended_value> owned(value);
    auto meter = dynamic_cast<supla_channel_em_extended_value *>(owned.get());
    ASSERT_NE(nullptr, meter);
    EXPECT_DOUBLE_EQ(energy, meter->get_fae_sum());
    EXPECT_DOUBLE_EQ(230, meter->get_voltage(1));
    EXPECT_DOUBLE_EQ(240, meter->get_voltage(2));
    EXPECT_DOUBLE_EQ(232, meter->get_voltage(3));
    EXPECT_DOUBLE_EQ(10, meter->get_current(1));
    EXPECT_DOUBLE_EQ(11, meter->get_current(2));
    EXPECT_DOUBLE_EQ(12, meter->get_current(3));
    EXPECT_DOUBLE_EQ(power, meter->get_power_active(1));
  };
  check_readings(channel.get_extended_value(), 17, 7500);
  supla_device_dao dao(&dba);
  check_readings(dao.get_channel_extended_value(2, 141), 17, 7500);

  // Zero is a real measurement; omission means keep the previous value.
  gateway.on_meter(suid, 73, 1, {{"energy", 13000}, {"power_phases", {0}}}, 90);
  channel = user->get_devices()->get_ocpp_channel(141);
  check_readings(channel.get_extended_value(), 18, 0);
  check_readings(channel.get_raw_extended_value(), 13, 0);
  check_readings(channel.get_extended_value(true), 13, 0);
  gateway.on_meter(suid, 73, 1, {{"power_phases", {100}}}, 90);
  channel = user->get_devices()->get_ocpp_channel(141);
  check_readings(channel.get_extended_value(), 18, 100);
  check_readings(channel.get_raw_extended_value(), 13, 100);

  int visits = 0;
  user->get_devices()->access_ocpp_data_analyzers(
      [&visits](supla_electricity_analyzer *analyzer) {
        visits++;
        EXPECT_EQ(nullptr, analyzer->get_voltage_phase1());
        EXPECT_EQ(nullptr, analyzer->get_voltage_phase3());
        EXPECT_EQ(nullptr, analyzer->get_current_phase1());
        ASSERT_NE(nullptr, analyzer->get_voltage_phase2());
        EXPECT_EQ(1U, analyzer->get_voltage_phase2()->get_sample_count());
      });
  EXPECT_EQ(1, visits);
}

TEST_F(OcppLoggerIntegrationTest, MissingLoadedChannelIsNotRecreatedOrWritten) {
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  user->get_devices()->on_channel_deleted(73, 141);
  ASSERT_EQ(0, user->get_devices()->get_ocpp_channel(141).get_channel_id());
  ASSERT_EQ(0, dba.query("UPDATE supla_dev_channel_value SET "
                         "valid_to='2020-01-01 00:00:00' "
                         "WHERE channel_id IN (140,141)",
                         true));
  gateway.on_meter(suid, 73, 1, {{"energy", 99000}, {"voltage", {240000}}}, 90);
  EXPECT_EQ(0, user->get_devices()->get_ocpp_channel(141).get_channel_id());
  supla_device_dao dao(&dba);
  EXPECT_DOUBLE_EQ(12, meter_energy(dao.get_channel_extended_value(2, 141)));
  expect_query(
      "SELECT COUNT(*) n FROM supla_dev_channel_value "
      "WHERE channel_id IN (140,141) AND "
      "valid_to<>'2020-01-01 00:00:00'",
      "n\n0\n");
}

TEST_F(OcppLoggerIntegrationTest, RestartWaitsForRawEnergyBeforeLogging) {
  ASSERT_EQ(
      0,
      dba.query(
          R"(UPDATE supla_dev_channel SET user_config='{"addToHistory":true,"electricityMeterInitialValues":{"forwardActiveEnergy":5}}' WHERE id=141)",
          true));
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  expect_meter_values(17, 12, 17);

  // Discard runtime readings and load only the adjusted value from the DB.
  user->get_devices()->on_channel_deleted(73, 141);
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_alive(suid, 73, 90);
  gateway.on_meter(suid, 73, 1, {{"voltage", {230000}}}, 90);
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_meter(suid, 73, 1, {{"current", {16000}}}, 90);
  auto channel = user->get_devices()->get_ocpp_channel(141);
  EXPECT_DOUBLE_EQ(17, displayed_energy(&channel));
  EXPECT_EQ(nullptr, channel.get_raw_extended_value());
  EXPECT_EQ(nullptr, channel.get_extended_value(true));
  supla_total_energy_logger logger;
  run_logger(&logger);
  expect_query("SELECT COUNT(*) n FROM supla_em_log", "n\n0\n");

  gateway.on_meter(suid, 73, 1, {{"energy", 13000}}, 90);
  expect_meter_values(18, 13, 18);
}

TEST_F(OcppLoggerIntegrationTest, PartialReportDoesNotInventInitialEnergy) {
  ASSERT_EQ(
      0,
      dba.query(
          R"(UPDATE supla_dev_channel SET user_config='{"addToHistory":true,"electricityMeterInitialValues":{"forwardActiveEnergy":5}}' WHERE id=141)",
          true));
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_meter(suid, 73, 1, {{"voltage", {230000}}}, 90);
  auto channel = user->get_devices()->get_ocpp_channel(141);
  EXPECT_EQ(nullptr, channel.get_value());
  EXPECT_DOUBLE_EQ(0, displayed_energy(&channel));
  EXPECT_EQ(nullptr, channel.get_raw_extended_value());
  EXPECT_EQ(nullptr, channel.get_extended_value(true));
  expect_query(
      "SELECT COUNT(*) n FROM supla_dev_channel_value "
      "WHERE channel_id=141",
      "n\n0\n");
  gateway.on_meter(suid, 73, 1, {{"energy", 0}}, 90);
  expect_meter_values(5, 0, 5);
}

TEST_F(OcppLoggerIntegrationTest,
       NegativeOffsetSurvivesRestartAndPartialReports) {
  ASSERT_EQ(
      0,
      dba.query(
          R"(UPDATE supla_dev_channel SET user_config='{"electricityMeterInitialValues":{"forwardActiveEnergy":-20}}' WHERE id=141)",
          true));
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  expect_meter_values(0, 12, 12);
  user->get_devices()->on_channel_deleted(73, 141);
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_meter(suid, 73, 1, {{"voltage", {230000}}}, 90);
  auto channel = user->get_devices()->get_ocpp_channel(141);
  EXPECT_DOUBLE_EQ(0, displayed_energy(&channel));
  EXPECT_EQ(nullptr, channel.get_raw_extended_value());
  EXPECT_EQ(nullptr, channel.get_extended_value(true));
  gateway.on_meter(suid, 73, 1, {{"energy", 13000}}, 90);
  expect_meter_values(0, 13, 13);
}

TEST_F(OcppLoggerIntegrationTest, PropertiesChangeOnlyWhenReloaded) {
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  ASSERT_EQ(
      0,
      dba.query(
          R"(UPDATE supla_dev_channel SET user_config=JSON_SET(user_config,'$.electricityMeterInitialValues',JSON_OBJECT('forwardActiveEnergy',5)),properties=JSON_SET(properties,'$.testProperty',true) WHERE id=141)",
          true));
  gateway.on_meter(suid, 73, 1, {{"voltage", {230000}}}, 90);
  expect_meter_values(12, 12, 12);
  // The same IPC path used by Cloud must reload configuration immediately.
  user->get_devices()->on_channel_config_changed(73, 141);
  expect_meter_values(17, 12, 12);
  expect_query(
      "SELECT JSON_EXTRACT(properties,'$.testProperty') p FROM "
      "supla_dev_channel WHERE id=141",
      "p\ntrue\n");
}

TEST_F(OcppLoggerIntegrationTest, DataSourceReloadUsesDatabaseValidity) {
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  gateway.on_disconnected(suid, 73);
  ASSERT_EQ(0, dba.query("DELETE FROM supla_ocpp_charging_station", true));
  user->get_devices()->reload_ocpp_devices(73);
  for (bool online : {true, false, true}) {
    // The ordinary updater selects records newer than its previous poll.
    // Advance the fixture timestamp without sleeping between database writes.
    ASSERT_EQ(0, dba.query(online ? "UPDATE supla_dev_channel_value SET "
                                    "update_time=UTC_TIMESTAMP()+INTERVAL 1 SECOND,"
                                    "valid_to=UTC_TIMESTAMP()+INTERVAL 90 "
                                    "SECOND WHERE channel_id=141"
                                  : "UPDATE supla_dev_channel_value SET "
                                    "update_time=UTC_TIMESTAMP()+INTERVAL 1 SECOND,"
                                    "valid_to=UTC_TIMESTAMP()-INTERVAL 1 "
                                    "SECOND WHERE channel_id=141",
                           true));
    user->get_devices()->update_virtual_channels();
    auto source = user->get_devices()->get_virtual_channel(141);
    EXPECT_EQ(141, source.get_channel_id());
    EXPECT_EQ(online, source.get_availability_status().is_online());
    EXPECT_EQ(0, user->get_devices()->get_ocpp_channel(141).get_channel_id());
  }
}

TEST_F(OcppLoggerIntegrationTest, ReplayRestoresValuesWithoutAnalyzerSamples) {
  const nlohmann::json snapshot = {{"energy", 12000},
                                   {"voltage", {230000}},
                                   {"current", {nullptr, 16000}},
                                   {"power_phases", {1000, 2000, 3000}},
                                   {"snapshot", true}};
  gateway.on_meter(suid, 73, 1, snapshot, 60);
  expect_meter_values(12, 12, 12);
  auto device = user->get_devices()->get_ocpp_device(73);
  device->access_data_analyzer([](supla_electricity_analyzer *a) {
    EXPECT_FALSE(a->is_any_data_for_logging_purpose());
  });
  gateway.on_meter(suid, 73, 1, {{"voltage", {240000}}}, 60);
  gateway.on_meter(suid, 73, 1, snapshot, 60);
  device->access_data_analyzer([](supla_electricity_analyzer *a) {
    ASSERT_NE(nullptr, a->get_voltage_phase1());
    EXPECT_EQ(1U, a->get_voltage_phase1()->get_sample_count());
    EXPECT_EQ(nullptr, a->get_current_phase2());
    EXPECT_FALSE(a->is_any_power_active_for_logging_purpose());
  });
}

TEST_F(OcppLoggerIntegrationTest,
       ConsumesGoMeterFramesWithoutReplayingSamples) {
  const char *path = getenv("SUPLA_OCPP_CONTRACT_INPUT");
  if (!path)
    GTEST_SKIP()
        << "Set SUPLA_OCPP_CONTRACT_INPUT to Go TestMeterIPCContract output";
  std::ifstream input(path, std::ios::binary);
  ASSERT_TRUE(input.is_open());
  std::string frames((std::istreambuf_iterator<char>(input)), {});
  ASSERT_FALSE(frames.empty());
  ASSERT_LT(frames.size(), 65536U);
  gateway.on_state(suid, 73, 1, false, 90);
  int sockets[2];
  ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
  ocpp_ipc_test_session session(sockets);
  ASSERT_TRUE(session.send_frames(frames));
  ASSERT_TRUE(session.wait_for_final_state(user.get()));
  expect_meter_values(12, 12, 12);
  auto device = user->get_devices()->get_ocpp_device(73);
  std::unique_ptr<supla_abstract_channel_extended_value> extended(
      device->get_channel(141).get_extended_value());
  auto meter = dynamic_cast<supla_channel_em_extended_value *>(extended.get());
  ASSERT_NE(nullptr, meter);
  EXPECT_DOUBLE_EQ(240, meter->get_voltage(1));
  EXPECT_DOUBLE_EQ(16, meter->get_current(2));
  EXPECT_DOUBLE_EQ(1000, meter->get_power_active(1));
  EXPECT_DOUBLE_EQ(2500, meter->get_power_active(2));
  EXPECT_DOUBLE_EQ(3000, meter->get_power_active(3));
  device->access_data_analyzer([](supla_electricity_analyzer *a) {
    ASSERT_NE(nullptr, a->get_voltage_phase1());
    EXPECT_EQ(1U, a->get_voltage_phase1()->get_sample_count());
    EXPECT_FALSE(a->is_any_current_for_logging_purpose());
    EXPECT_EQ(nullptr, a->get_power_active_phase1());
    EXPECT_EQ(nullptr, a->get_power_active_phase3());
    ASSERT_NE(nullptr, a->get_power_active_phase2());
    EXPECT_EQ(1U, a->get_power_active_phase2()->get_sample_count());
  });
  supla_voltage_logger voltage;
  supla_current_logger current;
  supla_power_active_logger power;
  supla_voltage_aberration_logger aberration;
  run_logger(&voltage);
  run_logger(&current);
  run_logger(&power);
  run_logger(&aberration);
  expect_query("SELECT phase_no,min,max,avg FROM supla_em_voltage_log",
               "phase_no\tmin\tmax\tavg\n1\t240.00\t240.00\t240.00\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_current_log", "n\n0\n");
  expect_query(
      "SELECT phase_no,min,max,avg FROM supla_em_power_active_log",
      "phase_no\tmin\tmax\tavg\n2\t2500.00000\t2500.00000\t2500.00000\n");
  expect_query(
      "SELECT phase_no,count_total FROM supla_em_voltage_aberration_log",
      "phase_no\tcount_total\n1\t1\n");
}

TEST_F(OcppLoggerIntegrationTest, RejectsInvalidSnapshotAndPhaseArraysOnWire) {
  gateway.on_state(suid, 73, 1, false, 90);
  int sockets[2];
  ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
  ocpp_ipc_test_session session(sockets);
  ASSERT_TRUE(session.sender.send_message(
      {{"type", "hello"}, {"version", 1}, {"validity", 90}}));
  const nlohmann::json base = {{"type", "meter"},
                               {"user", suid},
                               {"device", 73},
                               {"connector", 1},
                               {"energy", 12000}};
  for (const auto &invalid :
       {nlohmann::json{{"snapshot", 1}}, nlohmann::json{{"snapshot", nullptr}},
        nlohmann::json{{"power_phases", 1000}},
        nlohmann::json{{"power_phases", {-1}}},
        nlohmann::json{{"power_phases", {21474837}}},
        nlohmann::json{{"power_phases", {1, 2, 3, 4}}},
        nlohmann::json{{"energy_phases", 1000}},
        nlohmann::json{{"energy_phases", {-1}}},
        nlohmann::json{{"energy_phases", {92233720368547759LL}}},
        nlohmann::json{{"energy_phases", {1, 2, 3, 4}}}}) {
    auto message = base;
    message.update(invalid);
    ASSERT_TRUE(session.sender.send_message(message));
  }
  ASSERT_TRUE(session.sender.send_message({{"type", "state"},
                                           {"user", suid},
                                           {"device", 73},
                                           {"connector", 1},
                                           {"on", true}}));
  ASSERT_TRUE(session.wait_for_final_state(user.get()));
  EXPECT_EQ(nullptr, user->get_devices()->get_ocpp_channel(141).get_value());
}

TEST_F(OcppLoggerIntegrationTest, AggregatePowerIsNotAPhaseMeasurement) {
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}, {"power", 9000}}, 90);
  auto device = user->get_devices()->get_ocpp_device(73);
  std::unique_ptr<supla_abstract_channel_extended_value> extended(
      device->get_channel(141).get_extended_value());
  auto meter = dynamic_cast<supla_channel_em_extended_value *>(extended.get());
  ASSERT_NE(nullptr, meter);
  EXPECT_FALSE(meter->get_measured_values() & EM_VAR_POWER_ACTIVE);
  device->access_data_analyzer([](supla_electricity_analyzer *a) {
    EXPECT_FALSE(a->is_any_power_active_for_logging_purpose());
  });
  gateway.on_meter(suid, 73, 1,
                   {{"power", 9000}, {"power_phases", {1000, 2000, 3000}}}, 90);
  gateway.on_meter(suid, 73, 1, {{"power_phases", {nullptr, 0}}}, 90);
  extended.reset(device->get_channel(141).get_extended_value());
  meter = dynamic_cast<supla_channel_em_extended_value *>(extended.get());
  ASSERT_NE(nullptr, meter);
  EXPECT_DOUBLE_EQ(1000, meter->get_power_active(1));
  EXPECT_DOUBLE_EQ(0, meter->get_power_active(2));
  EXPECT_DOUBLE_EQ(3000, meter->get_power_active(3));
  device->access_data_analyzer([](supla_electricity_analyzer *a) {
    ASSERT_NE(nullptr, a->get_power_active_phase1());
    ASSERT_NE(nullptr, a->get_power_active_phase2());
    ASSERT_NE(nullptr, a->get_power_active_phase3());
    EXPECT_EQ(1U, a->get_power_active_phase1()->get_sample_count());
    EXPECT_EQ(2U, a->get_power_active_phase2()->get_sample_count());
    EXPECT_EQ(1U, a->get_power_active_phase3()->get_sample_count());
  });
}

TEST_F(OcppLoggerIntegrationTest, LogsFreshSamplesThroughExistingLoggers) {
  gateway.on_meter(suid, 73, 1,
                   {{"energy", 12000},
                    {"voltage", {230000}},
                    {"current", {nullptr, 16000}},
                    {"power_phases", {3680}}},
                   90);
  gateway.on_meter(suid, 73, 1, {{"voltage", {240000}}}, 90);
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  gateway.on_alive(suid, 73, 90);
  user->get_devices()->reload_ocpp_devices(73);
  // Heartbeats, partial readings and DB reloads must not duplicate samples.
  int visits = 0;
  user->get_devices()->access_ocpp_data_analyzers(
      [&visits](supla_electricity_analyzer *a) {
        visits++;
        ASSERT_NE(nullptr, a->get_voltage_phase1());
        EXPECT_EQ(2U, a->get_voltage_phase1()->get_sample_count());
        ASSERT_NE(nullptr, a->get_current_phase2());
        EXPECT_EQ(1U, a->get_current_phase2()->get_sample_count());
        EXPECT_EQ(nullptr, a->get_current_phase1());
        EXPECT_EQ(nullptr, a->get_voltage_phase2());
      });
  EXPECT_EQ(1, visits);

  supla_total_energy_logger energy;
  supla_voltage_logger voltage;
  supla_current_logger current;
  supla_power_active_logger power;
  supla_voltage_aberration_logger aberration;
  run_logger(&energy);
  run_logger(&voltage);
  run_logger(&current);
  run_logger(&power);
  run_logger(&aberration);
  expect_query(
      "SELECT channel_id,phase1_fae,phase2_fae,phase3_fae,fae_balanced FROM "
      "supla_em_log",
      "channel_id\tphase1_fae\tphase2_fae\tphase3_fae\tfae_"
      "balanced\n141\t1200000\tNULL\tNULL\tNULL\n");
  expect_query(
      "SELECT channel_id,phase_no,min,max,avg FROM supla_em_voltage_log",
      "channel_id\tphase_no\tmin\tmax\tavg\n141\t1\t230.00\t240.00\t235.00\n");
  expect_query(
      "SELECT channel_id,phase_no,min,max,avg FROM supla_em_current_log",
      "channel_id\tphase_no\tmin\tmax\tavg\n141\t2\t16.000\t16.000\t16.000\n");
  expect_query(
      "SELECT channel_id,phase_no,min,max,avg FROM supla_em_power_active_log",
      "channel_id\tphase_no\tmin\tmax\tavg\n141\t1\t3680.00000\t3680."
      "00000\t3680.00000\n");
  expect_query(
      "SELECT channel_id,phase_no,count_total FROM "
      "supla_em_voltage_aberration_log",
      "channel_id\tphase_no\tcount_total\n141\t1\t2\n");
  // Each logger consumes only its own statistics, exactly once.
  run_logger(&voltage);
  run_logger(&current);
  run_logger(&power);
  run_logger(&aberration);
  expect_query("SELECT COUNT(*) n FROM supla_em_voltage_log", "n\n1\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_current_log", "n\n1\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_power_active_log", "n\n1\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_voltage_aberration_log",
               "n\n1\n");
}

TEST_F(OcppLoggerIntegrationTest, SkipsUnknownAndOfflineMeters) {
  supla_total_energy_logger energy;
  supla_voltage_logger voltage;
  gateway.on_meter(suid, 73, 1, {{"voltage", {230000}}}, 90);
  gateway.on_alive(suid, 73, 90);
  run_logger(&energy);
  run_logger(&voltage);
  expect_query("SELECT COUNT(*) n FROM supla_em_log", "n\n0\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_voltage_log", "n\n0\n");
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}, {"voltage", {240000}}}, 90);
  gateway.on_disconnected(suid, 73);
  run_logger(&energy);
  run_logger(&voltage);
  expect_query("SELECT COUNT(*) n FROM supla_em_log", "n\n0\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_voltage_log", "n\n0\n");
  gateway.on_connected(suid, 73, 90);
  gateway.on_meter(suid, 73, 1, {{"voltage", {220000}}}, 90);
  run_logger(&voltage);
  expect_query("SELECT phase_no,min,max,avg FROM supla_em_voltage_log",
               "phase_no\tmin\tmax\tavg\n1\t220.00\t220.00\t220.00\n");
}

TEST_F(OcppLoggerIntegrationTest, ReloadAndSnapshotsKeepPendingSamples) {
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}, {"voltage", {230000}}}, 90);
  auto channel = user->get_devices()->get_ocpp_channel(141);
  channel = supla_ocpp_channel();
  ASSERT_EQ(0, dba.query("UPDATE supla_dev_channel SET user_config="
                         "JSON_SET(user_config,'$.pricePerUnit',0.65),"
                         "properties='{\"testProperty\":true}' WHERE id=141",
                         true));
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_meter(suid, 73, 1, {{"voltage", {240000}}}, 90);
  supla_voltage_logger logger;
  run_logger(&logger);
  expect_query("SELECT phase_no,min,max,avg FROM supla_em_voltage_log",
               "phase_no\tmin\tmax\tavg\n1\t230.00\t240.00\t235.00\n");
}

TEST_F(OcppLoggerIntegrationTest, TargetedReloadUpdatesOnlyRequestedDevice) {
  ASSERT_EQ(
      0,
      dba.query(
          R"(UPDATE supla_dev_channel SET user_config=JSON_SET(user_config,'$.electricityMeterInitialValues',JSON_OBJECT('forwardActiveEnergy',5)) WHERE id=141)",
          true));
  user->get_devices()->reload_ocpp_devices(999);
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  expect_meter_values(12, 12, 12);
  user->get_devices()->reload_ocpp_devices(73);
  expect_meter_values(17, 12, 12);
}

TEST_F(OcppLoggerIntegrationTest, UnchangedSwitchValueSkipsDatabaseWrite) {
  gateway.on_state(suid, 73, 1, false, 90);
  // Detect an unnecessary basic-value rewrite independently of the validity
  // UPDATE, which legitimately changes update_time on every heartbeat.
  ASSERT_EQ(0, dba.query("UPDATE supla_dev_channel_value SET "
                         "value=X'0900000000000000',valid_to='2020-01-01 "
                         "00:00:00' WHERE channel_id=140",
                         true));
  gateway.on_state(suid, 73, 1, false, 90);
  expect_query(
      "SELECT HEX(value) v,valid_to>UTC_TIMESTAMP() online FROM "
      "supla_dev_channel_value WHERE channel_id=140",
      "v\tonline\n0900000000000000\t1\n");
  gateway.on_state(suid, 73, 1, true, 90);
  expect_query(
      "SELECT HEX(value) v FROM supla_dev_channel_value WHERE channel_id=140",
      "v\n0100000000000000\n");
}

TEST_F(OcppLoggerIntegrationTest, LoadsDataSourceParametersFromDatabase) {
  gateway.on_state(suid, 73, 1, false, 90);
  ASSERT_EQ(0, dba.query("DELETE FROM supla_ocpp_charging_station", true));
  std::string sql = "UPDATE supla_dev_channel SET type=" +
                    std::to_string(SUPLA_CHANNELTYPE_DIGIGLASS) + ",func=" +
                    std::to_string(SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL) +
                    ",param1=5 WHERE id=140";
  ASSERT_EQ(0, dba.query(sql.c_str(), true));
  user->get_devices()->on_channel_config_changed(73, 140);
  auto channel = user->get_devices()->get_virtual_channel(140);
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  ASSERT_TRUE(channel.get_value(value));
  TDigiglass_Value dgf = {};
  std::memcpy(&dgf, value, sizeof(dgf));
  EXPECT_EQ(5, dgf.sectionCount);
}

TEST_F(OcppLoggerIntegrationTest, FullReloadLoadsAllDataSourceChannels) {
  gateway.on_state(suid, 73, 1, false, 90);
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  ASSERT_EQ(0, dba.query("DELETE FROM supla_ocpp_charging_station", true));
  supla_user_devices devices(user.get());
  devices.update_virtual_channels();
  EXPECT_EQ(141, devices.get_virtual_channel(141).get_channel_id());
  EXPECT_EQ(140, devices.get_virtual_channel(140).get_channel_id());
}

TEST_F(OcppLoggerIntegrationTest, DatabaseCannotRenewOcppValidity) {
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}, {"voltage", {230000}}}, 90);
  supla_ocpp_dao dao(&dba);
  std::vector<supla_ocpp_device_config> configs;
  ASSERT_TRUE(dao.get_devices(2, 73, &configs));
  ASSERT_EQ(1U, configs.size());
  auto restored =
      std::make_shared<supla_ocpp_device>(user.get(), configs.front());
  EXPECT_TRUE(
      restored->get_channel(141).get_availability_status().is_offline());
  EXPECT_EQ(nullptr, restored->get_channel(141).get_extended_value(true));
  gateway.on_disconnected(suid, 73);
  ASSERT_EQ(
      0, dba.query(
             "UPDATE supla_dev_channel_value SET "
             "valid_to=UTC_TIMESTAMP()+INTERVAL 90 SECOND WHERE channel_id=141",
             true));
  user->get_devices()->reload_ocpp_devices(73);
  EXPECT_TRUE(user->get_devices()
                  ->get_ocpp_channel(141)
                  .get_availability_status()
                  .is_offline());
  supla_total_energy_logger energy;
  supla_voltage_logger voltage;
  run_logger(&energy);
  run_logger(&voltage);
  expect_query("SELECT COUNT(*) n FROM supla_em_log", "n\n0\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_voltage_log", "n\n0\n");
  gateway.on_alive(suid, 73, 90);
  EXPECT_TRUE(user->get_devices()
                  ->get_ocpp_channel(141)
                  .get_availability_status()
                  .is_online());
}

TEST_F(OcppLoggerIntegrationTest, RespectsLoggerSettingsAndDisabledPhases) {
  ASSERT_EQ(
      0,
      dba.query(
          R"(UPDATE supla_dev_channel SET user_config='{"voltageLoggerEnabled":true,"disabledPhases":[1]}' WHERE id=141)",
          true));
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_meter(suid, 73, 1,
                   {{"energy", 12000},
                    {"voltage", {230000, 240000}},
                    {"current", {16000, 17000}},
                    {"power_phases", {3680}}},
                   90);
  supla_voltage_logger voltage;
  supla_current_logger current;
  supla_power_active_logger power;
  run_logger(&voltage);
  run_logger(&current);
  run_logger(&power);
  expect_query("SELECT phase_no,min,max,avg FROM supla_em_voltage_log",
               "phase_no\tmin\tmax\tavg\n2\t240.00\t240.00\t240.00\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_current_log", "n\n0\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_power_active_log", "n\n0\n");
}

TEST_F(OcppLoggerIntegrationTest, AnalyzerIsRemovedWithChannel) {
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}, {"voltage", {230000}}}, 90);
  user->get_devices()->on_channel_deleted(73, 141);
  user->get_devices()->access_ocpp_data_analyzers(
      [](supla_electricity_analyzer *) { FAIL(); });
  EXPECT_EQ(0, user->get_devices()->get_ocpp_channel(141).get_channel_id());
}

TEST_F(OcppLoggerIntegrationTest, ConfigurationReadFailureKeepsLiveDevice) {
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  auto device = user->get_devices()->get_ocpp_device(73);
  ASSERT_EQ(0, dba.query("RENAME TABLE supla_ocpp_charging_station TO "
                         "ocpp_mapping_unavailable",
                         true));
  user->get_devices()->reload_ocpp_devices(73);
  EXPECT_EQ(device, user->get_devices()->get_ocpp_device(73));
  EXPECT_TRUE(device->is_online());
  ASSERT_EQ(0, dba.query("RENAME TABLE ocpp_mapping_unavailable TO "
                         "supla_ocpp_charging_station",
                         true));
}

TEST_F(OcppLoggerIntegrationTest, ActionExecutorUsesOcppAndRollsBackRejection) {
  int sockets[2];
  ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
  auto connection = std::make_shared<supla_ocpp_connection>(sockets[0]);
  auto global = supla_ocpp_gateway::global_instance();
  auto cleanup = std::shared_ptr<void>(nullptr, [=](void *) {
    global->unregister_connection(connection.get());
    global->wait_until_idle();
    global->configure(false, 64 * 1024);
    close(sockets[1]);
  });
  global->configure(true, 64 * 1024);
  ASSERT_TRUE(global->register_connection(connection));
  global->on_state(suid, 73, 1, false, 90);
  global->wait_until_idle();
  supla_action_executor action;
  action.set_channel_id(user.get(), 73, 140);
  ASSERT_TRUE(action.set_on_with_result(true, 0));
  global->wait_until_idle();
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  ASSERT_TRUE(user->get_devices()->get_ocpp_channel(140).get_value(value));
  EXPECT_EQ(1, value[0]);
  pollfd descriptor = {sockets[1], POLLIN, 0};
  ASSERT_EQ(1, poll(&descriptor, 1, 1000));
  uint32_t size = 0;
  ASSERT_EQ(sizeof(size), recv(sockets[1], &size, sizeof(size), MSG_WAITALL));
  size = ntohl(size);
  ASSERT_GT(size, 0U);
  ASSERT_LT(size, 1024U);
  std::string payload(size, '\0');
  ASSERT_EQ(size, recv(sockets[1], &payload[0], size, MSG_WAITALL));
  auto message = nlohmann::json::parse(payload);
  EXPECT_EQ("on", message.at("type"));
  EXPECT_EQ(73, message.at("device"));
  global->on_result(message.at("id").get<unsigned long long>(), false, "test");
  global->wait_until_idle();
  ASSERT_TRUE(user->get_devices()->get_ocpp_channel(140).get_value(value));
  EXPECT_EQ(0, value[0]);
  global->on_disconnected(suid, 73);
  global->wait_until_idle();
  EXPECT_FALSE(action.set_on_with_result(true, 0));
}

TEST_F(OcppLoggerIntegrationTest, ChannelGroupSetsAndTogglesOcppMember) {
  int sockets[2];
  ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
  auto connection = std::make_shared<supla_ocpp_connection>(sockets[0]);
  auto global = supla_ocpp_gateway::global_instance();
  auto cleanup = std::shared_ptr<void>(nullptr, [=](void *) {
    global->unregister_connection(connection.get());
    global->wait_until_idle();
    global->configure(false, 64 * 1024);
    close(sockets[1]);
  });
  global->configure(true, 64 * 1024);
  ASSERT_TRUE(global->register_connection(connection));
  global->on_state(suid, 73, 1, false, 90);
  global->wait_until_idle();

  supla_action_executor action;
  action.set_group_id(user.get(), 1);
  ASSERT_TRUE(action.set_on_with_result(true, 0));
  global->wait_until_idle();
  nlohmann::json message;
  ASSERT_TRUE(receive_gateway_command(sockets[1], &message));
  EXPECT_EQ("on", message.at("type"));
  EXPECT_EQ(73, message.at("device"));
  global->on_result(message.at("id").get<unsigned long long>(), true, "");
  global->wait_until_idle();

  ASSERT_TRUE(action.toggle_with_result());
  global->wait_until_idle();
  ASSERT_TRUE(receive_gateway_command(sockets[1], &message));
  EXPECT_EQ("off", message.at("type"));
  EXPECT_EQ(73, message.at("device"));
  global->on_result(message.at("id").get<unsigned long long>(), true, "");
  global->wait_until_idle();

  ASSERT_TRUE(
      user->get_channel_groups()->set_char_value(supla_caller(ctIPC), 1, 1));
  global->wait_until_idle();
  ASSERT_TRUE(receive_gateway_command(sockets[1], &message));
  EXPECT_EQ("on", message.at("type"));
  global->on_result(message.at("id").get<unsigned long long>(), true, "");
  global->wait_until_idle();

  TCS_SuplaNewValue new_value = {};
  new_value.Target = SUPLA_TARGET_GROUP;
  new_value.Id = 1;
  new_value.value[0] = 0;
  ASSERT_TRUE(user->get_channel_groups()->set_new_value(
      supla_caller(ctClient, 1), &new_value));
  global->wait_until_idle();
  ASSERT_TRUE(receive_gateway_command(sockets[1], &message));
  EXPECT_EQ("off", message.at("type"));
  global->on_result(message.at("id").get<unsigned long long>(), true, "");
  global->wait_until_idle();

  global->unregister_connection(connection.get());
  global->wait_until_idle();
  EXPECT_EQ(supla_ocpp_action_result::rejected,
            global->try_set_charging(user.get(), 140, true));
  EXPECT_EQ(supla_ocpp_action_result::not_ocpp,
            global->try_set_charging(user.get(), 142, true));
  EXPECT_FALSE(action.set_on_with_result(true, 0));
}

TEST_F(OcppLoggerIntegrationTest, DisabledDeviceIsRemovedOnUserReconnect) {
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  auto previous = user->get_devices()->get_ocpp_device(73);
  ASSERT_TRUE(previous->is_online());
  ASSERT_EQ(0, dba.query("UPDATE supla_iodevice SET enabled=0 WHERE id=73", true));
  user->reconnect(supla_caller(ctIPC), true, false);
  EXPECT_EQ(nullptr, user->get_devices()->get_ocpp_device(73));
  EXPECT_FALSE(previous->is_online());
  EXPECT_FALSE(previous->can_set_charging(140));
  gateway.on_alive(suid, 73, 90);
  EXPECT_EQ(0, user->get_devices()->get_ocpp_channel(141).get_channel_id());
  EXPECT_EQ(0, user->get_devices()->get_virtual_channel(141).get_channel_id());
  ASSERT_EQ(0, dba.query("UPDATE supla_iodevice SET enabled=1 WHERE id=73", true));
  user->reconnect(supla_caller(ctIPC), true, false);
  EXPECT_EQ(141, user->get_devices()->get_ocpp_channel(141).get_channel_id());
  EXPECT_FALSE(user->get_devices()->get_ocpp_device(73)->is_online());
}

TEST_F(OcppLoggerIntegrationTest, IgnoresNonOcppVirtualChannels) {
  gateway.on_meter(suid, 73, 1,
                   {{"energy", 12000},
                    {"voltage", {240000}},
                    {"current", {16000}},
                    {"power_phases", {3680}}},
                   90);
  supla_user_dao dao(&dba);
  EXPECT_TRUE(dao.get_virtual_channels(user.get(), 0).empty());
  EXPECT_EQ(0, user->get_devices()->get_virtual_channel(141).get_channel_id());
  ASSERT_EQ(
      0, dba.query("DELETE FROM supla_ocpp_charging_station WHERE id=1", true));
  user->get_devices()->on_channel_config_changed(73, 141);
  EXPECT_EQ(0, user->get_devices()->get_ocpp_channel(141).get_channel_id());
  // Ordinary data sources load independently; an OCPP configuration callback
  // does not convert the existing virtual-channel cache in place.
  user.reset();
  user = std::make_unique<ocpp_test_user>(2, suid);
  auto channel = user->get_devices()->get_virtual_channel(141);
  ASSERT_EQ(141, channel.get_channel_id());
  EXPECT_TRUE(channel.get_availability_status().is_online());
  EXPECT_EQ(0, user->get_devices()->get_ocpp_channel(141).get_channel_id());
  supla_total_energy_logger energy;
  supla_voltage_logger voltage;
  supla_current_logger current;
  supla_power_active_logger power;
  supla_voltage_aberration_logger aberration;
  run_logger(&energy);
  run_logger(&voltage);
  run_logger(&current);
  run_logger(&power);
  run_logger(&aberration);
  expect_query("SELECT COUNT(*) n FROM supla_em_log", "n\n0\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_voltage_log", "n\n0\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_current_log", "n\n0\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_power_active_log", "n\n0\n");
  expect_query("SELECT COUNT(*) n FROM supla_em_voltage_aberration_log",
               "n\n0\n");
}

TEST_F(OcppLoggerIntegrationTest, AnalyzerBelongsOnlyToItsChannelAndDevice) {
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}, {"voltage", {230000}}}, 90);
  auto device = user->get_devices()->get_ocpp_device(73);
  user->get_devices()->on_channel_deleted(73, 999);
  int visits = 0;
  user->get_devices()->access_ocpp_data_analyzers(
      [&visits](supla_electricity_analyzer *analyzer) {
        visits++;
        EXPECT_EQ(141, analyzer->get_channel_id());
        ASSERT_NE(nullptr, analyzer->get_voltage_phase1());
        EXPECT_EQ(1U, analyzer->get_voltage_phase1()->get_sample_count());
      });
  EXPECT_EQ(1, visits);
  user->get_devices()->on_device_deleted(73);
  device->update_meter(supla_ocpp_meter_report{}, 90);
  EXPECT_EQ(0, device->get_channel(141).get_channel_id());
  user->get_devices()->access_ocpp_data_analyzers(
      [](supla_electricity_analyzer *) { FAIL(); });
  EXPECT_EQ(0, user->get_devices()->get_ocpp_channel(141).get_channel_id());
}

TEST_F(OcppLoggerIntegrationTest,
       FailedWritesDoNotBlockLiveStateAndAreRetried) {
  gateway.on_state(suid, 73, 1, false, 90);
  ASSERT_EQ(0, dba.query("CREATE TRIGGER reject_ocpp_update BEFORE UPDATE ON "
                         "supla_dev_channel_value "
                         "FOR EACH ROW SIGNAL SQLSTATE '45000' SET "
                         "MESSAGE_TEXT='test write failure'",
                         true));
  gateway.on_state(suid, 73, 1, true, 90);
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  ASSERT_TRUE(user->get_devices()->get_ocpp_channel(140).get_value(value));
  EXPECT_EQ(1, value[0]);
  gateway.on_disconnected(suid, 73);
  EXPECT_TRUE(user->get_devices()
                  ->get_ocpp_channel(140)
                  .get_availability_status()
                  .is_offline());
  expect_query(
      "SELECT HEX(value) v FROM supla_dev_channel_value WHERE channel_id=140",
      "v\n0000000000000000\n");
  ASSERT_EQ(0, dba.query("DROP TRIGGER reject_ocpp_update", true));
  gateway.on_alive(suid, 73, 90);
  expect_query(
      "SELECT HEX(value) v,valid_to>UTC_TIMESTAMP() online FROM "
      "supla_dev_channel_value WHERE channel_id=140",
      "v\tonline\n0100000000000000\t1\n");
}

TEST_F(OcppLoggerIntegrationTest,
       HeartbeatDoesNotRewriteSuccessfullySavedValues) {
  gateway.on_state(suid, 73, 1, false, 90);
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}, {"voltage", {230000}}}, 90);
  // Sentinels reveal redundant value writes. A validity-only UPDATE must
  // leave them alone, even though its update_time legitimately changes.
  ASSERT_EQ(0, dba.query("UPDATE supla_dev_channel_value SET "
                         "value=X'0900000000000000',valid_to='2020-01-01' "
                         "WHERE channel_id IN (140,141)",
                         true));
  ASSERT_EQ(0, dba.query("UPDATE supla_dev_channel_extended_value SET "
                         "update_time='2020-01-01' WHERE channel_id=141",
                         true));
  gateway.on_alive(suid, 73, 90);
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}, {"voltage", {230000}}}, 90);
  expect_query(
      "SELECT channel_id,HEX(value) v,valid_to>UTC_TIMESTAMP() online "
      "FROM supla_dev_channel_value WHERE channel_id IN (140,141) "
      "ORDER BY channel_id",
      "channel_id\tv\tonline\n140\t0900000000000000\t1\n"
      "141\t0900000000000000\t1\n");
  expect_query(
      "SELECT update_time FROM supla_dev_channel_extended_value "
      "WHERE channel_id=141",
      "update_time\n2020-01-01 00:00:00\n");
}

TEST_F(OcppLoggerIntegrationTest,
       FailedWriteRetriesWholeLatestStateUntilSuccess) {
  ASSERT_EQ(
      0,
      dba.query(
          R"(UPDATE supla_dev_channel SET user_config=JSON_SET(user_config,'$.electricityMeterInitialValues',JSON_OBJECT('forwardActiveEnergy',5)) WHERE id=141)",
          true));
  user->get_devices()->reload_ocpp_devices(73);
  gateway.on_state(suid, 73, 1, false, 90);
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}, {"voltage", {230000}}}, 90);
  ASSERT_EQ(
      0,
      dba.query("CREATE TRIGGER reject_ocpp_extended BEFORE UPDATE ON "
                "supla_dev_channel_extended_value FOR EACH ROW "
                "SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='test write failure'",
                true));
  gateway.on_meter(suid, 73, 1, {{"energy", 13000}}, 90);
  supla_device_dao dao(&dba);
  EXPECT_DOUBLE_EQ(17, meter_energy(dao.get_channel_extended_value(2, 141)));

  // One failed extended-value write makes the next attempt synchronize the
  // whole device, including the unchanged switch. The newest energy wins.
  ASSERT_EQ(0, dba.query("UPDATE supla_dev_channel_value SET "
                         "value=X'0900000000000000' WHERE channel_id=140",
                         true));
  gateway.on_meter(suid, 73, 1, {{"energy", 14000}}, 90);
  expect_query(
      "SELECT HEX(value) v FROM supla_dev_channel_value WHERE channel_id=140",
      "v\n0000000000000000\n");

  // Successful basic/validity writes cannot clear the pending synchronization
  // while the extended-value write still fails.
  ASSERT_EQ(0, dba.query("UPDATE supla_dev_channel_value SET "
                         "value=X'0900000000000000' WHERE channel_id=140",
                         true));
  gateway.on_alive(suid, 73, 90);
  expect_query(
      "SELECT HEX(value) v FROM supla_dev_channel_value WHERE channel_id=140",
      "v\n0000000000000000\n");
  EXPECT_DOUBLE_EQ(17, meter_energy(dao.get_channel_extended_value(2, 141)));
  ASSERT_EQ(0, dba.query("DROP TRIGGER reject_ocpp_extended", true));
  gateway.on_alive(suid, 73, 90);
  expect_meter_values(19, 14, 14);
  char raw[SUPLA_CHANNELVALUE_SIZE] = {};
  ASSERT_TRUE(user->get_devices()->get_ocpp_channel(141).get_raw_value(raw));
  std::string query = "SELECT HEX(value)='";
  const char *hex = "0123456789ABCDEF";
  for (unsigned char byte : raw) {
    query += hex[byte >> 4];
    query += hex[byte & 15];
  }
  query += "' matches_memory FROM supla_dev_channel_value WHERE channel_id=141";
  expect_query(query.c_str(), "matches_memory\n1\n");

  // Once all writes succeed, ordinary heartbeats stop rewriting values.
  ASSERT_EQ(0, dba.query("UPDATE supla_dev_channel_value SET "
                         "value=X'0900000000000000' WHERE channel_id=140",
                         true));
  ASSERT_EQ(0, dba.query("UPDATE supla_dev_channel_extended_value SET "
                         "update_time='2020-01-01' WHERE channel_id=141",
                         true));
  gateway.on_alive(suid, 73, 90);
  expect_query(
      "SELECT HEX(value) v FROM supla_dev_channel_value WHERE channel_id=140",
      "v\n0900000000000000\n");
  expect_query(
      "SELECT update_time FROM supla_dev_channel_extended_value "
      "WHERE channel_id=141",
      "update_time\n2020-01-01 00:00:00\n");
  user->get_devices()->get_ocpp_device(73)->access_data_analyzer(
      [](supla_electricity_analyzer *a) {
        ASSERT_NE(nullptr, a->get_voltage_phase1());
        EXPECT_EQ(1U, a->get_voltage_phase1()->get_sample_count());
      });
}

TEST_F(OcppLoggerIntegrationTest,
       FailedFirstWriteDoesNotInventUnknownMeterValue) {
  gateway.on_state(suid, 73, 1, false, 90);
  ASSERT_EQ(
      0,
      dba.query("CREATE TRIGGER reject_ocpp_update BEFORE UPDATE ON "
                "supla_dev_channel_value FOR EACH ROW "
                "SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='test write failure'",
                true));
  gateway.on_state(suid, 73, 1, true, 90);
  ASSERT_EQ(0, dba.query("DROP TRIGGER reject_ocpp_update", true));
  gateway.on_alive(suid, 73, 90);
  expect_query(
      "SELECT HEX(value) v FROM supla_dev_channel_value WHERE channel_id=140",
      "v\n0100000000000000\n");
  expect_query(
      "SELECT COUNT(*) n FROM supla_dev_channel_value WHERE channel_id=141",
      "n\n0\n");
  expect_query(
      "SELECT COUNT(*) n FROM supla_dev_channel_extended_value WHERE "
      "channel_id=141",
      "n\n0\n");
}

TEST_F(OcppLoggerIntegrationTest,
       FailedValidityWriteAlsoRequestsFullSynchronization) {
  gateway.on_state(suid, 73, 1, false, 90);
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}}, 90);
  ASSERT_EQ(
      0,
      dba.query("CREATE TRIGGER reject_ocpp_validity BEFORE UPDATE ON "
                "supla_dev_channel_value FOR EACH ROW "
                "SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='test write failure'",
                true));
  gateway.on_alive(suid, 73, 90);
  ASSERT_EQ(0, dba.query("DROP TRIGGER reject_ocpp_validity", true));
  ASSERT_EQ(0, dba.query("UPDATE supla_dev_channel_value SET "
                         "value=X'0900000000000000' WHERE channel_id=140",
                         true));
  gateway.on_alive(suid, 73, 90);
  expect_query(
      "SELECT HEX(value) v FROM supla_dev_channel_value WHERE channel_id=140",
      "v\n0000000000000000\n");
  expect_meter_values(12, 12, 12);
}

TEST_F(OcppLoggerIntegrationTest, ReloadKeepsTheSameRuntimeAndAnalyzer) {
  gateway.on_meter(suid, 73, 1, {{"energy", 12000}, {"voltage", {230000}}}, 90);
  auto device = user->get_devices()->get_ocpp_device(73);
  supla_electricity_analyzer *original = nullptr;
  device->access_data_analyzer(
      [&](supla_electricity_analyzer *a) { original = a; });
  user->get_devices()->reload_ocpp_devices(73);
  EXPECT_EQ(device, user->get_devices()->get_ocpp_device(73));
  device->access_data_analyzer([&](supla_electricity_analyzer *a) {
    EXPECT_EQ(original, a);
    EXPECT_EQ(1U, a->get_voltage_phase1()->get_sample_count());
  });
}

}  // namespace testing
