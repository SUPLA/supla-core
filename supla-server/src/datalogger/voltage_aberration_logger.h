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

#ifndef VOLTAGE_ABERRATION_LOGGER_H_
#define VOLTAGE_ABERRATION_LOGGER_H_

#include <vector>

#include "datalogger/abstract_electricity_logger.h"

class supla_voltage_aberration_logger
    : public supla_abstract_electricity_logger {
 protected:
  virtual unsigned int task_interval_sec(void);
  virtual bool is_any_data_for_logging_purposes(
      supla_electricity_analyzer *analyzer);
  virtual void reset(supla_electricity_analyzer *analyzer);
  virtual supla_abstract_electricity_logger_dao *get_dao(
      supla_abstract_db_access_provider *dba);

 public:
  supla_voltage_aberration_logger();
  virtual ~supla_voltage_aberration_logger();
};

#endif /* VOLTAGE_ABERRATION_LOGGER_H_ */
