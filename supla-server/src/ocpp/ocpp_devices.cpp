// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ocpp/ocpp_devices.h"

#include <utility>

#include "db/mariadb_access_provider.h"
#include "device/extended_value/channel_extended_value_envelope.h"
#include "ocpp/ocpp_dao.h"
#include "user/user.h"

supla_ocpp_devices::supla_ocpp_devices(supla_user *user) : user(user) {}

supla_ocpp_devices::~supla_ocpp_devices() {
  std::vector<std::shared_ptr<supla_ocpp_device>> removed;
  {
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto &item : devices) removed.push_back(item.second);
    devices.clear();
  }
  for (const auto &device : removed) device->deactivate();
}

void supla_ocpp_devices::load_if_not_loaded(void) {
  bool needs_load;
  {
    std::lock_guard<std::mutex> lock(mutex);
    needs_load = !loaded;
  }
  if (needs_load) reload();
}

std::shared_ptr<supla_ocpp_device> supla_ocpp_devices::get(int device_id,
                                                           int channel_id) {
  load_if_not_loaded();
  std::lock_guard<std::mutex> lock(mutex);
  if (device_id) {
    auto it = devices.find(device_id);
    return it == devices.end() ? nullptr : it->second;
  }
  if (channel_id) {
    for (const auto &item : devices) {
      if (item.second->get_meter_channel_id() == channel_id ||
          item.second->get_switch_channel_id() == channel_id) {
        return item.second;
      }
    }
  }
  return nullptr;
}

supla_ocpp_channel supla_ocpp_devices::get_channel(int channel_id) {
  auto device = get(0, channel_id);
  return device ? device->get_channel(channel_id) : supla_ocpp_channel{};
}

bool supla_ocpp_devices::is_online(int device_id) {
  auto device = get(device_id);
  return device && device->is_online();
}

void supla_ocpp_devices::reload(int device_id) {
  std::lock_guard<std::mutex> loading(load_mutex);
  supla_mariadb_access_provider dba;
  supla_ocpp_dao dao(&dba);
  std::vector<supla_ocpp_device_config> loaded_devices;
  if (!dao.get_devices(user->getUserID(), device_id, &loaded_devices)) return;

  std::vector<std::shared_ptr<supla_ocpp_device>> removed;
  std::vector<
      std::pair<std::shared_ptr<supla_ocpp_device>, supla_ocpp_device_config>>
      changed;
  {
    std::lock_guard<std::mutex> lock(mutex);
    unsigned long long revision = ++configuration_revision;
    for (auto it = devices.begin(); it != devices.end();) {
      if (device_id && it->first != device_id) {
        ++it;
        continue;
      }
      bool exists = false;
      for (const auto &config : loaded_devices) {
        if (config.device_id == it->first &&
            config.meter.id == it->second->get_meter_channel_id() &&
            config.power_switch.id == it->second->get_switch_channel_id()) {
          exists = true;
          break;
        }
      }
      if (exists) {
        ++it;
      } else {
        removed.push_back(it->second);
        it = devices.erase(it);
      }
    }
    for (auto &config : loaded_devices) {
      config.revision = revision;
      auto it = devices.find(config.device_id);
      if (it == devices.end()) {
        devices[config.device_id] =
            std::make_shared<supla_ocpp_device>(user, config);
      } else {
        changed.emplace_back(it->second, config);
      }
    }
    if (!device_id) loaded = true;
  }

  for (const auto &device : removed) device->deactivate();
  for (const auto &item : changed) item.first->reload(item.second);
}

void supla_ocpp_devices::on_channel_deleted(int device_id, int channel_id) {
  std::lock_guard<std::mutex> loading(load_mutex);
  std::shared_ptr<supla_ocpp_device> removed;
  {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = devices.find(device_id);
    if (it != devices.end() &&
        (it->second->get_meter_channel_id() == channel_id ||
         it->second->get_switch_channel_id() == channel_id)) {
      removed = it->second;
      devices.erase(it);
    }
  }
  if (removed) removed->deactivate();
}

void supla_ocpp_devices::on_device_deleted(int device_id) {
  std::lock_guard<std::mutex> loading(load_mutex);
  std::shared_ptr<supla_ocpp_device> removed;
  {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = devices.find(device_id);
    if (it != devices.end()) {
      removed = it->second;
      devices.erase(it);
    }
  }
  if (removed) removed->deactivate();
}

void supla_ocpp_devices::access_data_analyzers(
    std::function<void(supla_electricity_analyzer *)> callback) {
  std::vector<std::shared_ptr<supla_ocpp_device>> current;
  {
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto &item : devices) current.push_back(item.second);
  }
  for (const auto &device : current) device->access_data_analyzer(callback);
}

void supla_ocpp_devices::get_meter_values(
    std::vector<supla_abstract_channel_extended_value_envelope *> *values) {
  std::vector<std::shared_ptr<supla_ocpp_device>> current;
  {
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto &item : devices) current.push_back(item.second);
  }
  for (const auto &device : current) {
    auto value = device->get_channel(device->get_meter_channel_id())
                     .get_extended_value(true);
    if (value) {
      values->push_back(new supla_abstract_channel_extended_value_envelope(
          device->get_meter_channel_id(), value));
    }
  }
}
