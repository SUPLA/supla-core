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

#ifndef GOOGLE_HOME_CONFIG_H_
#define GOOGLE_HOME_CONFIG_H_

#include "jsonconfig/channel/abstract_voice_assistant_config.h"

class google_home_config : public supla_abstract_voice_assistant_config {
 private:
  static const char root_key[];
  static const char value_key[];

 protected:
  virtual const char *get_root_key(void);
  virtual const char *get_value_key(void);

 public:
  explicit google_home_config(supla_json_config *root);
  google_home_config(void);
};

#endif /* GOOGLE_HOME_CONFIG_H_ */
