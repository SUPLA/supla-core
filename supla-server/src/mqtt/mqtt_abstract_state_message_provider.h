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

#ifndef MQTT_ABSTRACT_STATE_MESSAGE_PROVIDER_H_
#define MQTT_ABSTRACT_STATE_MESSAGE_PROVIDER_H_

#include "devicechannel.h"
#include "mqtt_db.h"
#include "mqtt_message_provider.h"

class supla_mqtt_abstract_state_message_provider
    : public supla_mqtt_message_provider {
 private:
  char *user_suid;
  int user_id;
  int device_id;
  int channel_id;

  int channel_function;
  int channel_type;
  int channel_flags;

  channel_complex_value *cvalue;
  supla_channel_electricity_measurement *em;
  supla_channel_ic_measurement *icm;

 protected:
  int get_user_id(void);
  int get_device_id(void);
  int get_channel_id(void);

  virtual channel_complex_value *_get_complex_value(int user_id, int device_id,
                                                    int channel_id) = 0;
  virtual supla_channel_electricity_measurement *_get_electricity_measurement(
      void) = 0;
  virtual supla_channel_ic_measurement *_get_impulse_counter_measurement(
      void) = 0;
  virtual const char *_get_user_suid(void) = 0;

  void get_complex_value(void);
  void get_electricity_measurement(void);
  void get_impulse_counter_measurement(void);

  void get_temperature(char *buf, size_t buf_size);
  void get_humidity(char *buf, size_t buf_size);
  void get_brightness(char *brightness, char *buf, size_t buf_size);
  void get_color(char *buf, size_t buf_size);
  void get_rgb(char *buf, size_t buf_size);
  void get_valve_closed(char *buf, size_t buf_size);

  bool get_rs_message_at_index(unsigned short index, const char *topic_prefix,
                               char **topic_name, void **message,
                               size_t *message_size);

  bool get_lck_message_at_index(unsigned short index, const char *topic_prefix,
                                char **topic_name, void **message,
                                size_t *message_size);

  bool get_gate_message_at_index(unsigned short index, const char *topic_prefix,
                                 char **topic_name, void **message,
                                 size_t *message_size);

  bool get_onoff_message_at_index(bool on, unsigned short index,
                                  const char *topic_prefix, char **topic_name,
                                  void **message, size_t *message_size,
                                  bool *overcurrent_relay_off);

  bool get_onoff_message_at_index(bool on, unsigned short index,
                                  const char *topic_prefix, char **topic_name,
                                  void **message, size_t *message_size);

  bool get_depth_message_at_index(unsigned short index,
                                  const char *topic_prefix, char **topic_name,
                                  void **message, size_t *message_size);

  bool get_distance_message_at_index(unsigned short index,
                                     const char *topic_prefix,
                                     char **topic_name, void **message,
                                     size_t *message_size);

  bool get_doublevalue_message_at_index(unsigned short index,
                                        const char *topic_prefix,
                                        char **topic_name, void **message,
                                        size_t *message_size);

  bool get_sensor_message_at_index(unsigned short index,
                                   const char *topic_prefix, char **topic_name,
                                   void **message, size_t *message_size);

  bool get_temperature_message_at_index(unsigned short index,
                                        unsigned short expected_index,
                                        const char *topic_prefix,
                                        char **topic_name, void **message,
                                        size_t *message_size);

  bool get_humidity_message_at_index(unsigned short index,
                                     unsigned short expected_index,
                                     const char *topic_prefix,
                                     char **topic_name, void **message,
                                     size_t *message_size);

  bool get_brightness_message(const char *topic_prefix, char **topic_name,
                              void **message, size_t *message_size);

  bool get_color_brightness_message(const char *topic_prefix, char **topic_name,
                                    void **message, size_t *message_size);

  bool get_color_message(const char *topic_prefix, char **topic_name,
                         void **message, size_t *message_size);

  bool get_normalized_rgb_message(const char *topic_prefix, char **topic_name,
                                  void **message, size_t *message_size);

  bool get_valve_message_at_index(unsigned short index,
                                  const char *topic_prefix, char **topic_name,
                                  void **message, size_t *message_size);

  bool get_impulsecounter_message_at_index(unsigned short index,
                                           const char *topic_prefix,
                                           char **topic_name, void **message,
                                           size_t *message_size);

  void verify_flag(supla_channel_electricity_measurement **em, int flags,
                   int flag1, int flag2, void **message, size_t *message_size);

  bool get_electricitymeter_message_at_index(unsigned short index,
                                             const char *topic_prefix,
                                             char **topic_name, void **message,
                                             size_t *message_size);

 public:
  supla_mqtt_abstract_state_message_provider(void);
  virtual ~supla_mqtt_abstract_state_message_provider(void);
  virtual bool get_message_at_index(unsigned short index,
                                    const char *topic_prefix, char **topic_name,
                                    void **message, size_t *message_size);
  void set_ids(int user_id, int device_id, int channel_id);
  void set_channel_type_function_and_flags(int channel_type,
                                           int channel_function,
                                           int channel_flags);
  void set_user_suid(char *user_suid);
  void set_user_suid(void);
};

#endif /*MQTT_ABSTRACT_STATE_MESSAGE_PROVIDER_H_*/
