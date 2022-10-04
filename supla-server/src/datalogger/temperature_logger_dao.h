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

#ifndef TEMPERATURE_LOGGER_DAO_H_
#define TEMPERATURE_LOGGER_DAO_H_

#include <vector>

#include "db/abstract_db_access_provider.h"
#include "device/channel_temphum.h"
#include "proto.h"

class supla_temperature_logger_dao {
 private:
  supla_abstract_db_access_provider *dba;

 public:
  explicit supla_temperature_logger_dao(supla_abstract_db_access_provider *dba);
  void add_temperature(int channel_id, double temperature);
  void add_temperature_and_humidity(int channel_id, double temperature,
                                    double humidity);
  void load(int user_id, std::vector<supla_channel_temphum *> *result);
};

#endif /* TEMPERATURE_LOGGER_DAO_H_ */
