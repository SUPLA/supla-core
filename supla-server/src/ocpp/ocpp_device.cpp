// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ocpp/ocpp_device.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include "analyzer/data_analyzer_factory.h"
#include "analyzer/electricity_analyzer.h"
#include "db/mariadb_access_provider.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "device/extended_value/channel_extended_value_factory.h"
#include "device/value/channel_em_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_value_factory.h"
#include "jsonconfig/channel/electricity_meter_config.h"
#include "log.h"
#include "ocpp/ocpp_dao.h"
#include "user/user.h"
#include "vbt/value_based_triggers.h"

int supla_ocpp_channel::get_device_id(void) const { return device_id; }
int supla_ocpp_channel::get_channel_id(void) const { return channel_id; }
int supla_ocpp_channel::get_type(void) const { return type; }
int supla_ocpp_channel::get_func(void) const { return func; }

const supla_abstract_channel_value *supla_ocpp_channel::get_value(void) const {
  return value.get();
}

bool supla_ocpp_channel::get_value(char output[SUPLA_CHANNELVALUE_SIZE]) const {
  if (!value) return false;
  value->get_raw_value(output);
  return true;
}

bool supla_ocpp_channel::get_raw_value(
    char output[SUPLA_CHANNELVALUE_SIZE]) const {
  if (!value) return false;
  memcpy(output, raw_value.data(), raw_value.size());
  return true;
}

supla_abstract_channel_extended_value *supla_ocpp_channel::get_extended_value(
    bool for_logger) const {
  if (for_logger) {
    // Stored adjusted counters cannot be inverted safely (negative initial
    // values can clamp them to zero). History needs a real raw energy report.
    if (!get_availability_status().is_online() || !raw_extended_value ||
        !raw_energy_complete) {
      return nullptr;
    }
    if (!add_to_history) return raw_extended_value->copy();
  }
  return extended_value ? extended_value->copy() : nullptr;
}

supla_abstract_channel_extended_value *
supla_ocpp_channel::get_raw_extended_value(void) const {
  return raw_extended_value ? raw_extended_value->copy() : nullptr;
}

supla_channel_availability_status supla_ocpp_channel::get_availability_status(
    void) const {
  supla_channel_availability_status result(true);
  if (value && validity.is_valid()) {
    result.set_offline(false);
  }
  return result;
}

unsigned int supla_ocpp_channel::get_value_validity_time_sec(void) const {
  return value ? validity.remaining_seconds() : 0;
}

supla_ocpp_device::supla_ocpp_device(supla_user *user,
                                     const supla_ocpp_device_config &config)
    : user(user), id(config.device_id) {
  configuration_revision = config.revision;
  initialize(&meter, config.meter, config.meter_persisted_values);
  initialize(&power_switch, config.power_switch,
             config.power_switch_persisted_values);
  restoring_phase_energy = meter.channel.extended_value != nullptr;
  char raw[SUPLA_CHANNELVALUE_SIZE] = {};
  if (power_switch.channel.get_value(raw)) {
    confirmed_power_switch_value = supla_channel_onoff_value(raw).is_on();
  }
  analyzer.reset(supla_data_analyzer_factory::new_analyzer(config.meter.id,
                                                           config.meter.func));
}

supla_ocpp_device::~supla_ocpp_device() {}

void supla_ocpp_device::initialize(
    runtime_channel *target, const supla_ocpp_channel_config &config,
    const supla_ocpp_channel_persisted_values &persisted_values) {
  target->config = config;
  auto &channel = target->channel;
  channel.device_id = id;
  channel.channel_id = config.id;
  channel.type = config.type;
  channel.func = config.func;
  if (persisted_values.value) set_value(target, persisted_values.value->data());
  if (persisted_values.extended_value) {
    channel.extended_value.reset(
        supla_abstract_channel_extended_value_factory::new_value(
            persisted_values.extended_value.get(), nullptr, 0, user));
  }
  if (target == &meter) {
    electricity_meter_config meter_config(&target->config.json);
    channel.add_to_history = meter_config.should_be_added_to_history();
  }
  // The persisted basic value is raw; extended value is already adjusted.
  // No validity and no history sample are inferred from either stored value.
}

