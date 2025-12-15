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

#ifndef ABSTRACT_ELECTRICITY_LOGGER_DAO_H_
#define ABSTRACT_ELECTRICITY_LOGGER_DAO_H_

#include <ctime>
#include <string>

#include "analyzer/electricity_analyzer.h"
#include "cyclictasks/abstract_cyclictask_dao.h"

class supla_abstract_electricity_logger_dao
    : public supla_abstract_cyclictask_dao {
 private:
  void mariadb_add(const time_t &time, int channel_id, char phase,
                   const std::string &procedure, char *min, char *max,
                   char *avg);
  void tsdb_add(const time_t &time, int channel_id, short phase,
                const std::string &procedure, char *min, char *max, char *avg);

 protected:
  virtual void add(const std::time_t &time, int channel_id, char phase,
                   supla_simple_statiscics *stat, const std::string &procedure,
                   unsigned char precision);

 public:
  explicit supla_abstract_electricity_logger_dao(
      supla_abstract_db_access_provider *dba);
  virtual ~supla_abstract_electricity_logger_dao();

  virtual void add(supla_electricity_analyzer *analyzer) = 0;
};

#endif /* ABSTRACT_ELECTRICITY_LOGGER_DAO_H_ */
