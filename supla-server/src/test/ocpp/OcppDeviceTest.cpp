// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include <cmath>
#include <cstring>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>

#include "analyzer/electricity_analyzer.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "device/value/channel_em_value.h"
#include "ocpp/ocpp_device.h"
#include "user/virtualchannel.h"

namespace testing {
namespace {
supla_ocpp_device_config make_config(const char *json = "{}") {
  supla_ocpp_device_config config;
  config.device_id = 73;
  config.meter.id = 141;
  config.meter.type = SUPLA_CHANNELTYPE_ELECTRICITY_METER;
  config.meter.func = SUPLA_CHANNELFNC_ELECTRICITY_METER;
  config.meter.json.set_user_config(json);
  config.meter.json.set_properties(
      R"({"countersAvailable":["forwardActiveEnergy"]})");
  config.power_switch.id = 140;
  config.power_switch.type = SUPLA_CHANNELTYPE_RELAY;
  config.power_switch.func = SUPLA_CHANNELFNC_POWERSWITCH;
  return config;
}

double energy(supla_abstract_channel_extended_value *value) {
  std::unique_ptr<supla_abstract_channel_extended_value> owned(value);
  auto meter = dynamic_cast<supla_channel_em_extended_value *>(value);
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

bool is_on(const std::shared_ptr<supla_ocpp_device> &device) {
  char raw[SUPLA_CHANNELVALUE_SIZE] = {};
  EXPECT_TRUE(device->get_channel(140).get_value(raw));
  return raw[0] != 0;
}
}  // namespace

static_assert(!std::is_copy_constructible<supla_ocpp_device>::value,
              "Live charger state must never be copied with its analyzer");

TEST(OcppDeviceTest, HeartbeatCannotInventMissingValues) {
  auto device = std::make_shared<supla_ocpp_device>(nullptr, make_config());
  device->renew_validity(90);
  EXPECT_FALSE(device->is_online());
  for (int id : {140, 141}) {
    EXPECT_EQ(nullptr, device->get_channel(id).get_value());
    EXPECT_TRUE(device->get_channel(id).get_availability_status().is_offline());
  }
  supla_ocpp_meter_report report;
  report.voltage[0] = 230000;
  device->update_meter(report, 90);
  auto channel = device->get_channel(141);
  EXPECT_EQ(nullptr, channel.get_value());
  EXPECT_EQ(nullptr, channel.get_raw_extended_value());
  EXPECT_EQ(nullptr, channel.get_extended_value(true));
  report.energy = 0;
  device->update_meter(report, 90);
  EXPECT_TRUE(device->get_channel(141).get_availability_status().is_online());
  EXPECT_DOUBLE_EQ(0, energy(device->get_channel(141).get_extended_value()));
}

TEST(OcppDeviceTest, KeepsRawAdjustedAndHistoryCountersDistinct) {
  auto config = make_config(
      R"({"electricityMeterInitialValues":{"forwardActiveEnergy":5}})");
  auto device = std::make_shared<supla_ocpp_device>(nullptr, config);
  supla_ocpp_meter_report report;
  report.energy = 12000;
  device->update_meter(report, 90);
  auto channel = device->get_channel(141);
  EXPECT_DOUBLE_EQ(17, energy(channel.get_extended_value()));
  EXPECT_DOUBLE_EQ(12, energy(channel.get_raw_extended_value()));
  EXPECT_DOUBLE_EQ(12, energy(channel.get_extended_value(true)));
  config.meter.json.set_user_config(
      R"({"addToHistory":true,"electricityMeterInitialValues":{"forwardActiveEnergy":8}})");
  device->reload(config);
  EXPECT_DOUBLE_EQ(20, energy(device->get_channel(141).get_extended_value()));
  EXPECT_DOUBLE_EQ(20,
                   energy(device->get_channel(141).get_extended_value(true)));
  // A reader's existing channel copy is unaffected by subsequent changes.
  EXPECT_DOUBLE_EQ(17, energy(channel.get_extended_value()));
  EXPECT_DOUBLE_EQ(12, energy(channel.get_extended_value(true)));
  device->disconnect();
  EXPECT_EQ(nullptr, device->get_channel(141).get_extended_value(true));
}

TEST(OcppDeviceTest,
     AggregateEnergyUsesOnlyFirstSlotWithoutLosingPhaseReadings) {
  auto config = make_config(
      R"({"addToHistory":true,"electricityMeterInitialValues":{"forwardActiveEnergy":5}})");
  auto device = std::make_shared<supla_ocpp_device>(nullptr, config);
  supla_ocpp_meter_report report;
  report.energy = 12345;
  for (int phase = 0; phase < 3; phase++) {
    report.voltage[phase] = 230000 + phase * 1000;
    report.current[phase] = 10000 + phase * 1000;
    report.power[phase] = 2300 + phase * 100;
  }
  device->update_meter(report, 90);
  auto channel = device->get_channel(141);
  EXPECT_DOUBLE_EQ(12.345, energy(channel.get_raw_extended_value()));
  EXPECT_DOUBLE_EQ(17.345, energy(channel.get_extended_value()));
  EXPECT_DOUBLE_EQ(17.345, energy(channel.get_extended_value(true)));
  std::unique_ptr<supla_abstract_channel_extended_value> extended(
      channel.get_extended_value());
  auto meter = dynamic_cast<supla_channel_em_extended_value *>(extended.get());
  ASSERT_NE(nullptr, meter);
  EXPECT_TRUE(meter->get_measured_values() & EM_VAR_FORWARD_ACTIVE_ENERGY);
  EXPECT_DOUBLE_EQ(17.345, meter->get_fae(1));
  for (int phase = 1; phase <= 3; phase++) {
    EXPECT_DOUBLE_EQ(229 + phase, meter->get_voltage(phase));
    EXPECT_DOUBLE_EQ(9 + phase, meter->get_current(phase));
    EXPECT_DOUBLE_EQ(2200 + phase * 100, meter->get_power_active(phase));
  }
  char raw[SUPLA_CHANNELVALUE_SIZE] = {};
  ASSERT_TRUE(channel.get_raw_value(raw));
  EXPECT_EQ(
      1235U,
      supla_channel_em_value(raw).get_em_value()->total_forward_active_energy);
  ASSERT_TRUE(channel.get_value(raw));
  EXPECT_EQ(
      1735U,
      supla_channel_em_value(raw).get_em_value()->total_forward_active_energy);
}

TEST(OcppDeviceTest, ActualPhaseEnergyIsKeptSeparateAndCanReplaceAggregate) {
  auto device = std::make_shared<supla_ocpp_device>(nullptr, make_config());
  supla_ocpp_meter_report report;
  report.energy_phases = {1000, 2000, 3000};
  device->update_meter(report, 90);
  auto check = [&](double phase1, double phase2, double phase3) {
    auto channel = device->get_channel(141);
    std::unique_ptr<supla_abstract_channel_extended_value> extended(
        channel.get_extended_value());
    auto meter =
        dynamic_cast<supla_channel_em_extended_value *>(extended.get());
    ASSERT_NE(nullptr, meter);
    EXPECT_DOUBLE_EQ(phase1, meter->get_fae(1));
    EXPECT_DOUBLE_EQ(phase2, meter->get_fae(2));
    EXPECT_DOUBLE_EQ(phase3, meter->get_fae(3));
    EXPECT_DOUBLE_EQ(phase1 + phase2 + phase3, meter->get_fae_sum());
    EXPECT_DOUBLE_EQ(0, meter->get_fae_balanced());
    char raw[SUPLA_CHANNELVALUE_SIZE] = {};
    ASSERT_TRUE(channel.get_raw_value(raw));
    EXPECT_EQ(
        static_cast<unsigned int>(std::round((phase1 + phase2 + phase3) * 100)),
        supla_channel_em_value(raw)
            .get_em_value()
            ->total_forward_active_energy);
  };
  check(1, 2, 3);

  report = {};
  report.energy_phases[1] = 2500;
  device->update_meter(report, 90);
  check(1, 2.5, 3);

  report = {};
  report.energy = 9000;
  device->update_meter(report, 90);
  check(9, 0, 0);

  report = {};
  report.energy_phases[0] = 1000;
  report.energy_phases[2] = 3000;
  device->update_meter(report, 90);
  check(1, 0, 3);
}

TEST(OcppDeviceTest, PhaseEnergyUsesPhaseSpecificInitialValues) {
  auto config = make_config(
      R"({"electricityMeterInitialValues":{"forwardActiveEnergy":{"1":1,"2":2,"3":3}}})");
  auto device = std::make_shared<supla_ocpp_device>(nullptr, config);
  supla_ocpp_meter_report report;
  report.energy_phases = {1000, 2000, 3000};
  device->update_meter(report, 90);
  std::unique_ptr<supla_abstract_channel_extended_value> extended(
      device->get_channel(141).get_extended_value());
  auto meter = dynamic_cast<supla_channel_em_extended_value *>(extended.get());
  ASSERT_NE(nullptr, meter);
  EXPECT_DOUBLE_EQ(2, meter->get_fae(1));
  EXPECT_DOUBLE_EQ(4, meter->get_fae(2));
  EXPECT_DOUBLE_EQ(6, meter->get_fae(3));
}

TEST(OcppDeviceTest, PhaseInitialValuesAreCombinedIntoTheAggregateCounter) {
  auto config = make_config(
      R"({"electricityMeterInitialValues":{"forwardActiveEnergy":{"1":1,"2":2,"3":3}}})");
  auto device = std::make_shared<supla_ocpp_device>(nullptr, config);
  supla_ocpp_meter_report report;
  report.energy = 12000;
  device->update_meter(report, 90);
  auto channel = device->get_channel(141);
  EXPECT_DOUBLE_EQ(18, energy(channel.get_extended_value()));
  EXPECT_DOUBLE_EQ(12, energy(channel.get_raw_extended_value()));
  char raw[SUPLA_CHANNELVALUE_SIZE] = {};
  ASSERT_TRUE(channel.get_value(raw));
  EXPECT_EQ(
      1800U,
      supla_channel_em_value(raw).get_em_value()->total_forward_active_energy);
}

TEST(OcppDeviceTest, NegativeOffsetDoesNotLoseRawEnergy) {
  auto config = make_config(
      R"({"electricityMeterInitialValues":{"forwardActiveEnergy":-20}})");
  auto device = std::make_shared<supla_ocpp_device>(nullptr, config);
  supla_ocpp_meter_report report;
  report.energy = 12000;
  device->update_meter(report, 90);
  report.energy.reset();
  report.voltage[1] = 240000;
  device->update_meter(report, 90);
  auto channel = device->get_channel(141);
  EXPECT_DOUBLE_EQ(0, energy(channel.get_extended_value()));
  EXPECT_DOUBLE_EQ(12, energy(channel.get_raw_extended_value()));
  EXPECT_DOUBLE_EQ(12, energy(channel.get_extended_value(true)));
}

TEST(OcppDeviceTest, RestartDoesNotTreatAdjustedMissingPhasesAsRaw) {
  auto config = make_config(
      R"({"electricityMeterInitialValues":{"forwardActiveEnergy":{"1":1,"2":2,"3":-40}}})");
  auto device = std::make_shared<supla_ocpp_device>(nullptr, config);
  supla_ocpp_meter_report report;
  report.energy_phases = {10000, 20000, 30000};
  device->update_meter(report, 90);
  auto before = device->get_channel(141);
  std::array<char, SUPLA_CHANNELVALUE_SIZE> raw;
  ASSERT_TRUE(before.get_raw_value(raw.data()));
  config.meter_persisted_values.value = raw;
  auto persisted = std::make_shared<TSuplaChannelExtendedValue>();
  std::unique_ptr<supla_abstract_channel_extended_value> value(before.get_extended_value());
  ASSERT_TRUE(value->get_raw_value(persisted.get()));
  config.meter_persisted_values.extended_value = persisted;
  device = std::make_shared<supla_ocpp_device>(nullptr, config);
  report = {};
  report.energy_phases[0] = 12000;
  device->update_meter(report, 90);
  auto partial = device->get_channel(141);
  std::unique_ptr<supla_abstract_channel_extended_value> displayed(partial.get_extended_value());
  auto em = dynamic_cast<supla_channel_em_extended_value *>(displayed.get());
  ASSERT_NE(nullptr, em);
  EXPECT_DOUBLE_EQ(13, em->get_fae(1));
  EXPECT_DOUBLE_EQ(22, em->get_fae(2));
  EXPECT_DOUBLE_EQ(0, em->get_fae(3));
  EXPECT_EQ(nullptr, partial.get_extended_value(true));
  // The raw aggregate from the DB cannot be reconstructed from corrected
  // partial phase counters, especially if an initial value clamped a phase.
  std::array<char, SUPLA_CHANNELVALUE_SIZE> partial_raw;
  ASSERT_TRUE(partial.get_raw_value(partial_raw.data()));
  EXPECT_EQ(raw, partial_raw);
  report = {};
  report.energy_phases[1] = 21000;
  report.energy_phases[2] = 31000;
  device->update_meter(report, 90);
  std::unique_ptr<supla_abstract_channel_extended_value> history(
      device->get_channel(141).get_extended_value(true));
  auto raw_em = dynamic_cast<supla_channel_em_extended_value *>(history.get());
  ASSERT_NE(nullptr, raw_em);
  EXPECT_DOUBLE_EQ(12, raw_em->get_fae(1));
  EXPECT_DOUBLE_EQ(21, raw_em->get_fae(2));
  EXPECT_DOUBLE_EQ(31, raw_em->get_fae(3));
}

TEST(OcppDeviceTest, QueueWaitingDoesNotRenewExpiredReport) {
  auto device = std::make_shared<supla_ocpp_device>(nullptr, make_config());
  device->update_state(false, 90);
  std::promise<void> started, release;
  auto released = release.get_future();
  std::thread command([&]() {
    device->request_charging(1, true, false, [&](bool) {
      started.set_value();
      released.wait();
      return true;
    });
  });
  started.get_future().wait();
  device->renew_validity(1);
  std::this_thread::sleep_for(std::chrono::milliseconds(1100));
  release.set_value();
  command.join();
  EXPECT_FALSE(device->is_online());
  EXPECT_EQ(0U, device->get_channel(140).get_value_validity_time_sec());
}

TEST(OcppDeviceTest, PersistedSnapshotIsOfflineAndIsNotAHistorySample) {
  auto config = make_config(
      R"({"addToHistory":true,"electricityMeterInitialValues":{"forwardActiveEnergy":5}})");
  config.meter_persisted_values.value.emplace();
  supla_channel_em_value basic(config.meter_persisted_values.value->data());
  basic.set_total_forward_active_energy(12);
  basic.get_raw_value(config.meter_persisted_values.value->data());
  config.meter_persisted_values.extended_value =
      std::make_shared<TSuplaChannelExtendedValue>();
  supla_channel_em_extended_value extended;
  extended.set_fae(1, 17);
  extended.get_raw_value(config.meter_persisted_values.extended_value.get());
  auto device = std::make_shared<supla_ocpp_device>(nullptr, config);
  EXPECT_TRUE(device->get_channel(141).get_availability_status().is_offline());
  device->renew_validity(90);
  supla_ocpp_meter_report report;
  report.voltage[0] = 230000;
  device->update_meter(report, 90);
  auto channel = device->get_channel(141);
  EXPECT_DOUBLE_EQ(17, energy(channel.get_extended_value()));
  EXPECT_EQ(nullptr, channel.get_raw_extended_value());
  EXPECT_EQ(nullptr, channel.get_extended_value(true));
  report.energy = 13000;
  device->update_meter(report, 90);
  EXPECT_DOUBLE_EQ(18,
                   energy(device->get_channel(141).get_extended_value(true)));
}

TEST(OcppDeviceTest, OnlyFreshFieldsFeedTheSameAnalyzer) {
  auto config = make_config(
      R"({"voltageLoggerEnabled":true,"currentLoggerEnabled":true})");
  auto device = std::make_shared<supla_ocpp_device>(nullptr, config);
  supla_ocpp_meter_report report;
  report.energy = 12000;
  report.voltage[0] = 230000;
  device->update_meter(report, 90);
  supla_electricity_analyzer *original = nullptr;
  device->access_data_analyzer(
      [&](supla_electricity_analyzer *a) { original = a; });
  ASSERT_NE(nullptr, original);
  auto channel = device->get_channel(141);
  device->reload(config);
  device->renew_validity(90);
  report = {};
  report.current[1] = 16000;
  device->update_meter(report, 90);
  device->access_data_analyzer([&](supla_electricity_analyzer *a) {
    EXPECT_EQ(original, a);
    ASSERT_NE(nullptr, a->get_voltage_phase1());
    EXPECT_EQ(1U, a->get_voltage_phase1()->get_sample_count());
    ASSERT_NE(nullptr, a->get_current_phase2());
    EXPECT_EQ(1U, a->get_current_phase2()->get_sample_count());
    EXPECT_EQ(nullptr, a->get_current_phase1());
    EXPECT_EQ(nullptr, a->get_voltage_phase2());
  });
  device->disconnect();
  device->renew_validity(90);
  device->access_data_analyzer([&](supla_electricity_analyzer *a) {
    EXPECT_EQ(original, a);
    EXPECT_FALSE(a->is_any_data_for_logging_purpose());
  });
}

TEST(OcppDeviceTest, OptimisticCommandsRollbackAndRespectObservedState) {
  auto device = std::make_shared<supla_ocpp_device>(nullptr, make_config());
  device->update_state(false, 90);
  ASSERT_TRUE(
      device->request_charging(1, true, false, [](bool on) { return on; }));
  EXPECT_TRUE(is_on(device));
  device->on_result(1, false);
  EXPECT_FALSE(is_on(device));
  ASSERT_TRUE(
      device->request_charging(2, false, false, [](bool) { return true; }));
  device->on_result(2, false);
  EXPECT_FALSE(is_on(device));
  ASSERT_TRUE(
      device->request_charging(3, true, false, [](bool) { return true; }));
  device->update_state(true, 90);
  device->on_result(3, false);
  EXPECT_TRUE(is_on(device));
}

TEST(OcppDeviceTest, CommandBurstRollsBackToLastAcceptedState) {
  auto device = std::make_shared<supla_ocpp_device>(nullptr, make_config());
  device->update_state(false, 90);
  auto send = [](bool) { return true; };
  ASSERT_TRUE(device->request_charging(1, true, false, send));
  ASSERT_TRUE(device->request_charging(2, false, false, send));
  device->on_result(1, true);
  device->on_result(2, false);
  EXPECT_TRUE(is_on(device));
  ASSERT_TRUE(device->request_charging(3, false, false, send));
  device->disconnect();
  EXPECT_TRUE(is_on(device));
  EXPECT_FALSE(device->is_online());
}

TEST(OcppDeviceTest, FailedSendAndReentrantStateHaveOrderedEffects) {
  auto device = std::make_shared<supla_ocpp_device>(nullptr, make_config());
  device->update_state(false, 90);
  ASSERT_TRUE(
      device->request_charging(1, true, false, [](bool) { return false; }));
  EXPECT_FALSE(is_on(device));
  ASSERT_TRUE(device->request_charging(2, true, false, [&](bool) {
    device->update_state(true, 90);
    device->on_result(2, false);
    return true;
  }));
  EXPECT_TRUE(is_on(device));
  device->deactivate();
  EXPECT_FALSE(device->request_charging(3, false, false, [](bool) {
    ADD_FAILURE();
    return true;
  }));
  device->renew_validity(90);
  device->update_state(false, 90);
  EXPECT_EQ(0, device->get_channel(140).get_channel_id());
}

TEST(OcppDeviceTest, OlderConfigurationCannotOverwriteNewerReload) {
  auto config = make_config();
  auto device = std::make_shared<supla_ocpp_device>(nullptr, config);
  auto newer = config;
  newer.revision = 2;
  newer.meter.json.set_user_config(
      R"({"electricityMeterInitialValues":{"forwardActiveEnergy":5}})");
  device->reload(newer);
  config.revision = 1;
  device->reload(config);
  supla_ocpp_meter_report report;
  report.energy = 12000;
  device->update_meter(report, 90);
  EXPECT_DOUBLE_EQ(17, energy(device->get_channel(141).get_extended_value()));
}

TEST(OcppDeviceTest, DelayedSuccessCannotOverrideNewerAcceptedBaseline) {
  auto device = std::make_shared<supla_ocpp_device>(nullptr, make_config());
  device->update_state(false, 90);
  auto send = [](bool) { return true; };
  ASSERT_TRUE(device->request_charging(1, true, false, send));
  ASSERT_TRUE(device->request_charging(2, false, false, send));
  device->on_result(2, true);
  ASSERT_TRUE(device->request_charging(3, true, false, send));
  device->on_result(1, true);
  device->on_result(3, false);
  EXPECT_FALSE(is_on(device));
}

TEST(OcppDeviceTest, DeactivationDiscardsAlreadyQueuedReports) {
  auto device = std::make_shared<supla_ocpp_device>(nullptr, make_config());
  device->update_state(false, 90);
  std::promise<void> sending, release;
  auto released = release.get_future();
  std::thread command([&]() {
    EXPECT_TRUE(device->request_charging(1, true, false, [&](bool) {
      sending.set_value();
      released.wait();
      return true;
    }));
  });
  sending.get_future().wait();
  supla_ocpp_meter_report report;
  report.energy = 12000;
  device->update_meter(report, 90);
  device->update_state(true, 90);
  device->on_result(1, false);
  device->deactivate();
  release.set_value();
  command.join();
  EXPECT_FALSE(device->is_online());
  EXPECT_EQ(0, device->get_channel(140).get_channel_id());
  EXPECT_EQ(0, device->get_channel(141).get_channel_id());
  device->access_data_analyzer([](supla_electricity_analyzer *) { FAIL(); });
}

TEST(VirtualDataSourceTest,
     DatabaseSnapshotsKeepTheirOwnValidityAndParameters) {
  double input = 10;
  char raw[SUPLA_CHANNELVALUE_SIZE] = {};
  memcpy(raw, &input, sizeof(input));
  auto make_source = [&](unsigned int validity) {
    return supla_virtual_channel(
        nullptr, 1, 2, raw, validity,
        SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT,
        SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT, 0, 0, 0, 0,
        R"({"valueDivider":2000,"valueMultiplier":4000,"valueAdded":5000})",
        "{}");
  };
  auto source = make_source(90);
  char output[SUPLA_CHANNELVALUE_SIZE] = {};
  ASSERT_TRUE(source.get_value(output));
  double adjusted = 0;
  memcpy(&adjusted, output, sizeof(adjusted));
  EXPECT_DOUBLE_EQ(25, adjusted);
  EXPECT_TRUE(source.get_availability_status().is_online());
  source = make_source(0);
  EXPECT_TRUE(source.get_availability_status().is_offline());
  auto self = &source;
  source = *self;
  ASSERT_TRUE(source.get_value(output));
  memcpy(&adjusted, output, sizeof(adjusted));
  EXPECT_DOUBLE_EQ(25, adjusted);
  source = supla_virtual_channel();
  EXPECT_FALSE(source.get_value(output));

  memset(raw, 0, sizeof(raw));
  supla_virtual_channel glass(
      nullptr, 1, 2, raw, 90, SUPLA_CHANNELTYPE_DIGIGLASS,
      SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL, 5, 0, 0, 0, "{}", "{}");
  ASSERT_TRUE(glass.get_value(output));
  TDigiglass_Value value = {};
  memcpy(&value, output, sizeof(value));
  EXPECT_EQ(5, value.sectionCount);
}

}  // namespace testing