int supla_ocpp_device::get_id(void) const { return id; }
int supla_ocpp_device::get_user_id(void) const {
  return user ? user->getUserID() : 0;
}
int supla_ocpp_device::get_meter_channel_id(void) const {
  return meter.channel.channel_id;
}
int supla_ocpp_device::get_switch_channel_id(void) const {
  return power_switch.channel.channel_id;
}

bool supla_ocpp_device::is_online(void) const {
  std::lock_guard<std::mutex> lock(mutex);
  return active && connected &&
         (meter.channel.get_availability_status().is_online() ||
          power_switch.channel.get_availability_status().is_online());
}

bool supla_ocpp_device::can_set_charging(int channel_id) const {
  std::lock_guard<std::mutex> lock(mutex);
  return active && connected && channel_id == power_switch.channel.channel_id &&
         power_switch.channel.get_availability_status().is_online();
}

supla_ocpp_channel supla_ocpp_device::get_channel(int channel_id) const {
  std::lock_guard<std::mutex> lock(mutex);
  if (active) {
    if (channel_id == meter.channel.channel_id) return meter.channel;
    if (channel_id == power_switch.channel.channel_id)
      return power_switch.channel;
  }
  return {};
}

void supla_ocpp_device::access_data_analyzer(
    std::function<void(supla_electricity_analyzer *)> callback) {
  std::lock_guard<std::mutex> lock(mutex);
  if (active && meter.channel.get_availability_status().is_online()) {
    if (auto em = dynamic_cast<supla_electricity_analyzer *>(analyzer.get())) {
      callback(em);
    }
  }
}

void supla_ocpp_device::set_value(runtime_channel *target,
                                  const char raw[SUPLA_CHANNELVALUE_SIZE]) {
  auto &config = target->config;
  auto &channel = target->channel;
  std::shared_ptr<supla_abstract_channel_value> value(
      supla_abstract_channel_value_factory::new_value(
          raw, config.type, config.func, user, config.param2, config.param3));
  if (value) {
    value->apply_channel_properties(config.type, SUPLA_PROTO_VERSION,
                                    config.param1, config.param2, config.param3,
                                    config.param4, &config.json);
  }
  memcpy(channel.raw_value.data(), raw, SUPLA_CHANNELVALUE_SIZE);
  channel.value = std::move(value);
}

void supla_ocpp_device::set_extended_value(
    supla_abstract_channel_extended_value *value, bool is_raw,
    bool aggregate_energy) {
  std::shared_ptr<supla_abstract_channel_extended_value> adjusted(value);
  auto em = dynamic_cast<supla_channel_em_extended_value *>(value);
  // Keep readings before and after initial values: the raw counter is needed
  // for partial merges and history without offsets. Reversing the adjustment
  // is impossible after clamping a negative result to zero.
  meter.channel.raw_extended_value.reset(is_raw && em ? em->copy() : nullptr);
  meter.channel.raw_energy_complete = is_raw && !restoring_phase_energy;
  if (is_raw && em) {
    electricity_meter_config config(&meter.config.json);
    TElectricityMeter_ExtendedValue_V3 raw = {};
    em->get_raw_value(&raw);
    if (aggregate_energy) {
      // OCPP's aggregate counter occupies only the first energy slot. Apply
      // the entire initial value there as well, without creating counters on
      // L2/L3.
      config.add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY,
                               &raw.total_forward_active_energy[0]);
    } else {
      int flags = config.get_channel_user_flags();
      const int unsupported[] = {SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED,
                                 SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED,
                                 SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED};
      for (int phase = 0; phase < 3; phase++) {
        if (!(energy_phase_mask & (1 << phase))) flags |= unsupported[phase];
      }
      config.add_initial_values(flags, &raw);
    }
    em->set_raw_value(&raw);
    if (restoring_phase_energy) {
      auto previous = dynamic_cast<supla_channel_em_extended_value *>(
          meter.channel.extended_value.get());
      // A restored counter is already adjusted and cannot be inverted after
      // clamping. Keep it for display only, never in the raw/history copy.
      if (previous) {
        for (int phase = 0; phase < 3; phase++) {
          if (!(energy_phase_mask & (1 << phase)))
            em->set_fae(phase + 1, previous->get_fae(phase + 1));
        }
      }
    }
  }
  meter.channel.extended_value = std::move(adjusted);
}

