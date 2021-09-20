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

#include "ChannelActionExecutorMock.h"

namespace testing {

ChannelActionExecutorMock::ChannelActionExecutorMock()
    : supla_abstract_channel_action_executor() {
  clear();
}

ChannelActionExecutorMock::~ChannelActionExecutorMock() {}

void ChannelActionExecutorMock::clear(void) {
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
  this->open_close_wct_counter = 0;
  this->color = 0x01FFFFFF;
  this->brightness = -1;
  this->color_brightness = -1;
  this->closing_percentage = -1;
}

void ChannelActionExecutorMock::set_on(bool on) {
  if (on) {
    on_counter++;
  } else {
    off_counter++;
  }
}

void ChannelActionExecutorMock::set_color(unsigned int color) {
  this->color = color;
}

void ChannelActionExecutorMock::set_brightness(char brightness) {
  this->brightness = brightness;
}

void ChannelActionExecutorMock::set_color_brightness(char brightness) {
  this->color_brightness = color_brightness;
}

void ChannelActionExecutorMock::toggle(void) { toggle_counter++; }

void ChannelActionExecutorMock::shut(const char *closingPercentage) {
  shut_counter++;
  if (closingPercentage) {
    closing_percentage = *closingPercentage;
  }
}

void ChannelActionExecutorMock::reveal(void) { reveal_counter++; }

void ChannelActionExecutorMock::stop(void) { stop_counter++; }

void ChannelActionExecutorMock::open(void) { open_counter++; }

void ChannelActionExecutorMock::close(void) { close_counter++; }

void ChannelActionExecutorMock::open_close(void) { open_close_counter++; }

void ChannelActionExecutorMock::open_close_without_canceling_tasks(void) {
  open_close_wct_counter++;
}

int ChannelActionExecutorMock::getOnCounter(void) { return on_counter; }

int ChannelActionExecutorMock::getOffCounter(void) { return off_counter; }

int ChannelActionExecutorMock::getColorCounter(void) { return color_counter; }

int ChannelActionExecutorMock::getBrightnessCounter(void) {
  return brightness_counter;
}

int ChannelActionExecutorMock::getColorBrightnessCounter(void) {
  return color_brightness_counter;
}

int ChannelActionExecutorMock::getToggleCounter(void) { return toggle_counter; }

int ChannelActionExecutorMock::getShutCounter(void) { return shut_counter; }

int ChannelActionExecutorMock::getRevealCounter(void) { return reveal_counter; }

int ChannelActionExecutorMock::getStopCounter(void) { return stop_counter; }

int ChannelActionExecutorMock::getOpenCounter(void) { return open_counter; }

int ChannelActionExecutorMock::getCloseCounter(void) { return close_counter; }

int ChannelActionExecutorMock::getOpenCloseCounter(void) {
  return open_close_counter;
}

int ChannelActionExecutorMock::getOpenCloseWctCounter(void) {
  return open_close_wct_counter;
}

char ChannelActionExecutorMock::getClosingPercentage(void) {
  return closing_percentage;
}

unsigned int ChannelActionExecutorMock::getColor(void) { return color; }

char ChannelActionExecutorMock::getBrightness(void) { return brightness; }

char ChannelActionExecutorMock::getColorBrightness(void) {
  return color_brightness;
}

int ChannelActionExecutorMock::counterSetCount(void) {
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

  if (open_close_wct_counter > 0) {
    result++;
  }

  return result;
}

}  // namespace testing
