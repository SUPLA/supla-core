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

#ifndef CHANNEL_BILLING_VALUE_H_
#define CHANNEL_BILLING_VALUE_H_

#include <string>

#include "proto.h"

class supla_channel_billing_value {
 public:
  supla_channel_billing_value(void);
  virtual ~supla_channel_billing_value(void);
  void get_cost_and_currency(const char *text_param1, int param2,
                             char currency[3], _supla_int_t *total_cost,
                             _supla_int_t *price_per_unit, double count);
  void get_currency(char currency[3], const char *text_param1);
  std::string get_currency(const char *text_param1);
  _supla_int_t get_cost(_supla_int_t price_per_unit, double count);
};

#endif /*CHANNEL_BILLING_VALUE_H_*/