void supla_ocpp_device::renew_validity_locked(
    supla_ocpp_value_validity validity) {
  if (analyzer &&
      (!validity.is_valid() ||
       !meter.channel.get_availability_status().is_online())) {
    analyzer->reset();
  }
  connected = validity.is_valid();
  for (auto target : {&meter, &power_switch}) {
    target->channel.validity = validity;
  }
}

void supla_ocpp_device::persist(const supla_ocpp_channel &before_meter,
                                const supla_ocpp_channel &before_switch,
                                bool renew_validity, bool force) {
  // Coalesce meter/validity writes for five seconds, including retries after
  // SQL failures. The next report or heartbeat flushes the latest state.
  // Switch and availability changes remain immediate; IPC/UI/analyzers are
  // never delayed. Removed mappings are not flushed by deactivate().
  // Mutations are serialized, so a later report cannot overtake
  // this write. SQL runs outside the state lock and never rolls back live data.
  if (!user) return;
  supla_ocpp_channel meter_value, switch_value;
  {
    std::lock_guard<std::mutex> lock(mutex);
    if (!active) return;
    meter_value = meter.channel;
    switch_value = power_switch.channel;
  }
  auto needs_value_save = [](const supla_ocpp_channel &before,
                             const supla_ocpp_channel &after) {
    return after.value &&
           (!before.value || before.raw_value != after.raw_value);
  };
  bool switch_changed = needs_value_save(before_switch, switch_value);
  meter_save_pending |= needs_value_save(before_meter, meter_value);
  switch_save_pending |= switch_changed;
  extended_save_pending |=
      meter_value.extended_value &&
      (!before_meter.extended_value || meter_value.extended_value->is_differ(
                                           before_meter.extended_value.get()));
  validity_save_pending |= renew_validity;
  bool availability_changed =
      before_meter.get_availability_status().is_online() !=
          meter_value.get_availability_status().is_online() ||
      before_switch.get_availability_status().is_online() !=
          switch_value.get_availability_status().is_online();
  if (!meter_save_pending && !switch_save_pending && !extended_save_pending &&
      !validity_save_pending)
    return;
  auto now = std::chrono::steady_clock::now();
  if (!force && !switch_changed && !availability_changed &&
      now < next_db_attempt)
    return;
  supla_mariadb_access_provider dba;
  bool success = dba.connect();
  if (success) {
    supla_ocpp_dao dao(&dba);
    if (meter_save_pending && !dao.save_value(get_user_id(), meter_value))
      success = false;
    if (switch_save_pending && !dao.save_value(get_user_id(), switch_value))
      success = false;
    if (extended_save_pending &&
        !dao.save_extended_value(get_user_id(), meter_value))
      success = false;
    if (validity_save_pending) {
      // Connection/retry latency must not extend the charger's deadline.
      unsigned int seconds =
          std::max(meter_value.get_value_validity_time_sec(),
                   switch_value.get_value_validity_time_sec());
      if (!dao.renew_validity(get_user_id(), id, seconds)) success = false;
    }
  }
  // Measure from completion: even a slow failed connection attempt must not
  // make every already queued report immediately retry the same outage.
  now = std::chrono::steady_clock::now();
  next_db_attempt = now + std::chrono::seconds(5);
  if (success) {
    meter_save_pending = switch_save_pending = extended_save_pending =
        validity_save_pending = false;
    // Very short heartbeat/replay validity must be refreshed before the
    // stored deadline, not after the normal five-second coalescing window.
    unsigned int seconds = std::max(meter_value.get_value_validity_time_sec(),
                                    switch_value.get_value_validity_time_sec());
    if (seconds) {
      next_db_attempt =
          now + std::chrono::milliseconds(std::min(5000U, seconds * 500U));
    }
  } else {
    // A partial write is repaired from the current state on the next allowed
    // attempt. Never restore an older report over a newer live value.
    meter_save_pending = meter_value.value != nullptr;
    switch_save_pending = switch_value.value != nullptr;
    extended_save_pending = meter_value.extended_value != nullptr;
    validity_save_pending = true;
  }
}

