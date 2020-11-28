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

#include <mqtt_db.h>
#include <mysql.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

typedef struct {
  MYSQL_STMT *stmt;
  _mqtt_db_data_row_user_t *row;

  // Auxiliary variables
  unsigned long user_email_len;
  my_bool user_email_is_null;
  unsigned long user_timezone_len;
  my_bool user_timezone_is_null;
  unsigned long user_shortuniqueid_len;
  my_bool user_shortuniqueid_is_null;
} _mqtt_db_userquery_t;

typedef struct {
  MYSQL_STMT *stmt;
  _mqtt_db_data_row_device_t *row;

  // Auxiliary variables
  unsigned long user_email_len;
  my_bool user_email_is_null;
  int device_enabled;
  unsigned long device_location_len;
  my_bool device_location_is_null;
  unsigned long device_last_connected_len;
  my_bool device_last_connected_is_null;
  unsigned long device_last_ipv4_len;
  my_bool device_last_ipv4_is_null;
  unsigned long device_name_len;
  my_bool device_name_is_null;
  unsigned long device_softver_len;
  my_bool device_softver_is_null;
} _mqtt_db_devicequery_t;

typedef struct {
  MYSQL_STMT *stmt;
  _mqtt_db_data_row_channel_t *row;

  // Auxiliary variables
  unsigned long user_email_len;
  my_bool user_email_is_null;
  unsigned long user_shortuniqueid_len;
  my_bool user_shortuniqueid_is_null;
  int device_enabled;
  unsigned long channel_location_len;
  my_bool channel_location_is_null;
  unsigned long channel_caption_len;
  my_bool channel_caption_is_null;
  int channel_hidden;
} _mqtt_db_channelquery_t;

supla_mqtt_db::supla_mqtt_db(void) : svrdb() {
  userquery = NULL;
  devicequery = NULL;
  channelquery = NULL;
}

supla_mqtt_db::~supla_mqtt_db(void) {}

template <typename T>
void supla_mqtt_db::close_query(void *_query) {
  T *query = static_cast<T *>(_query);
  if (query) {
    if (query->stmt != NULL) {
      mysql_stmt_close(query->stmt);
    }
    free(query);
  }
}

void *supla_mqtt_db::open_userquery(int UserID, _mqtt_db_data_row_user_t *row) {
  _mqtt_db_userquery_t *query =
      (_mqtt_db_userquery_t *)malloc(sizeof(_mqtt_db_userquery_t));

  if (query == NULL) {
    return NULL;
  }

  memset(query, 0, sizeof(_mqtt_db_userquery_t));
  query->row = row;

  const char sql[] =
      "SELECT u.`id`, u.`email`, u.`timezone`, u.`short_unique_id` FROM "
      "`supla_user` u WHERE u.`mqtt_broker_enabled` = true AND (? = 0 OR "
      "u.`id` = ?)";

  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&UserID;

  if (stmt_execute((void **)&query->stmt, sql, pbind, 2, true)) {
    MYSQL_BIND rbind[4];
    memset(rbind, 0, sizeof(rbind));

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&query->row->user_id;
    rbind[0].buffer_length = sizeof(query->row->user_id);

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = query->row->user_email;
    rbind[1].buffer_length = sizeof(query->row->user_email);
    rbind[1].length = &query->user_email_len;
    rbind[1].is_null = &query->user_email_is_null;

    rbind[2].buffer_type = MYSQL_TYPE_STRING;
    rbind[2].buffer = query->row->user_timezone;
    rbind[2].buffer_length = sizeof(query->row->user_timezone);
    rbind[2].length = &query->user_timezone_len;
    rbind[2].is_null = &query->user_timezone_is_null;

    rbind[3].buffer_type = MYSQL_TYPE_STRING;
    rbind[3].buffer = query->row->user_shortuniqueid;
    rbind[3].buffer_length = sizeof(query->row->user_shortuniqueid);
    rbind[3].length = &query->user_shortuniqueid_len;
    rbind[3].is_null = &query->user_shortuniqueid_is_null;

    if (mysql_stmt_bind_result(query->stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(query->stmt));
    } else {
      mysql_stmt_store_result(query->stmt);
      if (mysql_stmt_num_rows(query->stmt) > 0) {
        return query;
      }
    }
  }

  close_userquery(query);
  return NULL;
}

