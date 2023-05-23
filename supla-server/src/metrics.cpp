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

#include "metrics.h"

#include <sys/time.h>

using std::function;

supla_metrics::supla_metrics(void) {}

supla_metrics::~supla_metrics(void) {}

// static
unsigned long long supla_metrics::measure_the_time_in_usec(
    function<void(void)> func) {
  struct timeval before = {};
  struct timeval now = {};
  gettimeofday(&before, nullptr);
  func();
  gettimeofday(&now, nullptr);

  return (now.tv_sec * 1000000UL + now.tv_usec) -
         (before.tv_sec * 1000000UL + before.tv_usec);
}
