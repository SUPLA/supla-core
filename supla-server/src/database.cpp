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

#include <ctype.h>
#include <my_global.h>
#include <mysql.h>
#include "amazon/alexa.h"
#include "google/googlehome.h"

// https://bugs.mysql.com/bug.php?id=28184
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <string.h>

#include "database.h"
#include "log.h"
#include "svrcfg.h"
#include "tools.h"
#include "userchannelgroups.h"

char *database::cfg_get_host(void) { return scfg_string(CFG_MYSQL_HOST); }

char *database::cfg_get_user(void) { return scfg_string(CFG_MYSQL_USER); }
char *database::cfg_get_password(void) {
  return scfg_string(CFG_MYSQL_PASSWORD);
}
char *database::cfg_get_database(void) { return scfg_string(CFG_MYSQL_DB); }
int database::cfg_get_port(void) { return scfg_int(CFG_MYSQL_PORT); }

bool database::auth(const char *query, int ID, char *_PWD, int _PWD_HEXSIZE,
                    int *UserID, bool *is_enabled) {
  if (_mysql == NULL || ID == 0 || strnlen(_PWD, 64) < 1) return false;

  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  char PWD[_PWD_HEXSIZE];
  st_str2hex(PWD, _PWD, 64);

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ID;

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)PWD;
  pbind[1].buffer_length = strnlen(PWD, 64);

  int _is_enabled = 0;

  int __ID = 0;
  MYSQL_STMT *stmt;
  if (!stmt_get_int((void **)&stmt, &__ID, UserID, &_is_enabled, NULL, query,
                    pbind, 2)) {
    __ID = 0;
    _is_enabled = 0;
  }

  *is_enabled = _is_enabled == 1;

  return __ID != 0;
}

bool database::location_auth(int LocationID, char *LocationPWD, int *UserID,
                             bool *is_enabled) {
  if (LocationID == 0) return false;

  return auth(
      "SELECT id, user_id, enabled FROM `supla_location` WHERE id = ? AND "
      "password = unhex(?)",
      LocationID, LocationPWD, SUPLA_LOCATION_PWDHEX_MAXSIZE, UserID,
      is_enabled);
}

bool database::accessid_auth(int AccessID, char *AccessIDpwd, int *UserID,
                             bool *is_enabled) {
  if (AccessID == 0) return false;

  return auth(
      "SELECT id, user_id, enabled FROM `supla_accessid` WHERE id = ? AND "
      "password = unhex(?)",
      AccessID, AccessIDpwd, SUPLA_ACCESSID_PWDHEX_MAXSIZE, UserID, is_enabled);
}

bool database::get_user_uniqueid(int UserID,
                                 char shortID[SHORT_UNIQUEID_MAXSIZE],
                                 char longID[LONG_UNIQUEID_MAXSIZE]) {
  bool result = false;
  char sql[] =
      "SELECT `short_unique_id`, `long_unique_id` FROM `supla_user` WHERE id = "
      "?";

  MYSQL_STMT *stmt = NULL;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  if (stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    MYSQL_BIND rbind[2];
    memset(rbind, 0, sizeof(rbind));

    unsigned long short_size = 0;
    my_bool short_is_null = true;

    unsigned long long_size = 0;
    my_bool long_is_null = true;

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = shortID;
    rbind[0].buffer_length = SHORT_UNIQUEID_MAXSIZE - 1;
    rbind[0].length = &short_size;
    rbind[0].is_null = &short_is_null;

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = longID;
    rbind[1].buffer_length = LONG_UNIQUEID_MAXSIZE - 1;
    rbind[1].length = &long_size;
    rbind[1].is_null = &long_is_null;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0 && !mysql_stmt_fetch(stmt)) {
        shortID[short_is_null ? 0 : short_size] = 0;
        longID[long_is_null ? 0 : long_size] = 0;
        result = true;
      }
    }
    mysql_stmt_close(stmt);
  }

  return result;
}

int database::get_user_id_by_email(const char Email[SUPLA_EMAIL_MAXSIZE]) {
  if (_mysql == NULL || strnlen(Email, SUPLA_EMAIL_MAXSIZE) == 0) return 0;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  char EmailHEX[SUPLA_EMAILHEX_MAXSIZE];
  memset(EmailHEX, 0, SUPLA_EMAILHEX_MAXSIZE);

  st_str2hex(EmailHEX, Email, SUPLA_EMAIL_MAXSIZE);

  pbind[0].buffer_type = MYSQL_TYPE_STRING;
  pbind[0].buffer = (char *)EmailHEX;
  pbind[0].buffer_length = strnlen(EmailHEX, SUPLA_EMAILHEX_MAXSIZE);

  int UserID = 0;
  MYSQL_STMT *stmt;

  if (stmt_get_int((void **)&stmt, &UserID, NULL, NULL, NULL,
                   "SELECT id FROM supla_user WHERE email = unhex(?)", pbind,
                   1)) {
    return UserID;
  }

  return 0;
}

bool database::get_authkey_hash(int ID, char *buffer, unsigned int buffer_size,
                                bool *is_null, const char *sql) {
  MYSQL_STMT *stmt = NULL;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ID;

  bool result = false;

  if (stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    my_bool _is_null = false;
    unsigned long size = 0;

    MYSQL_BIND rbind[1];
    memset(rbind, 0, sizeof(rbind));

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = buffer;
    rbind[0].buffer_length = buffer_size;
    rbind[0].length = &size;
    rbind[0].is_null = &_is_null;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0 && !mysql_stmt_fetch(stmt) &&
          buffer_size > size) {
        buffer[size] = 0;
        *is_null = _is_null > 0;

        result = true;
      }
    }

    mysql_stmt_close(stmt);
  }

  return result;
}

bool database::authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                            const char Email[SUPLA_EMAIL_MAXSIZE],
                            const char AuthKey[SUPLA_AUTHKEY_SIZE], int *UserID,
                            bool Client, const char *sql) {
  if (_mysql == NULL) {
    return false;
  }

  int ID = 0;
  int _UserID = get_user_id_by_email(Email);

  if (_UserID == 0 && !Client) {
    ID = get_device_id_and_user(GUID, &_UserID);
  }

  if (_UserID == 0) {
    return false;
  }

  if (ID == 0) ID = Client ? get_client_id(_UserID, GUID) : get_device_id(GUID);

  if (ID == 0) {  // Yes. When client not exists then is authorized
    *UserID = _UserID;
    return true;
  }

  bool is_null = false;
  char AuthKeyHash[BCRYPT_HASH_MAXSIZE];
  memset(AuthKeyHash, 0, BCRYPT_HASH_MAXSIZE);

  if (!get_authkey_hash(ID, AuthKeyHash, BCRYPT_HASH_MAXSIZE, &is_null, sql)) {
    return false;
  }

  if (is_null) {  // Yes. When is null then is authorized
    *UserID = _UserID;
    return true;
  }

  char AuthKeyHEX[SUPLA_AUTHKEY_HEXSIZE];
  memset(AuthKeyHEX, 0, SUPLA_AUTHKEY_HEXSIZE);

  st_authkey2hex(AuthKeyHEX, AuthKey);

  if (st_bcrypt_check(AuthKeyHEX, AuthKeyHash,
                      strnlen(AuthKeyHash, BCRYPT_HASH_MAXSIZE))) {
    *UserID = _UserID;
    return true;
  }

  return false;
}

bool database::client_authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                                   const char Email[SUPLA_EMAIL_MAXSIZE],
                                   const char AuthKey[SUPLA_AUTHKEY_SIZE],
                                   int *UserID) {
  return authkey_auth(GUID, Email, AuthKey, UserID, true,
                      "SELECT auth_key FROM supla_client WHERE id = ?");
}

