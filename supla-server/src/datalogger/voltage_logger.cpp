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

#include "datalogger/voltage_logger.h"

#include "datalogger/voltage_logger_dao.h"

supla_voltage_logger::supla_voltage_logger()
    : supla_abstract_electricity_logger() {}

supla_voltage_logger::~supla_voltage_logger() {}

unsigned int supla_voltage_logger::task_interval_sec(void) { return 180; }

bool supla_voltage_logger::is_any_data_for_logging_purposes(
    supla_electricity_analyzer *analyzer) {
  return analyzer->is_any_voltage_for_logging_purpose();
}

void supla_voltage_logger::reset(supla_electricity_analyzer *analyzer) {
  analyzer->reset_voltage();
}

supla_abstract_electricity_logger_dao *supla_voltage_logger::get_dao(
    supla_abstract_db_access_provider *dba) {
  return new supla_voltage_logger_dao(dba);
}
