// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SUPLA_OCPP_DEVICE_H_
#define SUPLA_OCPP_DEVICE_H_

#include <sys/time.h>

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>

#include "device/channel_availability_status.h"
#include "device/extended_value/abstract_channel_extended_value.h"
#include "device/value/abstract_channel_value.h"
#include "jsonconfig/json_config.h"
#include "ocpp/ocpp_task_queue.h"
#include "ocpp/ocpp_value_validity.h"

class supla_user;
class supla_electricity_analyzer;
class supla_abstract_data_analyzer;

struct supla_ocpp_channel_config {
  int id = 0;
  int type = 0;
  int func = 0;
  int param1 = 0;
  int param2 = 0;
  int param3 = 0;
  int param4 = 0;
  supla_json_config json;
};

// Loaded only to restore the last value after server startup. Persisted values
// is not evidence of a live charger and must never initialize its analyzer.
struct supla_ocpp_channel_persisted_values {
  std::optional<std::array<char, SUPLA_CHANNELVALUE_SIZE>> value;
  std::shared_ptr<TSuplaChannelExtendedValue> extended_value;
};

struct supla_ocpp_device_config {
  int device_id = 0;
  unsigned long long revision = 0;
  supla_ocpp_channel_config meter;
  supla_ocpp_channel_config power_switch;
  supla_ocpp_channel_persisted_values meter_persisted_values;
  supla_ocpp_channel_persisted_values power_switch_persisted_values;
};

// Live reports contain only fresh fields; missing fields remain absent.
// Reconnection snapshots restore values but contain no new analyzer samples.
struct supla_ocpp_meter_report {
  bool snapshot = false;
  std::optional<long long> energy;
  std::array<std::optional<long long>, 3> energy_phases;
  std::array<std::optional<long long>, 3> power;
  std::array<std::optional<long long>, 3> voltage;
  std::array<std::optional<long long>, 3> current;
};

// Cheap, read-only channel copy. Values are replaced, never mutated after they
// have been published. A copy shares immutable values, not config/analyzers.
class supla_ocpp_channel {
 private:
  friend class supla_ocpp_device;
  int device_id = 0;
  int channel_id = 0;
  int type = 0;
  int func = 0;
  std::array<char, SUPLA_CHANNELVALUE_SIZE> raw_value = {};
  std::shared_ptr<supla_abstract_channel_value> value;
  std::shared_ptr<supla_abstract_channel_extended_value> extended_value;
  std::shared_ptr<supla_abstract_channel_extended_value> raw_extended_value;
  supla_ocpp_value_validity validity;
  bool add_to_history = false;
  bool raw_energy_complete = false;

 public:
  int get_device_id(void) const;
  int get_channel_id(void) const;
  int get_type(void) const;
  int get_func(void) const;
  const supla_abstract_channel_value *get_value(void) const;
  bool get_value(char value[SUPLA_CHANNELVALUE_SIZE]) const;
  bool get_raw_value(char value[SUPLA_CHANNELVALUE_SIZE]) const;
  supla_abstract_channel_extended_value *get_extended_value(
      bool for_data_logger_purposes = false) const;
  supla_abstract_channel_extended_value *get_raw_extended_value(void) const;
  supla_channel_availability_status get_availability_status(void) const;
  unsigned int get_value_validity_time_sec(void) const;
};

// One stable owner per charger. All writes (including config reload) are
// serialized. Readers/logger callbacks hold only the short-lived state lock;
// database access and notifications always run after releasing that lock.
class supla_ocpp_device
    : public std::enable_shared_from_this<supla_ocpp_device> {
 private:
  struct runtime_channel {
    supla_ocpp_channel_config config;
    supla_ocpp_channel channel;
  };
  struct pending_command {
    bool on;
    bool previous_on;
  };

  supla_user *user;
  const int id;
  mutable std::mutex mutex;
  supla_ocpp_task_queue serializer;
  bool active = true;
  unsigned long long configuration_revision = 0;
  bool connected = false;
  // Accessed only by serializer jobs. Deferred reports retain the latest
  // state, not a queue of historical SQL writes. Failures retain these flags.
  bool meter_save_pending = false;
  bool switch_save_pending = false;
  bool extended_save_pending = false;
  bool validity_save_pending = false;
  std::chrono::steady_clock::time_point next_db_attempt;
  bool aggregate_energy_counter = false;
  unsigned char energy_phase_mask = 0;
  bool restoring_phase_energy = false;
  runtime_channel meter;
  runtime_channel power_switch;
  std::unique_ptr<supla_abstract_data_analyzer> analyzer;
  std::map<unsigned long long, pending_command> pending_commands;
  unsigned long long latest_command = 0;
  unsigned long long latest_accepted_command = 0;
  // The public channel may change optimistically. Value-based triggers compare
  // only station-confirmed values so a rejected command cannot fire actions.
  std::optional<bool> confirmed_power_switch_value;

  void initialize(runtime_channel *target,
                  const supla_ocpp_channel_config &config,
                  const supla_ocpp_channel_persisted_values &persisted_values);
  void set_value(runtime_channel *target,
                 const char value[SUPLA_CHANNELVALUE_SIZE]);
  void set_extended_value(supla_abstract_channel_extended_value *value,
                          bool is_raw, bool aggregate_energy);
  void renew_validity_locked(supla_ocpp_value_validity validity);
  // The caller must hold mutex.
  void set_power_switch_channel_value(bool on);
  void persist(const supla_ocpp_channel &before_meter,
               const supla_ocpp_channel &before_switch, bool renew_validity,
               bool force = false);
  void notify_channel_change(const supla_ocpp_channel &before,
                             const supla_ocpp_channel &after);
  void raise_value_change_events(const supla_ocpp_channel &before,
                                 const supla_ocpp_channel &after);
  void raise_power_switch_value_change_event(bool before, bool after);
  void finish_command(unsigned long long command_id, bool ok);

 public:
  supla_ocpp_device(supla_user *user, const supla_ocpp_device_config &config);
  ~supla_ocpp_device();
  supla_ocpp_device(const supla_ocpp_device &) = delete;
  supla_ocpp_device &operator=(const supla_ocpp_device &) = delete;

  int get_id(void) const;
  int get_user_id(void) const;
  int get_meter_channel_id(void) const;
  int get_switch_channel_id(void) const;
  bool is_online(void) const;
  bool can_set_charging(int channel_id) const;
  supla_ocpp_channel get_channel(int channel_id) const;
  void access_data_analyzer(
      std::function<void(supla_electricity_analyzer *)> callback);
  void reload(const supla_ocpp_device_config &config);
  void deactivate(void);
  void renew_validity(supla_ocpp_value_validity validity);
  void update_state(bool on, supla_ocpp_value_validity validity);
  void update_meter(const supla_ocpp_meter_report &report,
                    supla_ocpp_value_validity validity);
  bool request_charging(unsigned long long command_id, bool on, bool toggle,
                        std::function<bool(bool)> send);
  void on_result(unsigned long long command_id, bool ok);
  void disconnect(void);
};

#endif  // SUPLA_OCPP_DEVICE_H_