void supla_ocpp_device::notify_channel_change(const supla_ocpp_channel &before,
                                              const supla_ocpp_channel &after) {
  if (!user) return;
  {
    std::lock_guard<std::mutex> lock(mutex);
    if (!active) return;
  }
  bool significant = false;
  bool changed = bool(before.value) != bool(after.value) ||
                 (before.value && after.value &&
                  before.value->is_differ(after.value.get(), &significant));
  bool availability_changed = before.get_availability_status().is_online() !=
                              after.get_availability_status().is_online();
  auto caller = supla_caller(ctChannel, after.channel_id);
  if (changed || availability_changed) {
    user->on_channel_value_changed(caller, id, after.channel_id, false,
                                   significant);
  }
  bool extended_changed =
      bool(before.extended_value) != bool(after.extended_value) ||
      (before.extended_value && after.extended_value &&
       before.extended_value->is_differ(after.extended_value.get()));
  if (extended_changed) {
    user->on_channel_value_changed(caller, id, after.channel_id, true, false);
  }
}

void supla_ocpp_device::raise_value_change_events(
    const supla_ocpp_channel &before, const supla_ocpp_channel &after) {
  if (!user) return;
  {
    std::lock_guard<std::mutex> lock(mutex);
    if (!active) return;
  }
  auto caller = supla_caller(ctChannel, after.channel_id);
  if (before.value && after.value &&
      before.value->is_differ(after.value.get(), nullptr)) {
    user->get_value_based_triggers()->on_value_changed(
        caller, after.channel_id, before.value.get(), after.value.get());
  }
  if (before.extended_value && after.extended_value &&
      before.extended_value->is_differ(after.extended_value.get())) {
    user->get_value_based_triggers()->on_value_changed(
        caller, after.channel_id, before.extended_value.get(),
        after.extended_value.get());
  }
}

void supla_ocpp_device::raise_power_switch_value_change_event(bool before,
                                                              bool after) {
  if (!user || before == after) return;
  {
    std::lock_guard<std::mutex> lock(mutex);
    if (!active) return;
  }
  supla_channel_onoff_value before_value(before);
  supla_channel_onoff_value after_value(after);
  user->get_value_based_triggers()->on_value_changed(
      supla_caller(ctChannel, get_switch_channel_id()), get_switch_channel_id(),
      &before_value, &after_value);
}

void supla_ocpp_device::reload(const supla_ocpp_device_config &config) {
  serializer.post([self = shared_from_this(), config]() {
    supla_ocpp_channel before[2], after[2];
    {
      std::lock_guard<std::mutex> lock(self->mutex);
      if (!self->active || config.revision < self->configuration_revision ||
          config.device_id != self->id ||
          config.meter.id != self->get_meter_channel_id() ||
          config.power_switch.id != self->get_switch_channel_id())
        return;
      self->configuration_revision = config.revision;
      int i = 0;
      for (auto target : {&self->meter, &self->power_switch}) {
        before[i] = target->channel;
        target->config = i == 0 ? config.meter : config.power_switch;
        if (target->channel.value) {
          auto raw = target->channel.raw_value;
          self->set_value(target, raw.data());
        }
        i++;
      }
      electricity_meter_config meter_config(&self->meter.config.json);
      self->meter.channel.add_to_history =
          meter_config.should_be_added_to_history();
      if (self->meter.channel.raw_extended_value) {
        self->set_extended_value(self->meter.channel.get_raw_extended_value(),
                                 true, self->aggregate_energy_counter);
      }
      // Reload recalculates known raw readings without making them fresh
      // samples and never imports DB validity or resets the live analyzer.
      after[0] = self->meter.channel;
      after[1] = self->power_switch.channel;
    }
    self->persist(before[0], before[1], false, true);
    for (int i = 0; i < 2; i++)
      self->notify_channel_change(before[i], after[i]);
  });
}