bool database::device_authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                                   const char Email[SUPLA_EMAIL_MAXSIZE],
                                   const char AuthKey[SUPLA_AUTHKEY_SIZE],
                                   int *UserID) {
  return authkey_auth(GUID, Email, AuthKey, UserID, false,
                      "SELECT auth_key FROM supla_iodevice WHERE id = ?");
}

int database::get_device_id_and_user(const char GUID[SUPLA_GUID_SIZE],
                                     int *UserID) {
  if (_mysql == NULL) return false;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  char GUIDHEX[SUPLA_GUID_HEXSIZE];
  st_guid2hex(GUIDHEX, GUID);

  pbind[0].buffer_type = MYSQL_TYPE_STRING;
  pbind[0].buffer = (char *)GUIDHEX;
  pbind[0].buffer_length = SUPLA_GUID_HEXSIZE - 1;

  MYSQL_STMT *stmt;
  int dev_id;
  if (stmt_get_int(
          (void **)&stmt, &dev_id, UserID, NULL, NULL,
          "SELECT id, user_id FROM supla_iodevice WHERE guid = unhex(?)", pbind,
          1)) {
    return dev_id;
  }

  return 0;
}

int database::get_device_id(const char GUID[SUPLA_GUID_SIZE]) {
  int UserID = 0;
  return get_device_id_and_user(GUID, &UserID);
}

int database::get_device(int DeviceID, bool *device_enabled,
                         int *original_location_id, int *location_id,
                         bool *location_enabled, int *UserID) {
  if (_mysql == NULL || DeviceID == 0) return 0;

  MYSQL_STMT *stmt = NULL;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&DeviceID;

  int _device_enabled = 0;
  int _original_location_id = 0;
  int _location_id = 0;
  int _location_enabled = 0;
  int _UserID = 0;

  bool result = false;

  if (stmt_execute((void **)&stmt,
                   "SELECT CAST(d.`enabled` AS unsigned integer) `d_enabled`, "
                   "IFNULL(d.original_location_id, 0), IFNULL(d.location_id, "
                   "0), IFNULL(CAST(l.`enabled` AS unsigned integer), 0) "
                   "`l_enabled`, d.`user_id` FROM supla_iodevice d LEFT JOIN "
                   "supla_location l ON l.id = d.location_id WHERE d.id = ?",
                   pbind, 1, true)) {
    MYSQL_BIND rbind[5];
    memset(rbind, 0, sizeof(rbind));

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&_device_enabled;

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&_original_location_id;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&_location_id;

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&_location_enabled;

    rbind[4].buffer_type = MYSQL_TYPE_LONG;
    rbind[4].buffer = (char *)&_UserID;

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
        *UserID = _UserID;

        result = true;
      }
    }

    mysql_stmt_close(stmt);
  }

  return result ? DeviceID : 0;
}

bool database::on_newdevice(int DeviceID) {
  char sql[51];
  snprintf(sql, sizeof(sql), "CALL `supla_on_newdevice`(%i)", DeviceID);

  return query(sql) != 0;
}

bool database::on_newclient(int ClientID) {
  char sql[51];
  snprintf(sql, sizeof(sql), "CALL `supla_on_newclient`(%i)", ClientID);

  return query(sql) != 0;
}

bool database::on_channeladded(int DeviceID, int ChannelID) {
  char sql[51];
  snprintf(sql, sizeof(sql), "CALL `supla_on_channeladded`(%i, %i)", DeviceID,
           ChannelID);

  return query(sql) != 0;
}

int database::get_device_limit_left(int UserID) {
  return get_int(UserID, 0,
                 "SELECT IFNULL(limit_iodev, 0) - IFNULL(( SELECT COUNT(*) "
                 "FROM supla_iodevice WHERE user_id = supla_user.id ), 0) FROM "
                 "supla_user WHERE id = ?");
}

int database::get_device_count(int UserID) {
  return get_count(UserID,
                   "SELECT COUNT(*) FROM supla_iodevice WHERE user_id = ?");
}

int database::get_location_id(int UserID, bool enabled) {
  MYSQL_STMT *stmt;
  int Result = 0;
  int _enabled = enabled ? 1 : 0;

  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&_enabled;

  if (!stmt_get_int((void **)&stmt, &Result, NULL, NULL, NULL,
                    "SELECT id FROM `supla_location` WHERE user_id = ? AND "
                    "enabled = ? LIMIT 1",
                    pbind, 2))
    return 0;

  return Result;
}

bool database::get_device_reg_enabled(int UserID) {
  return get_count(UserID,
                   "SELECT COUNT(*) FROM `supla_user` WHERE id = ? AND "
                   "iodevice_reg_enabled IS NOT NULL AND iodevice_reg_enabled "
                   ">= UTC_TIMESTAMP()") > 0
             ? true
             : false;
}

int database::add_device(int LocationID, const char GUID[SUPLA_GUID_SIZE],
                         const char *AuthKey, const char *Name,
                         unsigned int ipv4, const char *softver,
                         int proto_version, short ManufacturerID,
                         short ProductID, int Flags, int UserID) {
  int DeviceID = 0;

  char NameHEX[SUPLA_DEVICE_NAMEHEX_MAXSIZE];
  st_str2hex(NameHEX, Name, SUPLA_DEVICE_NAME_MAXSIZE);

  char SoftverHEX[SUPLA_SOFTVERHEX_MAXSIZE];
  st_str2hex(SoftverHEX, softver, SUPLA_SOFTVER_MAXSIZE);

  char *AuthKeyHashHEX = NULL;

  MYSQL_BIND pbind[12];
  memset(pbind, 0, sizeof(pbind));

  char GUIDHEX[SUPLA_GUID_HEXSIZE];
  st_guid2hex(GUIDHEX, GUID);

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&LocationID;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&UserID;

  pbind[2].buffer_type = MYSQL_TYPE_STRING;
  pbind[2].buffer = (char *)GUIDHEX;
  pbind[2].buffer_length = SUPLA_GUID_HEXSIZE - 1;

  pbind[3].buffer_type = MYSQL_TYPE_STRING;
  pbind[3].buffer = (char *)NameHEX;
  pbind[3].buffer_length = strnlen(NameHEX, SUPLA_DEVICE_NAME_MAXSIZE);

  pbind[4].buffer_type = MYSQL_TYPE_LONG;
  pbind[4].buffer = (char *)&ipv4;
  pbind[4].is_unsigned = true;

  pbind[5].buffer_type = MYSQL_TYPE_STRING;
  pbind[5].buffer = (char *)SoftverHEX;
  pbind[5].buffer_length = strnlen(SoftverHEX, SUPLA_SOFTVERHEX_MAXSIZE);

  pbind[6].buffer_type = MYSQL_TYPE_LONG;
  pbind[6].buffer = (char *)&proto_version;

  pbind[7].buffer_type = MYSQL_TYPE_SHORT;
  pbind[7].buffer = (char *)&ProductID;

  pbind[8].buffer_type = MYSQL_TYPE_SHORT;
  pbind[8].buffer = (char *)&ManufacturerID;

  if (AuthKey == NULL) {
    pbind[10].buffer_type = MYSQL_TYPE_NULL;

    pbind[9].buffer_type = MYSQL_TYPE_LONG;
    pbind[9].buffer = (char *)&LocationID;

  } else {
    AuthKeyHashHEX = st_get_authkey_hash_hex(AuthKey);

    if (AuthKeyHashHEX == NULL) return 0;

    pbind[10].buffer_type = MYSQL_TYPE_STRING;
    pbind[10].buffer = (char *)AuthKeyHashHEX;
    pbind[10].buffer_length = strnlen(AuthKeyHashHEX, BCRYPT_HASH_MAXSIZE * 2);

    pbind[9].buffer_type = MYSQL_TYPE_NULL;
  }

  pbind[11].buffer_type = MYSQL_TYPE_LONG;
  pbind[11].buffer = (char *)&Flags;

  const char sql[] =
      "CALL  "
      "`supla_add_iodevice`(?,?,unhex(?),unhex(?),?,unhex(?),?,?,?,?,"
      "unhex(?),?,@id)";

  DeviceID = add_by_proc_call(sql, pbind, 12);

  if (AuthKeyHashHEX) {
    free(AuthKeyHashHEX);
    AuthKeyHashHEX = NULL;
  }

  return DeviceID;
}

