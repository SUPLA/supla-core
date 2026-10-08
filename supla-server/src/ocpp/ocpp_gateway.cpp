// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ocpp/ocpp_gateway.h"

#include <arpa/inet.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <poll.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "ipc/ipcsocket.h"
#include "log.h"
#include "ocpp/ocpp_accept_loop.h"
#include "ocpp/ocpp_device.h"
#include "proto.h"
#include "sthread.h"
#include "svrcfg.h"
#include "user/user.h"
#include "user/userdevices.h"

using nlohmann::json;

namespace {

constexpr unsigned int kProtocolVersion = 1;
constexpr unsigned int kDefaultMaxMessageBytes = 64 * 1024;

bool json_int(const json &message, const char *name, long long minimum,
              long long maximum, long long *value) {
  auto item = message.find(name);
  if (item == message.end() || !item->is_number_integer()) {
    return false;
  }
  try {
    if (item->is_number_unsigned() &&
        item->get<unsigned long long>() >
            static_cast<unsigned long long>(maximum)) {
      return false;
    }
    long long result = item->get<long long>();
    if (result < minimum || result > maximum) {
      return false;
    }
    *value = result;
    return true;
  } catch (...) {
    return false;
  }
}

bool json_string(const json &message, const char *name, size_t minimum_size,
                 size_t maximum_size, std::string *value) {
  auto item = message.find(name);
  if (item == message.end() || !item->is_string()) {
    return false;
  }
  try {
    std::string result = item->get<std::string>();
    if (result.size() < minimum_size || result.size() > maximum_size) {
      return false;
    }
    *value = result;
    return true;
  } catch (...) {
    return false;
  }
}

bool valid_optional_integer(const json &message, const char *name,
                            long long minimum, long long maximum) {
  auto item = message.find(name);
  if (item == message.end()) {
    return true;
  }
  long long ignored = 0;
  return json_int(message, name, minimum, maximum, &ignored);
}

bool valid_phase_array(const json &message, const char *name,
                       long long maximum) {
  auto item = message.find(name);
  if (item == message.end()) {
    return true;
  }
  if (!item->is_array() || item->size() > 3) {
    return false;
  }
  for (const auto &value : *item) {
    if (value.is_null()) {
      continue;
    }
    if (!value.is_number_integer()) {
      return false;
    }
    try {
      long long integer = value.get<long long>();
      if (integer < 0 || integer > maximum) {
        return false;
      }
    } catch (...) {
      return false;
    }
  }
  return true;
}

bool write_all(int socket_fd, const char *buffer, size_t size) {
  while (size > 0) {
#ifdef MSG_NOSIGNAL
    ssize_t written = send(socket_fd, buffer, size, MSG_NOSIGNAL);
#else
    ssize_t written = send(socket_fd, buffer, size, 0);
#endif
    if (written > 0) {
      buffer += written;
      size -= written;
      continue;
    }
    if (written < 0 && errno == EINTR) {
      continue;
    }
    if (written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      pollfd descriptor = {socket_fd, POLLOUT, 0};
      if (poll(&descriptor, 1, 5000) > 0 && (descriptor.revents & POLLOUT)) {
        continue;
      }
    }
    return false;
  }
  return true;
}

}  // namespace

supla_ocpp_gateway::supla_ocpp_gateway() {
  ipc = nullptr;
  accept_loop_thread = nullptr;
  connection = nullptr;
  enabled = false;
  unregistering = false;
  max_message_bytes.store(kDefaultMaxMessageBytes);
  next_command_id = 1;
}

supla_ocpp_gateway::~supla_ocpp_gateway() { workers.stop(); }

supla_ocpp_gateway *supla_ocpp_gateway::global_instance(void) {
  static supla_ocpp_gateway instance;
  return &instance;
}

