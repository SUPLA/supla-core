// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef INJA_SANDBOX_H_
#define INJA_SANDBOX_H_

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <string>

#include "device/abstract_channel_property_getter.h"

class supla_inja_sandbox {
 private:
  supla_abstract_channel_property_getter* getter;
  inja::Environment env;
  int max_blocks;
  int max_ifs;
  bool allow_for;
  size_t max_output_bytes;
  static int count_occ(const std::string& s, const std::string& needle);

 protected:
 public:
  supla_inja_sandbox(void);
  virtual ~supla_inja_sandbox(void);
  void register_abort_function(void);
  void register_get_channel_function(
      supla_abstract_channel_property_getter* getter);
  std::string validate_and_render(const std::string& tpl,
                                  const nlohmann::json& data);
};

#endif /* INJA_HELPER_H_*/