void supla_ocpp_device::deactivate(void) {
  std::lock_guard<std::mutex> lock(mutex);
  active = false;
  connected = false;
  pending_commands.clear();
  latest_command = 0;
  confirmed_power_switch_value.reset();
  if (analyzer) analyzer->reset();
}

void supla_ocpp_device::renew_validity(supla_ocpp_value_validity validity) {
  serializer.post([self = shared_from_this(), validity]() {
    supla_ocpp_channel before[2], after[2];
    {
      std::lock_guard<std::mutex> lock(self->mutex);
      if (!self->active) return;
      before[0] = self->meter.channel;
      before[1] = self->power_switch.channel;
      self->renew_validity_locked(validity);
      after[0] = self->meter.channel;
      after[1] = self->power_switch.channel;
    }
    self->persist(before[0], before[1], true);
    for (int i = 0; i < 2; i++)
      self->notify_channel_change(before[i], after[i]);
  });
}

void supla_ocpp_device::set_power_switch_channel_value(bool on) {
  char raw[SUPLA_CHANNELVALUE_SIZE] = {};
  power_switch.channel.get_raw_value(raw);
  supla_channel_onoff_value value(raw);
  value.set_on(on);
  value.get_raw_value(raw);
  set_value(&power_switch, raw);
}

void supla_ocpp_device::update_state(bool on,
                                    supla_ocpp_value_validity validity) {
  serializer.post([self = shared_from_this(), on, validity]() {
    supla_ocpp_channel before[2], after[2];
    std::optional<bool> confirmed_before;
    {
      std::lock_guard<std::mutex> lock(self->mutex);
      if (!self->active) return;
      before[0] = self->meter.channel;
      before[1] = self->power_switch.channel;
      confirmed_before = self->confirmed_power_switch_value;
      self->confirmed_power_switch_value = on;
      self->pending_commands.clear();
      self->latest_command = 0;
      self->set_power_switch_channel_value(on);
      self->renew_validity_locked(validity);
      after[0] = self->meter.channel;
      after[1] = self->power_switch.channel;
    }
    self->persist(before[0], before[1], true);
    for (int i = 0; i < 2; i++)
      self->notify_channel_change(before[i], after[i]);
    if (confirmed_before) {
      self->raise_power_switch_value_change_event(*confirmed_before, on);
    }
  });
}

