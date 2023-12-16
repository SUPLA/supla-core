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

#include "ChannelHpThermostatEvDecoratorTest.h"

#include "device/extended_value/channel_hp_thermostat_ev_decorator.h"
#include "srpc/srpc.h"

namespace testing {

TEST_F(ChannelHpThermostatEvDecoratorTest, getHomeAssistantAction) {
  TSuplaChannelExtendedValue native_ev = {};
  TThermostat_ExtendedValue native_th_ev = {};
  native_th_ev.Fields = THERMOSTAT_FIELD_Flags;

  {
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_EQ(decorator.get_home_assistant_action(), "off");
  }

  {
    native_th_ev.Flags[4] = HP_STATUS_POWERON;
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_EQ(decorator.get_home_assistant_action(), "idle");
  }

  {
    native_th_ev.Flags[4] |= HP_STATUS_HEATING;
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_EQ(decorator.get_home_assistant_action(), "heating");
  }
}

TEST_F(ChannelHpThermostatEvDecoratorTest, getHomeAssistantMode) {
  TSuplaChannelExtendedValue native_ev = {};
  TThermostat_ExtendedValue native_th_ev = {};
  native_th_ev.Fields = THERMOSTAT_FIELD_Flags;

  {
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_EQ(decorator.get_home_assistant_mode(), "off");
  }

  {
    native_th_ev.Flags[4] = HP_STATUS_POWERON;
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_EQ(decorator.get_home_assistant_mode(), "heat");
  }

  {
    native_th_ev.Flags[4] |= HP_STATUS_PROGRAMMODE;
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_EQ(decorator.get_home_assistant_mode(), "auto");
  }
}

TEST_F(ChannelHpThermostatEvDecoratorTest, getHvacMode) {
  TSuplaChannelExtendedValue native_ev = {};
  TThermostat_ExtendedValue native_th_ev = {};
  native_th_ev.Fields = THERMOSTAT_FIELD_Flags;

  {
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_EQ(decorator.get_hvac_mode(), SUPLA_HVAC_MODE_OFF);
  }

  {
    native_th_ev.Flags[4] = HP_STATUS_POWERON;
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_EQ(decorator.get_hvac_mode(), SUPLA_HVAC_MODE_HEAT);
  }
}

TEST_F(ChannelHpThermostatEvDecoratorTest, getHvacFlags) {
  TSuplaChannelExtendedValue native_ev = {};
  TThermostat_ExtendedValue native_th_ev = {};
  native_th_ev.Fields = THERMOSTAT_FIELD_Flags;

  {
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_EQ(decorator.get_hvac_flags(), 0);
  }

  {
    native_th_ev.Flags[4] = HP_STATUS_HEATING;
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_EQ(decorator.get_hvac_flags(), SUPLA_HVAC_VALUE_FLAG_HEATING);
  }

  {
    native_th_ev.Flags[4] = HP_STATUS_PROGRAMMODE;
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_EQ(decorator.get_hvac_flags(),
              SUPLA_HVAC_VALUE_FLAG_WEEKLY_SCHEDULE);
  }

  {
    native_th_ev.Flags[4] = HP_STATUS_PROGRAMMODE | HP_STATUS_HEATING;
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_EQ(
        decorator.get_hvac_flags(),
        SUPLA_HVAC_VALUE_FLAG_WEEKLY_SCHEDULE | SUPLA_HVAC_VALUE_FLAG_HEATING);
  }
}

TEST_F(ChannelHpThermostatEvDecoratorTest, getStateFlags) {
  TSuplaChannelExtendedValue native_ev = {};
  TThermostat_ExtendedValue native_th_ev = {};
  native_th_ev.Fields = THERMOSTAT_FIELD_Flags;
  native_th_ev.Flags[4] = 123;

  srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
  supla_channel_thermostat_extended_value ev(&native_ev);
  supla_channel_hp_thermostat_ev_decorator decorator(&ev);
  EXPECT_EQ(decorator.get_state_flags(), 123);
}

TEST_F(ChannelHpThermostatEvDecoratorTest, isHeating) {
  TSuplaChannelExtendedValue native_ev = {};
  TThermostat_ExtendedValue native_th_ev = {};
  native_th_ev.Fields = THERMOSTAT_FIELD_Flags;

  {
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_FALSE(decorator.is_heating());
  }

  {
    native_th_ev.Flags[4] = HP_STATUS_HEATING;
    srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
    supla_channel_thermostat_extended_value ev(&native_ev);
    supla_channel_hp_thermostat_ev_decorator decorator(&ev);
    EXPECT_TRUE(decorator.is_heating());
  }
}

}  // namespace testing
