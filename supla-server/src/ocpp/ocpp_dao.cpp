// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ocpp/ocpp_dao.h"

#include <cstring>
#include <map>
#include <string>

#include "db/mariadb_access_provider.h"
#include "device/device_dao.h"
#include "log.h"

supla_ocpp_dao::supla_ocpp_dao(supla_mariadb_access_provider *dba) : dba(dba) {}

bool supla_ocpp_dao::get_devices(
    int user_id, int requested_device_id,
    std::vector<supla_ocpp_device_config> *devices) {
  if (!dba->is_connected() && !dba->connect()) return false;
  std::string sql =
      "SELECT c.iodevice_id,c.id,c.type,c.func,IFNULL(c.param1,0),"
      "IFNULL(c.param2,0),IFNULL(c.param3,0),IFNULL(c.param4,0),"
      "c.user_config,c.properties,v.value,ev.type,ev.value "
      "FROM supla_ocpp_charging_station s "
      "JOIN supla_iodevice d ON d.id=s.iodevice_id "
      "AND d.user_id=s.user_id AND d.enabled=1 "
      "JOIN supla_dev_channel c ON c.iodevice_id=s.iodevice_id "
      "AND c.user_id=s.user_id "
      "LEFT JOIN supla_dev_channel_value v ON v.channel_id=c.id "
      "AND v.user_id=c.user_id "
      "LEFT JOIN supla_dev_channel_extended_value ev ON ev.channel_id=c.id "
      "AND ev.user_id=c.user_id WHERE s.user_id=? AND c.is_virtual=1";
  if (requested_device_id) sql += " AND s.iodevice_id=?";
  MYSQL_BIND params[2] = {};
  params[0].buffer_type = MYSQL_TYPE_LONG;
  params[0].buffer = &user_id;
  params[1].buffer_type = MYSQL_TYPE_LONG;
  params[1].buffer = &requested_device_id;
  MYSQL_STMT *stmt = nullptr;
  if (!dba->stmt_execute(reinterpret_cast<void **>(&stmt), sql.c_str(), params,
                         requested_device_id ? 2 : 1, true)) {
    if (stmt) mysql_stmt_close(stmt);
    return false;
  }
  int device_id = 0;
  supla_ocpp_channel_config channel;
  supla_ocpp_channel_persisted_values persisted_values;
  char user_config[8193] = {}, properties[8193] = {};
  unsigned long config_size = 0, properties_size = 0, value_size = 0;
  unsigned long extended_size = 0;
  my_bool config_null = true, properties_null = true, value_null = true;
  my_bool extended_type_null = true, extended_null = true;
  std::array<char, SUPLA_CHANNELVALUE_SIZE> value = {};
  TSuplaChannelExtendedValue extended = {};
  MYSQL_BIND result[13] = {};
  int *fields[] = {&device_id,      &channel.id,     &channel.type,
                   &channel.func,   &channel.param1, &channel.param2,
                   &channel.param3, &channel.param4};
  for (int i = 0; i < 8; i++) {
    result[i].buffer_type = MYSQL_TYPE_LONG;
    result[i].buffer = fields[i];
  }
  result[8].buffer_type = MYSQL_TYPE_STRING;
  result[8].buffer = user_config;
  result[8].buffer_length = sizeof(user_config) - 1;
  result[8].length = &config_size;
  result[8].is_null = &config_null;
  result[9].buffer_type = MYSQL_TYPE_STRING;
  result[9].buffer = properties;
  result[9].buffer_length = sizeof(properties) - 1;
  result[9].length = &properties_size;
  result[9].is_null = &properties_null;
  result[10].buffer_type = MYSQL_TYPE_BLOB;
  result[10].buffer = value.data();
  result[10].buffer_length = value.size();
  result[10].length = &value_size;
  result[10].is_null = &value_null;
  result[11].buffer_type = MYSQL_TYPE_TINY;
  result[11].buffer = &extended.type;
  result[11].is_null = &extended_type_null;
  result[12].buffer_type = MYSQL_TYPE_BLOB;
  result[12].buffer = extended.value;
  result[12].buffer_length = sizeof(extended.value);
  result[12].length = &extended_size;
  result[12].is_null = &extended_null;
  if (mysql_stmt_bind_result(stmt, result) || mysql_stmt_store_result(stmt)) {
    mysql_stmt_close(stmt);
    return false;
  }

  std::map<int, supla_ocpp_device_config> loaded;
  bool success = true;
  int status = 0;
  while ((status = mysql_stmt_fetch(stmt)) == 0) {
    user_config[config_null ? 0 : config_size] = 0;
    properties[properties_null ? 0 : properties_size] = 0;
    channel.json.set_user_config(config_null ? nullptr : user_config);
    channel.json.set_properties(properties_null ? nullptr : properties);
    persisted_values.value.reset();
    persisted_values.extended_value.reset();
    if (!value_null && value_size == value.size())
      persisted_values.value = value;
    if (!extended_type_null && !extended_null && extended_size > 0) {
      extended.size = extended_size;
      persisted_values.extended_value =
          std::make_shared<TSuplaChannelExtendedValue>(extended);
    }
    auto &device = loaded[device_id];
    device.device_id = device_id;
    supla_ocpp_channel_config *target_config = nullptr;
    supla_ocpp_channel_persisted_values *target_values = nullptr;
    if (channel.type == SUPLA_CHANNELTYPE_ELECTRICITY_METER &&
        channel.func == SUPLA_CHANNELFNC_ELECTRICITY_METER) {
      target_config = &device.meter;
      target_values = &device.meter_persisted_values;
    }
    if (channel.type == SUPLA_CHANNELTYPE_RELAY &&
        channel.func == SUPLA_CHANNELFNC_POWERSWITCH) {
      target_config = &device.power_switch;
      target_values = &device.power_switch_persisted_values;
    }
    if (target_config) {
      if (target_config->id) {
        supla_log(LOG_WARNING, "Ambiguous OCPP channel mapping at device %i",
                  device_id);
        success = false;
        break;
      }
      *target_config = channel;
      *target_values = persisted_values;
    }
  }
  success = success && status == MYSQL_NO_DATA;
  mysql_stmt_close(stmt);
  if (!success) return false;
  devices->clear();
  for (const auto &item : loaded) {
    if (item.second.meter.id && item.second.power_switch.id) {
      devices->push_back(item.second);
    }
  }
  return true;
}