int database::update_device(int DeviceID, int OriginalLocationID,
                            const char *AuthKey, const char *Name,
                            unsigned int ipv4, const char *softver,
                            int proto_version) {
  char NameHEX[SUPLA_DEVICE_NAMEHEX_MAXSIZE];
  st_str2hex(NameHEX, Name, SUPLA_DEVICE_NAME_MAXSIZE);

  char SoftverHEX[SUPLA_SOFTVERHEX_MAXSIZE];
  st_str2hex(SoftverHEX, softver, SUPLA_SOFTVER_MAXSIZE);

  char *AuthKeyHashHEX = NULL;

  MYSQL_BIND pbind[7];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_STRING;
  pbind[0].buffer = (char *)NameHEX;
  pbind[0].buffer_length = strnlen(NameHEX, 256);

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&ipv4;
  pbind[1].is_unsigned = true;

  pbind[2].buffer_type = MYSQL_TYPE_STRING;
  pbind[2].buffer = (char *)SoftverHEX;
  pbind[2].buffer_length = strnlen(SoftverHEX, SUPLA_SOFTVERHEX_MAXSIZE);

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&proto_version;

  if (OriginalLocationID == 0) {
    pbind[4].buffer_type = MYSQL_TYPE_NULL;

  } else {
    pbind[4].buffer_type = MYSQL_TYPE_LONG;
    pbind[4].buffer = (char *)&OriginalLocationID;
  }

  if (AuthKey == NULL) {
    pbind[5].buffer_type = MYSQL_TYPE_NULL;
  } else {
    AuthKeyHashHEX = st_get_authkey_hash_hex(AuthKey);

    if (AuthKeyHashHEX == NULL) return 0;

    pbind[5].buffer_type = MYSQL_TYPE_STRING;
    pbind[5].buffer = (char *)AuthKeyHashHEX;
    pbind[5].buffer_length = strnlen(AuthKeyHashHEX, BCRYPT_HASH_MAXSIZE * 2);
  }

  pbind[6].buffer_type = MYSQL_TYPE_LONG;
  pbind[6].buffer = (char *)&DeviceID;

  const char sql[] =
      "CALL "
      "`supla_update_iodevice`(unhex(?),?,unhex(?),?,?,unhex(?),?)";

  MYSQL_STMT *stmt;
  if (!stmt_execute((void **)&stmt, sql, pbind, 7, true)) {
    DeviceID = 0;
  }

  if (stmt != NULL) mysql_stmt_close(stmt);

  if (AuthKeyHashHEX) {
    free(AuthKeyHashHEX);
    AuthKeyHashHEX = NULL;
  }

  return DeviceID;
}

int database::get_device_channel_count(int DeviceID) {
  if (_mysql == NULL) return 0;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&DeviceID;

  MYSQL_STMT *stmt;
  int count = 0;

  stmt_get_int(
      (void **)&stmt, &count, NULL, NULL, NULL,
      "SELECT COUNT(*) FROM `supla_dev_channel` WHERE `iodevice_id` = ?", pbind,
      1);

  return count;
}

int database::get_device_channel(int DeviceID, int ChannelNumber, int *Type) {
  if (_mysql == NULL) return 0;

  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&DeviceID;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&ChannelNumber;

  MYSQL_STMT *stmt;
  int id = 0;
  int _type;

  if (Type == NULL) Type = &_type;

  stmt_get_int((void **)&stmt, &id, Type, NULL, NULL,
               "SELECT `id`, `type` FROM `supla_dev_channel` WHERE "
               "`iodevice_id` = ? AND `channel_number` = ?",
               pbind, 2);

  return id;
}

int database::add_device_channel(int DeviceID, int ChannelNumber, int Type,
                                 int Func, int FList, int Flags, int UserID,
                                 bool *new_channel) {
  MYSQL_BIND pbind[7];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&Type;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&Func;

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&UserID;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&ChannelNumber;

  pbind[4].buffer_type = MYSQL_TYPE_LONG;
  pbind[4].buffer = (char *)&DeviceID;

  pbind[5].buffer_type = MYSQL_TYPE_LONG;
  pbind[5].buffer = (char *)&FList;

  pbind[6].buffer_type = MYSQL_TYPE_LONG;
  pbind[6].buffer = (char *)&Flags;

  {
    const char sql[] = "CALL`supla_add_channel`(?,?,0,0,0,?,?,?,?,?)";

    MYSQL_STMT *stmt = NULL;
    if (!stmt_execute((void **)&stmt, sql, pbind, 7, true)) {
      if (stmt != NULL) mysql_stmt_close(stmt);
      return 0;
    } else if (new_channel) {
      *new_channel = true;
    }

    if (stmt != NULL) mysql_stmt_close(stmt);
  }

  return get_device_channel(DeviceID, ChannelNumber, NULL);
}

void database::get_device_channels(int DeviceID,
                                   supla_device_channels *channels) {
  MYSQL_STMT *stmt = NULL;
  const char sql[] =
      "SELECT `type`, `func`, `param1`, `param2`, `param3`, `text_param1`, "
      "`text_param2`, `text_param3`, `channel_number`, `id`, `hidden` FROM "
      "`supla_dev_channel` WHERE `iodevice_id` = ? ORDER BY `channel_number`";

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&DeviceID;

  if (stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    my_bool is_null[8];

    MYSQL_BIND rbind[11];
    memset(rbind, 0, sizeof(rbind));

    int type, func, param1, param2, param3, number, id, hidden;

    char text_param1[256];
    char text_param2[256];
    char text_param3[256];

    unsigned long text_param1_size = 0;
    unsigned long text_param2_size = 0;
    unsigned long text_param3_size = 0;

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

    rbind[5].buffer_type = MYSQL_TYPE_STRING;
    rbind[5].buffer = text_param1;
    rbind[5].is_null = &is_null[5];
    rbind[5].buffer_length = sizeof(text_param1) - 1;
    rbind[5].length = &text_param1_size;

    rbind[6].buffer_type = MYSQL_TYPE_STRING;
    rbind[6].buffer = text_param2;
    rbind[6].is_null = &is_null[6];
    rbind[6].buffer_length = sizeof(text_param2) - 1;
    rbind[6].length = &text_param2_size;

    rbind[7].buffer_type = MYSQL_TYPE_STRING;
    rbind[7].buffer = text_param3;
    rbind[7].is_null = &is_null[7];
    rbind[7].buffer_length = sizeof(text_param3) - 1;
    rbind[7].length = &text_param3_size;

    rbind[8].buffer_type = MYSQL_TYPE_LONG;
    rbind[8].buffer = (char *)&number;

    rbind[9].buffer_type = MYSQL_TYPE_LONG;
    rbind[9].buffer = (char *)&id;

    rbind[10].buffer_type = MYSQL_TYPE_LONG;
    rbind[10].buffer = (char *)&hidden;

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
          if (is_null[5] == true) text_param1_size = 0;
          if (is_null[6] == true) text_param2_size = 0;
          if (is_null[7] == true) text_param3_size = 0;

          text_param1[text_param1_size] = 0;
          text_param2[text_param2_size] = 0;
          text_param3[text_param3_size] = 0;

          channels->add_channel(id, number, type, func, param1, param2, param3,
                                text_param1, text_param2, text_param3,
                                hidden > 0);
        }
      }
    }

    mysql_stmt_close(stmt);
  }
}

