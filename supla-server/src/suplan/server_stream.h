// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SUPLA_SUPLAN_SERVER_STREAM_H_
#define SUPLA_SUPLAN_SERVER_STREAM_H_
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>

#include "proto.h"
namespace supla_suplan {
// One instance belongs to one registered D2S connection, never a DeviceId.
class stream_connection {
 public:
  using time_point_t = std::chrono::steady_clock::time_point;
  using clock_t = std::function<time_point_t()>;
  using authorize_callback_t =
      std::function<TSD_SuplaEnsureResourceAccessResult(uint32_t, bool, bool)>;
  struct snapshot_t {
    TSD_SuplaRemoteChannelState state{};
    time_point_t expires{};
  };
  using read_callback_t = std::function<bool(uint32_t, snapshot_t *)>;
  using send_callback_t =
      std::function<bool(const TSD_SuplaRemoteChannelState &)>;
  using reply_callback_t =
      std::function<bool(const TSD_SuplaEnsureResourceAccessResult &)>;
  stream_connection(size_t capacity, authorize_callback_t authorize,
                    read_callback_t read, send_callback_t send,
                    std::function<bool()> ready,
                    clock_t clock = std::chrono::steady_clock::now,
                    std::function<void()> notify = {});
  void ensure(const TDS_SuplaEnsureResourceAccess &request,
              reply_callback_t reply);
  void dirty(uint32_t channel);
  void iterate();
  void close();
  size_t size();
  // Zero means no deadline; overdue work requests one prompt loop turn.
  uint64_t time_left_usec() const;

 private:
  struct interest_t {
    bool active = false;
    snapshot_t last;
    uint64_t serial = 0;
    time_point_t check{};
    time_point_t retry_after{};
  };
  const size_t capacity;
  authorize_callback_t authorize;
  read_callback_t read;
  send_callback_t send;
  std::function<bool()> ready;
  clock_t clock;
  const std::function<void()> notify;
  std::mutex mutex, dirty_mutex;
  std::map<uint32_t, interest_t> interests;
  std::map<uint32_t, uint64_t> serials;
  std::atomic<bool> wake{false};
  std::atomic<time_point_t> due{time_point_t::max()};
  bool closed = false;
  uint32_t cursor = 0;
  uint64_t serial(uint32_t id);
  bool publish(uint32_t id, interest_t *interest);
  void terminal(uint32_t id, interest_t *interest);
  void suspend(interest_t *interest);
  void erase(uint32_t id);
  void schedule();
  void shutdown();
};
}  // namespace supla_suplan
#endif  // SUPLA_SUPLAN_SERVER_STREAM_H_
