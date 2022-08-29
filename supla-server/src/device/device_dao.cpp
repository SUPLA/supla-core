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

#include "device/device_dao.h"

#include <mysql.h>
#include <string.h>

#include "log.h"

supla_device_dao::supla_device_dao(supla_abstract_db_access_provider *dba)
    : supla_abstract_device_dao() {
  this->dba = dba;
}

supla_device_dao::~supla_device_dao() {}

bool supla_device_dao::connecton_init(void) {
  return !dba->is_connected() && dba->connect();
}

bool supla_device_dao::get_device_firmware_update_url(
    int device_id, TDS_FirmwareUpdateParams *params,
    TSD_FirmwareUpdate_UrlResult *url) {
  if (!device_id || params == nullptr || url == nullptr) {
    return false;
  }

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return false;
  }

  MYSQL_STMT *stmt = nullptr;
  MYSQL_BIND pbind[6] = {};
  memset(url, 0, sizeof(TSD_FirmwareUpdate_UrlResult));

  bool result = false;

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&device_id;

  pbind[1].buffer_type = MYSQL_TYPE_TINY;
  pbind[1].buffer = (char *)&params->Platform;

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&params->Param1;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&params->Param2;

  pbind[4].buffer_type = MYSQL_TYPE_LONG;
  pbind[4].buffer = (char *)&params->Param3;

  pbind[5].buffer_type = MYSQL_TYPE_LONG;
  pbind[5].buffer = (char *)&params->Param4;

  const char sql1[] = "SET @p0=?, @p1=?, @p2=?, @p3=?, @p4=?, @p5=?";
  const char sql2[] =
      "CALL `supla_get_device_firmware_url`(@p0, @p1, @p2, @p3, @p4, @p5, @p6, "
      "@p7, @p8, @p9)";
  const char sql3[] =
      "SELECT @p6 AS `protocols`, @p7 AS `host`, @p8 AS `port`, @p9 AS `path`";

  char q_executed = 0;

  if (dba->stmt_execute((void **)&stmt, sql1, pbind, 6, true)) {
    if (stmt != nullptr) mysql_stmt_close((MYSQL_STMT *)stmt);
    q_executed++;
  }

  if (dba->stmt_execute((void **)&stmt, sql2, nullptr, 0, true)) {
    if (stmt != nullptr) mysql_stmt_close((MYSQL_STMT *)stmt);
    q_executed++;
  }

  if (q_executed == 2 &&
      dba->stmt_execute((void **)&stmt, sql3, nullptr, 0, true)) {
    if (stmt != nullptr) mysql_stmt_store_result(stmt);

    if (mysql_stmt_num_rows(stmt) > 0) {
      MYSQL_BIND rbind[4];
      memset(rbind, 0, sizeof(rbind));

      unsigned long host_size = 0;
      my_bool host_is_null = true;

      unsigned long path_size = 0;
      my_bool path_is_null = true;

      rbind[0].buffer_type = MYSQL_TYPE_TINY;
      rbind[0].buffer = (char *)&url->url.available_protocols;

      rbind[1].buffer_type = MYSQL_TYPE_STRING;
      rbind[1].buffer = url->url.host;
      rbind[1].is_null = &host_is_null;
      rbind[1].buffer_length = SUPLA_URL_HOST_MAXSIZE;
      rbind[1].length = &host_size;

      rbind[2].buffer_type = MYSQL_TYPE_LONG;
      rbind[2].buffer = (char *)&url->url.port;

      rbind[3].buffer_type = MYSQL_TYPE_STRING;
      rbind[3].buffer = url->url.path;
      rbind[3].is_null = &path_is_null;
      rbind[3].buffer_length = SUPLA_URL_PATH_MAXSIZE;
      rbind[3].length = &path_size;

      if (mysql_stmt_bind_result(stmt, rbind)) {
        supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                  mysql_stmt_error(stmt));

      } else if (mysql_stmt_fetch(stmt) == 0) {
        if (host_is_null || host_size == 0 ||
            host_size >= SUPLA_URL_HOST_MAXSIZE)
          url->url.host[0] = 0;

        if (path_is_null || path_size == 0 ||
            path_size >= SUPLA_URL_PATH_MAXSIZE)
          url->url.path[0] = 0;

        if (url->url.available_protocols > 0 &&
            strnlen(url->url.host, SUPLA_URL_HOST_MAXSIZE) > 0 &&
            strnlen(url->url.path, SUPLA_URL_PATH_MAXSIZE) > 0)
          url->exists = 1;

        result = true;
      }
    }

    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);
  }

  if (already_connected) {
    dba->disconnect();
  }

  return result;
}

