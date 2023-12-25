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

#include "ChannelHvacValueWithTempHumTest.h"

#include <gmock/gmock.h>

#include "device/channel_fragment.h"
#include "device/extended_value/channel_hp_thermostat_ev_decorator.h"
#include "device/value/channel_hp_thermostat_value.h"
#include "device/value/channel_hvac_value_with_temphum.h"
#include "device/value/channel_temphum_value.h"
#include "doubles/device/ChannelPropertyGetterMock.h"
#include "jsonconfig/channel/hvac_config.h"
#include "srpc/srpc.h"

namespace testing {

TEST_F(ChannelHvacValueWithTempHumTest, setGetTemperature) {
  supla_channel_hvac_value_with_temphum vth;
  EXPECT_EQ(vth.get_temperature_dbl(),
            supla_channel_temphum_value::incorrect_temperature());
  vth.set_temperature(123);
  EXPECT_EQ(vth.get_temperature(), 123);
}

TEST_F(ChannelHvacValueWithTempHumTest, setGetHumidity) {
  supla_channel_hvac_value_with_temphum vth;
  EXPECT_EQ(vth.get_humidity_dbl(),
            supla_channel_temphum_value::incorrect_humidity());
  vth.set_humidity(321);
  EXPECT_EQ(vth.get_humidity(), 321);
}

TEST_F(ChannelHvacValueWithTempHumTest, expandHpThermostatToHvac) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TThermostat_Value *)raw_value)->MeasuredTemperature = 1234;
  ((TThermostat_Value *)raw_value)->PresetTemperature = 4567;

  supla_channel_fragment fragment(
      10, 20, 1, SUPLA_CHANNELTYPE_THERMOSTAT_HEATPOL_HOMEPLUS,
      SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS, 0, false);

  ChannelPropertyGetterMock getter(5, 0, 0);

  EXPECT_CALL(getter, _get_extended_value(Eq(5), Eq(10), Eq(20)))
      .WillOnce([](int user_id, int device_id, int channel_id) {
        TSuplaChannelExtendedValue native_ev = {};
        TThermostat_ExtendedValue native_th_ev = {};
        native_th_ev.Fields = THERMOSTAT_FIELD_Flags;
        native_th_ev.Flags[4] = HP_STATUS_POWERON | HP_STATUS_HEATING;
        srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);
        return new supla_channel_thermostat_extended_value(&native_ev);
      });

  supla_channel_value *hp_val =
      new supla_channel_hp_thermostat_value(raw_value);
  supla_channel_hvac_value_with_temphum::expand(&hp_val, &fragment, &getter);

  ASSERT_TRUE(hp_val != nullptr);
  supla_channel_hvac_value_with_temphum *hvac_th =
      dynamic_cast<supla_channel_hvac_value_with_temphum *>(hp_val);
  EXPECT_TRUE(hvac_th != nullptr);
  if (hvac_th) {
    EXPECT_EQ(hvac_th->get_setpoint_temperature(), 4567);
    EXPECT_EQ(hvac_th->get_temperature(), 1234);
    EXPECT_EQ(hvac_th->get_mode(), SUPLA_HVAC_MODE_HEAT);
    EXPECT_EQ(hvac_th->get_flags(),
              SUPLA_HVAC_VALUE_FLAG_HEATING |
                  SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET);
    EXPECT_TRUE(hvac_th->is_on());
    EXPECT_TRUE(hvac_th->is_heating());
  }
  delete hp_val;
}

TEST_F(ChannelHvacValueWithTempHumTest, expandHvacToHavacWithTempHum) {
  supla_channel_hvac_value *hvac_value = new supla_channel_hvac_value();
  hvac_value->set_setpoint_temperature_heat(1234);
  hvac_value->set_mode(SUPLA_HVAC_MODE_HEAT);

  supla_channel_fragment fragment(10, 20, 1, SUPLA_CHANNELTYPE_HVAC,
                                  SUPLA_CHANNELFNC_HVAC_THERMOSTAT, 0, false);

  ChannelPropertyGetterMock getter(5, 0, 0);

  EXPECT_CALL(getter, _get_detached_json_config(Eq(5), Eq(10), Eq(20)))
      .WillOnce([](int user_id, int device_id, int channel_id) {
        TChannelConfig_HVAC native_config = {};
        native_config.MainThermometerChannelNo = 3;
        hvac_config *cfg = new hvac_config();
        cfg->set_config(&native_config, 1);
        return cfg;
      });

  EXPECT_CALL(getter, _get_channel_id(Eq(5), Eq(10), Eq(3)))
      .WillOnce(Return(789));

  EXPECT_CALL(getter, _get_value(Eq(5), Eq(10), Eq(789), IsNull(), IsNull()))
      .WillOnce(Return(new supla_channel_temphum_value(true, 22.33, 35.36)));

  supla_channel_value *value = hvac_value;
  hvac_value = nullptr;

  supla_channel_hvac_value_with_temphum::expand(&value, &fragment, &getter);

  ASSERT_TRUE(value != nullptr);
  supla_channel_hvac_value_with_temphum *hvac_th =
      dynamic_cast<supla_channel_hvac_value_with_temphum *>(value);
  EXPECT_TRUE(hvac_th != nullptr);
  if (hvac_th) {
    EXPECT_EQ(hvac_th->get_setpoint_temperature(), 1234);
    EXPECT_EQ(hvac_th->get_temperature(), 2233);
    EXPECT_EQ(hvac_th->get_humidity(), 3536);
    EXPECT_EQ(hvac_th->get_mode(), SUPLA_HVAC_MODE_HEAT);
  }
  delete value;
}

}  // namespace testing
