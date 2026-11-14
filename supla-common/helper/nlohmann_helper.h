// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef NLOHMANN_HELPER_H_
#define NLOHMANN_HELPER_H_

#include <nlohmann/json.hpp>
#include <string>

class supla_nlohmann_helper {
 public:
  static std::string to_string(const nlohmann::json &value);
};

#endif /* NLOHMANN_HELPER_H_ */
