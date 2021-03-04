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

#include "MqttValueSetterMock.h"
#include <stdio.h>  // NOLINT
#include <string.h>
#include "proto.h"
#include "user.h"

MqttValueSetterMock::MqttValueSetterMock(supla_mqtt_client_settings *settings)
    : supla_mqtt_abstract_value_setter(settings) {
  clear();
}

MqttValueSetterMock::~MqttValueSetterMock(void) {}

void MqttValueSetterMock::clear(void) {
  this->on_counter = 0;
  this->color_counter = 0;
  this->brightness_counter = 0;
  this->color_brightness_counter = 0;
  this->off_counter = 0;
  this->toggle_counter = 0;
  this->shut_counter = 0;
  this->reveal_counter = 0;
  this->stop_counter = 0;
  this->open_counter = 0;
  this->close_counter = 0;
  this->open_close_counter = 0;
  this->refresh_all_existing_counter = 0;
  this->color = 0x01FFFFFF;
  this->brightness = -1;
  this->color_brightness = -1;
  this->closing_percentage = -1;
}

void MqttValueSetterMock::set_on(bool on) {
  if (on) {
    on_counter++;
  } else {
    off_counter++;
  }
}

void MqttValueSetterMock::set_color(unsigned int color) {
  this->color = color;
  color_counter++;
}

void MqttValueSetterMock::set_brightness(char brightness) {
  this->brightness = brightness;
  brightness_counter++;
}

void MqttValueSetterMock::set_color_brightness(char brightness) {
  this->color_brightness = brightness;
  color_brightness_counter++;
}

void MqttValueSetterMock::action_toggle(void) { toggle_counter++; }

void MqttValueSetterMock::action_shut(const char *closingPercentage) {
  shut_counter++;
  if (closingPercentage) {
    closing_percentage = *closingPercentage;
  }
}

void MqttValueSetterMock::action_reveal(void) { reveal_counter++; }

void MqttValueSetterMock::action_stop(void) { stop_counter++; }

void MqttValueSetterMock::action_open(void) { open_counter++; }

void MqttValueSetterMock::action_close(void) { close_counter++; }

void MqttValueSetterMock::action_open_close(void) { open_close_counter++; }

void MqttValueSetterMock::refresh_all_existing(void) {
  refresh_all_existing_counter++;
}

int MqttValueSetterMock::getOnCounter(void) { return on_counter; }

int MqttValueSetterMock::getOffCounter(void) { return off_counter; }

int MqttValueSetterMock::getColorCounter(void) { return color_counter; }

int MqttValueSetterMock::getBrightnessCounter(void) {
  return brightness_counter;
}

int MqttValueSetterMock::getColorBrightnessCounter(void) {
  return color_brightness_counter;
}

int MqttValueSetterMock::getToggleCounter(void) { return toggle_counter; }

int MqttValueSetterMock::getShutCounter(void) { return shut_counter; }

int MqttValueSetterMock::getRevealCounter(void) { return reveal_counter; }

int MqttValueSetterMock::getStopCounter(void) { return stop_counter; }

int MqttValueSetterMock::getOpenCounter(void) { return open_counter; }

int MqttValueSetterMock::getCloseCounter(void) { return close_counter; }

int MqttValueSetterMock::getOpenCloseCounter(void) {
  return open_close_counter;
}

int MqttValueSetterMock::getRefreshAllExistingCounter(void) {
  return refresh_all_existing_counter;
}

char MqttValueSetterMock::getClosingPercentage(void) {
  return closing_percentage;
}

unsigned int MqttValueSetterMock::getColor(void) { return color; }

char MqttValueSetterMock::getBrightness(void) { return brightness; }

char MqttValueSetterMock::getColorBrightness(void) { return color_brightness; }

bool MqttValueSetterMock::suidEqualTo(const char *suid) {
  if (suid == NULL && get_suid() == NULL) {
    return true;
  }

  if (suid != NULL && get_suid() != NULL) {
    return strncmp(suid, get_suid(), SHORT_UNIQUEID_MAXSIZE) == 0;
  }

  return false;
}

bool MqttValueSetterMock::channelEqualTo(int channel_id) {
  return channel_id == get_channel_id();
}

bool MqttValueSetterMock::deviceEqualTo(int device_id) {
  return device_id == get_device_id();
}

int MqttValueSetterMock::counterSetCount(void) {
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

  if (open_counter > 0) {
    result++;
  }

  if (close_counter > 0) {
    result++;
  }

  if (open_close_counter > 0) {
    result++;
  }

  if (refresh_all_existing_counter > 0) {
    result++;
  }

  return result;
}