bool supla_ocpp_gateway::start(void) {
  bool config_enabled = scfg_bool(CFG_OCPP_ENABLED) == 1;
  configure(config_enabled, scfg_int(CFG_OCPP_MAX_MESSAGE_BYTES));
  if (!config_enabled) {
    return true;
  }
  if (ipc) {
    return true;
  }

  const char *socket_path = scfg_string(CFG_OCPP_SOCKET_PATH);
  if (strcmp(scfg_string(CFG_IPC_SOCKET_PATH), socket_path) == 0) {
    supla_log(LOG_ERR, "OCPP and control IPC socket paths must be different");
    configure(false, scfg_int(CFG_OCPP_MAX_MESSAGE_BYTES));
    return false;
  }

  ipc = ipcsocket_init(socket_path);
  if (!ipc) {
    configure(false, scfg_int(CFG_OCPP_MAX_MESSAGE_BYTES));
    return false;
  }

  struct group socket_group_entry;
  struct group *socket_group = nullptr;
  long group_buffer_size = sysconf(_SC_GETGR_R_SIZE_MAX);
  if (group_buffer_size < 1024) {
    group_buffer_size = 16384;
  }
  std::vector<char> group_buffer(group_buffer_size);
  if (getgrnam_r(scfg_string(CFG_OCPP_SOCKET_GROUP), &socket_group_entry,
                 group_buffer.data(), group_buffer.size(),
                 &socket_group) != 0 ||
      !socket_group || chown(socket_path, -1, socket_group->gr_gid) != 0) {
    supla_log(LOG_ERR, "Cannot set group of the OCPP IPC socket");
    ipcsocket_free(ipc);
    ipc = nullptr;
    configure(false, scfg_int(CFG_OCPP_MAX_MESSAGE_BYTES));
    return false;
  }

  sthread_simple_run(ocpp_accept_loop, ipc, 0, &accept_loop_thread);
  return true;
}

void supla_ocpp_gateway::stop(void) {
  configure(false, max_message_bytes.load());
  if (ipc) {
    ipcsocket_close(ipc);
    if (accept_loop_thread) {
      sthread_twf(accept_loop_thread, true);
      accept_loop_thread = nullptr;
    }
    ipcsocket_free(ipc);
    ipc = nullptr;
  }
  workers.wait_until_idle();
}

void supla_ocpp_gateway::configure(bool enabled,
                                   unsigned int max_message_bytes) {
  std::lock_guard<std::mutex> lock(mutex);
  this->enabled = enabled;
  this->max_message_bytes.store(std::max(
      1024U,
      std::min(1024U * 1024U, max_message_bytes ? max_message_bytes
                                                : kDefaultMaxMessageBytes)));
}

unsigned int supla_ocpp_gateway::get_max_message_bytes(void) {
  return max_message_bytes.load();
}

void supla_ocpp_gateway::wait_until_idle(void) { workers.wait_until_idle(); }

std::shared_ptr<supla_ocpp_device> supla_ocpp_gateway::find_device(
    const std::string &user_suid, int device_id) {
  supla_user *user = supla_user::find_by_suid(user_suid.c_str());
  if (!user) {
    int user_id = supla_user::suid_to_user_id(user_suid.c_str(), true);
    user = supla_user::find(user_id, user_id > 0);
  }
  return user ? user->get_devices()->get_ocpp_device(device_id) : nullptr;
}

void supla_ocpp_gateway::remove_pending_for_device_locked(int device_id) {
  for (auto it = pending_commands.begin(); it != pending_commands.end();) {
    auto device = it->second.lock();
    if (!device || device->get_id() == device_id) {
      it = pending_commands.erase(it);
    } else {
      ++it;
    }
  }
}

supla_ocpp_action_result supla_ocpp_gateway::try_set_charging(supla_user *user,
                                                              int channel_id,
                                                              bool on) {
  if (!user) return supla_ocpp_action_result::not_ocpp;
  auto device = user->get_devices()->get_ocpp_device(0, channel_id);
  if (!device) return supla_ocpp_action_result::not_ocpp;
  if (!device->can_set_charging(channel_id) || !get_connection())
    return supla_ocpp_action_result::rejected;
  // Accepted for asynchronous processing; station reports/results update the
  // visible state. An unavailable OCPP channel never falls through to SRPC.
  return workers.post(device->get_id(),
                      [this, device, on]() { set_charging(device, on, false); })
             ? supla_ocpp_action_result::accepted
             : supla_ocpp_action_result::rejected;
}

