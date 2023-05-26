/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef JSON_CONFIG_H_
#define JSON_CONFIG_H_

#include <map>
#include <string>

#include "json/cJSON.h"

class supla_json_config {
 private:
  cJSON *user_root;
  supla_json_config *root;

 protected:
  supla_json_config *get_root(void);
  cJSON *get_user_root(void);

  bool equal_ci(const char *str1, const char *str2);
  bool equal_ci(cJSON *item, const char *str);

  int get_int(const char *key);
  bool get_bool(const char *key);
  double get_double(const char *key);

  cJSON *set_item_value(cJSON *parent, const std::string &name, int type,
                        bool force, const char *string_value,
                        double number_value);

  void merge_user_config(const std::string &path, std::map<int, std::string> m,
                         cJSON *dest);

 public:
  explicit supla_json_config(const supla_json_config &json_config);
  explicit supla_json_config(supla_json_config *root);
  supla_json_config(supla_json_config *root, bool copy_and_detach);
  supla_json_config(void);
  virtual ~supla_json_config(void);

  bool is_root_exists(void);
  void set_user_config(const char *config);
  char *get_user_config(void);

  supla_json_config &operator=(const supla_json_config &json_config);
};

#endif /* JSON_CONFIG_H_ */
