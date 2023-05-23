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

#include "ChannelBillingValueTest.h"

#include "device/extended_value/channel_billing_value.h"
#include "srpc/srpc.h"

namespace testing {

TEST_F(ChannelBillingValueTest, defaults) {
  supla_channel_billing_value bil;
  EXPECT_EQ(bil.get_currency("PLN"), "PLN");
}

TEST_F(ChannelBillingValueTest, costAndCurrency) {
  char currency[3] = {};
  _supla_int_t total_cost = 0;
  _supla_int_t price_per_unit = 0;
  double count = 1000;

  supla_channel_billing_value bil;
  bil.get_cost_and_currency("EUR", 12345, currency, &total_cost,
                            &price_per_unit, count);

  EXPECT_EQ(currency[0], 'E');
  EXPECT_EQ(currency[1], 'U');
  EXPECT_EQ(currency[2], 'R');

  EXPECT_EQ(total_cost, 123450);
  EXPECT_EQ(price_per_unit, 12345);
}

}  // namespace testing