int database::get_client_id(int UserID, const char GUID[SUPLA_GUID_SIZE]) {
  MYSQL_STMT *stmt;
  int Result = 0;

  char GUIDHEX[SUPLA_GUID_HEXSIZE];
  st_guid2hex(GUIDHEX, GUID);

  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)GUIDHEX;
  pbind[1].buffer_length = SUPLA_GUID_HEXSIZE - 1;

  if (!stmt_get_int(
          (void **)&stmt, &Result, NULL, NULL, NULL,
          "SELECT id FROM supla_client WHERE user_id = ? AND guid = unhex(?)",
          pbind, 2))
    return 0;

  return Result;
}

int database::get_client(int ClientID, bool *client_enabled, int *access_id,
                         bool *accessid_enabled) {
  if (_mysql == NULL || ClientID == 0) return 0;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ClientID;

  int _client_enabled = 0;
  int _access_id = 0;
  int _accessid_enabled = 0;

  MYSQL_STMT *stmt;

  if (stmt_get_int((void **)&stmt, &_client_enabled, &_access_id,
                   &_accessid_enabled, NULL,
                   "SELECT CAST(c.`enabled` AS unsigned integer) `c_enabled`, "
                   "IFNULL(c.access_id, 0), IFNULL(CAST(a.`enabled` AS "
                   "unsigned integer), 0) `a_enabled` FROM supla_client c LEFT "
                   "JOIN supla_accessid a ON a.id = c.access_id WHERE c.id = ?",
                   pbind, 1)) {
    *client_enabled = _client_enabled == 1;
    *access_id = _access_id;
    *accessid_enabled = _accessid_enabled == 1;
    return ClientID;
  }

  return 0;
}

int database::get_client_limit_left(int UserID) {
  return get_int(UserID, 0,
                 "SELECT IFNULL(limit_client, 0) - IFNULL(( SELECT COUNT(*) "
                 "FROM supla_client WHERE user_id = supla_user.id ), 0) FROM "
                 "supla_user WHERE id = ?");
}

int database::get_client_count(int UserID) {
  return get_count(UserID,
                   "SELECT COUNT(*) FROM supla_client WHERE user_id = ?");
}

int database::get_access_id(int UserID, bool enabled) {
  MYSQL_STMT *stmt;
  int Result = 0;
  int _enabled = enabled ? 1 : 0;

  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&_enabled;

  if (!stmt_get_int((void **)&stmt, &Result, NULL, NULL, NULL,
                    "SELECT id FROM `supla_accessid` WHERE user_id = ? AND "
                    "enabled = ? LIMIT 1",
                    pbind, 2))
    return 0;

  return Result;
}

int database::get_client_access_id(int ClientID, bool *accessid_enabled) {
  MYSQL_STMT *stmt;
  int Result = 0;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  int _accessid_enabled = 0;

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ClientID;

  if (!stmt_get_int((void **)&stmt, &Result, &_accessid_enabled, NULL, NULL,
                    "SELECT c.access_id, a.enabled FROM `supla_client` c JOIN "
                    "`supla_accessid` a ON a.id = c.access_id WHERE c.id = ?",
                    pbind, 1))
    return 0;

  if (accessid_enabled && _accessid_enabled == 1) {
    *accessid_enabled = true;
  }

  return Result;
}

bool database::get_client_reg_enabled(int UserID) {
  return get_count(UserID,
                   "SELECT COUNT(*) FROM `supla_user` WHERE id = ? AND "
                   "client_reg_enabled IS NOT NULL AND client_reg_enabled >= "
                   "UTC_TIMESTAMP()") > 0
             ? true
             : false;
}

int database::add_client(int AccessID, const char *GUID, const char *AuthKey,
                         const char *Name, unsigned int ipv4,
                         const char *softver, int proto_version, int UserID) {
  int ClientID = 0;

  char NameHEX[SUPLA_CLIENT_NAMEHEX_MAXSIZE];
  st_str2hex(NameHEX, Name, SUPLA_CLIENT_NAME_MAXSIZE);

  char SoftverHEX[SUPLA_SOFTVERHEX_MAXSIZE];
  st_str2hex(SoftverHEX, softver, SUPLA_SOFTVER_MAXSIZE);

  char *AuthKeyHashHEX = NULL;

  MYSQL_BIND pbind[8];
  memset(pbind, 0, sizeof(pbind));

  char GUIDHEX[SUPLA_GUID_HEXSIZE];
  st_guid2hex(GUIDHEX, GUID);

  if (AccessID == 0) {
    pbind[0].buffer_type = MYSQL_TYPE_NULL;
  } else {
    pbind[0].buffer_type = MYSQL_TYPE_LONG;
    pbind[0].buffer = (char *)&AccessID;
  }

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)GUIDHEX;
  pbind[1].buffer_length = SUPLA_GUID_HEXSIZE - 1;

  pbind[2].buffer_type = MYSQL_TYPE_STRING;
  pbind[2].buffer = (char *)NameHEX;
  pbind[2].buffer_length = strnlen(NameHEX, SUPLA_CLIENT_NAMEHEX_MAXSIZE);

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&ipv4;
  pbind[3].is_unsigned = true;

  pbind[4].buffer_type = MYSQL_TYPE_STRING;
  pbind[4].buffer = (char *)SoftverHEX;
  pbind[4].buffer_length = strnlen(SoftverHEX, SUPLA_SOFTVERHEX_MAXSIZE);

  pbind[5].buffer_type = MYSQL_TYPE_LONG;
  pbind[5].buffer = (char *)&proto_version;

  pbind[6].buffer_type = MYSQL_TYPE_LONG;
  pbind[6].buffer = (char *)&UserID;

  if (AuthKey == NULL) {
    pbind[7].buffer_type = MYSQL_TYPE_NULL;
  } else {
    AuthKeyHashHEX = st_get_authkey_hash_hex(AuthKey);

    if (AuthKeyHashHEX == NULL) return 0;

    pbind[7].buffer_type = MYSQL_TYPE_STRING;
    pbind[7].buffer = (char *)AuthKeyHashHEX;
    pbind[7].buffer_length = strnlen(AuthKeyHashHEX, BCRYPT_HASH_MAXSIZE * 2);
  }

  const char sql[] =
      "CALL "
      "`supla_add_client`(?,unhex(?),unhex(?),?,unhex(?),?,?,unhex(?),@id)";

  ClientID = add_by_proc_call(sql, pbind, 8);

  if (AuthKeyHashHEX) {
    free(AuthKeyHashHEX);
    AuthKeyHashHEX = NULL;
  }

  if (ClientID != 0) {
    on_newclient(ClientID);
  }

  return ClientID;
}

