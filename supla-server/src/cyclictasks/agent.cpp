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

#include "cyclictasks/agent.h"

#include <unistd.h>

#include "cyclictasks/auto_gate_closing.h"
#include "datalogger/electricity_logger.h"
#include "datalogger/impulse_logger.h"
#include "datalogger/temperature_logger.h"
#include "datalogger/thermostat_logger.h"
#include "datalogger/voltage_threshold_logger.h"
#include "db/db_access_provider.h"
#include "sthread.h"

using std::vector;

supla_cyclictasks_agent::supla_cyclictasks_agent() {
  add(new supla_electricity_logger());
  add(new supla_impulse_logger());
  add(new supla_temperature_logger());
  add(new supla_thermostat_logger());
  add(new supla_voltage_threshold_logger());
  add(new supla_auto_gate_closing());

  this->sthread = nullptr;
  sthread_simple_run(loop, this, 0, &sthread);
}

supla_cyclictasks_agent::~supla_cyclictasks_agent() {
  sthread_twf(sthread);
  for (auto it = tasks.cbegin(); it != tasks.cend(); ++it) {
    delete *it;
  }
}

void supla_cyclictasks_agent::add(supla_abstract_cyclictask *task) {
  tasks.push_back(task);
}

// static
void supla_cyclictasks_agent::loop(void *agent, void *sthread) {
  static_cast<supla_cyclictasks_agent *>(agent)->loop(sthread);
}

void supla_cyclictasks_agent::loop(void *sthread) {
  dbcommon::thread_init();

  struct timeval now = {};

  while (sthread_isterminated(sthread) == 0) {
    gettimeofday(&now, NULL);

    bool is_it_time = false;

    for (auto it = tasks.cbegin(); it != tasks.cend(); ++it) {
      if ((*it)->is_it_time(&now)) {
        is_it_time = true;
        break;
      }
    }

    // We wait a second on each iteration of the loop. This can happen at any
    // position of loop code, however it is better to do this between calling
    // of is_it_time_method and log methods. Perhaps there will be some more
    // data to register.
    usleep(1000000);

    if (is_it_time) {
      vector<supla_user *> users;
      supla_db_access_provider dba;

      for (auto it = tasks.cbegin(); it != tasks.cend(); ++it) {
        if (sthread_isterminated(sthread)) {
          break;
        }

        if ((*it)->db_access_needed() && !dba.is_connected() &&
            !dba.connect()) {
          continue;
        }

        if ((*it)->user_access_needed() && users.size() == 0) {
          users = supla_user::get_all_users();
          if (users.size() == 0) {
            continue;
          }
        }

        (*it)->run(&now, (*it)->user_access_needed() ? &users : nullptr,
                   (*it)->db_access_needed() ? &dba : nullptr);
      }
    }
  }

  dbcommon::thread_end();
}
