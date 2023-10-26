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

#include "doubles/actions/ActionExecutorMock.h"

#include <sys/time.h>

namespace testing {

using std::function;
using std::list;
using std::map;
using std::shared_ptr;
using std::string;

ActionExecutorMock::ActionExecutorMock() : supla_abstract_action_executor() {
  temperature = nullptr;
  temperatures = nullptr;
  clear();
}

ActionExecutorMock::~ActionExecutorMock() {
  if (temperature) {
    delete temperature;
  }

  if (temperatures) {
    delete temperatures;
  }
}

shared_ptr<supla_device> ActionExecutorMock::get_device(void) {
  return supla_abstract_action_executor::get_device();
}

map<string, string> ActionExecutorMock::get_replacement_map(void) {
  return replacement_map;
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
  this->enable_counter = 0;
  this->disable_counter = 0;
  this->sent_counter = 0;
  this->execute_counter = 0;
  this->interrupt_counter = 0;
  this->interrupt_and_execute_counter = 0;
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
  this->hvac_set_parameters_counter = 0;
  this->hvac_switch_to_program_mode_counter = 0;
  this->hvac_switch_to_manual_mode_counter = 0;
  this->hvac_set_temperature_counter = 0;
  this->hvac_set_temperatures_counter = 0;
  this->rgbw_on_off = -1;
  this->delta = false;
  if (this->temperature) {
    delete this->temperature;
    this->temperature = nullptr;
  }

  if (this->temperatures) {
    delete this->temperatures;
    this->temperatures = nullptr;
  }
}

void ActionExecutorMock::addTime(void) {
  struct timeval now = {};
  gettimeofday(&now, NULL);
  times.push_back(now);
}

void ActionExecutorMock::set_on(bool on, unsigned long long duration_ms) {
  addTime();
  if (on) {
    on_counter++;
  } else {
    off_counter++;
  }
}

void ActionExecutorMock::set_color(unsigned int color) {
  addTime();
  color_counter++;
  this->color = color;
}

void ActionExecutorMock::set_brightness(char brightness) {
  addTime();
  brightness_counter++;
  this->brightness = brightness;
}

void ActionExecutorMock::set_color_brightness(char brightness) {
  addTime();
  color_brightness_counter++;
  this->color_brightness = color_brightness;
}

void ActionExecutorMock::set_rgbw(unsigned int *color, char *color_brightness,
                                  char *brightness, char *on_off) {
  addTime();
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

void ActionExecutorMock::toggle(void) {
  addTime();
  toggle_counter++;
}

void ActionExecutorMock::shut(const char *closingPercentage, bool delta) {
  addTime();
  shut_counter++;
  if (closingPercentage) {
    closing_percentage = *closingPercentage;
  }
  this->delta = delta;
}

void ActionExecutorMock::reveal(void) {
  addTime();
  reveal_counter++;
}

void ActionExecutorMock::up(void) {
  addTime();
  up_counter++;
}

void ActionExecutorMock::down(void) {
  addTime();
  down_counter++;
}

void ActionExecutorMock::up_or_stop(void) {
  addTime();
  up_or_stop_counter++;
}

void ActionExecutorMock::down_or_stop(void) {
  addTime();
  down_or_stop_counter++;
}

void ActionExecutorMock::step_by_step(void) {
  addTime();
  step_by_step_counter++;
}

void ActionExecutorMock::stop(void) {
  addTime();
  stop_counter++;
}

void ActionExecutorMock::execute(void) {
  addTime();
  execute_counter++;
}

void ActionExecutorMock::enable(void) {
  addTime();
  enable_counter++;
}

void ActionExecutorMock::disable(void) {
  addTime();
  disable_counter++;
}

void ActionExecutorMock::send(
    std::map<std::string, std::string> *replacement_map) {
  addTime();
  sent_counter++;
  if (replacement_map) {
    this->replacement_map = *replacement_map;
  }
}

void ActionExecutorMock::interrupt(void) {
  addTime();
  interrupt_counter++;
}

void ActionExecutorMock::interrupt_and_execute(void) {
  addTime();
  interrupt_and_execute_counter++;
}

void ActionExecutorMock::open(void) {
  addTime();
  open_counter++;
}

void ActionExecutorMock::close(void) {
  addTime();
  close_counter++;
}

void ActionExecutorMock::open_close(void) {
  addTime();
  open_close_counter++;
}

void ActionExecutorMock::open_close_without_canceling_tasks(void) {
  addTime();
  open_close_wct_counter++;
}

void ActionExecutorMock::forward_outside(int cap) {
  addTime();
  forward_outside_counter++;
}

void ActionExecutorMock::hvac_set_parameters(
    supla_action_hvac_parameters *params) {
  addTime();
  hvac_set_parameters_counter++;
}

void ActionExecutorMock::hvac_switch_to_program_mode(void) {
  addTime();
  hvac_switch_to_program_mode_counter++;
}

void ActionExecutorMock::hvac_switch_to_manual_mode(void) {
  addTime();
  hvac_switch_to_manual_mode_counter++;
}

void ActionExecutorMock::hvac_set_temperature(
    supla_action_hvac_setpoint_temperature *temperature) {
  addTime();
  hvac_set_temperature_counter++;

  if (this->temperature) {
    delete this->temperature;
    this->temperature = nullptr;
  }

  if (temperature) {
    this->temperature = dynamic_cast<supla_action_hvac_setpoint_temperature *>(
        temperature->copy());
  }
}

void ActionExecutorMock::hvac_set_temperatures(
    supla_action_hvac_setpoint_temperatures *temperatures) {
  addTime();
  hvac_set_temperatures_counter++;

  if (this->temperatures) {
    delete this->temperatures;
    this->temperatures = nullptr;
  }

  if (temperatures) {
    this->temperatures =
        dynamic_cast<supla_action_hvac_setpoint_temperatures *>(
            temperatures->copy());
  }
}

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

int ActionExecutorMock::getExecuteCounter(void) { return execute_counter; }

int ActionExecutorMock::getInterruptCounter(void) { return interrupt_counter; }

int ActionExecutorMock::getInterruptAndExecuteCounter(void) {
  return interrupt_and_execute_counter;
}

int ActionExecutorMock::getEnableCounter(void) { return enable_counter; }

int ActionExecutorMock::getDisableCounter(void) { return disable_counter; }

int ActionExecutorMock::getSentCounter(void) { return sent_counter; }

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

int ActionExecutorMock::getHvacSetParametersCounter(void) {
  return hvac_set_parameters_counter;
}

int ActionExecutorMock::getHvacSwitchToProgramModeCounter(void) {
  return hvac_switch_to_program_mode_counter;
}

int ActionExecutorMock::getHvacSwitchToManualModeCounter(void) {
  return hvac_switch_to_manual_mode_counter;
}

int ActionExecutorMock::getHvacSetTemperatureCounter(void) {
  return hvac_set_temperature_counter;
}

int ActionExecutorMock::getHvacSetTemperaturesCounter(void) {
  return hvac_set_temperatures_counter;
}

char ActionExecutorMock::getClosingPercentage(void) {
  return closing_percentage;
}

unsigned int ActionExecutorMock::getColor(void) { return color; }

char ActionExecutorMock::getBrightness(void) { return brightness; }

char ActionExecutorMock::getColorBrightness(void) { return color_brightness; }

char ActionExecutorMock::getRGBWOnOff(void) { return rgbw_on_off; }

bool ActionExecutorMock::getDelta(void) { return delta; }

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

  if (disable_counter > 0) {
    result++;
  }

  if (enable_counter > 0) {
    result++;
  }

  if (sent_counter > 0) {
    result++;
  }

  if (execute_counter > 0) {
    result++;
  }

  if (interrupt_counter > 0) {
    result++;
  }

  if (interrupt_and_execute_counter > 0) {
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

  if (hvac_set_parameters_counter > 0) {
    result++;
  }

  if (hvac_switch_to_program_mode_counter > 0) {
    result++;
  }

  if (hvac_switch_to_manual_mode_counter > 0) {
    result++;
  }

  if (hvac_set_temperature_counter > 0) {
    result++;
  }

  if (hvac_set_temperatures_counter > 0) {
    result++;
  }

  return result;
}

list<struct timeval> ActionExecutorMock::getTimes(void) { return times; }

supla_action_hvac_setpoint_temperature *
ActionExecutorMock::getHvacSetpointTemperature(void) {
  return temperature;
}

supla_action_hvac_setpoint_temperatures *
ActionExecutorMock::getHvacSetpointTemperatures(void) {
  return temperatures;
}

}  // namespace testing
