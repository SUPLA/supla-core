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

#include "pn_throttling.h"

#include "db/db_access_provider.h"
#include "lck.h"
#include "push/pn_dao.h"

// static
supla_pn_throttling supla_pn_throttling::instance;

supla_pn_throttling::supla_pn_throttling() {
  lck = lck_init();
  time_window_sec = 3600;
}

supla_pn_throttling::~supla_pn_throttling(void) { lck_free(lck); }

unsigned int supla_pn_throttling::get_time_window_sec(void) {
  lck_lock(lck);
  unsigned int result = time_window_sec;
  lck_unlock(lck);
  return result;
}

void supla_pn_throttling::set_time_window_sec(unsigned int time_window_sec) {
  lck_lock(lck);
  this->time_window_sec = time_window_sec;
  lck_unlock(lck);
}

bool supla_pn_throttling::is_delivery_possible(int user_id,
                                               bool* first_time_exceeded,
                                               unsigned int* limit) {
  bool result = false;

  struct timeval now = {};
  gettimeofday(&now, NULL);

  lck_lock(lck);
  auto it = items.begin();

  while (it != items.end()) {
    if (it->user_id == user_id) {
      break;
    }
    ++it;
  }

  if (it == items.end()) {
    items.resize(items.size() + 1);
    it = items.end();  // Probably not necessary, but better to make sure.
    --it;
    it->user_id = user_id;
    it->counter = 0;
    it->limit = 0;
    it->first_time = {};
  }

  if (it->first_time.tv_sec &&
      now.tv_sec - it->first_time.tv_sec >= time_window_sec) {
    it->first_time = {};
  }

  if (it->first_time.tv_sec == 0) {
    it->first_time = now;
    it->counter = 0;

    supla_db_access_provider dba;
    supla_pn_dao dao(&dba);

    // Get / Refresh limit.
    // Retrieving data from dao could be a potential bottleneck.
    it->limit = dao.get_limit(user_id);
  }

  it->counter++;

  if (limit) {
    *limit = it->limit;
  }

  if (it->counter < it->limit) {
    result = true;
  } else if (it->counter == it->limit && first_time_exceeded) {
    *first_time_exceeded = true;
  }

  lck_unlock(lck);

  return result;
}

size_t supla_pn_throttling::get_size(void) {
  lck_lock(lck);
  size_t result = items.size();
  lck_unlock(lck);

  return result;
}

unsigned int supla_pn_throttling::get_count(int user_id) {
  lck_lock(lck);
  unsigned int result = 0;
  for (auto it = items.rbegin(); it != items.rend(); ++it) {
    if (it->user_id == user_id) {
      result = it->counter;
      break;
    }
  }
  lck_unlock(lck);

  return result;
}

// static
supla_pn_throttling* supla_pn_throttling::get_instance(void) {
  return &instance;
}
