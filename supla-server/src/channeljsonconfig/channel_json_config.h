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

#ifndef CHANNELJSONCONFIG_H_
#define CHANNELJSONCONFIG_H_

#include "json/cJSON.h"

class channel_json_config {
 private:
  cJSON *user_root;
  cJSON *properties_root;

  channel_json_config *root;
  void json_clear(void);

 protected:
  cJSON *get_user_root(void);
  cJSON *get_properties_root(void);
  bool equal(const char *str1, const char *str2);
  bool equal(cJSON *item, const char *str);
  virtual int get_map_size(void);
  virtual int get_map_key(int index);
  virtual const char *get_map_str(int index);

  bool key_exists(int key);
  const char *string_with_key(int key);
  int key_with_string(const char *str);
  int json_to_key(cJSON *item);
  int get_int(const char *key);
  bool get_bool(const char *key);

 public:
  explicit channel_json_config(const channel_json_config &json_config);
  explicit channel_json_config(channel_json_config *root);
  channel_json_config &operator=(const channel_json_config &json_config);
  channel_json_config(channel_json_config *root, bool copy_and_detach);
  channel_json_config(void);
  virtual ~channel_json_config(void);
  bool is_root_exists(void);
  void set_user_config(const char *config);
  char *get_user_config(void);
  void set_properties(const char *config);
  char *get_properties(void);
};

#endif /* CHANNELJSONCONFIG_H_ */
