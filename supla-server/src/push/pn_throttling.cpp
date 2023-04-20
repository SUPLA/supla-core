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

#include "pn_throttling.h"

// static
supla_pn_throttling supla_pn_throttling::instance;

supla_pn_throttling::supla_pn_throttling() : supla_http_throttling() {}

supla_pn_throttling::~supla_pn_throttling(void) {}

int supla_pn_throttling::get_default_delay_time(int func) {
  return 0;  // 1.5 sek.
}

unsigned int supla_pn_throttling::get_counter_threadshold(int func) {
  return 5;
}

// static
supla_pn_throttling* supla_pn_throttling::get_instance(void) {
  return &instance;
}
