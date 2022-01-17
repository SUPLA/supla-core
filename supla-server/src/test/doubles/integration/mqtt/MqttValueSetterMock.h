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

#ifndef MQTT_CHANNEL_VALUE_SETTER_MOCK_H_
#define MQTT_CHANNEL_VALUE_SETTER_MOCK_H_

#include "mqtt_abstract_value_setter.h"

class MqttValueSetterMock : public supla_mqtt_abstract_value_setter {
 protected:
  int on_counter;
  int color_counter;
  int brightness_counter;
  int color_brightness_counter;
  int off_counter;
  int toggle_counter;
  int shut_counter;
  int reveal_counter;
  int up_counter;
  int down_counter;
  int up_or_stop_counter;
  int down_or_stop_counter;
  int step_by_step_counter;
  int stop_counter;
  int open_counter;
  int close_counter;
  int open_close_counter;
  int refresh_all_existing_counter;

  unsigned int color;
  char brightness;
  char color_brightness;
  char closing_percentage;

 protected:
  virtual void set_on(bool on);
  virtual void set_color(unsigned int color);
  virtual void set_brightness(char brightness);
  virtual void set_color_brightness(char brightness);
  virtual void action_toggle(void);
  virtual void action_shut(const char *closingPercentage);
  virtual void action_reveal(void);
  virtual void action_up(void);
  virtual void action_down(void);
  virtual void action_up_or_stop(void);
  virtual void action_down_or_stop(void);
  virtual void action_step_by_step(void);
  void action_stop(void);
  void action_open(void);
  void action_close(void);
  void action_open_close(void);
  void refresh_all_existing(void);

 public:
  explicit MqttValueSetterMock(supla_mqtt_client_settings *settings);
  ~MqttValueSetterMock(void);
  void clear(void);
  int counterSetCount(void);

  int getOnCounter(void);
  int getOffCounter(void);
  int getColorCounter(void);
  int getBrightnessCounter(void);
  int getColorBrightnessCounter(void);
  int getToggleCounter(void);
  int getShutCounter(void);
  int getRevealCounter(void);
  int getUpCounter(void);
  int getDownCounter(void);
  int getUpOrStopCounter(void);
  int getDownOrStopCounter(void);
  int getStepByStepCounter(void);
  int getStopCounter(void);
  int getOpenCounter(void);
  int getCloseCounter(void);
  int getOpenCloseCounter(void);
  int getRefreshAllExistingCounter(void);
  char getClosingPercentage(void);
  unsigned int getColor(void);
  char getBrightness(void);
  char getColorBrightness(void);

  bool suidEqualTo(const char *suid);
  bool deviceEqualTo(int device_id);
  bool channelEqualTo(int channel_id);
};

#endif /* MQTT_CHANNEL_VALUE_SETTER_MOCK_H_ */
