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

#ifndef MQTT_ABSTRACT_CHANNEl_VALUE_SETTER_H_
#define MQTT_ABSTRACT_CHANNEl_VALUE_SETTER_H_

#include "mqtt_client_settings.h"
#include "stddef.h"

class supla_mqtt_abstract_channel_value_setter {
 private:
  char *topic_name;
  size_t topic_name_size;
  char *message;
  size_t message_size;
  int device_id;
  int channel_id;
  char *suid_ptr;
  size_t suid_len;
  char *suid;

  supla_mqtt_client_settings *settings;
  bool lc_equal(const char *str);

  bool parse_on(void);
  bool parse_perecntage(void);
  bool parse_action(void);
  bool parse_brightness(void);
  bool parse_color(void);
  int str2int(const char *str, size_t len, bool *err);
  unsigned int hex2int(const char *str, size_t len, bool *err);
  void suid_free(void);
  int parse_int_with_prefix(const char *prefix, size_t prefix_len,
                            char **topic_name, size_t *topic_name_size,
                            bool *err);

 protected:
  const char *get_suid(void);
  int get_device_id(void);
  int get_channel_id(void);

  virtual void set_on(bool on) = 0;
  virtual void set_color(unsigned int color) = 0;
  virtual void set_brightness(char brightness) = 0;
  virtual void set_color_brightness(char brightness) = 0;
  virtual void action_toggle(void) = 0;
  virtual void action_shut(const char *closingPercentage) = 0;
  virtual void action_reveal(void) = 0;
  virtual void action_stop(void) = 0;
  virtual void action_open(void) = 0;
  virtual void action_close(void) = 0;
  virtual void action_open_close(void) = 0;

 public:
  supla_mqtt_abstract_channel_value_setter(
      supla_mqtt_client_settings *settings);
  virtual ~supla_mqtt_abstract_channel_value_setter(void);

  void set_value(char *topic_name, size_t topic_name_size, char *message,
                 size_t message_size);
};

#endif /*MQTT_ABSTRACT_CHANNEl_VALUE_SETTER_H_*/
