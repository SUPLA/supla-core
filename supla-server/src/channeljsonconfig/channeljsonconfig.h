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

 protected:
  cJSON *get_user_root(void);
  cJSON *get_properties_root(void);

 public:
  channel_json_config(channel_json_config *root);
  channel_json_config(void);
  virtual ~channel_json_config(void);
  void set_user_config(const char *config);
  char *get_user_config(void);
  void set_properties(const char *config);
  char *get_properties(void);
};

#endif /* CHANNELJSONCONFIG_H_ */