supla_ocpp_action_result supla_ocpp_gateway::try_toggle_charging(
    supla_user *user, int channel_id) {
  if (!user) return supla_ocpp_action_result::not_ocpp;
  auto device = user->get_devices()->get_ocpp_device(0, channel_id);
  if (!device) return supla_ocpp_action_result::not_ocpp;
  if (!device->can_set_charging(channel_id) || !get_connection())
    return supla_ocpp_action_result::rejected;
  return workers.post(device->get_id(),
                      [this, device]() { set_charging(device, false, true); })
             ? supla_ocpp_action_result::accepted
             : supla_ocpp_action_result::rejected;
}

std::shared_ptr<supla_ocpp_connection> supla_ocpp_gateway::get_connection(
    void) {
  std::lock_guard<std::mutex> lock(mutex);
  return enabled ? connection : nullptr;
}

void supla_ocpp_gateway::set_charging(std::shared_ptr<supla_ocpp_device> device,
                                      bool on, bool toggle) {
  std::shared_ptr<supla_ocpp_connection> target;
  unsigned long long id;
  {
    std::lock_guard<std::mutex> lock(mutex);
    if (!enabled || !connection) return;
    target = connection;
    id = next_command_id++;
    if (next_command_id > static_cast<unsigned long long>(LLONG_MAX)) {
      next_command_id = 1;
    }
    // Only a weak command route belongs to the transport. The state and
    // rollback baseline are owned by the user's device, not by a second
    // gateway cache.
    pending_commands[id] = device;
  }
  int device_id = device->get_id();
  if (!device->request_charging(
          id, on, toggle, [target, id, device_id](bool on) {
            return target->send_message({{"id", id},
                                         {"type", on ? "on" : "off"},
                                         {"device", device_id},
                                         {"connector", 1}});
          })) {
    std::lock_guard<std::mutex> lock(mutex);
    pending_commands.erase(id);
  }
}

bool supla_ocpp_gateway::register_connection(
    std::shared_ptr<supla_ocpp_connection> connection) {
  std::lock_guard<std::mutex> lock(mutex);
  if (!enabled || unregistering || !connection) return false;
  if (this->connection && this->connection != connection) {
    supla_log(LOG_WARNING, "Rejected a second OCPP gateway connection");
    return false;
  }
  this->connection = connection;
  return true;
}

void supla_ocpp_gateway::unregister_connection(
    supla_ocpp_connection *connection) {
  std::shared_ptr<supla_ocpp_connection> previous;
  {
    std::lock_guard<std::mutex> lock(mutex);
    if (this->connection.get() != connection) return;
    unregistering = true;
    previous = std::move(this->connection);
  }

  // All frames preceding the disconnect have already been dispatched. Wait
  // for them before taking the connected-device snapshot, then enqueue each
  // disconnect on the same per-device worker used by its earlier reports.
  workers.wait_until_idle();
  std::vector<std::shared_ptr<supla_ocpp_device>> devices;
  {
    std::lock_guard<std::mutex> lock(mutex);
    pending_commands.clear();
    for (const auto &item : connected_devices) {
      if (auto device = item.second.lock()) devices.push_back(device);
    }
    connected_devices.clear();
  }
  for (const auto &device : devices) {
    workers.post(device->get_id(), [device]() { device->disconnect(); });
  }
  {
    std::lock_guard<std::mutex> lock(mutex);
    unregistering = false;
  }
}

