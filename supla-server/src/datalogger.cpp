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

#include <unistd.h>

#include "datalogger.h"
#include "log.h"
#include "safearray.h"
#include "sthread.h"
#include "user.h"

supla_datalogger::supla_datalogger() {
  this->temperature_tv.tv_sec = 0;
  this->temperature_tv.tv_usec = 0;
  this->electricitymeter_tv = this->temperature_tv;
  this->impulsecounter_tv = this->temperature_tv;
  this->thermostat_tv = this->temperature_tv;
  this->db = NULL;
}

void supla_datalogger::log_temperature() {
  supla_user *user;
  int a;
  int n = 0;
  void *tarr = safe_array_init();

  while ((user = supla_user::get_user(n)) != NULL) {
    n++;
    user->get_temp_and_humidity(tarr);
  }

  for (a = 0; a < safe_array_count(tarr); a++) {
    supla_channel_temphum *sct =
        (supla_channel_temphum *)safe_array_get(tarr, a);

    if (sct->isTempAndHumidity() == 1)
      db->add_temperature_and_humidity(
          sct->getChannelId(), sct->getTemperature(), sct->getHumidity());
    else
      db->add_temperature(sct->getChannelId(), sct->getTemperature());
  }

  supla_channel_temphum::free(tarr);
}

void supla_datalogger::log_electricity_measurement(void) {
  supla_user *user;
  int a;
  int n = 0;
  void *emarr = safe_array_init();

  while ((user = supla_user::get_user(n)) != NULL) {
    n++;
    user->get_electricity_measurements(emarr);
  }

  for (a = 0; a < safe_array_count(emarr); a++) {
    supla_channel_electricity_measurement *em =
        (supla_channel_electricity_measurement *)safe_array_get(emarr, a);
    if (em) {
      db->add_electricity_measurement(em);
    }
  }

  supla_channel_electricity_measurement::free(emarr);
}

void supla_datalogger::log_ic_measurement(void) {
  supla_user *user;
  int a;
  int n = 0;
  void *icarr = safe_array_init();

  while ((user = supla_user::get_user(n)) != NULL) {
    n++;
    user->get_ic_measurements(icarr);
  }

  for (a = 0; a < safe_array_count(icarr); a++) {
    supla_channel_ic_measurement *ic =
        (supla_channel_ic_measurement *)safe_array_get(icarr, a);
    if (ic) {
      db->add_impulses(ic);
    }
  }

  supla_channel_ic_measurement::free(icarr);
}

void supla_datalogger::log_thermostat_measurement(void) {
  supla_user *user;
  int a;
  int n = 0;
  void *tharr = safe_array_init();

  while ((user = supla_user::get_user(n)) != NULL) {
    n++;
    user->get_thermostat_measurements(tharr);
  }

  for (a = 0; a < safe_array_count(tharr); a++) {
    supla_channel_thermostat_measurement *th =
        (supla_channel_thermostat_measurement *)safe_array_get(tharr, a);
    if (th) {
      db->add_thermostat_measurements(th);
    }
  }

  supla_channel_thermostat_measurement::free(tharr);
}

bool supla_datalogger::dbinit(void) {
  if (db == NULL) {
    db = new database();

    if (!db->connect()) {
      delete db;
      db = NULL;
    }
  }

  return db != NULL;
}

void supla_datalogger::log(void) {
  gettimeofday(&now, NULL);

  if (now.tv_sec - temperature_tv.tv_sec >= TEMPLOG_INTERVAL) {
    temperature_tv = now;

    if (dbinit()) log_temperature();
  }

  if (now.tv_sec - electricitymeter_tv.tv_sec >= ELECTRICITYMETERLOG_INTERVAL) {
    electricitymeter_tv = now;

    if (dbinit()) log_electricity_measurement();
  }

  if (now.tv_sec - impulsecounter_tv.tv_sec >= IMPULSECOUNTERLOG_INTERVAL) {
    impulsecounter_tv = now;

    if (dbinit()) log_ic_measurement();
  }

  if (now.tv_sec - thermostat_tv.tv_sec >= THERMOSTATLOG_INTERVAL) {
    thermostat_tv = now;

    if (dbinit()) log_thermostat_measurement();
  }

  if (db != NULL) {
    delete db;
    db = NULL;
  }
}

void datalogger_loop(void *ssd, void *dl_sthread) {
  supla_datalogger *logger = new supla_datalogger();
  database::thread_init();

  while (sthread_isterminated(dl_sthread) == 0) {
    logger->log();
    usleep(1000000);
  }

  delete logger;
  database::thread_end();
}
