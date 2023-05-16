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

#include "timer_state_extended_value.h"

#include <string.h>

#include <memory>

#include "user/user.h"

using std::shared_ptr;
using std::string;

supla_timer_state_extended_value::supla_timer_state_extended_value(
    const TSuplaChannelExtendedValue *value, supla_user *user)
    : supla_channel_extended_value() {
  // Don't use supla_channel_extended_value(value) here because we want to
  // overwrite the set_raw_value method
  this->user = user;
  set_raw_value(value);
}

supla_timer_state_extended_value::~supla_timer_state_extended_value(void) {}

string supla_timer_state_extended_value::get_sender_name(int client_id) {
  if (user) {
    shared_ptr<supla_client> client = user->get_clients()->get(client_id);
    if (client != nullptr) {
      return client->get_name();
    }
  }

  return "";
}

supla_user *supla_timer_state_extended_value::get_user(void) { return user; }

void supla_timer_state_extended_value::update_sender_name(void) {
  if (!get_value_ptr()) {
    return;
  }

  TTimerState_ExtendedValue *value =
      (TTimerState_ExtendedValue *)get_value_ptr()->value;

  if (value && value->SenderID) {
    string name = get_sender_name(value->SenderID);
    size_t size = name.size();
    if (size > 0) {
      size++;
    }

    if (size >= SUPLA_SENDER_NAME_MAXSIZE) {
      size = SUPLA_SENDER_NAME_MAXSIZE;
    }

    _realloc(sizeof(TTimerState_ExtendedValue) - SUPLA_SENDER_NAME_MAXSIZE +
             size);

    if (!get_value_ptr()) {
      return;
    }

    value = (TTimerState_ExtendedValue *)get_value_ptr()->value;

    value->SenderNameSize = size;
    if (size) {
      snprintf(value->SenderName, size, "%s", name.c_str());
    } else {
      value->SenderID = 0;
    }
  }
}

void supla_timer_state_extended_value::update_time(void) {
  if (!get_value_ptr()) {
    return;
  }

  TTimerState_ExtendedValue *value =
      (TTimerState_ExtendedValue *)get_value_ptr()->value;

  if (value->RemainingTimeMs > 0) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    unsigned _supla_int64_t time =
        now.tv_sec * (unsigned _supla_int64_t)1000000 + now.tv_usec;
    time /= 1000;
    time += value->RemainingTimeMs;
    time /= 1000;

    value->CountdownEndsAt = time;
  }
}

void supla_timer_state_extended_value::set_raw_value(
    const TTimerState_ExtendedValue *value) {
  if (value) {
    size_t sender_name_size = value->SenderNameSize;
    if (sender_name_size > SUPLA_SENDER_NAME_MAXSIZE) {
      sender_name_size = SUPLA_SENDER_NAME_MAXSIZE;
    }
    size_t size = sizeof(TTimerState_ExtendedValue) -
                  SUPLA_SENDER_NAME_MAXSIZE + sender_name_size;

    TSuplaChannelExtendedValue *ev = _realloc(size);
    if (ev) {
      ev->type = EV_TYPE_TIMER_STATE_V1;
      ev->size = size;
      memcpy(ev->value, value, size);
    }
  } else {
    supla_channel_extended_value::set_raw_value(nullptr);
  }

  update_sender_name();
  update_time();
}

void supla_timer_state_extended_value::set_raw_value(
    const TSuplaChannelExtendedValue *value) {
  if (value && is_ev_type_supported(value->type) && valid_size(value->size) &&
      ((TTimerState_ExtendedValue *)value->value)->SenderNameSize <=
          SUPLA_SENDER_NAME_MAXSIZE) {
    set_raw_value((TTimerState_ExtendedValue *)value->value);
  } else {
    supla_channel_extended_value::set_raw_value(nullptr);
  }
}

bool supla_timer_state_extended_value::get_raw_value(
    TTimerState_ExtendedValue *value) {
  if (value) {
    memset(value, 0, sizeof(TTimerState_ExtendedValue));

    if (get_value_ptr()) {
      memcpy(value, get_value_ptr()->value,
             supla_timer_state_extended_value::
                 get_value_size());  // The method can be overridden so we refer
                                     // directly to the one from this class.
      return true;
    }
  }

  return false;
}

bool supla_timer_state_extended_value::get_raw_value(
    TSuplaChannelExtendedValue *value) {
  return supla_channel_extended_value::get_raw_value(value);
}

supla_channel_extended_value *supla_timer_state_extended_value::copy(  // NOLINT
    void) {                                                            // NOLINT
  supla_timer_state_extended_value *result =
      new supla_timer_state_extended_value(nullptr, get_user());
  result->supla_channel_extended_value::set_raw_value(get_value_ptr());

  return result;
}

// static
bool supla_timer_state_extended_value::is_ev_type_supported(char type) {
  return type == EV_TYPE_TIMER_STATE_V1;
}

// static
bool supla_timer_state_extended_value::valid_size(size_t size) {
  return size <= sizeof(TTimerState_ExtendedValue) &&
         size >= sizeof(TTimerState_ExtendedValue) - SUPLA_SENDER_NAME_MAXSIZE;
}
