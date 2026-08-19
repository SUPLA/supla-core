// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef JSON_HELPER_H_
#define JSON_HELPER_H_

#include <map>
#include <nlohmann/json.hpp>
#include <string>

#include "cJSON.h"
#include "proto.h"

class supla_json_helper {
 protected:
  bool equal_ci(const char *str1, const char *str2);
  bool equal_ci(cJSON *item, const char *str);
  bool get_bool(cJSON *parent, const char *key, bool *value);
  bool get_double(cJSON *parent, const char *key, double *value);
  bool get_string(cJSON *parent, const char *key, std::string *value);

  cJSON *set_item_value(cJSON *parent, const std::string &name, int type,
                        bool force, cJSON *obj, const char *string_value,
                        double number_value);

  bool merge(cJSON *src_parent, cJSON *dst_parent,
             const std::map<unsigned _supla_int16_t, std::string> &m,
             bool delete_nonexistent);

 public:
  supla_json_helper(void);
  virtual ~supla_json_helper(void);
  static std::string to_string(const nlohmann::json &j);
};

#endif /* JSON_CONFIG_H_ */
