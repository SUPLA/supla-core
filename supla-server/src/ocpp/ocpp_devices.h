// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SUPLA_OCPP_DEVICES_H_
#define SUPLA_OCPP_DEVICES_H_

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "ocpp/ocpp_device.h"

class supla_abstract_channel_extended_value_envelope;
class supla_electricity_analyzer;
class supla_user;

class supla_ocpp_devices {
 private:
  supla_user *user;
  std::mutex mutex;
  std::mutex load_mutex;
  std::map<int, std::shared_ptr<supla_ocpp_device>> devices;
  bool loaded = false;
  unsigned long long configuration_revision = 0;

  void load_if_not_loaded(void);

 public:
  explicit supla_ocpp_devices(supla_user *user);
  ~supla_ocpp_devices();
  supla_ocpp_devices(const supla_ocpp_devices &) = delete;
  supla_ocpp_devices &operator=(const supla_ocpp_devices &) = delete;

  std::shared_ptr<supla_ocpp_device> get(int device_id, int channel_id = 0);
  supla_ocpp_channel get_channel(int channel_id);
  bool is_online(int device_id);
  void reload(int device_id = 0);
  void on_channel_deleted(int device_id, int channel_id);
  void on_device_deleted(int device_id);
  void access_data_analyzers(
      std::function<void(supla_electricity_analyzer *)> callback);
  void get_meter_values(
      std::vector<supla_abstract_channel_extended_value_envelope *> *values);
};

#endif  // SUPLA_OCPP_DEVICES_H_
