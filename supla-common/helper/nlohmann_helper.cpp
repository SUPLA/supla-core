// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include <helper/nlohmann_helper.h>

#include <exception>

using std::string;

// static
string supla_nlohmann_helper::to_string(const nlohmann::json &value) {
  string result;

  try {
    if (value.is_string()) {
      result = value.get<string>();
    } else if (value.is_number_integer()) {
      result = std::to_string(value.get<int>());
    } else if (value.is_number_float()) {
      result = std::to_string(value.get<double>());
    } else if (value.is_boolean()) {
      result = value.get<bool>() ? "true" : "false";
    } else {
      result = value.dump();
    }
  } catch (const std::exception &e) {
  }

  return result;
}
