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

#include <mysql.h>
#include <string.h>

#include "analyzer/electricity_analyzer.h"
#include "db/abstract_db_access_provider.h"

class supla_abstract_electricity_logger_dao {
 private:
  supla_abstract_db_access_provider *dba;

 protected:
  supla_abstract_db_access_provider *get_dba(void);
  bool get_utc_timestamp(MYSQL_TIME *time);
  void add(MYSQL_TIME *time, int channel_id, char phase,
           supla_simple_statiscics *stat, const std::string &procedure,
           unsigned char precision);

 public:
  supla_abstract_electricity_logger_dao(supla_abstract_db_access_provider *dba);
  virtual ~supla_abstract_electricity_logger_dao();

  virtual void add(supla_electricity_analyzer *analyzer) = 0;
};

#endif /* ABSTRACT_ELECTRICITY_LOGGER_DAO_H_ */
