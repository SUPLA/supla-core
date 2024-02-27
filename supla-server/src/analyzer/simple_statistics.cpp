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

#include "analyzer/simple_statistics.h"

#include <math.h>

supla_simple_statiscics::supla_simple_statiscics() {
  first = NAN;
  min = NAN;
  max = NAN;
  avg = NAN;
  sum = 0;
  last = NAN;
  count = 0;
  non_nan_count = 0;
  first_update_time = {};
}

supla_simple_statiscics::~supla_simple_statiscics(void) {}

double supla_simple_statiscics::get_first(void) { return first; }

double supla_simple_statiscics::get_min(void) { return min; }

double supla_simple_statiscics::get_max(void) { return max; }

double supla_simple_statiscics::get_avg(void) { return avg; }

double supla_simple_statiscics::get_last(void) { return last; }

unsigned int supla_simple_statiscics::get_sample_count(void) { return count; }

unsigned int supla_simple_statiscics::get_non_nan_sample_count(void) {
  return non_nan_count;
}

void supla_simple_statiscics::add_sample(double value) {
  if (count == 0) {
    gettimeofday(&first_update_time, nullptr);
  }

  if (!isnan(value)) {
    if (isnan(min) || value < min) {
      min = value;
    }

    if (isnan(max) || value > max) {
      max = value;
    }

    if (isnan(first)) {
      first = value;
    }

    non_nan_count++;
    sum += value;
    avg = sum / non_nan_count;
    last = value;
  }

  count++;
}

void supla_simple_statiscics::reset(void) {
  first = NAN;
  min = NAN;
  max = NAN;
  avg = NAN;
  sum = 0;
  last = NAN;
  count = 0;
  first_update_time = {};
}

unsigned int supla_simple_statiscics::get_total_time_msec(void) {
  if (first_update_time.tv_sec || first_update_time.tv_usec) {
    struct timeval now = {};
    gettimeofday(&now, nullptr);

    return ((now.tv_sec * 1000000 + now.tv_usec) -
            (first_update_time.tv_sec * 1000000 + first_update_time.tv_usec)) /
           1000;
  }

  return 0;
}