bool supla_ocpp_dao::save_value(int user_id,
                                const supla_ocpp_channel &channel) {
  char raw[SUPLA_CHANNELVALUE_SIZE] = {};
  if (!channel.get_raw_value(raw)) return true;
  int channel_id = channel.get_channel_id();
  unsigned int seconds = channel.get_value_validity_time_sec();
  MYSQL_BIND params[4] = {};
  params[0].buffer_type = MYSQL_TYPE_LONG;
  params[0].buffer = &channel_id;
  params[1].buffer_type = MYSQL_TYPE_LONG;
  params[1].buffer = &user_id;
  params[2].buffer_type = MYSQL_TYPE_BLOB;
  params[2].buffer = raw;
  params[2].buffer_length = sizeof(raw);
  params[3].buffer_type = MYSQL_TYPE_LONG;
  params[3].buffer = &seconds;
  params[3].is_unsigned = true;
  MYSQL_STMT *stmt = nullptr;
  bool result = dba->stmt_execute(reinterpret_cast<void **>(&stmt),
                                  "CALL supla_update_channel_value(?,?,?,?)",
                                  params, 4, true);
  if (stmt) mysql_stmt_close(stmt);
  return result;
}

bool supla_ocpp_dao::save_extended_value(int user_id,
                                         const supla_ocpp_channel &channel) {
  std::unique_ptr<supla_abstract_channel_extended_value> extended(
      channel.get_extended_value());
  if (!extended) return true;
  int channel_id = channel.get_channel_id();
  char type = extended->get_type();
  std::vector<char> buffer(extended->get_value_size());
  if (!buffer.empty()) extended->get_value(buffer.data());
  MYSQL_BIND params[4] = {};
  params[0].buffer_type = MYSQL_TYPE_LONG;
  params[0].buffer = &channel_id;
  params[1].buffer_type = MYSQL_TYPE_LONG;
  params[1].buffer = &user_id;
  params[2].buffer_type = MYSQL_TYPE_TINY;
  params[2].buffer = &type;
  params[3].buffer_type = buffer.empty() ? MYSQL_TYPE_NULL : MYSQL_TYPE_BLOB;
  params[3].buffer = buffer.data();
  params[3].buffer_length = buffer.size();
  MYSQL_STMT *stmt = nullptr;
  bool result = dba->stmt_execute(
      reinterpret_cast<void **>(&stmt),
      "CALL supla_update_channel_extended_value(?,?,?,?)", params, 4, true);
  if (stmt) mysql_stmt_close(stmt);
  return result;
}

bool supla_ocpp_dao::renew_validity(int user_id, int device_id,
                                    unsigned int seconds) {
  supla_device_dao dao(dba);
  return dao.touch_channel_values(device_id, user_id, seconds);
}
