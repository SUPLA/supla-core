// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ocpp/ocpp_task_queue.h"

#include <exception>
#include <utility>

#include "log.h"

void supla_ocpp_task_queue::post(std::function<void()> task) {
  std::unique_lock<std::mutex> lock(mutex);
  queue.push_back(std::move(task));
  if (draining) {
    return;
  }

  draining = true;
  while (!queue.empty()) {
    auto next = std::move(queue.front());
    queue.pop_front();
    lock.unlock();
    try {
      next();
    } catch (const std::exception &error) {
      supla_log(LOG_ERR, "OCPP operation failed: %s", error.what());
    } catch (...) {
      supla_log(LOG_ERR, "OCPP operation failed");
    }
    lock.lock();
  }
  draining = false;
}