bool supla_device_dao::location_auth(int location_id, char *location_pwd,
                                     int *user_id, bool *is_enabled) {
  if (location_id == 0 || location_pwd == nullptr ||
      strnlen(location_pwd, SUPLA_LOCATION_PWD_MAXSIZE) < 1)
    return false;

  MYSQL_BIND pbind[2] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&location_id;

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)location_pwd;
  pbind[1].buffer_length = strnlen(location_pwd, SUPLA_LOCATION_PWD_MAXSIZE);

  int _is_enabled = 0;

  const char query[] =
      "SELECT id, user_id, enabled FROM `supla_location` WHERE id = ? AND "
      "password = ?";

  int _ID = 0;
  MYSQL_STMT *stmt = nullptr;
  if (!dba->stmt_get_int((void **)&stmt, &_ID, user_id, &_is_enabled, nullptr,
                         query, pbind, 2)) {
    _ID = 0;
    _is_enabled = 0;
  }

  *is_enabled = _is_enabled == 1;

  return _ID != 0;
}

int supla_device_dao::get_location_id(int user_id, bool enabled) {
  MYSQL_STMT *stmt = nullptr;
  int result = 0;
  int _enabled = enabled ? 1 : 0;

  MYSQL_BIND pbind[2] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_d;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&_enabled;

  if (!stmt_get_int((void **)&stmt, &Result, NULL, NULL, NULL,
                    "SELECT id FROM `supla_location` WHERE user_id = ? AND "
                    "enabled = ? LIMIT 1",
                    pbind, 2)) {
    result = 0;
  }

  return result;
}

int supla_device_dao::get_device_id(int user_id,
                                    const char guid[SUPLA_GUID_SIZE]) {
  const char query[] =
      "SELECT id FROM supla_iodevice WHERE user_id = ? AND guid = unhex(?)";

  MYSQL_STMT *stmt = nullptr;
  int result = 0;

  char guidhex[SUPLA_GUID_HEXSIZE];
  st_guid2hex(GUIDHEX, guid);

  MYSQL_BIND pbind[2] = 0;

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)guidhex;
  pbind[1].buffer_length = SUPLA_GUID_HEXSIZE - 1;

  if (!stmt_get_int((void **)&stmt, &result, nullptr, nullptr, nullptr, query,
                    pbind, 2)) {
    result = 0;
  }

  return result;
}

bool supla_device_dao::get_device_reg_enabled(int user_id) {
  const char query[] =
      "SELECT COUNT(*) FROM `supla_user` WHERE id = ? AND iodevice_reg_enabled "
      "IS NOT NULL AND iodevice_reg_enabled >= UTC_TIMESTAMP()";

  bool result = dba->get_count(user_id, query) > 0 ? true : false;

  return result;
}

int supla_device_dao::get_device_limit_left(int user_id) {
  return dba->get_int(
      user_id, 0,
      "SELECT IFNULL(limit_client, 0) - IFNULL(( SELECT COUNT(*) "
      "FROM supla_client WHERE user_id = supla_user.id ), 0) FROM "
      "supla_user WHERE id = ?");
}

int supla_device_dao::get_device_variables(int device_id, bool *device_enabled,
                                           int *original_location_id,
                                           int *location_id,
                                           bool *location_enabled) {
  if (device_id == 0) return 0;

  MYSQL_STMT *stmt = nullptr;

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&DeviceID;

  int _device_enabled = 0;
  int _original_location_id = 0;
  int _location_id = 0;
  int _location_enabled = 0;

  bool result = false;

  if (stmt_execute((void **)&stmt,
                   "SELECT CAST(d.`enabled` AS unsigned integer) `d_enabled`, "
                   "IFNULL(d.original_location_id, 0), IFNULL(d.location_id, "
                   "0), IFNULL(CAST(l.`enabled` AS unsigned integer), 0) "
                   "`l_enabled` FROM supla_iodevice d LEFT JOIN supla_location "
                   "l ON l.id = d.location_id WHERE d.id = ?",
                   &pbind, 1, true)) {
    MYSQL_BIND rbind[4];
    memset(rbind, 0, sizeof(rbind));

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&_device_enabled;

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&_original_location_id;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&_location_id;

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&_location_enabled;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0 && !mysql_stmt_fetch(stmt)) {
        *device_enabled = _device_enabled == 1;
        *original_location_id = _original_location_id;
        *location_id = _location_id;
        *location_enabled = _location_enabled == 1;

        result = true;
      }
    }

    mysql_stmt_close(stmt);
  }

  return result ? device_id : 0;
}
