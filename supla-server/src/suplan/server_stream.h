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
class StreamConnection {
 public:
  using Time = std::chrono::steady_clock::time_point;
  using Clock = std::function<Time()>;
  using Authorize =
      std::function<TSD_SuplaEnsureResourceAccessResult(uint32_t, bool, bool)>;
  struct Snapshot {
    TSD_SuplaRemoteChannelState state{};
    Time expires{};
  };
  using Read = std::function<bool(uint32_t, Snapshot *)>;
  using Send = std::function<bool(const TSD_SuplaRemoteChannelState &)>;
  using Reply =
      std::function<bool(const TSD_SuplaEnsureResourceAccessResult &)>;
  StreamConnection(size_t capacity, Authorize authorize, Read read, Send send,
                   std::function<bool()> ready,
                   Clock clock = std::chrono::steady_clock::now,
                   std::function<void()> notify = {});
  void ensure(const TDS_SuplaEnsureResourceAccess &request, Reply reply);
  void dirty(uint32_t channel);
  void iterate();
  void close();
  size_t size();
  // Zero means no deadline; overdue work requests one prompt loop turn.
  uint64_t time_left_usec() const;

 private:
  struct Interest {
    bool active = false;
    Snapshot last;
    uint64_t serial = 0;
    Time check{};
    Time retry_after{};
  };
  const size_t capacity;
  Authorize authorize;
  Read read;
  Send send;
  std::function<bool()> ready;
  Clock clock;
  const std::function<void()> notify;
  std::mutex mutex, dirty_mutex;
  std::map<uint32_t, Interest> interests;
  std::map<uint32_t, uint64_t> serials;
  std::atomic<bool> wake{false};
  std::atomic<Time> due{Time::max()};
  bool closed = false;
  uint32_t cursor = 0;
  uint64_t serial(uint32_t id);
  bool publish(uint32_t id, Interest *interest);
  void terminal(uint32_t id, Interest *interest);
  void suspend(Interest *interest);
  void erase(uint32_t id);
  void schedule();
  void shutdown();
};
}  // namespace supla_suplan
#endif  // SUPLA_SUPLAN_SERVER_STREAM_H_
