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

#ifndef CONTROLLINGTHEGATECONFIG_H_
#define CONTROLLINGTHEGATECONFIG_H_

#include <string>

#include "jsonconfig/json_config.h"
#include "proto.h"

class controlling_the_gate_config : public supla_json_config {
 private:
  static const char number_of_attempts_to_open_key[];
  static const char number_of_attempts_to_close_key[];
  static const char state_verification_method_active_key[];
  int get_number_of_attempts(const char *key);

 protected:
 public:
  explicit controlling_the_gate_config(supla_json_config *root);
  controlling_the_gate_config(void);
  int get_number_of_attempts_to_open(void);
  int get_number_of_attempts_to_close(void);
  bool is_active_state_verification_method_enabled(void);
};

#endif /* CONTROLLINGTHEGATECONFIG_H_ */
