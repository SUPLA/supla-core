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
#include <stdio.h>
#include <string.h>

#include "device/device.h"
#include "device/extended_value/channel_extended_value_factory.h"
#include "log.h"
#include "tools.h"

using std::string;
using std::vector;

supla_device_dao::supla_device_dao(supla_abstract_db_access_provider *dba)
    : supla_abstract_device_dao() {
  this->dba = dba;
}

supla_device_dao::~supla_device_dao() {}

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

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

bool supla_device_dao::location_auth(int location_id, char *location_pwd,
                                     int *user_id, bool *is_enabled) {
  if (location_id == 0 || location_pwd == nullptr || user_id == nullptr ||
      is_enabled == nullptr ||
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

bool supla_device_dao::get_authkey_hash(int id,
                                        char authkey_hash[BCRYPT_HASH_MAXSIZE],
                                        bool *is_null) {
  return dba->get_string(id, authkey_hash, BCRYPT_HASH_MAXSIZE, is_null,
                         "SELECT auth_key FROM supla_iodevice WHERE id = ?");
}

int supla_device_dao::get_location_id(int user_id, bool enabled) {
  MYSQL_STMT *stmt = nullptr;
  int result = 0;
  int _enabled = enabled ? 1 : 0;

  MYSQL_BIND pbind[2] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&_enabled;

  if (!dba->stmt_get_int(
          (void **)&stmt, &result, nullptr, nullptr, nullptr,
          "SELECT id FROM `supla_location` WHERE user_id = ? AND "
          "enabled = ? LIMIT 1",
          pbind, 2)) {
    result = 0;
  }

  return result;
}

bool supla_device_dao::get_device_id(int user_id,
                                     const char guid[SUPLA_GUID_SIZE],
                                     int *id) {
  if (!id) {
    return false;
  }

  // We intentionally use IFNULL(MIN (id), 0) so that the query result always
  // contains one row. Otherwise, we assume that an error has occurred.

  const char query[] =
      "SELECT IFNULL(MIN(id), 0) FROM supla_iodevice WHERE user_id = ? AND "
      "guid = unhex(?)";

  MYSQL_STMT *stmt = nullptr;

  char guidhex[SUPLA_GUID_HEXSIZE];
  st_guid2hex(guidhex, guid);

  MYSQL_BIND pbind[2] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)guidhex;
  pbind[1].buffer_length = SUPLA_GUID_HEXSIZE - 1;

  if (dba->stmt_get_int((void **)&stmt, id, nullptr, nullptr, nullptr, query,
                        pbind, 2)) {
    return true;
  }

  *id = 0;
  return false;
}

int supla_device_dao::get_device_id(int user_id,
                                    const char guid[SUPLA_GUID_SIZE]) {
  int result = 0;
  if (get_device_id(user_id, guid, &result)) {
    return result;
  }

  return 0;
}

bool supla_device_dao::get_device_reg_enabled(int user_id) {
  const char query[] =
      "SELECT COUNT(*) FROM `supla_user` WHERE id = ? AND iodevice_reg_enabled "
      "IS NOT NULL AND iodevice_reg_enabled >= UTC_TIMESTAMP()";

  return dba->get_count(user_id, query) > 0 ? true : false;
}

int supla_device_dao::get_device_limit_left(int user_id) {
  return dba->get_int(
      user_id, 0,
      "SELECT IFNULL(limit_iodev, 0) - IFNULL(( SELECT COUNT(*) "
      "FROM supla_iodevice WHERE user_id = supla_user.id ), 0) FROM "
      "supla_user WHERE id = ?");
}

bool supla_device_dao::get_device_variables(int device_id, bool *device_enabled,
                                            int *original_location_id,
                                            int *location_id,
                                            bool *location_enabled) {
  if (device_id == 0) return false;

  MYSQL_STMT *stmt = nullptr;

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&device_id;

  int _device_enabled = 0;
  int _original_location_id = 0;
  int _location_id = 0;
  int _location_enabled = 0;

  bool result = false;

  if (dba->stmt_execute(
          (void **)&stmt,
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

  return result;
}

int supla_device_dao::get_channel_id_and_type(int device_id, int channel_number,
                                              int *type) {
  MYSQL_BIND pbind[2] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&device_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&channel_number;

  MYSQL_STMT *stmt = NULL;
  int id = 0;
  int _type;

  if (type == NULL) type = &_type;

  if (!dba->stmt_get_int((void **)&stmt, &id, type, NULL, NULL,
                         "SELECT `id`, `type` FROM `supla_dev_channel` WHERE "
                         "`iodevice_id` = ? AND `channel_number` = ?",
                         pbind, 2)) {
    id = 0;
  }

  return id;
}

int supla_device_dao::get_device_channel_count(int device_id) {
  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&device_id;

  MYSQL_STMT *stmt = nullptr;
  int count = 0;

  dba->stmt_get_int(
      (void **)&stmt, &count, NULL, NULL, NULL,
      "SELECT COUNT(*) FROM `supla_dev_channel` WHERE `iodevice_id` = ?",
      &pbind, 1);

  return count;
}

int supla_device_dao::add_device(int location_id,
                                 const char guid[SUPLA_GUID_SIZE],
                                 const char *authkey, const char *name,
                                 unsigned int ipv4, const char *softver,
                                 int proto_version, short manufacturer_id,
                                 short product_id, int flags, int user_id) {
  int device_id = 0;

  char *authkey_hash_hex = nullptr;

  MYSQL_BIND pbind[12] = {};

  char guid_hex[SUPLA_GUID_HEXSIZE] = {};
  st_guid2hex(guid_hex, guid);

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&location_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&user_id;

  pbind[2].buffer_type = MYSQL_TYPE_STRING;
  pbind[2].buffer = (char *)guid_hex;
  pbind[2].buffer_length = SUPLA_GUID_HEXSIZE - 1;

  pbind[3].buffer_type = MYSQL_TYPE_STRING;
  pbind[3].buffer = (char *)name;
  pbind[3].buffer_length = strnlen(name, SUPLA_DEVICE_NAME_MAXSIZE);

  pbind[4].buffer_type = MYSQL_TYPE_LONG;
  pbind[4].buffer = (char *)&ipv4;
  pbind[4].is_unsigned = true;

  pbind[5].buffer_type = MYSQL_TYPE_STRING;
  pbind[5].buffer = (char *)softver;
  pbind[5].buffer_length = strnlen(softver, SUPLA_SOFTVER_MAXSIZE);

  pbind[6].buffer_type = MYSQL_TYPE_LONG;
  pbind[6].buffer = (char *)&proto_version;

  pbind[7].buffer_type = MYSQL_TYPE_SHORT;
  pbind[7].buffer = (char *)&product_id;

  pbind[8].buffer_type = MYSQL_TYPE_SHORT;
  pbind[8].buffer = (char *)&manufacturer_id;

  if (authkey == nullptr) {
    pbind[10].buffer_type = MYSQL_TYPE_NULL;

    pbind[9].buffer_type = MYSQL_TYPE_LONG;
    pbind[9].buffer = (char *)&location_id;

  } else {
    authkey_hash_hex = st_get_authkey_hash_hex(authkey);

    if (authkey_hash_hex == nullptr) return 0;

    pbind[10].buffer_type = MYSQL_TYPE_STRING;
    pbind[10].buffer = (char *)authkey_hash_hex;
    pbind[10].buffer_length =
        strnlen(authkey_hash_hex, BCRYPT_HASH_MAXSIZE * 2);

    pbind[9].buffer_type = MYSQL_TYPE_NULL;
  }

  pbind[11].buffer_type = MYSQL_TYPE_LONG;
  pbind[11].buffer = (char *)&flags;

  const char sql[] =
      "CALL  "
      "`supla_add_iodevice`(?,?,unhex(?),?,?,?,?,?,?,?,"
      "unhex(?),?,@id)";

  device_id = dba->add_by_proc_call(sql, pbind, 12);

  if (authkey_hash_hex) {
    free(authkey_hash_hex);
    authkey_hash_hex = nullptr;
  }

  return device_id;
}

bool supla_device_dao::update_device(int device_id, int original_location_id,
                                     const char *authkey, const char *name,
                                     unsigned int ipv4, const char *softver,
                                     int proto_version, int flags) {
  char *authkey_hash_hex = nullptr;
  bool result = false;

  MYSQL_BIND pbind[8] = {};

  pbind[0].buffer_type = MYSQL_TYPE_STRING;
  pbind[0].buffer = (char *)name;
  pbind[0].buffer_length = strnlen(name, SUPLA_DEVICE_NAME_MAXSIZE);

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&ipv4;
  pbind[1].is_unsigned = true;

  pbind[2].buffer_type = MYSQL_TYPE_STRING;
  pbind[2].buffer = (char *)softver;
  pbind[2].buffer_length = strnlen(softver, SUPLA_SOFTVER_MAXSIZE);

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&proto_version;

  if (original_location_id == 0) {
    pbind[4].buffer_type = MYSQL_TYPE_NULL;

  } else {
    pbind[4].buffer_type = MYSQL_TYPE_LONG;
    pbind[4].buffer = (char *)&original_location_id;
  }

  if (authkey == nullptr) {
    pbind[5].buffer_type = MYSQL_TYPE_NULL;
  } else {
    authkey_hash_hex = st_get_authkey_hash_hex(authkey);

    if (authkey_hash_hex == nullptr) return 0;

    pbind[5].buffer_type = MYSQL_TYPE_STRING;
    pbind[5].buffer = (char *)authkey_hash_hex;
    pbind[5].buffer_length = strnlen(authkey_hash_hex, BCRYPT_HASH_MAXSIZE * 2);
  }

  pbind[6].buffer_type = MYSQL_TYPE_LONG;
  pbind[6].buffer = (char *)&device_id;

  pbind[7].buffer_type = MYSQL_TYPE_LONG;
  pbind[7].buffer = (char *)&flags;

  const char sql[] =
      "CALL "
      "`supla_update_iodevice`(?,?,?,?,?,unhex(?),?,?)";

  MYSQL_STMT *stmt = nullptr;
  if (dba->stmt_execute((void **)&stmt, sql, pbind, 8, true)) {
    result = true;
  }

  if (stmt != nullptr) mysql_stmt_close(stmt);

  if (authkey_hash_hex) {
    free(authkey_hash_hex);
    authkey_hash_hex = nullptr;
  }

  return result;
}

int supla_device_dao::add_channel(int device_id, int channel_number, int type,
                                  int func, int param1, int param2, int flist,
                                  int flags, int user_id) {
  MYSQL_BIND pbind[9] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&type;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&func;

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&param1;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&param2;

  pbind[4].buffer_type = MYSQL_TYPE_LONG;
  pbind[4].buffer = (char *)&user_id;

  pbind[5].buffer_type = MYSQL_TYPE_LONG;
  pbind[5].buffer = (char *)&channel_number;

  pbind[6].buffer_type = MYSQL_TYPE_LONG;
  pbind[6].buffer = (char *)&device_id;

  pbind[7].buffer_type = MYSQL_TYPE_LONG;
  pbind[7].buffer = (char *)&flist;

  pbind[8].buffer_type = MYSQL_TYPE_LONG;
  pbind[8].buffer = (char *)&flags;

  {
    const char sql[] = "CALL`supla_add_channel`(?,?,?,?,0,?,?,?,?,?)";

    MYSQL_STMT *stmt = NULL;
    if (!dba->stmt_execute((void **)&stmt, sql, pbind, 9, true)) {
      if (stmt != NULL) mysql_stmt_close(stmt);
      return 0;
    }

    if (stmt != NULL) mysql_stmt_close(stmt);
  }

  return get_channel_id_and_type(device_id, channel_number, nullptr);
}

bool supla_device_dao::on_new_device(int device_id) {
  char sql[51];
  snprintf(sql, sizeof(sql), "CALL `supla_on_newdevice`(%i)", device_id);

  return dba->query(sql, true) == 0;
}

bool supla_device_dao::on_channel_added(int device_id, int channel_id) {
  char sql[71];
  snprintf(sql, sizeof(sql), "CALL `supla_on_channeladded`(%i, %i)", device_id,
           channel_id);

  return dba->query(sql, true) == 0;
}

bool supla_device_dao::set_device_config(
    int user_id, int device_id, device_json_config *config,
    unsigned _supla_int16_t available_fields) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return false;
  }

  bool result = false;

  for (char a = 0; a < 3; a++) {
    string user_config_md5sum, properties_md5sum;
    device_json_config *_config =
        get_device_config(device_id, &user_config_md5sum, &properties_md5sum);
    if (_config) {
      config->merge(_config);
      _config->leave_only_thise_fields(available_fields);

      char *user_config_str = _config->get_user_config();
      if (!user_config_str) {
        user_config_str = strndup("{}", 3);
      }

      char *properties_str = _config->get_properties();
      if (!properties_str) {
        properties_str = strndup("{}", 3);
      }

      MYSQL_BIND pbind[6] = {};

      pbind[0].buffer_type = MYSQL_TYPE_LONG;
      pbind[0].buffer = (char *)&user_id;

      pbind[1].buffer_type = MYSQL_TYPE_LONG;
      pbind[1].buffer = (char *)&device_id;

      pbind[2].buffer_type = MYSQL_TYPE_STRING;
      pbind[2].buffer = (char *)user_config_str;
      pbind[2].buffer_length = strnlen(user_config_str, 4096);

      pbind[3].buffer_type = MYSQL_TYPE_STRING;
      pbind[3].buffer = (char *)user_config_md5sum.c_str();
      pbind[3].buffer_length = user_config_md5sum.size();

      pbind[4].buffer_type = MYSQL_TYPE_STRING;
      pbind[4].buffer = (char *)properties_str;
      pbind[4].buffer_length = strnlen(properties_str, 2048);

      pbind[5].buffer_type = MYSQL_TYPE_STRING;
      pbind[5].buffer = (char *)properties_md5sum.c_str();
      pbind[5].buffer_length = properties_md5sum.size();

      const char sql[] =
          "CALL "
          "`supla_set_device_json_config`(?,?,?,?,?,?)";

      MYSQL_STMT *stmt = nullptr;
      if (dba->stmt_execute((void **)&stmt, sql, pbind, 6, true)) {
        MYSQL_BIND rbind = {};

        int _result = -1;

        rbind.buffer_type = MYSQL_TYPE_LONG;
        rbind.buffer = (char *)&_result;

        if (mysql_stmt_bind_result(stmt, &rbind)) {
          supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                    mysql_stmt_error(stmt));
        } else {
          mysql_stmt_store_result(stmt);

          if (mysql_stmt_num_rows(stmt) == 1 && !mysql_stmt_fetch(stmt)) {
            result = _result == 0;
          }
        }
      }

      if (stmt != nullptr) mysql_stmt_close(stmt);

      free(user_config_str);
      free(properties_str);

      delete _config;

    } else {
      break;
    }

    if (result) {
      break;
    }
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

device_json_config *supla_device_dao::get_device_config(
    int device_id, string *user_config_md5sum, string *properties_md5sum) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return nullptr;
  }

  device_json_config *result = nullptr;

  MYSQL_STMT *stmt = nullptr;
  const char sql[] =
      "SELECT user_config, MD5(IFNULL(user_config, '')), properties, "
      "MD5(IFNULL(properties, '')) FROM supla_iodevice WHERE id = ?";

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&device_id;

  if (dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    char user_config[4097] = {};
    char user_config_md5[33] = {};
    char properties[2049] = {};
    char properties_md5[33] = {};

    unsigned long user_config_size = 0;
    my_bool user_config_is_null = true;

    unsigned long properties_size = 0;
    my_bool properties_are_null = true;

    unsigned long user_config_md5_size = 0;
    my_bool user_config_md5_is_null = true;

    unsigned long properties_md5_size = 0;
    my_bool properties_md5_is_null = true;

    MYSQL_BIND rbind[4] = {};

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = user_config;
    rbind[0].is_null = &user_config_is_null;
    rbind[0].buffer_length = sizeof(user_config);
    rbind[0].length = &user_config_size;

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = user_config_md5;
    rbind[1].is_null = &user_config_md5_is_null;
    rbind[1].buffer_length = sizeof(user_config_md5);
    rbind[1].length = &user_config_md5_size;

    rbind[2].buffer_type = MYSQL_TYPE_STRING;
    rbind[2].buffer = properties;
    rbind[2].is_null = &properties_are_null;
    rbind[2].buffer_length = sizeof(properties);
    rbind[2].length = &properties_size;

    rbind[3].buffer_type = MYSQL_TYPE_STRING;
    rbind[3].buffer = properties_md5;
    rbind[3].is_null = &properties_md5_is_null;
    rbind[3].buffer_length = sizeof(properties_md5);
    rbind[3].length = &properties_md5_size;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) == 1 && !mysql_stmt_fetch(stmt)) {
        dba->set_terminating_byte(user_config, sizeof(user_config),
                                  user_config_size, user_config_is_null);
        dba->set_terminating_byte(user_config_md5, sizeof(user_config_md5),
                                  user_config_md5_size,
                                  user_config_md5_is_null);
        dba->set_terminating_byte(properties, sizeof(properties),
                                  properties_size, properties_are_null);
        dba->set_terminating_byte(properties_md5, sizeof(properties_md5),
                                  properties_md5_size, properties_md5_is_null);

        result = new device_json_config();
        result->set_user_config(user_config);
        result->set_properties(properties);

        if (user_config_md5sum) {
          *user_config_md5sum = user_config_md5;
        }

        if (properties_md5sum) {
          *properties_md5sum = properties_md5;
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

vector<supla_device_channel *> supla_device_dao::get_channels(
    supla_device *device) {
  vector<supla_device_channel *> result;

  MYSQL_STMT *stmt = nullptr;
  const char sql[] =
      "SELECT c.`type`, c.`func`, c.`param1`, c.`param2`, c.`param3`, "
      "c.`param4`, c.`text_param1`, c.`text_param2`, c.`text_param3`, "
      "c.`channel_number`, c.`id`, c.`hidden`, c.`flags`, v.`value`, CASE WHEN "
      "v.`valid_to` >= UTC_TIMESTAMP() THEN TIME_TO_SEC(TIMEDIFF(v.`valid_to`, "
      "UTC_TIMESTAMP())) + 2 ELSE NULL END, ev.`type`, ev.`value`, "
      "c.`user_config`, c.`properties` FROM `supla_dev_channel` c  LEFT JOIN "
      "`supla_dev_channel_value` v ON v.channel_id = c.id LEFT JOIN "
      "`supla_dev_channel_extended_value` ev ON ev.channel_id = c.id WHERE "
      "c.`iodevice_id` = ? ORDER BY c.`channel_number`";

  MYSQL_BIND pbind = {};

  int device_id = device->get_id();

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&device_id;

  if (dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    my_bool is_null[11] = {};
    MYSQL_BIND rbind[19] = {};

    int type = 0;
    int func = 0;
    int param1 = 0;
    int param2 = 0;
    int param3 = 0;
    int param4 = 0;
    int number = 0;
    int id = 0;
    int hidden = 0;
    int flags = 0;

    char text_param1[256] = {};
    char text_param2[256] = {};
    char text_param3[256] = {};

    unsigned long text_param1_size = 0;
    unsigned long text_param2_size = 0;
    unsigned long text_param3_size = 0;

    char user_config[4097] = {};
    char properties[2049] = {};

    unsigned long user_config_size = 0;
    unsigned long properties_size = 0;

    char value[SUPLA_CHANNELVALUE_SIZE] = {};
    my_bool value_is_null = true;

    unsigned _supla_int_t validity_time_sec = 0;
    my_bool validity_time_is_null = true;

    TSuplaChannelExtendedValue ev = {};
    unsigned long ev_value_size = 0;
    my_bool ev_value_is_null = true;

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&type;
    rbind[0].is_null = &is_null[0];

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&func;
    rbind[1].is_null = &is_null[1];

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&param1;
    rbind[2].is_null = &is_null[2];

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&param2;
    rbind[3].is_null = &is_null[3];

    rbind[4].buffer_type = MYSQL_TYPE_LONG;
    rbind[4].buffer = (char *)&param3;
    rbind[4].is_null = &is_null[4];

    rbind[5].buffer_type = MYSQL_TYPE_LONG;
    rbind[5].buffer = (char *)&param4;
    rbind[5].is_null = &is_null[5];

    rbind[6].buffer_type = MYSQL_TYPE_STRING;
    rbind[6].buffer = text_param1;
    rbind[6].is_null = &is_null[6];
    rbind[6].buffer_length = sizeof(text_param1) - 1;
    rbind[6].length = &text_param1_size;

    rbind[7].buffer_type = MYSQL_TYPE_STRING;
    rbind[7].buffer = text_param2;
    rbind[7].is_null = &is_null[7];
    rbind[7].buffer_length = sizeof(text_param2) - 1;
    rbind[7].length = &text_param2_size;

    rbind[8].buffer_type = MYSQL_TYPE_STRING;
    rbind[8].buffer = text_param3;
    rbind[8].is_null = &is_null[8];
    rbind[8].buffer_length = sizeof(text_param3) - 1;
    rbind[8].length = &text_param3_size;

    rbind[9].buffer_type = MYSQL_TYPE_LONG;
    rbind[9].buffer = (char *)&number;

    rbind[10].buffer_type = MYSQL_TYPE_LONG;
    rbind[10].buffer = (char *)&id;

    rbind[11].buffer_type = MYSQL_TYPE_LONG;
    rbind[11].buffer = (char *)&hidden;

    rbind[12].buffer_type = MYSQL_TYPE_LONG;
    rbind[12].buffer = (char *)&flags;

    rbind[13].buffer_type = MYSQL_TYPE_BLOB;
    rbind[13].buffer = value;
    rbind[13].buffer_length = SUPLA_CHANNELVALUE_SIZE;
    rbind[13].is_null = &value_is_null;

    rbind[14].buffer_type = MYSQL_TYPE_LONG;
    rbind[14].buffer = (char *)&validity_time_sec;
    rbind[14].buffer_length = sizeof(unsigned _supla_int_t);
    rbind[14].is_null = &validity_time_is_null;

    rbind[15].buffer_type = MYSQL_TYPE_TINY;
    rbind[15].buffer = &ev.type;
    rbind[15].buffer_length = sizeof(ev.type);

    rbind[16].buffer_type = MYSQL_TYPE_BLOB;
    rbind[16].buffer = ev.value;
    rbind[16].buffer_length = sizeof(ev.value);
    rbind[16].is_null = &ev_value_is_null;
    rbind[16].length = &ev_value_size;

    rbind[17].buffer_type = MYSQL_TYPE_STRING;
    rbind[17].buffer = user_config;
    rbind[17].is_null = &is_null[9];
    rbind[17].buffer_length = sizeof(user_config) - 1;
    rbind[17].length = &user_config_size;

    rbind[18].buffer_type = MYSQL_TYPE_STRING;
    rbind[18].buffer = properties;
    rbind[18].is_null = &is_null[10];
    rbind[18].buffer_length = sizeof(properties) - 1;
    rbind[18].length = &properties_size;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          if (is_null[0] == true) type = 0;
          if (is_null[1] == true) func = 0;
          if (is_null[2] == true) param1 = 0;
          if (is_null[3] == true) param2 = 0;
          if (is_null[4] == true) param3 = 0;
          if (is_null[5] == true) param4 = 0;
          if (is_null[6] == true) text_param1_size = 0;
          if (is_null[7] == true) text_param2_size = 0;
          if (is_null[8] == true) text_param3_size = 0;
          if (is_null[9] == true) user_config_size = 0;
          if (is_null[10] == true) properties_size = 0;

          text_param1[text_param1_size] = 0;
          text_param2[text_param2_size] = 0;
          text_param3[text_param3_size] = 0;

          user_config[user_config_size] = 0;
          properties[properties_size] = 0;

          if (value_is_null) {
            memset(value, 0, SUPLA_CHANNELVALUE_SIZE);
          }

          if (validity_time_is_null) {
            validity_time_sec = 0;
          }

          supla_channel_extended_value *extended_value = nullptr;

          if (!ev_value_is_null && ev_value_size > 0) {
            ev.size = ev_value_size;
            extended_value =
                supla_channel_extended_value_factory::new_value(&ev);
          }

          ev = {};

          supla_device_channel *channel = new supla_device_channel(
              device, id, number, type, func, param1, param2, param3, param4,
              text_param1, text_param2, text_param3, hidden > 0, flags, value,
              validity_time_sec, extended_value,
              user_config_size ? user_config : nullptr,
              properties_size ? properties : nullptr);

          result.push_back(channel);
        }
      }
    }

    mysql_stmt_close(stmt);
  }

  return result;
}

