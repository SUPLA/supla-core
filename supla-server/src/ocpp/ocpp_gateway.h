// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SUPLA_OCPP_GATEWAY_H_
#define SUPLA_OCPP_GATEWAY_H_

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>

#include "ocpp/ocpp_worker_pool.h"
#include "ocpp/ocpp_value_validity.h"

class supla_user;
class supla_ocpp_connection;
class supla_ocpp_device;

enum class supla_ocpp_action_result { not_ocpp, rejected, accepted };

class supla_ocpp_gateway {
 private:
  std::mutex mutex;
  supla_ocpp_worker_pool workers;
  void *ipc;
  void *accept_loop_thread;
  std::shared_ptr<supla_ocpp_connection> connection;
  bool enabled;
  bool unregistering;
  std::atomic_uint max_message_bytes;
  unsigned long long next_command_id;
  std::map<unsigned long long, std::weak_ptr<supla_ocpp_device>>
      pending_commands;
  std::map<int, std::weak_ptr<supla_ocpp_device>> connected_devices;

  std::shared_ptr<supla_ocpp_device> find_device(const std::string &user_suid,
                                                 int device_id);
  // The caller must hold mutex.
  void remove_pending_for_device_locked(int device_id);
  std::shared_ptr<supla_ocpp_connection> get_connection(void);
  void set_charging(std::shared_ptr<supla_ocpp_device> device, bool on,
                    bool toggle);

 public:
  supla_ocpp_gateway();
  ~supla_ocpp_gateway();
  static supla_ocpp_gateway *global_instance(void);

  bool start(void);
  void stop(void);
  void configure(bool enabled, unsigned int max_message_bytes);
  unsigned int get_max_message_bytes(void);
  void wait_until_idle(void);
  supla_ocpp_action_result try_set_charging(supla_user *user, int channel_id,
                                            bool on);
  supla_ocpp_action_result try_toggle_charging(supla_user *user,
                                               int channel_id);

  bool register_connection(std::shared_ptr<supla_ocpp_connection> connection);
  void unregister_connection(supla_ocpp_connection *connection);
  void on_result(unsigned long long id, bool ok, const std::string &error);
  void on_connected(const std::string &user_suid, int device_id,
                    supla_ocpp_value_validity validity);
  void on_disconnected(const std::string &user_suid, int device_id);
  void on_alive(const std::string &user_suid, int device_id,
                supla_ocpp_value_validity validity);
  void on_state(const std::string &user_suid, int device_id, int connector_id,
                bool on, supla_ocpp_value_validity validity);
  void on_meter(const std::string &user_suid, int device_id, int connector_id,
                const nlohmann::json &message,
                supla_ocpp_value_validity validity);
};

class supla_ocpp_connection
    : public std::enable_shared_from_this<supla_ocpp_connection> {
 private:
  int socket_fd;
  bool registered;
  std::mutex write_mutex;
  bool write_failed = false;
  unsigned int channel_validity_time_sec = 0;

  bool read_exact(void *sthread, char *buffer, size_t size);
  bool read_frame(void *sthread, std::string *payload);
  bool process_message(const nlohmann::json &message);

 public:
  explicit supla_ocpp_connection(int socket_fd);
  ~supla_ocpp_connection();
  void execute(void *sthread);
  bool send_message(const nlohmann::json &message);
};

#endif  // SUPLA_OCPP_GATEWAY_H_
