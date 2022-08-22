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

supla_device_dao::supla_device_dao(void)
    : supla_abstract_device_dao(), svrdb() {}

supla_device_dao::~supla_device_dao() {}

bool supla_device_dao::get_device_firmware_update_url(
    int device_id, TDS_FirmwareUpdateParams *params,
    TSD_FirmwareUpdate_UrlResult *url) {
  if (!device_id || params == nullptr || url == nullptr || !connect()) {
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

  if (stmt_execute((void **)&stmt, sql1, pbind, 6, true)) {
    if (stmt != nullptr) mysql_stmt_close((MYSQL_STMT *)stmt);
    q_executed++;
  }

  if (stmt_execute((void **)&stmt, sql2, nullptr, 0, true)) {
    if (stmt != nullptr) mysql_stmt_close((MYSQL_STMT *)stmt);
    q_executed++;
  }

  if (q_executed == 2 && stmt_execute((void **)&stmt, sql3, nullptr, 0, true)) {
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

  disconnect();

  return result;
}