bool supla_mqtt_db::userquery_fetch_row(void *_query) {
  _mqtt_db_userquery_t *query = static_cast<_mqtt_db_userquery_t *>(_query);
  if (query && !mysql_stmt_fetch(query->stmt)) {
    set_terminating_byte(query->row->user_email, sizeof(query->row->user_email),
                         query->user_email_len, query->user_email_is_null);
    set_terminating_byte(
        query->row->user_timezone, sizeof(query->row->user_timezone),
        query->user_timezone_len, query->user_timezone_is_null);
    set_terminating_byte(
        query->row->user_shortuniqueid, sizeof(query->row->user_shortuniqueid),
        query->user_shortuniqueid_len, query->user_shortuniqueid_is_null);
    return true;
  }

  return false;
}

void supla_mqtt_db::close_userquery(void *query) {
  close_query<_mqtt_db_userquery_t>(query);
}

void *supla_mqtt_db::open_devicequery(int UserID, int DeviceID,
                                      _mqtt_db_data_row_device_t *row) {
  _mqtt_db_devicequery_t *query =
      (_mqtt_db_devicequery_t *)malloc(sizeof(_mqtt_db_devicequery_t));

  if (query == NULL) {
    return NULL;
  }

  memset(query, 0, sizeof(_mqtt_db_devicequery_t));
  query->row = row;

  const char sql[] =
      "SELECT u.`email`, d.`id`, d.`enabled`, l.`caption`, "
      "DATE_FORMAT(d.`last_connected`, '%Y-%m-%dT%TZ'), "
      "INET_NTOA(d.`last_ipv4`), d.`manufacturer_id`, d.`name`, "
      "d.`protocol_version`, d.`software_version` FROM `supla_iodevice` d LEFT "
      "JOIN `supla_user` u ON u.id = d.`user_id` LEFT JOIN `supla_location` l "
      "ON l.id = d.`location_id` WHERE u.`mqtt_broker_enabled` = true AND (? = "
      "0 OR u.`id` = ?) AND (? = 0 OR d.`id` = ?)";

  MYSQL_BIND pbind[4];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&UserID;

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&DeviceID;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&DeviceID;

  if (stmt_execute((void **)&query->stmt, sql, pbind, 4, true)) {
    MYSQL_BIND rbind[10];
    memset(rbind, 0, sizeof(rbind));

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = query->row->user_email;
    rbind[0].buffer_length = sizeof(query->row->user_email);
    rbind[0].length = &query->user_email_len;
    rbind[0].is_null = &query->user_email_is_null;

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&query->row->device_id;
    rbind[1].buffer_length = sizeof(query->row->device_id);

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&query->device_enabled;
    rbind[2].buffer_length = sizeof(int);

    rbind[3].buffer_type = MYSQL_TYPE_STRING;
    rbind[3].buffer = query->row->device_location;
    rbind[3].buffer_length = sizeof(query->row->device_location);
    rbind[3].length = &query->device_location_len;
    rbind[3].is_null = &query->device_location_is_null;

    rbind[4].buffer_type = MYSQL_TYPE_STRING;
    rbind[4].buffer = query->row->device_last_connected;
    rbind[4].buffer_length = sizeof(query->row->device_last_connected);
    rbind[4].length = &query->device_last_connected_len;
    rbind[4].is_null = &query->device_last_connected_is_null;

    rbind[5].buffer_type = MYSQL_TYPE_STRING;
    rbind[5].buffer = query->row->device_last_ipv4;
    rbind[5].buffer_length = sizeof(query->row->device_last_ipv4);
    rbind[5].length = &query->device_last_ipv4_len;
    rbind[5].is_null = &query->device_last_ipv4_is_null;

    rbind[6].buffer_type = MYSQL_TYPE_LONG;
    rbind[6].buffer = (char *)&query->row->device_mfr_id;
    rbind[6].buffer_length = sizeof(query->row->device_mfr_id);

    rbind[7].buffer_type = MYSQL_TYPE_STRING;
    rbind[7].buffer = query->row->device_name;
    rbind[7].buffer_length = sizeof(query->row->device_name);
    rbind[7].length = &query->device_name_len;
    rbind[7].is_null = &query->device_name_is_null;

    rbind[8].buffer_type = MYSQL_TYPE_LONG;
    rbind[8].buffer = (char *)&query->row->device_proto_version;
    rbind[8].buffer_length = sizeof(query->row->device_proto_version);

    rbind[9].buffer_type = MYSQL_TYPE_STRING;
    rbind[9].buffer = query->row->device_softver;
    rbind[9].buffer_length = sizeof(query->row->device_softver);
    rbind[9].length = &query->device_softver_len;
    rbind[9].is_null = &query->device_softver_is_null;

    if (mysql_stmt_bind_result(query->stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(query->stmt));
    } else {
      mysql_stmt_store_result(query->stmt);

      if (mysql_stmt_num_rows(query->stmt) > 0) {
        return query;
      }
    }
  }

  close_devicequery(query);
  return NULL;
}

