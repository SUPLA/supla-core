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

#ifndef VOLTAGE_ABERRATION_LOGGER_DAO_H_
#define VOLTAGE_ABERRATION_LOGGER_DAO_H_

#include <ctime>
#include <string>

#include "datalogger/abstract_electricity_logger_dao.h"

class supla_voltage_aberration_logger_dao
    : public supla_abstract_electricity_logger_dao {
 private:
  void mariadb_add(const time_t &time, int channel_id, char phase,
                   int count_total, int count_above, int count_below,
                   int sec_above, int sec_below, int max_sec_above,
                   int max_sec_below, char *min_voltage, char *max_voltage,
                   char *avg_voltage, int measurement_time_sec);
  void tsdb_add(const time_t &time, int channel_id, short phase,
                int count_total, int count_above, int count_below,
                int sec_above, int sec_below, int max_sec_above,
                int max_sec_below, std::string min_voltage,
                std::string max_voltage, std::string avg_voltage,
                int measurement_time_sec);

  void add(const time_t &time, int channel_id, char phase,
           supla_voltage_aberration_analyzer *va);

 protected:
  virtual void add(const std::time_t &time, int channel_id, char phase,
                   supla_simple_statiscics *stat, const std::string &procedure,
                   unsigned char precision);

 public:
  explicit supla_voltage_aberration_logger_dao(
      supla_abstract_db_access_provider *dba);
  virtual void add(supla_electricity_analyzer *vas);
};

#endif /* VOLTAGE_ABERRATION_LOGGER_DAO_H_ */