bool database::update_client(int ClientID, int AccessID, const char *AuthKey,
                             const char *Name, unsigned int ipv4,
                             const char *softver, int proto_version) {
  bool result = false;

  char NameHEX[SUPLA_DEVICE_NAMEHEX_MAXSIZE];
  st_str2hex(NameHEX, Name, SUPLA_DEVICE_NAME_MAXSIZE);

  char SoftverHEX[SUPLA_SOFTVERHEX_MAXSIZE];
  st_str2hex(SoftverHEX, softver, SUPLA_SOFTVER_MAXSIZE);

  MYSQL_BIND pbind[7];
  memset(pbind, 0, sizeof(pbind));

  char *AuthKeyHashHEX = NULL;

  if (AccessID == 0) {
    pbind[0].buffer_type = MYSQL_TYPE_NULL;
  } else {
    pbind[0].buffer_type = MYSQL_TYPE_LONG;
    pbind[0].buffer = (char *)&AccessID;
  }

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)NameHEX;
  pbind[1].buffer_length = strnlen(NameHEX, 256);

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&ipv4;
  pbind[2].is_unsigned = true;

  pbind[3].buffer_type = MYSQL_TYPE_STRING;
  pbind[3].buffer = (char *)SoftverHEX;
  pbind[3].buffer_length = strnlen(SoftverHEX, SUPLA_SOFTVERHEX_MAXSIZE);

  pbind[4].buffer_type = MYSQL_TYPE_LONG;
  pbind[4].buffer = (char *)&proto_version;

  if (AuthKey == NULL) {
    pbind[5].buffer_type = MYSQL_TYPE_NULL;
  } else {
    AuthKeyHashHEX = st_get_authkey_hash_hex(AuthKey);

    if (AuthKeyHashHEX == NULL) return 0;

    pbind[5].buffer_type = MYSQL_TYPE_STRING;
    pbind[5].buffer = (char *)AuthKeyHashHEX;
    pbind[5].buffer_length = strnlen(AuthKeyHashHEX, BCRYPT_HASH_MAXSIZE * 2);
  }

  pbind[6].buffer_type = MYSQL_TYPE_LONG;
  pbind[6].buffer = (char *)&ClientID;

  const char sql[] =
      "CALL `supla_update_client`(?,unhex(?),?,unhex(?),?,unhex(?),?)";

  MYSQL_STMT *stmt;
  if (stmt_execute((void **)&stmt, sql, pbind, 7, true)) {
    result = true;
  }

  if (stmt != NULL) mysql_stmt_close(stmt);

  if (AuthKeyHashHEX) {
    free(AuthKeyHashHEX);
    AuthKeyHashHEX = NULL;
  }

  return result;
}

void database::get_client_locations(int ClientID,
                                    supla_client_locations *locs) {
  MYSQL_STMT *stmt;

  const char sql[] =
      "SELECT `id`, `caption` FROM `supla_v_client_location`  WHERE "
      "`client_id` = ?";

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ClientID;

  if (stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    my_bool is_null[2] = {false, false};

    MYSQL_BIND rbind[2];
    memset(rbind, 0, sizeof(rbind));

    int id = 0;
    unsigned long size = 0;
    char caption[SUPLA_LOCATION_CAPTION_MAXSIZE];  // utf8

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&id;
    rbind[0].is_null = &is_null[0];

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = caption;
    rbind[1].buffer_length = SUPLA_LOCATION_CAPTION_MAXSIZE;
    rbind[1].length = &size;
    rbind[1].is_null = &is_null[1];

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          if (size >= SUPLA_LOCATION_CAPTION_MAXSIZE) {
            size = SUPLA_LOCATION_CAPTION_MAXSIZE - 1;
          }
          caption[is_null[1] ? 0 : size] = 0;
          locs->add_location(id, caption);
        }
      }
    }

    mysql_stmt_close(stmt);
  }
}

void database::get_client_channels(int ClientID, int *DeviceID,
                                   supla_client_channels *channels) {
  MYSQL_STMT *stmt;
  const char sql1[] =
      "SELECT `id`, `type`, `func`, `param1`, `param2`, `param3`, "
      "`text_param1`, "
      "`text_param2`, `text_param3`, `iodevice_id`, `location_id`, `caption`, "
      "`alt_icon`, `user_icon_id`, `manufacturer_id`, `product_id`, "
      "`protocol_version` FROM `supla_v_client_channel` WHERE `client_id` = ? "
      "ORDER BY `iodevice_id`, `channel_number`";
  const char sql2[] =
      "SELECT `id`, `type`, `func`, `param1`, `param2`, `param3`, "
      "`text_param1`, "
      "`text_param2`, `text_param3`, `iodevice_id`, `location_id`, `caption`, "
      "`alt_icon`, `user_icon_id`, `manufacturer_id`, `product_id`, "
      "`protocol_version` FROM `supla_v_client_channel` WHERE `client_id` = ? "
      "AND `iodevice_id` = ? ORDER BY `channel_number`";

  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ClientID;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)DeviceID;

  if (stmt_execute((void **)&stmt, DeviceID ? sql2 : sql1, pbind,
                   DeviceID ? 2 : 1, true)) {
    MYSQL_BIND rbind[17];
    memset(rbind, 0, sizeof(rbind));

    int id, type, func, param1, param2, param3, iodevice_id, location_id,
        alt_icon, user_icon, protocol_version;
    short manufacturer_id = 0;
    short product_id = 0;
    char text_param1[256];
    char text_param2[256];
    char text_param3[256];

    unsigned long caption_size = 0;
    unsigned long text_param1_size = 0;
    unsigned long text_param2_size = 0;
    unsigned long text_param3_size = 0;

    my_bool caption_is_null = true;
    my_bool text_param1_is_null = true;
    my_bool text_param2_is_null = true;
    my_bool text_param3_is_null = true;

    char caption[SUPLA_CHANNEL_CAPTION_MAXSIZE];

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&id;

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&type;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&func;

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&param1;

    rbind[4].buffer_type = MYSQL_TYPE_LONG;
    rbind[4].buffer = (char *)&param2;

    rbind[5].buffer_type = MYSQL_TYPE_LONG;
    rbind[5].buffer = (char *)&param3;

    rbind[6].buffer_type = MYSQL_TYPE_STRING;
    rbind[6].buffer = text_param1;
    rbind[6].is_null = &text_param1_is_null;
    rbind[6].buffer_length = sizeof(text_param1) - 1;
    rbind[6].length = &text_param1_size;

    rbind[7].buffer_type = MYSQL_TYPE_STRING;
    rbind[7].buffer = text_param2;
    rbind[7].is_null = &text_param2_is_null;
    rbind[7].buffer_length = sizeof(text_param2) - 1;
    rbind[7].length = &text_param2_size;

    rbind[8].buffer_type = MYSQL_TYPE_STRING;
    rbind[8].buffer = text_param3;
    rbind[8].is_null = &text_param3_is_null;
    rbind[8].buffer_length = sizeof(text_param3) - 1;
    rbind[8].length = &text_param3_size;

    rbind[9].buffer_type = MYSQL_TYPE_LONG;
    rbind[9].buffer = (char *)&iodevice_id;

    rbind[10].buffer_type = MYSQL_TYPE_LONG;
    rbind[10].buffer = (char *)&location_id;

    rbind[11].buffer_type = MYSQL_TYPE_STRING;
    rbind[11].buffer = caption;
    rbind[11].is_null = &caption_is_null;
    rbind[11].buffer_length = SUPLA_CHANNEL_CAPTION_MAXSIZE - 1;
    rbind[11].length = &caption_size;

    rbind[12].buffer_type = MYSQL_TYPE_LONG;
    rbind[12].buffer = (char *)&alt_icon;

    rbind[13].buffer_type = MYSQL_TYPE_LONG;
    rbind[13].buffer = (char *)&user_icon;

    rbind[14].buffer_type = MYSQL_TYPE_SHORT;
    rbind[14].buffer = (char *)&manufacturer_id;

    rbind[15].buffer_type = MYSQL_TYPE_SHORT;
    rbind[15].buffer = (char *)&product_id;

    rbind[16].buffer_type = MYSQL_TYPE_LONG;
    rbind[16].buffer = (char *)&protocol_version;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          if (caption_size >= SUPLA_CHANNEL_CAPTION_MAXSIZE) {
            caption_size = SUPLA_CHANNEL_CAPTION_MAXSIZE - 1;
          }

          caption[caption_size] = 0;
          text_param1[text_param1_size] = 0;
          text_param2[text_param2_size] = 0;
          text_param3[text_param3_size] = 0;

          supla_client_channel *channel = new supla_client_channel(
              channels, id, iodevice_id, location_id, type, func, param1,
              param2, param3, text_param1_is_null ? NULL : text_param1,
              text_param2_is_null ? NULL : text_param2,
              text_param3_is_null ? NULL : text_param3,
              caption_is_null ? NULL : caption, alt_icon, user_icon,
              manufacturer_id, product_id, protocol_version);

          if (!channels->add(channel)) {
            delete channel;
          }
        }
      }
    }

    mysql_stmt_close(stmt);
  }
}

