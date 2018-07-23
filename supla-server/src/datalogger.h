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

#ifndef DATALOGGER_H_
#define DATALOGGER_H_

#include "database.h"

#define TEMPLOG_INTERVAL 600
#define ELECTRICITYMETERLOG_INTERVAL 600

class supla_datalogger {
 private:
  database *db;
  struct timeval now;
  struct timeval temperature_tv;
  struct timeval electricitymeter_tv;

  void log_temperature();
  void log_electricity_measurement(void);
  bool dbinit(void);

 public:
  supla_datalogger();
  void log(void);
};

void datalogger_loop(void *ssd, void *dl_sthread);

#endif /* DATALOGGER_H_ */
