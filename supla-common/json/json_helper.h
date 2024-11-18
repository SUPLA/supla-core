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

#ifndef JSON_HELPER_H_
#define JSON_HELPER_H_

#include <map>
#include <string>

#include "json/cJSON.h"
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
};

#endif /* JSON_CONFIG_H_ */
