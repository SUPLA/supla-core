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

#include "conn/connection_dao.h"

#include <mysql.h>

supla_connection_dao::supla_connection_dao(void)
    : supla_abstract_connection_dao(), svrdb() {}

supla_connection_dao::~supla_connection_dao() {}

bool supla_connection_dao::get_reg_enabled(int user_id, unsigned int *client,
                                           unsigned int *iodevice) {
  if (user_id == 0 || !connect()) {
    return false;
  }

  bool result = false;

  MYSQL_STMT *stmt = nullptr;

  MYSQL_BIND pbind[1] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  if (stmt_get_int(
          (void **)&stmt, (int *)client, (int *)iodevice, nullptr, nullptr,
          "SELECT CAST(IFNULL(UNIX_TIMESTAMP(CONVERT_TZ(IF(client_reg_enabled "
          ">= UTC_TIMESTAMP(), client_reg_enabled, NULL),'+00:00','SYSTEM')), "
          "0) AS UNSIGNED INT), "
          "CAST(IFNULL(UNIX_TIMESTAMP(CONVERT_TZ(IF(iodevice_reg_enabled >= "
          "UTC_TIMESTAMP(), iodevice_reg_enabled, NULL),'+00:00','SYSTEM')), "
          "0) AS UNSIGNED INT) FROM supla_user WHERE id = ?",
          pbind, 1)) {
    result = true;
  }

  disconnect();

  return result;
}
