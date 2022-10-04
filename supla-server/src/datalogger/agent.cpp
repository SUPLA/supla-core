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

#include "datalogger/agent.h"

#include <unistd.h>

#include "datalogger/electricity_logger.h"
#include "datalogger/impulse_logger.h"
#include "datalogger/temperature_logger.h"
#include "datalogger/thermostat_logger.h"
#include "db/db_access_provider.h"
#include "sthread.h"

using std::vector;

supla_datalogger_agent::supla_datalogger_agent() {
  add(new supla_electricity_logger());
  add(new supla_impulse_logger());
  add(new supla_temperature_logger());
  add(new supla_thermostat_logger());

  this->sthread = nullptr;
  sthread_simple_run(loop, this, 0, &sthread);
}

supla_datalogger_agent::~supla_datalogger_agent() {
  sthread_twf(sthread);
  for (auto it = loggers.cbegin(); it != loggers.cend(); ++it) {
    delete *it;
  }
}

void supla_datalogger_agent::add(supla_abstract_datalogger *datalogger) {
  loggers.push_back(datalogger);
}

// static
void supla_datalogger_agent::loop(void *agent, void *sthread) {
  static_cast<supla_datalogger_agent *>(agent)->loop(sthread);
}

void supla_datalogger_agent::loop(void *sthread) {
  dbcommon::thread_init();

  struct timeval now = {};

  while (sthread_isterminated(sthread) == 0) {
    gettimeofday(&now, NULL);

    bool is_it_time = false;

    for (auto it = loggers.cbegin(); it != loggers.cend(); ++it) {
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
      vector<supla_user *> users = supla_user::get_all_users();
      supla_db_access_provider dba;
      if (dba.connect()) {
        for (auto it = loggers.cbegin(); it != loggers.cend(); ++it) {
          (*it)->log(&now, &users, &dba);

          if (sthread_isterminated(sthread)) {
            break;
          }
        }
      }
    }
  }

  dbcommon::thread_end();
}