void supla_ocpp_gateway::on_result(unsigned long long id, bool ok,
                                   const std::string &error) {
  std::shared_ptr<supla_ocpp_device> device;
  {
    std::lock_guard<std::mutex> lock(mutex);
    auto item = pending_commands.find(id);
    if (item == pending_commands.end()) return;
    device = item->second.lock();
    pending_commands.erase(item);
  }
  if (device) {
    workers.post(device->get_id(), [device, id, ok, error]() {
      if (!ok) {
        supla_log(LOG_WARNING, "OCPP command %llu rejected for device %i: %s",
                  id, device->get_id(), error.c_str());
      }
      device->on_result(id, ok);
    });
  }
}

void supla_ocpp_gateway::on_connected(const std::string &user_suid,
                                      int device_id,
                                      supla_ocpp_value_validity validity) {
  workers.post(device_id, [this, user_suid, device_id, validity]() {
    auto device = find_device(user_suid, device_id);
    if (!device) {
      supla_log(LOG_WARNING, "Unknown or incomplete OCPP device %i", device_id);
      return;
    }
    {
      std::lock_guard<std::mutex> lock(mutex);
      connected_devices[device_id] = device;
    }
    device->renew_validity(validity);
  });
}

void supla_ocpp_gateway::on_disconnected(const std::string &user_suid,
                                         int device_id) {
  workers.post(device_id, [this, user_suid, device_id]() {
    auto device = find_device(user_suid, device_id);
    if (!device) return;
    {
      std::lock_guard<std::mutex> lock(mutex);
      connected_devices.erase(device_id);
      remove_pending_for_device_locked(device_id);
    }
    device->disconnect();
  });
}

void supla_ocpp_gateway::on_alive(const std::string &user_suid, int device_id,
                                  supla_ocpp_value_validity validity) {
  on_connected(user_suid, device_id, validity);
}

void supla_ocpp_gateway::on_state(const std::string &user_suid, int device_id,
                                  int connector_id, bool on,
                                  supla_ocpp_value_validity validity) {
  workers.post(device_id,
               [this, user_suid, device_id, connector_id, on, validity]() {
                 if (connector_id != 1) return;
                 auto device = find_device(user_suid, device_id);
                 if (!device) return;
                 {
                   std::lock_guard<std::mutex> lock(mutex);
                   connected_devices[device_id] = device;
                   remove_pending_for_device_locked(device_id);
                 }
                 device->update_state(on, validity);
               });
}

void supla_ocpp_gateway::on_meter(const std::string &user_suid, int device_id,
                                  int connector_id, const json &message,
                                  supla_ocpp_value_validity validity) {
  workers.post(device_id, [this, user_suid, device_id, connector_id, message,
                           validity]() {
    if (connector_id != 0 && connector_id != 1) return;
    auto device = find_device(user_suid, device_id);
    if (!device) return;
    supla_ocpp_meter_report report;
    report.snapshot = message.value("snapshot", false);
    if (message.contains("energy"))
      report.energy = message.at("energy").get<long long>();
    // Phase arrays contain only actual OCPP phase readings. Never invent
    // a phase split from an aggregate power or energy value.
    auto read_phases = [&](const char *name, auto &target) {
      auto it = message.find(name);
      if (it == message.end()) return;
      for (int phase = 0; phase < 3 && phase < static_cast<int>(it->size());
           phase++) {
        if (!(*it)[phase].is_null())
          target[phase] = (*it)[phase].get<long long>();
      }
    };
    read_phases("voltage", report.voltage);
    read_phases("current", report.current);
    read_phases("power_phases", report.power);
    read_phases("energy_phases", report.energy_phases);
    {
      std::lock_guard<std::mutex> lock(mutex);
      connected_devices[device_id] = device;
    }
    device->update_meter(report, validity);
  });
}

supla_ocpp_connection::supla_ocpp_connection(int socket_fd) {
  this->socket_fd = socket_fd;
  registered = false;
#if !defined(MSG_NOSIGNAL) && defined(SO_NOSIGPIPE)
  int enabled = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_NOSIGPIPE, &enabled, sizeof(enabled));
#endif
}

supla_ocpp_connection::~supla_ocpp_connection() {
  if (socket_fd >= 0) {
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
  }
}