void supla_device_dao::set_channel_properties(int user_id, int channel_id,
                                              supla_json_config *config) {
  if (!config) {
    return;
  }

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return;
  }

  char *properties = config->get_properties();

  if (!properties) {
    return;
  }

  MYSQL_STMT *stmt = NULL;
  MYSQL_BIND pbind[3];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&channel_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&user_id;

  pbind[2].buffer_type = MYSQL_TYPE_STRING;
  pbind[2].buffer = (char *)properties;
  pbind[2].buffer_length = strnlen(properties, 2049);

  const char sql[] = "CALL `supla_update_channel_properties`(?, ?, ?)";

  if (dba->stmt_execute((void **)&stmt, sql, pbind, 3, true)) {
    if (stmt != NULL) mysql_stmt_close((MYSQL_STMT *)stmt);
  }

  free(properties);

  if (!already_connected) {
    dba->disconnect();
  }
}

supla_json_config *supla_device_dao::get_channel_config(
    int channel_id, std::string *user_config_md5sum,
    std::string *properties_md5sum) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return nullptr;
  }

  supla_json_config *result = nullptr;

  MYSQL_STMT *stmt = nullptr;
  const char sql[] =
      "SELECT user_config, properties, MD5(IFNULL(user_config, '')), "
      "MD5(IFNULL(properties, '')) FROM supla_dev_channel WHERE id = ?";

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&channel_id;

  if (dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    char user_config[4097] = {};
    char user_config_md5[33] = {};

    char properties[2049] = {};
    char properties_md5[33] = {};

    unsigned long user_config_size = 0;
    my_bool user_config_is_null = true;

    unsigned long user_config_md5_size = 0;
    my_bool user_config_md5_is_null = true;

    unsigned long properties_size = 0;
    my_bool properties_is_null = true;

    unsigned long properties_md5_size = 0;
    my_bool properties_md5_is_null = true;

    MYSQL_BIND rbind[4] = {};

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = user_config;
    rbind[0].is_null = &user_config_is_null;
    rbind[0].buffer_length = sizeof(user_config);
    rbind[0].length = &user_config_size;

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = properties;
    rbind[1].is_null = &properties_is_null;
    rbind[1].buffer_length = sizeof(properties);
    rbind[1].length = &properties_size;

    rbind[2].buffer_type = MYSQL_TYPE_STRING;
    rbind[2].buffer = user_config_md5;
    rbind[2].is_null = &user_config_md5_is_null;
    rbind[2].buffer_length = sizeof(user_config_md5);
    rbind[2].length = &user_config_md5_size;

    rbind[3].buffer_type = MYSQL_TYPE_STRING;
    rbind[3].buffer = properties_md5;
    rbind[3].is_null = &properties_md5_is_null;
    rbind[3].buffer_length = sizeof(properties_md5);
    rbind[3].length = &properties_md5_size;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) == 1 && !mysql_stmt_fetch(stmt)) {
        dba->set_terminating_byte(user_config, sizeof(user_config),
                                  user_config_size, user_config_is_null);
        dba->set_terminating_byte(user_config_md5, sizeof(user_config_md5),
                                  user_config_md5_size,
                                  user_config_md5_is_null);

        dba->set_terminating_byte(properties, sizeof(properties),
                                  properties_size, properties_is_null);
        dba->set_terminating_byte(properties_md5, sizeof(properties_md5),
                                  properties_md5_size, properties_md5_is_null);

        result = new supla_json_config();
        result->set_user_config(user_config);
        result->set_properties(properties);

        if (user_config_md5sum) {
          *user_config_md5sum = user_config_md5;
        }

        if (properties_md5sum) {
          *properties_md5sum = properties_md5;
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

bool supla_device_dao::set_channel_config(int user_id, int channel_id,
                                          supla_json_config *config) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return false;
  }

  bool result = false;

  for (char a = 0; a < 3; a++) {
    string user_config_md5sum, properties_md5sum;
    supla_json_config *_config =
        get_channel_config(channel_id, &user_config_md5sum, &properties_md5sum);
    if (_config) {
      config->merge(_config);

      char *user_config_str = _config->get_user_config();
      if (!user_config_str) {
        user_config_str = strndup("{}", 2);
      }

      char *properties_str = _config->get_properties();
      if (!properties_str) {
        properties_str = strndup("{}", 2);
      }

      MYSQL_BIND pbind[6] = {};

      pbind[0].buffer_type = MYSQL_TYPE_LONG;
      pbind[0].buffer = (char *)&user_id;

      pbind[1].buffer_type = MYSQL_TYPE_LONG;
      pbind[1].buffer = (char *)&channel_id;

      pbind[2].buffer_type = MYSQL_TYPE_STRING;
      pbind[2].buffer = (char *)user_config_str;
      pbind[2].buffer_length = strnlen(user_config_str, 4096);

      pbind[3].buffer_type = MYSQL_TYPE_STRING;
      pbind[3].buffer = (char *)user_config_md5sum.c_str();
      pbind[3].buffer_length = user_config_md5sum.size();

      pbind[4].buffer_type = MYSQL_TYPE_STRING;
      pbind[4].buffer = (char *)properties_str;
      pbind[4].buffer_length = strnlen(properties_str, 4096);

      pbind[5].buffer_type = MYSQL_TYPE_STRING;
      pbind[5].buffer = (char *)properties_md5sum.c_str();
      pbind[5].buffer_length = properties_md5sum.size();

      const char sql[] =
          "CALL "
          "`supla_set_channel_json_config`(?,?,?,?,?,?)";

      MYSQL_STMT *stmt = nullptr;
      if (dba->stmt_execute((void **)&stmt, sql, pbind, 6, true)) {
        MYSQL_BIND rbind = {};

        int _result = -1;

        rbind.buffer_type = MYSQL_TYPE_LONG;
        rbind.buffer = (char *)&_result;

        if (mysql_stmt_bind_result(stmt, &rbind)) {
          supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                    mysql_stmt_error(stmt));
        } else {
          mysql_stmt_store_result(stmt);

          if (mysql_stmt_num_rows(stmt) == 1 && !mysql_stmt_fetch(stmt)) {
            result = _result == 0;
          }
        }
      }

      if (stmt != nullptr) mysql_stmt_close(stmt);

      free(user_config_str);
      free(properties_str);

      delete _config;

    } else {
      break;
    }

    if (result) {
      break;
    }
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

void supla_device_dao::update_channel_value(
    int channel_id, int user_id, const char value[SUPLA_CHANNELVALUE_SIZE],
    unsigned _supla_int_t validity_time_sec) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return;
  }

  MYSQL_STMT *stmt = nullptr;
  MYSQL_BIND pbind[4] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&channel_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&user_id;

  pbind[2].buffer_type = MYSQL_TYPE_BLOB;
  pbind[2].buffer = (char *)value;
  pbind[2].buffer_length = SUPLA_CHANNELVALUE_SIZE;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&validity_time_sec;

  const char sql[] = "CALL `supla_update_channel_value`(?, ?, ?, ?)";

  if (dba->stmt_execute((void **)&stmt, sql, pbind, 4, true)) {
    if (stmt != NULL) mysql_stmt_close((MYSQL_STMT *)stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }
}