void database::get_user_channel_groups(int UserID,
                                       supla_user_channelgroups *cgroups) {
  MYSQL_STMT *stmt;
  const char sql[] =
      "SELECT `id`, `channel_id`, `iodevice_id` FROM "
      "`supla_v_user_channel_group` "
      "WHERE `user_id` = ? ORDER BY `id`";

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  if (stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    MYSQL_BIND rbind[3];
    memset(rbind, 0, sizeof(rbind));

    int group_id, channel_id, iodevice_id;

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&group_id;

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&channel_id;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&iodevice_id;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          supla_user_channelgroup *cg = new supla_user_channelgroup(
              cgroups, group_id, channel_id, iodevice_id);
          if (!cgroups->add(cg, master)) {
            delete cg;
          }
        }
      }
    }

    mysql_stmt_close(stmt);
  }
}

void database::get_client_channel_groups(int ClientID,
                                         supla_client_channelgroups *cgroups) {
  MYSQL_STMT *stmt;
  const char sql[] =
      "SELECT `id`, `func`, `location_id`, `caption`, `alt_icon`, "
      "`user_icon_id`  FROM `supla_v_client_channel_group` WHERE `client_id` = "
      "? ORDER BY `id`";

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ClientID;

  if (stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    my_bool is_null;

    MYSQL_BIND rbind[6];
    memset(rbind, 0, sizeof(rbind));

    int id = 0;
    int func = 0;
    int location_id = 0;
    int alt_icon = 0;
    int user_icon = 0;
    unsigned long size = 0;
    char caption[SUPLA_CHANNELGROUP_CAPTION_MAXSIZE];

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&id;

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&func;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&location_id;

    rbind[3].buffer_type = MYSQL_TYPE_STRING;
    rbind[3].buffer = caption;
    rbind[3].is_null = &is_null;
    rbind[3].buffer_length = SUPLA_CHANNELGROUP_CAPTION_MAXSIZE;
    rbind[3].length = &size;

    rbind[4].buffer_type = MYSQL_TYPE_LONG;
    rbind[4].buffer = (char *)&alt_icon;

    rbind[5].buffer_type = MYSQL_TYPE_LONG;
    rbind[5].buffer = (char *)&user_icon;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          if (size >= SUPLA_CHANNELGROUP_CAPTION_MAXSIZE) {
            size = SUPLA_CHANNELGROUP_CAPTION_MAXSIZE - 1;
          }
          caption[size] = 0;

          supla_client_channelgroup *cg = new supla_client_channelgroup(
              cgroups, id, location_id, func, is_null ? NULL : caption,
              alt_icon, user_icon);
          if (!cgroups->add(cg, master)) {
            delete cg;
          }
        }
      }
    }

    mysql_stmt_close(stmt);
  }
}

void database::get_client_channel_group_relations(
    int ClientID, supla_client_channelgroups *cgroups) {
  MYSQL_STMT *stmt;
  const char sql[] =
      "SELECT `channel_id`, `group_id`, `channel_hidden`, `iodevice_id` FROM "
      "`supla_v_rel_cg` "
      "WHERE `client_id` "
      "= ? ORDER BY `channel_id`";

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ClientID;

  if (stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    MYSQL_BIND rbind[4];
    memset(rbind, 0, sizeof(rbind));

    int channel_id, group_id, hidden, iodevice_id;

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&channel_id;

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&group_id;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&hidden;

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&iodevice_id;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          supla_client_channelgroup_relation *cg_rel =
              new supla_client_channelgroup_relation(cgroups, iodevice_id,
                                                     channel_id, group_id);
          if (!cgroups->add(cg_rel, detail1)) {
            delete cg_rel;
          }
          if (hidden > 0) {
            supla_client_channelgroup_value *cg_value =
                new supla_client_channelgroup_value(cgroups, channel_id,
                                                    iodevice_id);
            if (!cgroups->add(cg_value, detail2)) {
              delete cg_value;
            }
          }
        }
      }
    }

    mysql_stmt_close(stmt);
  }
}

void database::add_temperature(int ChannelID, double temperature) {
  char buff[20];
  memset(buff, 0, sizeof(buff));

  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  snprintf(buff, sizeof(buff), "%04.4f", temperature);

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ChannelID;

  pbind[1].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[1].buffer = (char *)buff;
  pbind[1].buffer_length = strnlen(buff, 20);

  const char sql[] = "CALL `supla_add_temperature_log_item`(?,?)";

  MYSQL_STMT *stmt;
  stmt_execute((void **)&stmt, sql, pbind, 2, true);

  if (stmt != NULL) mysql_stmt_close(stmt);
}

void database::add_temperature_and_humidity(int ChannelID, double temperature,
                                            double humidity) {
  char buff1[20];
  memset(buff1, 0, sizeof(buff1));

  char buff2[20];
  memset(buff2, 0, sizeof(buff2));

  MYSQL_BIND pbind[3];
  memset(pbind, 0, sizeof(pbind));

  snprintf(buff1, sizeof(buff1), "%04.3f", temperature);
  snprintf(buff2, sizeof(buff2), "%04.3f", humidity);

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ChannelID;

  pbind[1].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[1].buffer = (char *)buff1;
  pbind[1].buffer_length = strnlen(buff1, 20);

  pbind[2].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[2].buffer = (char *)buff2;
  pbind[2].buffer_length = strnlen(buff2, 20);

  const char sql[] = "CALL `supla_add_temphumidity_log_item`(?,?,?)";

  MYSQL_STMT *stmt;
  stmt_execute((void **)&stmt, sql, pbind, 3, true);

  if (stmt != NULL) mysql_stmt_close(stmt);
}
void database::em_set_longlong(unsigned _supla_int64_t *v, void *pbind) {
  if (*v == 0) {
    ((MYSQL_BIND *)pbind)->buffer_type = MYSQL_TYPE_NULL;
  } else {
    ((MYSQL_BIND *)pbind)->buffer_type = MYSQL_TYPE_LONGLONG;
    ((MYSQL_BIND *)pbind)->buffer = (char *)v;
  }
}

void database::add_electricity_measurement(
    supla_channel_electricity_measurement *em) {
  MYSQL_BIND pbind[14];
  memset(pbind, 0, sizeof(pbind));

  int ChannelID = em->getChannelId();
  TElectricityMeter_ExtendedValue em_ev;
  em->getMeasurement(&em_ev);

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ChannelID;

  int n = 0;
  for (int a = 0; a < 3; a++) {
    em_set_longlong(&em_ev.total_forward_active_energy[a], &pbind[1 + n]);
    em_set_longlong(&em_ev.total_reverse_active_energy[a], &pbind[2 + n]);
    em_set_longlong(&em_ev.total_forward_reactive_energy[a], &pbind[3 + n]);
    em_set_longlong(&em_ev.total_reverse_reactive_energy[a], &pbind[4 + n]);

    n += 4;
  }

  const char sql[] = "CALL `supla_add_em_log_item`(?,?,?,?,?,?,?,?,?,?,?,?,?)";

  MYSQL_STMT *stmt;
  stmt_execute((void **)&stmt, sql, pbind, 13, true);

  if (stmt != NULL) mysql_stmt_close(stmt);
}