bool supla_ocpp_connection::read_exact(void *sthread, char *buffer,
                                       size_t size) {
  while (size > 0 && !sthread_isterminated(sthread)) {
    ssize_t received = recv(socket_fd, buffer, size, 0);
    if (received > 0) {
      buffer += received;
      size -= received;
      continue;
    }
    if (received == 0) {
      return false;
    }
    if (errno == EINTR) {
      continue;
    }
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      pollfd descriptor = {socket_fd, POLLIN, 0};
      int result = poll(&descriptor, 1, 250);
      if (result == 0) {
        continue;
      }
      if (result > 0 && (descriptor.revents & POLLIN)) {
        continue;
      }
    }
    return false;
  }
  return size == 0;
}

bool supla_ocpp_connection::read_frame(void *sthread, std::string *payload) {
  uint32_t network_size = 0;
  if (!read_exact(sthread, reinterpret_cast<char *>(&network_size),
                  sizeof(network_size))) {
    return false;
  }
  uint32_t size = ntohl(network_size);
  if (!size ||
      size > supla_ocpp_gateway::global_instance()->get_max_message_bytes()) {
    supla_log(LOG_WARNING, "Invalid OCPP gateway frame size: %u", size);
    return false;
  }
  payload->resize(size);
  return read_exact(sthread, &(*payload)[0], size);
}

bool supla_ocpp_connection::send_message(const json &message) {
  std::string payload;
  try {
    payload = message.dump();
  } catch (...) {
    return false;
  }
  if (payload.empty() ||
      payload.size() >
          supla_ocpp_gateway::global_instance()->get_max_message_bytes()) {
    return false;
  }

  uint32_t network_size = htonl(static_cast<uint32_t>(payload.size()));
  std::lock_guard<std::mutex> lock(write_mutex);
  if (write_failed) {
    return false;
  }
  bool success =
      write_all(socket_fd, reinterpret_cast<const char *>(&network_size),
                sizeof(network_size)) &&
      write_all(socket_fd, payload.data(), payload.size());
  if (!success) {
    write_failed = true;
    shutdown(socket_fd, SHUT_RDWR);
  }
  return success;
}

