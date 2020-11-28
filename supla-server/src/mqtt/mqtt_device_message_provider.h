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

#ifndef MQTT_DEVICE_MESSAGE_PROVIDER_H_
#define MQTT_DEVICE_MESSAGE_PROVIDER_H_

#include "mqtt_db.h"
#include "mqtt_message_provider.h"

class supla_mqtt_device_message_provider : public supla_mqtt_message_provider {
 private:
  _mqtt_db_data_row_device_t *row;

 protected:
  void get_mfr_name(int mfr_id, char *buf, size_t buf_size);

 public:
  supla_mqtt_device_message_provider(void);
  virtual ~supla_mqtt_device_message_provider(void);
  bool get_message_at_index(unsigned short index, const char *topic_prefix,
                            char **topic_name, void **message,
                            size_t *message_size);
  void set_data_row(_mqtt_db_data_row_device_t *row);
};

#endif /*MQTT_USER_MESSAGE_PROVIDER_H_*/
