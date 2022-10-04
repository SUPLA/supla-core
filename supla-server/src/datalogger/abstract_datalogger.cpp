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

#include "datalogger/abstract_datalogger.h"

using std::vector;

supla_abstract_datalogger::supla_abstract_datalogger() {
  last_log_time = {};
  gettimeofday(&last_log_time, NULL);
}

supla_abstract_datalogger::~supla_abstract_datalogger() {}

bool supla_abstract_datalogger::is_it_time(const struct timeval *now) {
  return now->tv_sec - last_log_time.tv_sec >= log_interval_sec();
}

void supla_abstract_datalogger::log(const struct timeval *now,
                                    const vector<supla_user *> *users,
                                    supla_abstract_db_access_provider *dba) {
  last_log_time = *now;
  log(users, dba);
}
