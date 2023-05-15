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

#ifndef TIMER_STATE_EXTENDED_VALUE_H_
#define TIMER_STATE_EXTENDED_VALUE_H_

#include <string>

#include "device/extended_value/channel_extended_value.h"

class supla_user;
class supla_timer_state_extended_value : public supla_channel_extended_value {
 private:
  supla_user *user;
  void update_sender_name(void);
  void update_time(void);

 protected:
  virtual std::string get_sender_name(int client_id);

 public:
  explicit supla_timer_state_extended_value(
      const TSuplaChannelExtendedValue *value, supla_user *user);
  virtual ~supla_timer_state_extended_value(void);

  virtual void set_raw_value(const TTimerState_ExtendedValue *value);
  virtual void set_raw_value(const TSuplaChannelExtendedValue *value);
  virtual bool get_raw_value(TTimerState_ExtendedValue *value);
  virtual bool get_raw_value(TSuplaChannelExtendedValue *value);
  static bool is_ev_type_supported(char type);
  static bool valid_size(size_t size);
};

#endif /*TIMER_STATE_EXTENDED_VALUE_H_*/
