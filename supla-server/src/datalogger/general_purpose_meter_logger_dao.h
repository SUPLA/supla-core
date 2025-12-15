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

#ifndef GENERAL_PURPOSE_METER_LOGGER_DAO_H_
#define GENERAL_PURPOSE_METER_LOGGER_DAO_H_

#include "cyclictasks/abstract_cyclictask_dao.h"
#include "device/value/channel_general_purpose_meter_value.h"

class supla_general_purpose_meter_logger_dao
    : public supla_abstract_cyclictask_dao {
 private:
  void mariadb_add(int channel_id, double value);
  void tsdb_add(int channel_id, double value);

 public:
  explicit supla_general_purpose_meter_logger_dao(
      supla_abstract_db_access_provider *dba);
  void add(int channel_id, supla_channel_general_purpose_meter_value *value);
};

#endif /* GENERAL_PURPOSE_METER_LOGGER_DAO_H_ */
