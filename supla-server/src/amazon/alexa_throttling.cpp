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

#include "alexa_throttling.h"

#include "proto.h"

// static
supla_alexa_throttling supla_alexa_throttling::instance;

supla_alexa_throttling::supla_alexa_throttling() : supla_http_throttling() {}

supla_alexa_throttling::~supla_alexa_throttling(void) {}

int supla_alexa_throttling::get_default_delay_time(int func) {
  return 1500000;  // 1.5 sek.
}

int supla_alexa_throttling::get_delay_time_over_threadshold(int func) {
  return 10000000;  // 10 sek.
}

int supla_alexa_throttling::get_reset_time_us(int func) {
  return 10000000;  // 10 sek.
}

unsigned int supla_alexa_throttling::get_counter_threadshold(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      return 20;
  }
  return 5;
}

// static
supla_alexa_throttling *supla_alexa_throttling::get_instance(void) {
  return &instance;
}
