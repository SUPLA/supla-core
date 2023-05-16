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

using std::string;

supla_channel_billing_value::supla_channel_billing_value() {}

supla_channel_billing_value::~supla_channel_billing_value(void) {}

void supla_channel_billing_value::get_cost_and_currency(
    const char *text_param1, int param2, char currency[3],
    _supla_int_t *total_cost, _supla_int_t *price_per_unit, double count) {
  currency[0] = 0;
  *total_cost = 0;
  *price_per_unit = 0;

  get_currency(currency, text_param1);

  if (param2 > 0) {
    *price_per_unit = param2;
    *total_cost = get_cost(param2, count);
  }
}

_supla_int_t supla_channel_billing_value::get_cost(_supla_int_t price_per_unit,
                                                   double count) {
  // *total_cost = (double)(Param2 * 0.0001 * count) * 100;
  return (double)(price_per_unit * 0.01 * count);
}

void supla_channel_billing_value::get_currency(char currency[3],
                                               const char *text_param1) {
  if (text_param1 && strnlen(text_param1, 4) == 3) {
    memcpy(currency, text_param1, 3);
  } else {
    memset(currency, 0, 3);
  }
}

string supla_channel_billing_value::get_currency(const char *text_param1) {
  if (text_param1 && strnlen(text_param1, 4) == 3) {
    return text_param1;
  }

  return "";
}
