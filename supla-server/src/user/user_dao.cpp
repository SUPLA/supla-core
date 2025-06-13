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

#include "user_dao.h"

#include "log.h"
#include "user/user.h"

using std::vector;

supla_user_dao::supla_user_dao(supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}
supla_user_dao::~supla_user_dao() {}

std::vector<supla_virtual_channel> supla_user_dao::get_virtual_channels(
    supla_user *user, long long int since_the_value_update_timestamp) {
  std::vector<supla_virtual_channel> result;

  if (!user) {
    return result;
  }

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return result;
  }

  const char sql[] =
      "SELECT c.iodevice_id, c.id, c.type, c.func, c.param1, c.param2, "
      "c.param3, c.param4, c.user_config, c.properties, v.value, case when "
      "`v`.`valid_to` >= utc_timestamp() then "
      "time_to_sec(timediff(`v`.`valid_to`,utc_timestamp())) else NULL end AS "
      "`validity_time_sec` FROM `supla_dev_channel` c, "
      "`supla_dev_channel_value` v WHERE c.is_virtual = 1 AND v.channel_id = "
      "c.id AND c.user_id = ? AND UNIX_TIMESTAMP(CONVERT_TZ(update_time, "
      "'UTC', @@session.time_zone)) > ?";

  MYSQL_STMT *stmt = nullptr;
  MYSQL_BIND pbind[2] = {};

  int user_id = user->getUserID();

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONGLONG;
  pbind[1].buffer = (char *)&since_the_value_update_timestamp;

  if (dba->stmt_execute((void **)&stmt, sql, pbind, 2, true)) {
    int device_id = 0;
    int channel_id = 0;
    int type = 0;
    int func = 0;
    int param1 = 0;
    int param2 = 0;
    int param3 = 0;
    int param4 = 0;

    char user_config[8193] = {};
    char properties[8193] = {};

    unsigned long user_config_size = 0;
    my_bool user_config_is_null = true;

    unsigned long properties_size = 0;
    my_bool properties_are_null = true;

    char value[SUPLA_CHANNELVALUE_SIZE] = {};
    my_bool value_is_null = true;

    unsigned _supla_int_t validity_time_sec = 0;
    my_bool validity_time_is_null = true;

    MYSQL_BIND rbind[12] = {};

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&device_id;

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&channel_id;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&type;

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&func;

    rbind[4].buffer_type = MYSQL_TYPE_LONG;
    rbind[4].buffer = (char *)&param1;

    rbind[5].buffer_type = MYSQL_TYPE_LONG;
    rbind[5].buffer = (char *)&param2;

    rbind[6].buffer_type = MYSQL_TYPE_LONG;
    rbind[6].buffer = (char *)&param3;

    rbind[7].buffer_type = MYSQL_TYPE_LONG;
    rbind[7].buffer = (char *)&param4;

    rbind[8].buffer_type = MYSQL_TYPE_STRING;
    rbind[8].buffer = user_config;
    rbind[8].is_null = &user_config_is_null;
    rbind[8].buffer_length = sizeof(user_config);
    rbind[8].length = &user_config_size;

    rbind[9].buffer_type = MYSQL_TYPE_STRING;
    rbind[9].buffer = properties;
    rbind[9].is_null = &properties_are_null;
    rbind[9].buffer_length = sizeof(properties);
    rbind[9].length = &properties_size;

    rbind[10].buffer_type = MYSQL_TYPE_BLOB;
    rbind[10].buffer = value;
    rbind[10].buffer_length = SUPLA_CHANNELVALUE_SIZE;
    rbind[10].is_null = &value_is_null;

    rbind[11].buffer_type = MYSQL_TYPE_LONG;
    rbind[11].buffer = (char *)&validity_time_sec;
    rbind[11].buffer_length = sizeof(unsigned _supla_int_t);
    rbind[11].is_null = &validity_time_is_null;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          if (user_config_is_null) {
            user_config_size = 0;
          }

          if (properties_are_null) {
            properties_size = 0;
          }

          if (value_is_null) {
            memset(value, 0, SUPLA_CHANNELVALUE_SIZE);
          }

          if (validity_time_is_null) {
            validity_time_sec = 0;
          }

          user_config[user_config_size] = 0;
          properties[properties_size] = 0;

          result.push_back(supla_virtual_channel(
              user, device_id, channel_id, value, validity_time_sec, type, func,
              param1, param2, param3, param4, user_config, properties));
        }
      }
    }

    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

vector<int> supla_user_dao::get_users_with_virtual_channels(void) {
  vector<int> result;

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return result;
  }

  MYSQL_STMT *stmt = NULL;
  const char sql[] =
      "SELECT user_id FROM `supla_dev_channel` WHERE is_virtual = 1 GROUP BY "
      "user_id";

  if (dba->stmt_execute((void **)&stmt, sql, nullptr, 0, true)) {
    MYSQL_BIND rbind = {};

    int user_id = 0;

    rbind.buffer_type = MYSQL_TYPE_LONG;
    rbind.buffer = (char *)&user_id;

    if (mysql_stmt_bind_result(stmt, &rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          result.push_back(user_id);
        }
      }
    }

    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}
