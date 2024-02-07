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

#include "power_active_logger_dao.h"

#include "log.h"

using std::string;

supla_power_active_logger_dao::supla_power_active_logger_dao(
    supla_abstract_db_access_provider *dba)
    : supla_abstract_electricity_logger_dao(dba) {}

void supla_power_active_logger_dao::add(supla_electricity_analyzer *vas) {
  if (!vas) {
    return;
  }

  MYSQL_TIME time = {};

  if (get_utc_timestamp(&time)) {
    string procedure = "supla_add_em_power_active_log_item";

    supla_abstract_electricity_logger_dao::add(&time, vas->get_channel_id(), 1,
                                               vas->get_power_active_phase1(),
                                               procedure, 5);
    supla_abstract_electricity_logger_dao::add(&time, vas->get_channel_id(), 2,
                                               vas->get_power_active_phase1(),
                                               procedure, 5);
    supla_abstract_electricity_logger_dao::add(&time, vas->get_channel_id(), 3,
                                               vas->get_power_active_phase1(),
                                               procedure, 5);
  }
}
