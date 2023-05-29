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

#ifndef JSON_CONFIG_STUB_H_
#define JSON_CONFIG_STUB_H_

#include "jsonconfig/json_config.h"

namespace testing {

class JSONConfigStub : public supla_json_config {
 public:
  JSONConfigStub(void);
  virtual ~JSONConfigStub(void);

  cJSON *get_user_root(void);
  cJSON *set_item_value(cJSON *parent, const std::string &name, int type,
                        bool force, const char *string_value,
                        double number_value);
  bool merge(cJSON *src_parent, cJSON *dst_parent,
             const std::map<int, std::string> &m);
};

} /* namespace testing */

#endif /* JSON_CONFIG_STUB_H_ */
