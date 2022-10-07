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

supla_simple_statiscics::supla_simple_statiscics() {
  channel_id = 0;
  min = 0;
  max = 0;
  avg = 0;
  sum = 0;
  count = 0;
  first_update_time = {};
}

supla_simple_statiscics::~supla_simple_statiscics(void) {}

void supla_simple_statiscics::set_channel_id(int channel_id) {
  this->channel_id = channel_id;
}

int supla_simple_statiscics::get_channel_id(void) { return this->channel_id; }

double supla_simple_statiscics::get_min(void) { return min; }

double supla_simple_statiscics::get_max(void) { return max; }

double supla_simple_statiscics::get_avg(void) { return avg; }

unsigned int supla_simple_statiscics::get_sample_count(void) { return count; }

void supla_simple_statiscics::add_sample(double value) {
  if (count == 0) {
    min = value;
    max = value;
    gettimeofday(&first_update_time, nullptr);
  } else {
    if (value < min) {
      min = value;
    } else if (value > max) {
      max = value;
    }
  }

  sum += value;
  count++;
  avg = sum / count;
}

void supla_simple_statiscics::reset(void) {
  min = 0;
  max = 0;
  avg = 0;
  sum = 0;
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