void supla_ocpp_device::update_meter(const supla_ocpp_meter_report &report,
                                     supla_ocpp_value_validity validity) {
  serializer.post([self = shared_from_this(), report, validity]() {
    supla_ocpp_channel before[2], after[2];
    {
      std::lock_guard<std::mutex> lock(self->mutex);
      if (!self->active) return;
      before[0] = self->meter.channel;
      before[1] = self->power_switch.channel;
      char raw[SUPLA_CHANNELVALUE_SIZE] = {};
      bool has_basic = self->meter.channel.get_raw_value(raw);
      supla_channel_em_value basic(raw);
      auto merged = std::make_unique<supla_channel_em_extended_value>();
      supla_channel_em_extended_value samples[3];
      auto current = self->meter.channel.raw_extended_value;
      bool is_raw = current != nullptr;
      if (!current) current = self->meter.channel.extended_value;
      if (current) {
        TSuplaChannelExtendedValue extended = {};
        if (current->get_raw_value(&extended)) merged->set_raw_value(&extended);
      }
      bool has_phase_energy = false;
      for (const auto &energy : report.energy_phases) {
        if (energy) has_phase_energy = true;
      }
      if (self->restoring_phase_energy &&
          (is_raw || report.energy || has_phase_energy)) {
        for (int phase = 0; phase < 3; phase++) {
          if (!(self->energy_phase_mask & (1 << phase)))
            merged->set_fae(phase + 1, 0);
        }
      }
      if (has_phase_energy && self->aggregate_energy_counter) {
        // The charger started reporting real phase counters. Drop the former
        // aggregate-in-L1 convention before applying those measurements.
        for (int phase = 1; phase <= 3; phase++) merged->set_fae(phase, 0);
        self->energy_phase_mask = 0;
      } else if (report.energy && !has_phase_energy &&
                 self->energy_phase_mask) {
        // The reverse mode change must not leave stale L2/L3 phase counters.
        for (int phase = 1; phase <= 3; phase++) merged->set_fae(phase, 0);
        self->energy_phase_mask = 0;
      }
      // Partial-report merging is OCPP-specific. The analyzer receives only
      // this report's fields, never the merged channel containing old fields.
      for (int phase = 0; phase < 3; phase++) {
        if (report.energy_phases[phase]) {
          merged->set_fae(phase + 1, *report.energy_phases[phase] * 0.001);
          self->energy_phase_mask |= 1 << phase;
        }
        if (report.voltage[phase]) {
          double voltage = *report.voltage[phase] * 0.001;
          merged->set_voltage(phase + 1, voltage);
          samples[phase].set_voltage(phase + 1, voltage);
          basic.set_phase_on(phase + 1, true);
        }
        if (report.current[phase]) {
          double current = *report.current[phase] * 0.001;
          merged->set_current(phase + 1, current);
          samples[phase].set_current(phase + 1, current);
          basic.set_phase_on(phase + 1, true);
        }
        if (report.power[phase]) {
          merged->set_power_active(phase + 1, *report.power[phase]);
          samples[phase].set_power_active(phase + 1, *report.power[phase]);
          basic.set_phase_on(phase + 1, true);
        }
      }
      if (has_phase_energy) {
        self->aggregate_energy_counter = false;
      }
      if (report.energy && !has_phase_energy) {
        double energy = *report.energy * 0.001;
        // The IPC energy field is an aggregate, not an L1 measurement or a
        // vector-balanced counter. Keep it in the first ordinary energy slot
        // as a PoC compatibility convention: the existing SUPLA extended value
        // and history have no independent aggregate counter. L2/L3 stay empty;
        // actual phase voltage, current and power readings remain independent.
        merged->set_fae(1, energy);
        self->aggregate_energy_counter = true;
        self->restoring_phase_energy = false;
      }
      if (has_phase_energy) {
        electricity_meter_config config(&self->meter.config.json);
        int flags = config.get_channel_user_flags();
        const int unsupported[] = {SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED,
                                   SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED,
                                   SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED};
        unsigned char expected_phases = 0;
        for (int phase = 0; phase < 3; phase++) {
          if (!(flags & unsupported[phase])) expected_phases |= 1 << phase;
        }
        if ((self->energy_phase_mask & expected_phases) == expected_phases)
          self->restoring_phase_energy = false;
      }
      if (report.energy && !has_phase_energy) {
        basic.set_total_forward_active_energy(*report.energy * 0.001);
        has_basic = true;
      } else if (has_phase_energy && !self->restoring_phase_energy) {
        basic.set_total_forward_active_energy(merged->get_fae_sum());
        has_basic = true;
      }
      if (report.energy || has_phase_energy) {
        // Only received energy fields are raw. Missing restored counters are
        // retained separately in the display value until raw readings arrive.
        is_raw = true;
      }
      if (has_basic) {
        basic.get_raw_value(raw);
        self->set_value(&self->meter, raw);
      }
      self->set_extended_value(merged.release(), is_raw,
                               self->aggregate_energy_counter);
      self->renew_validity_locked(validity);
      // Reconnection restores cached fields of different ages. Keep their
      // display/counter values, but never count them as new analyzer samples.
      if (!report.snapshot && self->analyzer &&
          self->meter.channel.get_availability_status().is_online()) {
        if ((report.energy || has_phase_energy) &&
            self->meter.channel.raw_energy_complete) {
          self->analyzer->add_sample(self->meter.channel.value.get(),
                                     &self->meter.config.json);
        }
        const int flags[] = {SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED,
                             SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED,
                             SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED};
        for (int phase = 0; phase < 3; phase++) {
          if (samples[phase].get_measured_values()) {
            self->analyzer->add_sample(
                (flags[0] | flags[1] | flags[2]) & ~flags[phase],
                &self->meter.config.json, &samples[phase]);
          }
        }
      }
      after[0] = self->meter.channel;
      after[1] = self->power_switch.channel;
    }
    self->persist(before[0], before[1], true);
    for (int i = 0; i < 2; i++) {
      self->notify_channel_change(before[i], after[i]);
      self->raise_value_change_events(before[i], after[i]);
    }
  });
}

