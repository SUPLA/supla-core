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

#include "cyclictasks/abstract_cyclictask.h"

using std::vector;

supla_abstract_cyclictask::supla_abstract_cyclictask() {
  last_run_time = {};
  gettimeofday(&last_run_time, NULL);
}

supla_abstract_cyclictask::~supla_abstract_cyclictask() {}

bool supla_abstract_cyclictask::is_it_time(const struct timeval *now) {
  return now->tv_sec - last_run_time.tv_sec >= task_interval_sec();
}

void supla_abstract_cyclictask::run(const struct timeval *now,
                                    const vector<supla_user *> *users,
                                    supla_abstract_db_access_provider *dba) {
  last_run_time = *now;
  run(users, dba);
}

bool supla_abstract_cyclictask::db_access_needed(void) { return true; }

bool supla_abstract_cyclictask::user_access_needed(void) { return true; }