bool supla_mqtt_db::devicequery_fetch_row(void *_query) {
  _mqtt_db_devicequery_t *query = static_cast<_mqtt_db_devicequery_t *>(_query);
  if (query && !mysql_stmt_fetch(query->stmt)) {
    set_terminating_byte(query->row->user_email, sizeof(query->row->user_email),
                         query->user_email_len, query->user_email_is_null);
    set_terminating_byte(
        query->row->device_location, sizeof(query->row->device_location),
        query->device_location_len, query->device_location_is_null);
    set_terminating_byte(query->row->device_last_connected,
                         sizeof(query->row->device_last_connected),
                         query->device_last_connected_len,
                         query->device_last_connected_is_null);
    set_terminating_byte(
        query->row->device_last_ipv4, sizeof(query->row->device_last_ipv4),
        query->device_last_ipv4_len, query->device_last_ipv4_is_null);
    set_terminating_byte(query->row->device_name,
                         sizeof(query->row->device_name),
                         query->device_name_len, query->device_name_is_null);
    set_terminating_byte(
        query->row->device_softver, sizeof(query->row->device_softver),
        query->device_softver_len, query->device_softver_is_null);

    query->row->device_enabled = query->device_enabled > 0;
    return true;
  }

  return false;
}

void supla_mqtt_db::close_devicequery(void *query) {
  close_query<_mqtt_db_devicequery_t>(query);
}

