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

#include <doubles/actions/ActionExecutorMock.h>

namespace testing {

ActionExecutorMock::ActionExecutorMock() : supla_abstract_action_executor() {
  clear();
}

ActionExecutorMock::~ActionExecutorMock() {}

void ActionExecutorMock::access_device(
    std::function<void(supla_device *device)> on_device) {
  supla_abstract_action_executor::access_device(on_device);
}

void ActionExecutorMock::clear(void) {
  this->on_counter = 0;
  this->color_counter = 0;
  this->brightness_counter = 0;
  this->color_brightness_counter = 0;
  this->off_counter = 0;
  this->toggle_counter = 0;
  this->shut_counter = 0;
  this->reveal_counter = 0;
  this->up_counter = 0;
  this->down_counter = 0;
  this->up_or_stop_counter = 0;
  this->down_or_stop_counter = 0;
  this->step_by_step_counter = 0;
  this->stop_counter = 0;
  this->open_counter = 0;
  this->close_counter = 0;
  this->open_close_counter = 0;
  this->open_close_wct_counter = 0;
  this->color = 0x01FFFFFF;
  this->brightness = -1;
  this->color_brightness = -1;
  this->closing_percentage = -1;
  this->rgbw_counter = 0;
  this->forward_outside_counter = 0;
  this->rgbw_on_off = -1;
}

void ActionExecutorMock::set_on(bool on) {
  if (on) {
    on_counter++;
  } else {
    off_counter++;
  }
}

void ActionExecutorMock::set_color(unsigned int color) {
  color_counter++;
  this->color = color;
}

void ActionExecutorMock::set_brightness(char brightness) {
  brightness_counter++;
  this->brightness = brightness;
}

void ActionExecutorMock::set_color_brightness(char brightness) {
  color_brightness_counter++;
  this->color_brightness = color_brightness;
}

void ActionExecutorMock::set_rgbw(unsigned int *color, char *color_brightness,
                                  char *brightness, char *on_off) {
  rgbw_counter++;
  if (color) {
    this->color = *color;
  }

  if (color_brightness) {
    this->color_brightness = *color_brightness;
  }

  if (brightness) {
    this->brightness = *brightness;
  }

  if (on_off) {
    this->rgbw_on_off = *on_off;
  }
}

void ActionExecutorMock::toggle(void) { toggle_counter++; }

void ActionExecutorMock::shut(const char *closingPercentage) {
  shut_counter++;
  if (closingPercentage) {
    closing_percentage = *closingPercentage;
  }
}

void ActionExecutorMock::reveal(void) { reveal_counter++; }

void ActionExecutorMock::up(void) { up_counter++; }

void ActionExecutorMock::down(void) { down_counter++; }

void ActionExecutorMock::up_or_stop(void) { up_or_stop_counter++; }

void ActionExecutorMock::down_or_stop(void) { down_or_stop_counter++; }

void ActionExecutorMock::step_by_step(void) { step_by_step_counter++; }

void ActionExecutorMock::stop(void) { stop_counter++; }

void ActionExecutorMock::open(void) { open_counter++; }

void ActionExecutorMock::close(void) { close_counter++; }

void ActionExecutorMock::open_close(void) { open_close_counter++; }

void ActionExecutorMock::open_close_without_canceling_tasks(void) {
  open_close_wct_counter++;
}

void ActionExecutorMock::forward_outside(int cap) { forward_outside_counter++; }

int ActionExecutorMock::getOnCounter(void) { return on_counter; }

int ActionExecutorMock::getOffCounter(void) { return off_counter; }

int ActionExecutorMock::getColorCounter(void) { return color_counter; }

int ActionExecutorMock::getBrightnessCounter(void) {
  return brightness_counter;
}

int ActionExecutorMock::getColorBrightnessCounter(void) {
  return color_brightness_counter;
}

int ActionExecutorMock::getRGBWCounter(void) { return rgbw_counter; }

int ActionExecutorMock::getToggleCounter(void) { return toggle_counter; }

int ActionExecutorMock::getShutCounter(void) { return shut_counter; }

int ActionExecutorMock::getRevealCounter(void) { return reveal_counter; }

int ActionExecutorMock::getUpCounter(void) { return up_counter; }

int ActionExecutorMock::getDownCounter(void) { return down_counter; }

int ActionExecutorMock::getUpOrStopCounter(void) { return up_or_stop_counter; }

int ActionExecutorMock::getDownOrStopCounter(void) {
  return down_or_stop_counter;
}

int ActionExecutorMock::getStepByStepCounter(void) {
  return step_by_step_counter;
}

int ActionExecutorMock::getStopCounter(void) { return stop_counter; }

int ActionExecutorMock::getOpenCounter(void) { return open_counter; }

int ActionExecutorMock::getCloseCounter(void) { return close_counter; }

int ActionExecutorMock::getOpenCloseCounter(void) { return open_close_counter; }

int ActionExecutorMock::getOpenCloseWctCounter(void) {
  return open_close_wct_counter;
}

int ActionExecutorMock::getForwardOutsideCounter(void) {
  return forward_outside_counter;
}

char ActionExecutorMock::getClosingPercentage(void) {
  return closing_percentage;
}

unsigned int ActionExecutorMock::getColor(void) { return color; }

char ActionExecutorMock::getBrightness(void) { return brightness; }

char ActionExecutorMock::getColorBrightness(void) { return color_brightness; }

char ActionExecutorMock::getRGBWOnOff(void) { return rgbw_on_off; }

int ActionExecutorMock::counterSetCount(void) {
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

  if (up_counter > 0) {
    result++;
  }

  if (down_counter > 0) {
    result++;
  }

  if (up_or_stop_counter > 0) {
    result++;
  }

  if (down_or_stop_counter > 0) {
    result++;
  }

  if (step_by_step_counter > 0) {
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

  if (rgbw_counter > 0) {
    result++;
  }

  if (forward_outside_counter > 0) {
    result++;
  }

  return result;
}

}  // namespace testing