void database::add_impulses(supla_channel_ic_measurement *ic) {
  MYSQL_BIND pbind[3];
  memset(pbind, 0, sizeof(pbind));

  int ChannelId = ic->getChannelId();
  unsigned _supla_int64_t counter = ic->getCounter();
  unsigned _supla_int64_t calculatedValue = ic->getCalculatedValue();

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ChannelId;
  pbind[1].buffer_type = MYSQL_TYPE_LONGLONG;
  pbind[1].buffer = (char *)&counter;
  pbind[2].buffer_type = MYSQL_TYPE_LONGLONG;
  pbind[2].buffer = (char *)&calculatedValue;

  const char sql[] = "CALL `supla_add_ic_log_item`(?,?,?)";

  MYSQL_STMT *stmt;
  stmt_execute((void **)&stmt, sql, pbind, 3, true);

  if (stmt != NULL) mysql_stmt_close(stmt);
}

void database::add_thermostat_measurements(
    supla_channel_thermostat_measurement *th) {
  char buff1[20];
  memset(buff1, 0, sizeof(buff1));

  char buff2[20];
  memset(buff2, 0, sizeof(buff2));

  MYSQL_BIND pbind[3];
  memset(pbind, 0, sizeof(pbind));

  int ChannelId = th->getChannelId();
  snprintf(buff1, sizeof(buff1), "%05.2f", th->getMeasuredTemperature());
  snprintf(buff2, sizeof(buff2), "%05.2f", th->getPresetTemperature());

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ChannelId;

  pbind[1].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[1].buffer = (char *)buff1;
  pbind[1].buffer_length = strnlen(buff1, 20);

  pbind[2].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[2].buffer = (char *)buff2;
  pbind[2].buffer_length = strnlen(buff2, 20);

  const char sql[] = "CALL `supla_add_thermostat_log_item`(?,?,?)";

  MYSQL_STMT *stmt;
  stmt_execute((void **)&stmt, sql, pbind, 3, true);

  if (stmt != NULL) mysql_stmt_close(stmt);
}

bool database::get_device_firmware_update_url(
    int DeviceID, TDS_FirmwareUpdateParams *params,
    TSD_FirmwareUpdate_UrlResult *url) {
  MYSQL_STMT *stmt;
  MYSQL_BIND pbind[6];
  memset(pbind, 0, sizeof(pbind));
  memset(url, 0, sizeof(TSD_FirmwareUpdate_UrlResult));

  bool result = false;

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&DeviceID;

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
    if (stmt != NULL) mysql_stmt_close((MYSQL_STMT *)stmt);
    q_executed++;
  }

  if (stmt_execute((void **)&stmt, sql2, NULL, 0, true)) {
    if (stmt != NULL) mysql_stmt_close((MYSQL_STMT *)stmt);
    q_executed++;
  }

  if (q_executed == 2 && stmt_execute((void **)&stmt, sql3, NULL, 0, true)) {
    if (stmt != NULL) mysql_stmt_store_result(stmt);

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

  return result;
}

bool database::get_reg_enabled(int UserID, unsigned int *client,
                               unsigned int *iodevice) {
  if (UserID == 0) return false;

  MYSQL_STMT *stmt;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  if (!stmt_get_int(
          (void **)&stmt, (int *)client, (int *)iodevice, NULL, NULL,
          "SELECT CAST(IFNULL(UNIX_TIMESTAMP(CONVERT_TZ(IF(client_reg_enabled "
          ">= UTC_TIMESTAMP(), client_reg_enabled, NULL),'+00:00','SYSTEM')), "
          "0) AS UNSIGNED INT), "
          "CAST(IFNULL(UNIX_TIMESTAMP(CONVERT_TZ(IF(iodevice_reg_enabled >= "
          "UTC_TIMESTAMP(), iodevice_reg_enabled, NULL),'+00:00','SYSTEM')), "
          "0) AS UNSIGNED INT) FROM supla_user WHERE id = ?",
          pbind, 1))
    return false;

  return true;
}

int database::oauth_add_client_id(void) {
  char random_id[51];
  char secret[51];

  st_random_alpha_string(random_id, 51);
  st_random_alpha_string(secret, 51);

  for (int a = 0; a < 51; a++) {
    random_id[a] = tolower(random_id[a]);
    secret[a] = tolower(secret[a]);
  }

  char sql[] = "CALL `supla_oauth_add_client_for_app`(?,?,@id)";

  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_STRING;
  pbind[0].buffer = (char *)random_id;
  pbind[0].buffer_length = strnlen(random_id, 50);

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)secret;
  pbind[1].buffer_length = strnlen(secret, 50);

  return add_by_proc_call(sql, pbind, 2);
}

int database::oauth_get_client_id(bool create) {
  int id = 0;
  MYSQL_STMT *stmt;

  if (!stmt_get_int(
          (void **)&stmt, &id, NULL, NULL, NULL,
          "SELECT `id` FROM `supla_oauth_clients` WHERE `type` = 2 LIMIT 1",
          NULL, 0)) {
    id = 0;
  }

  if (id == 0 && create) {
    id = oauth_add_client_id();
  }

  return id;
}

bool database::oauth_get_token(TSC_OAuthToken *token, int user_id,
                               int access_id) {
  if (token == NULL) {
    return false;
  }

  memset(token, 0, sizeof(TSC_OAuthToken));

  if (oauth_get_client_id(true) == 0 || svrcfg_oauth_url_base64 == NULL ||
      svrcfg_oauth_url_base64_len <= 0 ||
      svrcfg_oauth_url_base64_len + CFG_OAUTH_TOKEN_SIZE + 2 >
          SUPLA_OAUTH_TOKEN_MAXSIZE) {
    return false;
  }

  st_random_alpha_string(token->Token, CFG_OAUTH_TOKEN_SIZE + 1);
  token->Token[CFG_OAUTH_TOKEN_SIZE] = '.';

  memcpy(&token->Token[CFG_OAUTH_TOKEN_SIZE + 1], svrcfg_oauth_url_base64,
         svrcfg_oauth_url_base64_len);
  token->Token[svrcfg_oauth_url_base64_len + CFG_OAUTH_TOKEN_SIZE + 1] = 0;
  token->TokenSize = svrcfg_oauth_url_base64_len + CFG_OAUTH_TOKEN_SIZE + 2;
  token->ExpiresIn = (unsigned)scfg_int(CFG_OAUTH_TOKEN_LIFETIME);

  int ExpiresIn = token->ExpiresIn + (unsigned)time(NULL);

  char sql[] = "CALL `supla_oauth_add_token_for_app`(?,?,?,?,@id)";

  MYSQL_BIND pbind[4];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)token->Token;
  pbind[1].buffer_length = token->TokenSize - 1;

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&ExpiresIn;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&access_id;

  return add_by_proc_call(sql, pbind, 4) > 0;
}

bool database::superuser_authorization(int UserID,
                                       char email[SUPLA_EMAIL_MAXSIZE],
                                       char password[SUPLA_PASSWORD_MAXSIZE]) {
  MYSQL_STMT *stmt = NULL;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  bool result = false;

  if (stmt_execute((void **)&stmt,
                   "SELECT email, password FROM supla_user WHERE id = ?", pbind,
                   1, true)) {
    MYSQL_BIND rbind[2];
    memset(rbind, 0, sizeof(rbind));

    char buffer_email[256];
    unsigned long email_size = 0;
    my_bool email_is_null = true;

    char buffer_password[65];
    unsigned long password_size = 0;
    my_bool password_is_null = true;

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = buffer_email;
    rbind[0].buffer_length = sizeof(buffer_email);
    rbind[0].length = &email_size;
    rbind[0].is_null = &email_is_null;

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = buffer_password;
    rbind[1].buffer_length = sizeof(buffer_password);
    rbind[1].length = &password_size;
    rbind[1].is_null = &password_is_null;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0 && !mysql_stmt_fetch(stmt) &&
          !email_is_null && !password_is_null &&
          email_size < rbind[0].buffer_length &&
          password_size < rbind[1].buffer_length) {
        buffer_email[email_size] = 0;
        buffer_password[password_size] = 0;

        if (strncmp(email, buffer_email, email_size) == 0 &&
            st_bcrypt_check(password, buffer_password, password_size) == 1) {
          result = 1;
        }
      }
    }

    mysql_stmt_close(stmt);
  }

  return result;
}