void *supla_mqtt_db::open_channelquery(int UserID, int DeviceID, int ChannelID,
                                       _mqtt_db_data_row_channel_t *row) {
  _mqtt_db_channelquery_t *query =
      (_mqtt_db_channelquery_t *)malloc(sizeof(_mqtt_db_channelquery_t));

  if (query == NULL) {
    return NULL;
  }

  memset(query, 0, sizeof(_mqtt_db_channelquery_t));
  query->row = row;

  const char sql[] =
      "SELECT u.`id`, u.`email`, u.`short_unique_id`, d.`id`, d.`enabled`, "
      "c.`id`, "
      "c.`type`, c.`func`, IFNULL(l.`caption`, dl.`caption`), c.`caption`, "
      "c.`hidden` FROM `supla_dev_channel` c LEFT JOIN `supla_iodevice` d ON "
      "d.`id` = c.`iodevice_id` LEFT JOIN `supla_location` l ON l.`id` = "
      "c.`location_id` LEFT JOIN `supla_location` dl ON dl.`id` = "
      "d.`location_id` LEFT JOIN `supla_user` u ON u.id = c.`user_id` WHERE "
      "u.`mqtt_broker_enabled` = true AND (? = 0 OR u.`id` = ?) AND (? = 0 OR "
      "d.`id` = ?) AND (? = 0 OR c.`id` = ?)";

  MYSQL_BIND pbind[6];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&UserID;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&UserID;

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&DeviceID;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&DeviceID;

  pbind[4].buffer_type = MYSQL_TYPE_LONG;
  pbind[4].buffer = (char *)&ChannelID;

  pbind[5].buffer_type = MYSQL_TYPE_LONG;
  pbind[5].buffer = (char *)&ChannelID;

  if (stmt_execute((void **)&query->stmt, sql, pbind, 6, true)) {
    MYSQL_BIND rbind[11];
    memset(rbind, 0, sizeof(rbind));

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&query->row->user_id;
    rbind[0].buffer_length = sizeof(query->row->user_id);

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = query->row->user_email;
    rbind[1].buffer_length = sizeof(query->row->user_email);
    rbind[1].length = &query->user_email_len;
    rbind[1].is_null = &query->user_email_is_null;

    rbind[2].buffer_type = MYSQL_TYPE_STRING;
    rbind[2].buffer = query->row->user_shortuniqueid;
    rbind[2].buffer_length = sizeof(query->row->user_shortuniqueid);
    rbind[2].length = &query->user_shortuniqueid_len;
    rbind[2].is_null = &query->user_shortuniqueid_is_null;

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&query->row->device_id;
    rbind[3].buffer_length = sizeof(query->row->device_id);

    rbind[4].buffer_type = MYSQL_TYPE_LONG;
    rbind[4].buffer = (char *)&query->device_enabled;
    rbind[4].buffer_length = sizeof(int);

    rbind[5].buffer_type = MYSQL_TYPE_LONG;
    rbind[5].buffer = (char *)&query->row->channel_id;
    rbind[5].buffer_length = sizeof(query->row->channel_id);

    rbind[6].buffer_type = MYSQL_TYPE_LONG;
    rbind[6].buffer = (char *)&query->row->channel_type;
    rbind[6].buffer_length = sizeof(query->row->channel_type);

    rbind[7].buffer_type = MYSQL_TYPE_LONG;
    rbind[7].buffer = (char *)&query->row->channel_func;
    rbind[7].buffer_length = sizeof(query->row->channel_func);

    rbind[8].buffer_type = MYSQL_TYPE_STRING;
    rbind[8].buffer = query->row->channel_location;
    rbind[8].buffer_length = sizeof(query->row->channel_location);
    rbind[8].length = &query->channel_location_len;
    rbind[8].is_null = &query->channel_location_is_null;

    rbind[9].buffer_type = MYSQL_TYPE_STRING;
    rbind[9].buffer = query->row->channel_caption;
    rbind[9].buffer_length = sizeof(query->row->channel_caption);
    rbind[9].length = &query->channel_caption_len;
    rbind[9].is_null = &query->channel_caption_is_null;

    rbind[10].buffer_type = MYSQL_TYPE_LONG;
    rbind[10].buffer = (char *)&query->channel_hidden;
    rbind[10].buffer_length = sizeof(int);

    if (mysql_stmt_bind_result(query->stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(query->stmt));
    } else {
      mysql_stmt_store_result(query->stmt);

      if (mysql_stmt_num_rows(query->stmt) > 0) {
        return query;
      }
    }
  }

  close_channelquery(query);
  return NULL;
}

bool supla_mqtt_db::channelquery_fetch_row(void *_query) {
  _mqtt_db_channelquery_t *query =
      static_cast<_mqtt_db_channelquery_t *>(_query);
  if (query && !mysql_stmt_fetch(query->stmt)) {
    set_terminating_byte(query->row->user_email, sizeof(query->row->user_email),
                         query->user_email_len, query->user_email_is_null);
    set_terminating_byte(
        query->row->user_shortuniqueid, sizeof(query->row->user_shortuniqueid),
        query->user_shortuniqueid_len, query->user_shortuniqueid_is_null);
    set_terminating_byte(
        query->row->channel_caption, sizeof(query->row->channel_location),
        query->channel_location_len, query->channel_location_is_null);
    set_terminating_byte(
        query->row->channel_caption, sizeof(query->row->channel_caption),
        query->channel_caption_len, query->channel_caption_is_null);

    query->row->device_enabled = query->device_enabled > 0;
    query->row->channel_hidden = query->channel_hidden > 0;
    return true;
  }

  return false;
}

void supla_mqtt_db::close_channelquery(void *query) {
  close_query<_mqtt_db_devicequery_t>(query);
}
