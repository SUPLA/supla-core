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

#ifndef CHANNEL_AND_TIMER_STATE_EXTENDED_VALUE_H_
#define CHANNEL_AND_TIMER_STATE_EXTENDED_VALUE_H_

#include "device/extended_value/channel_state_extended_value.h"
#include "device/extended_value/timer_state_extended_value.h"

class supla_channel_and_timer_state_extended_value
    : public supla_timer_state_extended_value {
 private:
  supla_channel_state_extended_value channel;

  const TChannelState_ExtendedValue *get_channel_state(
      const TSuplaChannelExtendedValue *value);
  const TTimerState_ExtendedValue *get_timer_state(
      const TSuplaChannelExtendedValue *value);

 public:
  explicit supla_channel_and_timer_state_extended_value(
      const TSuplaChannelExtendedValue *value);
  supla_channel_and_timer_state_extended_value(
      const TSuplaChannelExtendedValue *value, supla_user *user);
  virtual ~supla_channel_and_timer_state_extended_value(void);
  virtual size_t get_real_size(void);
  virtual size_t get_value_size(void);
  virtual void set_raw_value(const TSuplaChannelExtendedValue *value);
  virtual void set_raw_value(const TChannelAndTimerState_ExtendedValue *value);
  virtual bool get_raw_value(TChannelAndTimerState_ExtendedValue *value);
  virtual bool get_raw_value(TSuplaChannelExtendedValue *value);
  static bool is_ev_type_supported(char type);
  virtual supla_channel_extended_value *copy(void);  // NOLINT
};

#endif /*CHANNEL_AND_TIMER_STATE_EXTENDED_VALUE_H_*/