bool database::amazon_alexa_load_token(supla_amazon_alexa *alexa) {
  bool result = false;
  char sql[] =
      "SELECT `access_token`, `refresh_token`, TIMESTAMPDIFF(SECOND, "
      "UTC_TIMESTAMP(), expires_at) `expires_in`, `region` "
      "FROM "
      "`supla_amazon_alexa` WHERE user_id = ? AND LENGTH(access_token) > 0";

  MYSQL_STMT *stmt = NULL;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  int UserID = alexa->getUserID();

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  if (stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    MYSQL_BIND rbind[4];
    memset(rbind, 0, sizeof(rbind));

    char buffer_token[ALEXA_TOKEN_MAXSIZE + 1];
    buffer_token[0] = 0;
    unsigned long token_size = 0;
    my_bool token_is_null = true;

    char buffer_refresh_token[ALEXA_TOKEN_MAXSIZE + 1];
    buffer_refresh_token[0] = 0;
    unsigned long refresh_token_size = 0;
    my_bool refresh_token_is_null = true;

    char buffer_region[ALEXA_REGION_MAXSIZE + 1];
    buffer_region[0] = 0;
    unsigned long region_size = 0;
    my_bool region_is_null = true;

    int expires_in = 0;

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = buffer_token;
    rbind[0].buffer_length = ALEXA_TOKEN_MAXSIZE;
    rbind[0].length = &token_size;
    rbind[0].is_null = &token_is_null;

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = buffer_refresh_token;
    rbind[1].buffer_length = ALEXA_TOKEN_MAXSIZE;
    rbind[1].length = &refresh_token_size;
    rbind[1].is_null = &refresh_token_is_null;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&expires_in;

    rbind[3].buffer_type = MYSQL_TYPE_STRING;
    rbind[3].buffer = buffer_region;
    rbind[3].buffer_length = ALEXA_REGION_MAXSIZE;
    rbind[3].length = &region_size;
    rbind[3].is_null = &region_is_null;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0 && !mysql_stmt_fetch(stmt)) {
        buffer_token[token_is_null ? 0 : token_size] = 0;
        buffer_refresh_token[refresh_token_is_null ? 0 : refresh_token_size] =
            0;
        buffer_region[region_is_null ? 0 : region_size] = 0;

        alexa->set(buffer_token, buffer_refresh_token, expires_in,
                   region_is_null ? NULL : buffer_region);
        result = true;
      }
    }
    mysql_stmt_close(stmt);
  }

  return result;
}

void database::amazon_alexa_remove_token(supla_amazon_alexa *alexa) {
  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  int UserID = alexa->getUserID();

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  const char sql[] = "CALL `supla_update_amazon_alexa`('','',0,?)";

  MYSQL_STMT *stmt;
  stmt_execute((void **)&stmt, sql, pbind, 1, true);

  if (stmt != NULL) mysql_stmt_close(stmt);
}

void database::amazon_alexa_update_token(supla_amazon_alexa *alexa,
                                         const char *token,
                                         const char *refresh_token,
                                         int expires_in) {
  MYSQL_BIND pbind[4];
  memset(pbind, 0, sizeof(pbind));

  int UserID = alexa->getUserID();

  pbind[0].buffer_type = MYSQL_TYPE_STRING;
  pbind[0].buffer = (char *)token;
  pbind[0].buffer_length = strnlen((char *)token, ALEXA_TOKEN_MAXSIZE);

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)refresh_token;
  pbind[1].buffer_length = strnlen((char *)refresh_token, ALEXA_TOKEN_MAXSIZE);

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&expires_in;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&UserID;

  const char sql[] = "CALL `supla_update_amazon_alexa`(?,?,?,?)";

  MYSQL_STMT *stmt;
  stmt_execute((void **)&stmt, sql, pbind, 4, true);

  if (stmt != NULL) mysql_stmt_close(stmt);
}

bool database::google_home_load_token(supla_google_home *google_home) {
  bool result = false;
  char sql[] =
      "SELECT `access_token` FROM `supla_google_home` WHERE user_id = ? AND "
      "LENGTH(access_token) > 0";

  MYSQL_STMT *stmt = NULL;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  int UserID = google_home->getUserID();

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  if (stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    MYSQL_BIND rbind[1];
    memset(rbind, 0, sizeof(rbind));

    char buffer_token[ALEXA_TOKEN_MAXSIZE + 1];
    buffer_token[0] = 0;
    unsigned long token_size = 0;
    my_bool token_is_null = true;

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = buffer_token;
    rbind[0].buffer_length = ALEXA_TOKEN_MAXSIZE;
    rbind[0].length = &token_size;
    rbind[0].is_null = &token_is_null;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0 && !mysql_stmt_fetch(stmt)) {
        buffer_token[token_is_null ? 0 : token_size] = 0;

        google_home->set(buffer_token);
        result = true;
      }
    }
    mysql_stmt_close(stmt);
  }

  return result;
}

bool database::get_user_localtime(int UserID, TSDC_UserLocalTimeResult *time) {
  bool result = false;
  char sql[] =
      "SELECT @date := IFNULL(CONVERT_TZ(UTC_TIMESTAMP, 'UTC', timezone), "
      "UTC_TIMESTAMP) t, CASE CONVERT_TZ(UTC_TIMESTAMP, 'UTC', timezone) WHEN "
      "NULL THEN 'UTC' ELSE timezone END tz, YEAR(@date) y, MONTH(@date) m, "
      "DAY(@date) d, DAYOFWEEK(@date) w, HOUR(@date) h, MINUTE(@date) n, "
      "SECOND(@date) s FROM `supla_user` WHERE id = ?";

  MYSQL_STMT *stmt = NULL;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  if (stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    MYSQL_BIND rbind[9];
    memset(rbind, 0, sizeof(rbind));

    char date[30];
    unsigned long date_size = 0;
    my_bool date_is_null = true;

    unsigned long timezone_size = 0;
    my_bool timezone_is_null = true;

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = date;
    rbind[0].buffer_length = sizeof(date);
    rbind[0].length = &date_size;
    rbind[0].is_null = &date_is_null;

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = time->timezone;
    rbind[1].buffer_length = SUPLA_TIMEZONE_MAXSIZE;
    rbind[1].length = &timezone_size;
    rbind[1].is_null = &timezone_is_null;

    rbind[2].buffer_type = MYSQL_TYPE_SHORT;
    rbind[2].buffer = (char *)&time->year;

    rbind[3].buffer_type = MYSQL_TYPE_TINY;
    rbind[3].buffer = (char *)&time->month;

    rbind[4].buffer_type = MYSQL_TYPE_TINY;
    rbind[4].buffer = (char *)&time->day;

    rbind[5].buffer_type = MYSQL_TYPE_TINY;
    rbind[5].buffer = (char *)&time->dayOfWeek;

    rbind[6].buffer_type = MYSQL_TYPE_TINY;
    rbind[6].buffer = (char *)&time->hour;

    rbind[7].buffer_type = MYSQL_TYPE_TINY;
    rbind[7].buffer = (char *)&time->min;

    rbind[8].buffer_type = MYSQL_TYPE_TINY;
    rbind[8].buffer = (char *)&time->sec;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0 && !mysql_stmt_fetch(stmt) &&
          !timezone_is_null) {
        time->timezone[timezone_size] = 0;
        result = true;
      }
    }
    mysql_stmt_close(stmt);
  }

  return result;
}
