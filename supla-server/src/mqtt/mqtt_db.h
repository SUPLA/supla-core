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

#ifndef MQTT_DB_H_
#define MQTT_DB_H_

#define IPV4_STRING_MAXSIZE 16
#define DATE_STRING_MAXSIZE 21

#include <svrdb.h>
#include "proto.h"
#include "user.h"

typedef struct {
  int user_id;
  char user_email[SUPLA_EMAIL_MAXSIZE];
  char user_timezone[SUPLA_TIMEZONE_MAXSIZE];
  char user_shortuniqueid[SHORT_UNIQUEID_MAXSIZE];
} _mqtt_db_data_row_user_t;

typedef struct {
  char user_email[SUPLA_EMAIL_MAXSIZE];
  int device_id;
  bool device_enabled;
  char device_location[SUPLA_LOCATION_CAPTION_MAXSIZE];
  char device_last_connected[DATE_STRING_MAXSIZE];
  char device_last_ipv4[IPV4_STRING_MAXSIZE];
  int device_mfr_id;
  char device_name[SUPLA_DEVICE_NAME_MAXSIZE];
  int device_proto_version;
  char device_softver[SUPLA_SOFTVER_MAXSIZE];
} _mqtt_db_data_row_device_t;

typedef struct {
  int user_id;
  char user_email[SUPLA_EMAIL_MAXSIZE];
  char user_shortuniqueid[SHORT_UNIQUEID_MAXSIZE];
  int device_id;
  bool device_enabled;
  int channel_id;
  int channel_type;
  int channel_func;
  char channel_location[SUPLA_LOCATION_CAPTION_MAXSIZE];
  char channel_caption[SUPLA_CHANNEL_CAPTION_MAXSIZE];
  bool channel_hidden;
} _mqtt_db_data_row_channel_t;

class supla_mqtt_db : public svrdb {
 private:
  void *userquery;
  void *devicequery;
  void *channelquery;

  template <typename T>
  void close_query(void *_query);

 public:
  supla_mqtt_db(void);
  virtual ~supla_mqtt_db(void);

  bool mqtt_enabled(int UserID);

  void *open_mqttenabledquery(void);
  int mqttenabledquery_fetch_row(void *query);
  void close_mqttenabledquery(void *query);

  void *open_userquery(int UserID, bool OnlyEnabled,
                       _mqtt_db_data_row_user_t *row);
  bool userquery_fetch_row(void *query);
  void close_userquery(void *query);

  void *open_devicequery(int UserID, int DeviceID,
                         _mqtt_db_data_row_device_t *row);
  bool devicequery_fetch_row(void *query);
  void close_devicequery(void *query);

  void *open_channelquery(int UserID, int DeviceID, int ChannelID,
                          _mqtt_db_data_row_channel_t *row);
  bool channelquery_fetch_row(void *query);
  void close_channelquery(void *query);
};

#endif /*MQTT_CLIENT_DBDATASOURCE_H_*/
