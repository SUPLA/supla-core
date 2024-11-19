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

#ifndef CHANNEL_STATE_H_
#define CHANNEL_STATE_H_

#include <map>
#include <string>

#include "json/json_helper.h"
#include "vbt/vbt_value.h"

class supla_channel_state : supla_json_helper, public supla_vbt_value {
 private:
  TDSC_ChannelState state;
  static const std::map<unsigned _supla_int16_t, std::string> field_map;
  const std::string reset_cause_to_string(unsigned char reset_cause);
  unsigned char string_to_reset_cause(const std::string &reset_cause);
  void apply_json(const char *json);

 public:
  supla_channel_state(void);
  explicit supla_channel_state(TDSC_ChannelState *state);
  explicit supla_channel_state(const char *json);
  virtual ~supla_channel_state();

  const TDSC_ChannelState *get_state(void);
  char *get_json(void);
  void merge_old_if_needed(supla_channel_state *old);
  bool equal_fields(supla_channel_state *state);
  virtual bool get_vbt_value(_vbt_var_name_e var_name, double *value);
};

#endif /* CHANNEL_STATE_H_ */
