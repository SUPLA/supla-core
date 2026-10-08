// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SUPLA_OCPP_TASK_QUEUE_H_
#define SUPLA_OCPP_TASK_QUEUE_H_

#include <deque>
#include <functional>
#include <mutex>

// A caller drains the queue; concurrent/reentrant callers only append work.
// No application callbacks, database access or socket I/O run under the lock.
class supla_ocpp_task_queue {
 private:
  std::mutex mutex;
  std::deque<std::function<void()>> queue;
  bool draining = false;

 public:
  void post(std::function<void()> task);
};

#endif  // SUPLA_OCPP_TASK_QUEUE_H_
