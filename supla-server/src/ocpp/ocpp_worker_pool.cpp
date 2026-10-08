// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ocpp/ocpp_worker_pool.h"

#include <exception>
#include <utility>

#include "log.h"

supla_ocpp_worker_pool::supla_ocpp_worker_pool() {
  try {
    for (auto &target : workers) {
      target.thread = std::thread([this, &target]() { run(&target); });
    }
  } catch (...) {
    accepting.store(false);
    for (auto &target : workers) {
      {
        std::lock_guard<std::mutex> lock(target.mutex);
        target.stopping = true;
      }
      target.ready.notify_one();
    }
    for (auto &target : workers) {
      if (target.thread.joinable()) target.thread.join();
    }
    throw;
  }
}

supla_ocpp_worker_pool::~supla_ocpp_worker_pool() { stop(); }

void supla_ocpp_worker_pool::run(worker *target) {
  while (true) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(target->mutex);
      target->ready.wait(lock, [target]() {
        return target->stopping || !target->tasks.empty();
      });
      if (target->stopping && target->tasks.empty()) return;
      task = std::move(target->tasks.front());
      target->tasks.pop_front();
    }

    try {
      task();
    } catch (const std::exception &error) {
      supla_log(LOG_ERR, "OCPP worker failed: %s", error.what());
    } catch (...) {
      supla_log(LOG_ERR, "OCPP worker failed");
    }
    task_finished();
  }
}

void supla_ocpp_worker_pool::task_finished(void) {
  if (pending.fetch_sub(1) == 1) {
    // Synchronize with condition_variable::wait so the transition to zero
    // cannot be notified between its predicate check and sleeping.
    std::lock_guard<std::mutex> lock(idle_mutex);
    idle.notify_all();
  }
}

bool supla_ocpp_worker_pool::post(int device_id, std::function<void()> task) {
  if (!task || !accepting.load()) return false;
  std::size_t index = static_cast<unsigned int>(device_id) % worker_count;
  auto &target = workers[index];
  {
    std::lock_guard<std::mutex> lock(target.mutex);
    if (target.stopping) return false;
    target.tasks.push_back(std::move(task));
    pending.fetch_add(1);
  }
  target.ready.notify_one();
  return true;
}

void supla_ocpp_worker_pool::wait_until_idle(void) {
  std::unique_lock<std::mutex> lock(idle_mutex);
  idle.wait(lock, [this]() { return pending.load() == 0; });
}

void supla_ocpp_worker_pool::stop(void) {
  if (!accepting.exchange(false)) return;
  for (auto &target : workers) {
    {
      std::lock_guard<std::mutex> lock(target.mutex);
      target.stopping = true;
    }
    target.ready.notify_one();
  }
  for (auto &target : workers) {
    if (target.thread.joinable()) target.thread.join();
  }
}
