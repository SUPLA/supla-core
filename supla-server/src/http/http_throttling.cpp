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

#include "http_throttling.h"

#include "lck.h"
#include "proto.h"

supla_http_throttling::supla_http_throttling() { lck = lck_init(); }

supla_http_throttling::~supla_http_throttling(void) { lck_free(lck); }

int supla_http_throttling::get_default_delay_time(int func) {
  return 1500000;  // 1.5 sek.
}

int supla_http_throttling::get_delay_time_over_threadshold(int func) {
  return 10000000;  // 10 sek.
}

int supla_http_throttling::get_reset_time_us(int func) {
  return 10000000;  // 10 sek.
}

unsigned int supla_http_throttling::get_counter_threadshold(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      return 20;
  }
  return 5;
}

int supla_http_throttling::get_delay_time(int channel_id, int func) {
  int result = get_default_delay_time(func);

  if (result >= get_reset_time_us(func)) {
    return result;
  }

  struct timeval now = {};
  gettimeofday(&now, NULL);

  lck_lock(lck);
  auto it = items.begin();

  while (it != items.end()) {
    if (it->channel_id == channel_id) {
      break;
    }
    ++it;
  }

  if (it == items.end()) {
    items.resize(items.size() + 1);
    --it;
    it->channel_id = channel_id;
    it->counter = 0;
    it->last = now;
  }

  it->counter++;
  long long time_diff = (now.tv_sec * 1000000 + now.tv_usec) -
                        (it->last.tv_sec * 1000000 + it->last.tv_usec);

  if (time_diff >= get_reset_time_us(func)) {
    it->counter = 0;
  } else if (it->counter >= get_counter_threadshold(func)) {
    result = get_delay_time_over_threadshold(func);
  }

  it->last = now;

  lck_unlock(lck);

  return result;
}
