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

#ifndef MQTT_CHANNEL_MESSAGE_PROVIDER_H_
#define MQTT_CHANNEL_MESSAGE_PROVIDER_H_

#include "json/cJSON.h"
#include "mqtt_db.h"
#include "mqtt_message_provider.h"

class supla_mqtt_channel_message_provider : public supla_mqtt_message_provider {
 private:
  _mqtt_db_data_row_channel_t *row;

 protected:
  void ha_json_set_name(cJSON *root, const char *name_if_empty,
                        const char *name_second_segment);
  void ha_json_set_uniq_id(cJSON *root, int sub_id = 0,
                           bool set_sub_id = false);
  void ha_json_set_qos(cJSON *root, int qos = 0);
  void ha_json_set_retain(cJSON *root, bool retain = false);
  void ha_json_set_optimistic(cJSON *root, bool optimistic = false);
  void ha_json_set_topic_base(cJSON *root, const char *topic_prefix);
  void ha_json_set_full_topic(cJSON *root, const char *param_name,
                              const char *topic_prefix,
                              const char *topic_suffix);
  void ha_json_set_short_topic(cJSON *root, const char *param_name,
                               const char *topic_suffix);
  void ha_json_set_string_param(cJSON *root, const char *param_name,
                                const char *value);
  void ha_json_set_int_param(cJSON *root, const char *param_name, int value);
  void ha_json_set_availability(cJSON *root, const char *topic_prefix,
                                const char *avil, const char *notavil);
  void ha_json_set_device_info(cJSON *root);
  void ha_json_set_brightness(cJSON *root, const char *bri_cmd_t,
                              const char *bti_stat_t);

  bool ha_get_message(cJSON *root, const char *component, int sub_id,
                      bool set_sub_id, char **topic_name, void **message,
                      size_t *message_size);

  cJSON *ha_json_create_root(const char *topic_prefix,
                             const char *name_if_empty = NULL,
                             const char *name_second_segment = NULL,
                             bool avil_topic = true, int sub_id = 0,
                             bool set_sub_id = false);
  bool ha_light_or_powerswitch(bool light, const char *topic_prefix,
                               char **topic_name, void **message,
                               size_t *message_size);
  bool ha_dimmer(int sub_id, bool set_sub_id, const char *topic_prefix,
                 char **topic_name, void **message, size_t *message_size);
  bool ha_rgb(int sub_id, bool set_sub_id, const char *topic_prefix,
              char **topic_name, void **message, size_t *message_size);
  bool ha_binary_sensor(const char *device_class, const char *topic_prefix,
                        char **topic_name, void **message,
                        size_t *message_size);

  bool ha_sensor(const char *unit, int precision, int sub_id, bool set_sub_id,
                 const char *state_topic, const char *name_if_empty,
                 const char *topic_prefix, char **topic_name, void **message,
                 size_t *message_size);
  bool ha_sensor_temperature(int sub_id, bool set_sub_id,
                             const char *topic_prefix, char **topic_name,
                             void **message, size_t *message_size);
  bool ha_sensor_humidity(int sub_id, bool set_sub_id, const char *topic_prefix,
                          char **topic_name, void **message,
                          size_t *message_size);
  bool ha_roller_shutter(const char *topic_prefix, char **topic_name,
                         void **message, size_t *message_size);
  bool ha_impulse_counter(unsigned short index, const char *topic_prefix,
                          char **topic_name, void **message,
                          size_t *message_size);
  bool ha_electricity_meter(unsigned short index, const char *topic_prefix,
                            char **topic_name, void **message,
                            size_t *message_size);
  bool get_home_assistant_cfgitem(unsigned short index,
                                  const char *topic_prefix, char **topic_name,
                                  void **message, size_t *message_size);

 public:
  supla_mqtt_channel_message_provider(void);
  virtual ~supla_mqtt_channel_message_provider(void);
  void get_not_empty_caption(int func, const char *caption_in,
                             const char *caption_if_empty, char *caption_out);
  void channel_type_to_string(int type, char *buf, size_t buf_size);
  void channel_function_to_string(int func, char *buf, size_t buf_size);
  virtual bool get_message_at_index(unsigned short index,
                                    const char *topic_prefix, char **topic_name,
                                    void **message, size_t *message_size);
  void set_data_row(_mqtt_db_data_row_channel_t *row);
};

#endif /*MQTT_CHANNEL_MESSAGE_PROVIDER_H_*/
