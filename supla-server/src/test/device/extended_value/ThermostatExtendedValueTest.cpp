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

#include "ThermostatExtendedValueTest.h"

#include "device/extended_value/channel_hp_thermostat_ev_decorator.h"
#include "device/extended_value/channel_thermostat_extended_value.h"
#include "srpc/srpc.h"

namespace testing {

TEST_F(ThermostatExtendedValueTest, getVbtValue) {
  TSuplaChannelExtendedValue native_ev = {};
  TThermostat_ExtendedValue native_th_ev = {};
  native_th_ev.Fields = THERMOSTAT_FIELD_Flags;

  srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
  supla_channel_thermostat_extended_value value(&native_ev);

  double vbt_value = 0;

  EXPECT_FALSE(value.get_vbt_value(var_name_none, &vbt_value));

  EXPECT_TRUE(value.get_vbt_value(var_name_heating, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_is_on, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  native_th_ev.Flags[4] |= HP_STATUS_HEATING;
  srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
  supla_channel_thermostat_extended_value value2(&native_ev);

  EXPECT_TRUE(value2.get_vbt_value(var_name_heating, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  EXPECT_TRUE(value2.get_vbt_value(var_name_is_on, &vbt_value));
  EXPECT_EQ(vbt_value, 1);
}

}  // namespace testing
