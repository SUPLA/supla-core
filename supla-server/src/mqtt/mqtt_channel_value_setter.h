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

#ifndef MQTT_CHANNEl_VALUE_SETTER_H_
#define MQTT_CHANNEl_VALUE_SETTER_H_

#include "mqtt_abstract_channel_value_setter.h"

class supla_mqtt_channel_value_setter
    : public supla_mqtt_abstract_channel_value_setter {
 protected:
  void set_on(bool on);
  void set_color(unsigned int color);
  void set_brightness(char brightness);
  void set_color_brightness(char brightness);
  void action_toggle(void);
  void action_shut(const char *closingPercentage);
  void action_reveal(void);
  void action_stop(void);
  void action_open_close(void);

 public:
  explicit supla_mqtt_channel_value_setter(
      supla_mqtt_client_settings *settings);
  virtual ~supla_mqtt_channel_value_setter(void);
};

#endif /*MQTT_CHANNEl_VALUE_SETTER_H_*/