bool supla_ocpp_connection::process_message(const json &message) {
  if (!message.is_object()) {
    return false;
  }
  long long validity = channel_validity_time_sec;
  if (message.contains("validity") &&
      !json_int(message, "validity", 1, channel_validity_time_sec, &validity)) {
    return false;
  }
  auto type_item = message.find("type");
  std::string type;
  if (type_item != message.end()) {
    if (!type_item->is_string()) {
      return false;
    }
    type = type_item->get<std::string>();
  }

  if (type == "connected" || type == "disconnected" || type == "alive") {
    std::string user_suid;
    long long device = 0;
    if (!json_string(message, "user", 1, SHORT_UNIQUEID_MAXSIZE - 1,
                     &user_suid) ||
        !json_int(message, "device", 1, INT_MAX, &device)) {
      return false;
    }
    if (type == "connected") {
      supla_ocpp_gateway::global_instance()->on_connected(user_suid, device,
                                                          validity);
    } else if (type == "disconnected") {
      supla_ocpp_gateway::global_instance()->on_disconnected(user_suid, device);
    } else {
      supla_ocpp_gateway::global_instance()->on_alive(user_suid, device,
                                                      validity);
    }
    return true;
  }

  if (type == "state") {
    std::string user_suid;
    long long device = 0;
    long long connector = 0;
    auto on_item = message.find("on");
    if (!json_string(message, "user", 1, SHORT_UNIQUEID_MAXSIZE - 1,
                     &user_suid) ||
        !json_int(message, "device", 1, INT_MAX, &device) ||
        !json_int(message, "connector", 1, INT_MAX, &connector) ||
        on_item == message.end() || !on_item->is_boolean()) {
      return false;
    }
    supla_ocpp_gateway::global_instance()->on_state(
        user_suid, device, connector, on_item->get<bool>(), validity);
    return true;
  }

  if (type == "meter") {
    std::string user_suid;
    long long device = 0;
    long long connector = 0;
    if (!json_string(message, "user", 1, SHORT_UNIQUEID_MAXSIZE - 1,
                     &user_suid) ||
        !json_int(message, "device", 1, INT_MAX, &device) ||
        !json_int(message, "connector", 0, INT_MAX, &connector) ||
        !valid_optional_integer(message, "time", 0,
                                std::numeric_limits<long long>::max()) ||
        !valid_phase_array(message, "voltage", 655350) ||
        !valid_phase_array(message, "current", 655350) ||
        !valid_phase_array(message, "power_phases", INT_MAX / 100) ||
        !valid_phase_array(message, "energy_phases",
                           std::numeric_limits<long long>::max() / 100) ||
        !valid_optional_integer(message, "power", 0, INT_MAX / 100) ||
        (message.contains("snapshot") &&
         !message.at("snapshot").is_boolean()) ||
        !valid_optional_integer(message, "energy", 0,
                                std::numeric_limits<long long>::max() / 100)) {
      return false;
    }
    if (message.find("voltage") == message.end() &&
        message.find("current") == message.end() &&
        message.find("power") == message.end() &&
        message.find("power_phases") == message.end() &&
        message.find("energy") == message.end() &&
        message.find("energy_phases") == message.end()) {
      return false;
    }
    supla_ocpp_gateway::global_instance()->on_meter(
        user_suid, device, connector, message, validity);
    return true;
  }

  if (!type.empty()) {
    return false;
  }

  long long id = 0;
  auto ok_item = message.find("ok");
  if (!json_int(message, "id", 1, std::numeric_limits<long long>::max(), &id) ||
      ok_item == message.end() || !ok_item->is_boolean()) {
    return false;
  }
  std::string error;
  auto error_item = message.find("error");
  if (error_item != message.end()) {
    if (!error_item->is_string()) {
      return false;
    }
    error = error_item->get<std::string>();
    if (error.size() > 128) {
      return false;
    }
  }
  supla_ocpp_gateway::global_instance()->on_result(
      static_cast<unsigned long long>(id), ok_item->get<bool>(), error);
  return true;
}

void supla_ocpp_connection::execute(void *sthread) {
  try {
    std::string payload;
    if (!read_frame(sthread, &payload)) {
      return;
    }
    json hello = json::parse(payload, nullptr, false);
    long long version = 0;
    long long validity = 0;
    std::string type;
    if (hello.is_discarded() || !hello.is_object() ||
        !json_string(hello, "type", 5, 5, &type) || type != "hello" ||
        !json_int(hello, "version", kProtocolVersion, kProtocolVersion,
                  &version) ||
        !json_int(hello, "validity", 1, 86400, &validity)) {
      supla_log(LOG_WARNING, "Invalid OCPP gateway hello message");
      return;
    }
    if (!supla_ocpp_gateway::global_instance()->register_connection(
            shared_from_this())) {
      return;
    }
    channel_validity_time_sec = validity;
    registered = true;
    supla_log(LOG_INFO, "OCPP gateway connected through local socket");

    while (!sthread_isterminated(sthread)) {
      payload.clear();
      if (!read_frame(sthread, &payload)) {
        break;
      }
      json message = json::parse(payload, nullptr, false);
      if (message.is_discarded() || !process_message(message)) {
        supla_log(LOG_WARNING, "Invalid message from OCPP gateway");
        continue;
      }
    }
    supla_log(LOG_INFO, "OCPP gateway disconnected from local socket");
  } catch (const std::exception &error) {
    supla_log(LOG_WARNING, "OCPP connection failed: %s", error.what());
  } catch (...) {
    supla_log(LOG_WARNING, "OCPP connection failed");
  }
  if (registered) {
    registered = false;
    supla_ocpp_gateway::global_instance()->unregister_connection(this);
  }
}
