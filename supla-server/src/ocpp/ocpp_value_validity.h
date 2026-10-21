// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SUPLA_OCPP_VALUE_VALIDITY_H_
#define SUPLA_OCPP_VALUE_VALIDITY_H_

#include <chrono>

// Capture once, when IPC receives a report. Copies retain the same monotonic
// deadline through both worker and device queues; waiting never renews it.
class supla_ocpp_value_validity {
 private:
  std::chrono::steady_clock::time_point deadline;

 public:
  supla_ocpp_value_validity(unsigned int seconds = 0)
      : deadline(std::chrono::steady_clock::now() +
                 std::chrono::seconds(seconds)) {}

  bool is_valid(void) const {
    return deadline > std::chrono::steady_clock::now();
  }

  unsigned int remaining_seconds(void) const {
    auto remaining = deadline - std::chrono::steady_clock::now();
    return remaining > decltype(remaining)::zero()
               ? std::chrono::ceil<std::chrono::seconds>(remaining).count()
               : 0;
  }
};

#endif  // SUPLA_OCPP_VALUE_VALIDITY_H_
