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

#include "MqttChannelValueSetterMock.h"
#include <stdio.h>
#include <string.h>
#include "proto.h"

MqttChannelValueSetterMock::MqttChannelValueSetterMock(
    supla_mqtt_client_settings *settings)
    : supla_mqtt_abstract_channel_value_setter(settings) {
  clear();
}

MqttChannelValueSetterMock::~MqttChannelValueSetterMock(void) {}

void MqttChannelValueSetterMock::clear(void) {
  this->on_counter = 0;
  this->color_counter = 0;
  this->brightness_counter = 0;
  this->color_brightness_counter = 0;
  this->off_counter = 0;
  this->toggle_counter = 0;
  this->shut_counter = 0;
  this->reveal_counter = 0;
  this->stop_counter = 0;
  this->open_close_counter = 0;
  this->color = 0x01FFFFFF;
  this->brightness = -1;
  this->color_brightness = -1;
  this->closing_percentage = -1;
}

void MqttChannelValueSetterMock::set_on(bool on) {
  if (on) {
    on_counter++;
  } else {
    off_counter++;
  }
}

void MqttChannelValueSetterMock::set_color(unsigned int color) {
  this->color = color;
  color_counter++;
}

void MqttChannelValueSetterMock::set_brightness(char brightness) {
  this->brightness = brightness;
  brightness_counter++;
}

void MqttChannelValueSetterMock::set_color_brightness(char brightness) {
  this->color_brightness = brightness;
  color_brightness_counter++;
}

void MqttChannelValueSetterMock::action_toggle(void) { toggle_counter++; }

void MqttChannelValueSetterMock::action_shut(const char *closingPercentage) {
  shut_counter++;
  if (closingPercentage) {
    closing_percentage = *closingPercentage;
  }
}

void MqttChannelValueSetterMock::action_reveal(void) { reveal_counter++; }

void MqttChannelValueSetterMock::action_stop(void) { stop_counter++; }

void MqttChannelValueSetterMock::action_open_close(void) {
  open_close_counter++;
}

int MqttChannelValueSetterMock::getOnCounter(void) { return on_counter; }

int MqttChannelValueSetterMock::getOffCounter(void) { return off_counter; }

int MqttChannelValueSetterMock::getColorCounter(void) { return color_counter; }

int MqttChannelValueSetterMock::getBrightnessCounter(void) {
  return brightness_counter;
}

int MqttChannelValueSetterMock::getColorBrightnessCounter(void) {
  return color_brightness_counter;
}

int MqttChannelValueSetterMock::getToggleCounter(void) {
  return toggle_counter;
}

int MqttChannelValueSetterMock::getShutCounter(void) { return shut_counter; }

int MqttChannelValueSetterMock::getRevealCounter(void) {
  return reveal_counter;
}

int MqttChannelValueSetterMock::getStopCounter(void) { return stop_counter; }

int MqttChannelValueSetterMock::getOpenCloseCounter(void) {
  return open_close_counter;
}

char MqttChannelValueSetterMock::getClosingPercentage(void) {
  return closing_percentage;
}

unsigned int MqttChannelValueSetterMock::getColor(void) { return color; }

char MqttChannelValueSetterMock::getBrightness(void) { return brightness; }

char MqttChannelValueSetterMock::getColorBrightness(void) {
  return color_brightness;
}

bool MqttChannelValueSetterMock::emailEqualTo(const char *email) {
  if (email == NULL && get_email() == NULL) {
    return true;
  }

  if (email != NULL && get_email() != NULL) {
    return strncmp(email, get_email(), SUPLA_EMAIL_MAXSIZE) == 0;
  }

  return false;
}

bool MqttChannelValueSetterMock::channelEqualTo(int channel_id) {
  return channel_id == get_channel_id();
}

int MqttChannelValueSetterMock::counterSetCount(void) {
  int result = 0;
  if (on_counter > 0) {
    result++;
  }

  if (off_counter > 0) {
    result++;
  }

  if (color_counter > 0) {
    result++;
  }

  if (brightness_counter > 0) {
    result++;
  }

  if (color_brightness_counter > 0) {
    result++;
  }

  if (toggle_counter > 0) {
    result++;
  }

  if (shut_counter > 0) {
    result++;
  }

  if (reveal_counter > 0) {
    result++;
  }

  if (stop_counter > 0) {
    result++;
  }

  if (open_close_counter > 0) {
    result++;
  }

  return result;
}
