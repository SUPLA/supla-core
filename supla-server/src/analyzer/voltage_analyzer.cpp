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

#include "analyzer/voltage_analyzer.h"

supla_voltage_analyzer::supla_voltage_analyzer() : supla_simple_statiscics() {
  lower_threshold = 0;
  upper_threshold = 0;
  lower_counter = 0;
  upper_counter = 0;
  lower_time = {};
  upper_time = {};
  total_sec_above = 0;
  total_sec_below = 0;
  max_sec_above = 0;
  max_sec_below = 0;
}

supla_voltage_analyzer::~supla_voltage_analyzer(void) {}

void supla_voltage_analyzer::set_lower_threshold(double lower_threshold) {
  this->lower_threshold = lower_threshold;
}

void supla_voltage_analyzer::set_upper_threshold(double upper_threshold) {
  this->upper_threshold = upper_threshold;
}

int supla_voltage_analyzer::get_lower_counter(void) { return lower_counter; }

int supla_voltage_analyzer::get_upper_counter(void) { return upper_counter; }

int supla_voltage_analyzer::get_total_sec_above(void) {
  return total_sec_above;
}

int supla_voltage_analyzer::get_total_sec_below(void) {
  return total_sec_below;
}

int supla_voltage_analyzer::get_max_sec_above(void) { return max_sec_above; }

int supla_voltage_analyzer::get_max_sec_below(void) { return max_sec_below; }

void supla_voltage_analyzer::update(double value) {
  supla_simple_statiscics::update(value);

  struct timeval now = {};
  if (upper_threshold || lower_threshold) {
    gettimeofday(&now, nullptr);
  }

  if (upper_threshold) {
    if (upper_time.tv_sec == 0 && upper_time.tv_usec == 0) {
      if (value > upper_threshold) {
        gettimeofday(&upper_time, nullptr);
        upper_counter++;
      }
    } else if (value <= upper_threshold) {
      int sec_diff = ((now.tv_sec * 1000000 + now.tv_usec) -
                      (upper_time.tv_sec * 1000000 + upper_time.tv_usec)) /
                     1000000;
      total_sec_above += sec_diff;
      if (sec_diff > max_sec_above) {
        max_sec_above = sec_diff;
      }
    }
  }

  if (lower_threshold) {
    if (lower_time.tv_sec == 0 && lower_time.tv_usec == 0) {
      if (value < lower_threshold) {
        gettimeofday(&lower_time, nullptr);
        lower_counter++;
      }
    } else if (value >= lower_threshold) {
      int sec_diff = ((now.tv_sec * 1000000 + now.tv_usec) -
                      (lower_time.tv_sec * 1000000 + lower_time.tv_usec)) /
                     1000000;
      total_sec_below += sec_diff;
      if (sec_diff > max_sec_below) {
        max_sec_below = sec_diff;
      }
    }
  }
}

int supla_voltage_analyzer::reset(void) {
  lower_counter = 0;
  upper_counter = 0;
  lower_time = {};
  upper_time = {};
  return supla_simple_statiscics::reset();
}
