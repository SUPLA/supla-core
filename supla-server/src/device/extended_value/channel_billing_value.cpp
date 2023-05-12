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

#include "channel_billing_value.h"

#include <string.h>

supla_channel_billing_value::supla_channel_billing_value() {}

supla_channel_billing_value::~supla_channel_billing_value(void) {}

void supla_channel_billing_value::get_cost_and_currency(
    const char *text_param1, int param2, char currency[3],
    _supla_int_t *total_cost, _supla_int_t *price_per_unit, double count) {
  currency[0] = 0;
  *total_cost = 0;
  *price_per_unit = 0;

  if (text_param1 && strlen(text_param1) == 3) {
    memcpy(currency, text_param1, 3);
  }

  if (param2 > 0) {
    *price_per_unit = param2;
    // *total_cost = (double)(Param2 * 0.0001 * count) * 100;
    *total_cost = (double)(param2 * 0.01 * count);
  }
}