void supla_device_dao::update_channel_extended_value(
    int channel_id, int user_id, supla_channel_extended_value *ev) {
  if (!ev) {
    return;
  }

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return;
  }

  MYSQL_STMT *stmt = nullptr;
  MYSQL_BIND pbind[4] = {};

  char type = ev->get_type();

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&channel_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&user_id;

  pbind[2].buffer_type = MYSQL_TYPE_TINY;
  pbind[2].buffer = (char *)&type;

  char *buffer = nullptr;

  if (ev->get_value_size()) {
    buffer = new char[ev->get_value_size()]{};
    ev->get_value(buffer);

    pbind[3].buffer_type = MYSQL_TYPE_BLOB;
    pbind[3].buffer = buffer;
    pbind[3].buffer_length = ev->get_value_size();
  } else {
    pbind[3].buffer_type = MYSQL_TYPE_NULL;
  }

  const char sql[] = "CALL `supla_update_channel_extended_value`(?, ?, ?, ?)";

  if (dba->stmt_execute((void **)&stmt, sql, pbind, 4, true)) {
    if (stmt != NULL) mysql_stmt_close((MYSQL_STMT *)stmt);
  }

  if (buffer) {
    delete[] buffer;
  }

  if (!already_connected) {
    dba->disconnect();
  }
}
