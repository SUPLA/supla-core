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
  total_msec_above = 0;
  total_msec_below = 0;
  max_msec_above = 0;
  max_msec_below = 0;
}

supla_voltage_analyzer::~supla_voltage_analyzer(void) {}

void supla_voltage_analyzer::set_lower_threshold(double lower_threshold) {
  this->lower_threshold = lower_threshold;
}

void supla_voltage_analyzer::set_upper_threshold(double upper_threshold) {
  this->upper_threshold = upper_threshold;
}

unsigned int supla_voltage_analyzer::get_lower_counter(void) {
  return lower_counter;
}

unsigned int supla_voltage_analyzer::get_upper_counter(void) {
  return upper_counter;
}

unsigned int supla_voltage_analyzer::time_diff(struct timeval *tv) {
  if (tv->tv_sec != 0 || tv->tv_usec != 0) {
    struct timeval now = {};
    gettimeofday(&now, nullptr);
    return ((now.tv_sec * 1000000 + now.tv_usec) -
            (tv->tv_sec * 1000000 + tv->tv_usec)) /
           1000;
  }
  return 0;
}

unsigned int supla_voltage_analyzer::upper_time_diff(void) {
  return time_diff(&upper_time);
}

unsigned int supla_voltage_analyzer::lower_time_diff(void) {
  return time_diff(&lower_time);
}

unsigned int supla_voltage_analyzer::get_total_msec_above(void) {
  return total_msec_above + upper_time_diff();
}

unsigned int supla_voltage_analyzer::get_total_msec_below(void) {
  return total_msec_below + lower_time_diff();
}

unsigned int supla_voltage_analyzer::get_max_msec_above(void) {
  unsigned int t = upper_time_diff();
  return t > max_msec_above ? t : max_msec_above;
}

unsigned int supla_voltage_analyzer::get_max_msec_below(void) {
  unsigned int t = lower_time_diff();
  return t > max_msec_below ? t : max_msec_below;
}

void supla_voltage_analyzer::add_sample(double value) {
  supla_simple_statiscics::add_sample(value);

  if (upper_threshold) {
    if (value > upper_threshold) {
      if (upper_time.tv_sec == 0 && upper_time.tv_usec == 0) {
        gettimeofday(&upper_time, nullptr);
      }
      upper_counter++;
    } else {
      unsigned int msec_diff = upper_time_diff();
      upper_time = {};
      total_msec_above += msec_diff;
      if (msec_diff > max_msec_above) {
        max_msec_above = msec_diff;
      }
    }
  }

  if (lower_threshold) {
    if (value < lower_threshold) {
      if (lower_time.tv_sec == 0 && lower_time.tv_usec == 0) {
        gettimeofday(&lower_time, nullptr);
      }
      lower_counter++;
    } else {
      unsigned int msec_diff = lower_time_diff();
      lower_time = {};
      total_msec_below += msec_diff;
      if (msec_diff > max_msec_below) {
        max_msec_below = msec_diff;
      }
    }
  }
}

void supla_voltage_analyzer::reset(void) {
  lower_counter = 0;
  upper_counter = 0;
  lower_time = {};
  upper_time = {};
  total_msec_above = 0;
  total_msec_below = 0;
  max_msec_above = 0;
  max_msec_below = 0;
  supla_simple_statiscics::reset();
}
