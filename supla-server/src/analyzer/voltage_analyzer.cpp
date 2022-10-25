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
  below_count = 0;
  above_count = 0;
  time_below = {};
  time_above = {};
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

unsigned int supla_voltage_analyzer::get_below_count(void) {
  return below_count;
}

unsigned int supla_voltage_analyzer::get_above_count(void) {
  return above_count;
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

unsigned int supla_voltage_analyzer::time_above_diff(void) {
  return time_diff(&time_above);
}

unsigned int supla_voltage_analyzer::time_below_diff(void) {
  return time_diff(&time_below);
}

unsigned int supla_voltage_analyzer::get_total_msec_above(void) {
  return total_msec_above + time_above_diff();
}

unsigned int supla_voltage_analyzer::get_total_msec_below(void) {
  return total_msec_below + time_below_diff();
}

unsigned int supla_voltage_analyzer::get_max_msec_above(void) {
  unsigned int t = time_above_diff();
  return t > max_msec_above ? t : max_msec_above;
}

unsigned int supla_voltage_analyzer::get_max_msec_below(void) {
  unsigned int t = time_below_diff();
  return t > max_msec_below ? t : max_msec_below;
}

void supla_voltage_analyzer::add_sample(double value) {
  supla_simple_statiscics::add_sample(value);

  if (upper_threshold) {
    if (value > upper_threshold) {
      if (time_above.tv_sec == 0 && time_above.tv_usec == 0) {
        gettimeofday(&time_above, nullptr);
        above_count++;
      }
    } else {
      unsigned int msec_diff = time_above_diff();
      time_above = {};
      total_msec_above += msec_diff;
      if (msec_diff > max_msec_above) {
        max_msec_above = msec_diff;
      }
    }
  }

  if (lower_threshold) {
    if (value < lower_threshold) {
      if (time_below.tv_sec == 0 && time_below.tv_usec == 0) {
        gettimeofday(&time_below, nullptr);
        below_count++;
      }
    } else {
      unsigned int msec_diff = time_below_diff();
      time_below = {};
      total_msec_below += msec_diff;
      if (msec_diff > max_msec_below) {
        max_msec_below = msec_diff;
      }
    }
  }
}

void supla_voltage_analyzer::reset(void) {
  below_count = 0;
  above_count = 0;
  time_below = {};
  time_above = {};
  total_msec_above = 0;
  total_msec_below = 0;
  max_msec_above = 0;
  max_msec_below = 0;
  supla_simple_statiscics::reset();
}
