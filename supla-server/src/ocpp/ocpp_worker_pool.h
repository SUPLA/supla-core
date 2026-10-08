// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SUPLA_OCPP_WORKER_POOL_H_
#define SUPLA_OCPP_WORKER_POOL_H_

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

// Events for one device always use the same serial worker. Different devices
// can be processed in parallel without creating one thread per charger.
class supla_ocpp_worker_pool {
 private:
  static constexpr std::size_t worker_count = 4;

  struct worker {
    std::mutex mutex;
    std::condition_variable ready;
    std::deque<std::function<void()>> tasks;
    std::thread thread;
    bool stopping = false;
  };

  std::array<worker, worker_count> workers;
  std::atomic_bool accepting{true};
  std::atomic_size_t pending{0};
  std::mutex idle_mutex;
  std::condition_variable idle;

  void run(worker *target);
  void task_finished(void);

 public:
  supla_ocpp_worker_pool();
  ~supla_ocpp_worker_pool();
  supla_ocpp_worker_pool(const supla_ocpp_worker_pool &) = delete;
  supla_ocpp_worker_pool &operator=(const supla_ocpp_worker_pool &) = delete;

  bool post(int device_id, std::function<void()> task);
  void wait_until_idle(void);
  void stop(void);
};

#endif  // SUPLA_OCPP_WORKER_POOL_H_