bool supla_ocpp_device::request_charging(unsigned long long command_id, bool on,
                                         bool toggle,
                                         std::function<bool(bool)> send) {
  if (!can_set_charging(get_switch_channel_id())) return false;
  serializer.post([self = shared_from_this(), command_id, on, toggle, send]() {
    supla_ocpp_channel before, after, before_meter;
    bool requested = on;
    {
      std::lock_guard<std::mutex> lock(self->mutex);
      if (!self->active || !self->connected ||
          self->power_switch.channel.get_availability_status().is_offline())
        return;
      before = self->power_switch.channel;
      before_meter = self->meter.channel;
      char raw[SUPLA_CHANNELVALUE_SIZE] = {};
      before.get_value(raw);
      bool previous = supla_channel_onoff_value(raw).is_on();
      if (toggle) requested = !previous;
      if (self->latest_command) {
        previous = self->pending_commands.at(self->latest_command).previous_on;
      }
      self->pending_commands[command_id] = {requested, previous};
      self->latest_command = command_id;
      self->set_power_switch_channel_value(requested);
      after = self->power_switch.channel;
    }
    self->persist(before_meter, before, false);
    self->notify_channel_change(before, after);
    bool sent = false;
    try {
      sent = send(requested);
    } catch (...) {
      supla_log(LOG_WARNING, "OCPP command send failed for device %i",
                self->id);
    }
    if (!sent) self->finish_command(command_id, false);
  });
  return true;
}

void supla_ocpp_device::finish_command(unsigned long long command_id, bool ok) {
  supla_ocpp_channel before, after, before_meter;
  bool rollback = false;
  {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = pending_commands.find(command_id);
    if (!active || it == pending_commands.end()) return;
    auto command = it->second;
    pending_commands.erase(it);
    // A delayed success must not replace a newer accepted rollback baseline.
    if (ok && command_id > latest_accepted_command) {
      latest_accepted_command = command_id;
      // Accepted is a command acknowledgement, not a state report. It may
      // update the rollback baseline, but only update_state can fire VBT.
      for (auto &pending : pending_commands) {
        if (pending.first > command_id) pending.second.previous_on = command.on;
      }
    }
    if (latest_command == command_id) {
      latest_command = 0;
      if (!ok) {
        before = power_switch.channel;
        before_meter = meter.channel;
        set_power_switch_channel_value(command.previous_on);
        after = power_switch.channel;
        rollback = true;
      }
    }
  }
  if (rollback) {
    persist(before_meter, before, false);
    notify_channel_change(before, after);
  }
}

void supla_ocpp_device::on_result(unsigned long long command_id, bool ok) {
  serializer.post([self = shared_from_this(), command_id, ok]() {
    self->finish_command(command_id, ok);
  });
}

void supla_ocpp_device::disconnect(void) {
  serializer.post([self = shared_from_this()]() {
    supla_ocpp_channel before[2], after[2];
    {
      std::lock_guard<std::mutex> lock(self->mutex);
      if (!self->active) return;
      before[0] = self->meter.channel;
      before[1] = self->power_switch.channel;
      if (self->latest_command) {
        self->set_power_switch_channel_value(
            self->pending_commands.at(self->latest_command).previous_on);
      }
      self->pending_commands.clear();
      self->latest_command = 0;
      self->renew_validity_locked(0);
      after[0] = self->meter.channel;
      after[1] = self->power_switch.channel;
    }
    self->persist(before[0], before[1], true, true);
    for (int i = 0; i < 2; i++)
      self->notify_channel_change(before[i], after[i]);
  });
}
