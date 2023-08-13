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

#include "pn_dao.h"

#include <mysql.h>

#include "log.h"

supla_pn_dao::supla_pn_dao(supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

supla_pn_dao::~supla_pn_dao(void) {}

bool supla_pn_dao::get(int user_id, supla_push_notification *push) {
  if (!user_id || !push || !push->get_id()) {
    return false;
  }

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return false;
  }

  MYSQL_BIND pbind[2] = {};

  int id = push->get_id();

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&user_id;

  const char sql[] =
      "SELECT pn.title, pn.body, pn.sound, "
      "DATE_FORMAT(IFNULL(CONVERT_TZ(UTC_TIMESTAMP, 'UTC', u.`timezone`), "
      "UTC_TIMESTAMP), '%Y-%m-%d %T') FROM `supla_push_notification` pn, "
      "`supla_user` u WHERE u.id = pn.user_id AND pn.id = ? AND pn.user_id = ?";

  MYSQL_STMT *stmt = nullptr;
  bool result = false;

  if (dba->stmt_execute((void **)&stmt, sql, pbind, 2, true)) {
    MYSQL_BIND rbind[4] = {};

    char title[100] = {};
    char body[256] = {};
    char date_time[20] = {};

    unsigned long title_len = 0;
    unsigned long body_len = 0;
    unsigned long date_time_len = 0;
    my_bool title_is_null = false;
    my_bool body_is_null = false;
    my_bool date_time_is_null = false;
    int sound = 0;
    my_bool sound_is_null = false;

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = title;
    rbind[0].buffer_length = sizeof(title);
    rbind[0].length = &title_len;
    rbind[0].is_null = &title_is_null;

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = body;
    rbind[1].buffer_length = sizeof(body);
    rbind[1].length = &body_len;
    rbind[1].is_null = &body_is_null;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&sound;
    rbind[2].buffer_length = sizeof(int);
    rbind[2].is_null = &sound_is_null;

    rbind[3].buffer_type = MYSQL_TYPE_STRING;
    rbind[3].buffer = date_time;
    rbind[3].buffer_length = sizeof(date_time);
    rbind[3].length = &date_time_len;
    rbind[3].is_null = &date_time_is_null;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) == 1 && !mysql_stmt_fetch(stmt)) {
        dba->set_terminating_byte(title, sizeof(title), title_len,
                                  title_is_null);

        dba->set_terminating_byte(body, sizeof(body), body_len, body_is_null);

        dba->set_terminating_byte(date_time, sizeof(date_time), date_time_len,
                                  date_time_is_null);

        if (!title_is_null) {
          push->set_title(title);
        }

        if (!body_is_null) {
          push->set_body(body);
        }

        if (!sound_is_null) {
          push->set_sound(sound);
        }

        if (date_time_is_null) {
          push->set_date_time(date_time);
        }

        result = true;
      }
    }

    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

bool supla_pn_dao::get_date_time(int user_id, supla_push_notification *push) {
  if (!user_id || !push) {
    return false;
  }

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return false;
  }

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&user_id;

  const char sql[] =
      "SELECT DATE_FORMAT(IFNULL(CONVERT_TZ(UTC_TIMESTAMP, 'UTC', `timezone`), "
      "UTC_TIMESTAMP), '%Y-%m-%d %T') FROM `supla_user` WHERE id = ?";

  MYSQL_STMT *stmt = nullptr;
  bool result = false;

  if (dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    MYSQL_BIND rbind = {};

    char date_time[20] = {};
    unsigned long date_time_len = 0;
    my_bool date_time_is_null = false;

    rbind.buffer_type = MYSQL_TYPE_STRING;
    rbind.buffer = date_time;
    rbind.buffer_length = sizeof(date_time);
    rbind.length = &date_time_len;
    rbind.is_null = &date_time_is_null;

    if (mysql_stmt_bind_result(stmt, &rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) == 1 && !mysql_stmt_fetch(stmt)) {
        dba->set_terminating_byte(date_time, sizeof(date_time), date_time_len,
                                  date_time_is_null);

        if (date_time_is_null) {
          push->set_date_time(date_time);
        }

        result = true;
      }
    }

    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

int supla_pn_dao::get_limit(int user_id) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return 0;
  }

  int result = dba->get_int(
      user_id, 0,
      "SELECT limit_push_notifications_per_hour FROM supla_user WHERE id = ?");

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

int supla_pn_dao::get_device_managed_push_id(int user_id, int device_id,
                                             int channel_id) {
  if (!user_id || !device_id) return 0;

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return 0;
  }

  int result = 0;

  MYSQL_STMT *stmt = nullptr;

  const char sql[] =
      "SELECT id FROM supla_push_notification WHERE user_id = ? AND "
      "managed_by_device = 1 AND iodevice_id = ? AND ((? = 0 AND channel_id IS "
      "NULL) OR (? != 0 AND channel_id = ?)) LIMIT 1";  // We add a LIMIT 1 if
                                                        // for some reason more
                                                        // than one notification
                                                        // for the pair
                                                        // device_id/channel_id
                                                        // appears in the
                                                        // database.

  MYSQL_BIND pbind[5] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&device_id;

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&channel_id;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&channel_id;

  pbind[4].buffer_type = MYSQL_TYPE_LONG;
  pbind[4].buffer = (char *)&channel_id;

  int _id = 0;

  if (dba->stmt_execute((void **)&stmt, sql, &pbind, 5, true)) {
    MYSQL_BIND rbind = {};

    rbind.buffer_type = MYSQL_TYPE_LONG;
    rbind.buffer = (char *)&_id;

    if (mysql_stmt_bind_result(stmt, &rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) == 1 && !mysql_stmt_fetch(stmt)) {
        result = _id;
      }
    }
  }

  mysql_stmt_close(stmt);

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

void supla_pn_dao::register_device_managed_push(int user_id, int device_id,
                                                int channel_id, bool sm_title,
                                                bool sm_body, bool sm_sound) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return;
  }

  MYSQL_BIND pbind[6] = {};

  char _sm_title = sm_title ? 1 : 0;
  char _sm_body = sm_body ? 1 : 0;
  char _sm_sound = sm_sound ? 1 : 0;

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&device_id;

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&channel_id;

  pbind[3].buffer_type = MYSQL_TYPE_TINY;
  pbind[3].buffer = (char *)&_sm_title;

  pbind[4].buffer_type = MYSQL_TYPE_TINY;
  pbind[4].buffer = (char *)&_sm_body;

  pbind[5].buffer_type = MYSQL_TYPE_TINY;
  pbind[5].buffer = (char *)&_sm_sound;

  const char sql[] = "CALL `supla_register_device_managed_push`(?,?,?,?,?,?)";

  MYSQL_STMT *stmt = nullptr;
  dba->stmt_execute((void **)&stmt, sql, pbind, 6, true);

  if (stmt != nullptr) {
    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }
}
