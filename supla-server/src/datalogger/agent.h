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

#ifndef DATALOGGER_AGENT_H_
#define DATALOGGER_AGENT_H_

#include <vector>

#include "datalogger/abstract_datalogger.h"
#include "db/database.h"

class supla_datalogger_agent {
 private:
  void *sthread;
  std::vector<supla_abstract_datalogger *> loggers;

  /*
  database *db;
  struct timeval now;
  struct timeval temperature_tv;
  struct timeval electricitymeter_tv;
  struct timeval impulsecounter_tv;
  struct timeval thermostat_tv;

  void log_temperature();
  void log_electricity_measurement(void);
  void log_ic_measurement(void);
  void log_thermostat_measurement(void);
  bool dbinit(void);
  */

  static void loop(void *agent, void *sthread);
  void loop(void);

 public:
  void add(supla_abstract_datalogger *datalogger);
  supla_datalogger_agent();
  ~supla_datalogger_agent();
};

#endif /* DATALOGGER_AGENT_H_ */
