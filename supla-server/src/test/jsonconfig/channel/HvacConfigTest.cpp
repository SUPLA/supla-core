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

#include "HvacConfigTest.h"

#include "TestHelper.h"
#include "json/cJSON.h"
#include "jsonconfig/channel/hvac_config.h"
#include "log.h"
#include "proto.h"

namespace testing {

HvacConfigTest::HvacConfigTest(void) {}

HvacConfigTest::~HvacConfigTest(void) {}

TEST_F(HvacConfigTest, setAndGetConfig) {
  TSD_ChannelConfig_HVAC ds_hvac1 = {};
  ds_hvac1.MainThermometerChannelNo = 1;
  ds_hvac1.AuxThermometerChannelNo = 2;
  ds_hvac1.AuxThermometerType = SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_HEATER;
  ds_hvac1.AntiFreezeAndOverheatProtectionEnabled = true;
  ds_hvac1.AvailableAlgorithms = 0xFFFF;
  ds_hvac1.UsedAlgorithm = SUPLA_HVAC_ALGORITHM_ON_OFF;
  ds_hvac1.MinOnTimeS = 10;
  ds_hvac1.MinOffTimeS = 600;
  ds_hvac1.OutputValueOnError = 55;

  int size = sizeof(ds_hvac1.Temperatures.Temperature) / sizeof(_supla_int16_t);

  for (int a = 0; a < size; a++) {
    ds_hvac1.Temperatures.Index |= 1 << a;
    ds_hvac1.Temperatures.Temperature[a] = a + 1;
  }

  hvac_config config1;
  config1.set_config(&ds_hvac1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"hvac\":{\"mainThermometerChannelNo\":1,\"auxThermometerChannelNo\":2,"
      "\"auxThermometerType\":\"GenericHeater\","
      "\"antiFreezeAndOverheatProtectionEnabled\":true,\"availableAlgorithms\":"
      "[\"OnOff\"],\"usedAlgorithm\":\"OnOff\",\"minOffTimeS\":600,"
      "\"minOnTimeS\":10,\"outputValueOnError\":55,\"temperatures\":{\"1\":1,"
      "\"2\":2,\"3\":3,\"4\":4,\"5\":5,\"6\":6,\"7\":7,\"8\":8,\"9\":9,\"10\":"
      "10,\"11\":11,\"12\":12,\"13\":13,\"14\":14,\"15\":15,\"16\":16,\"17\":"
      "17,\"18\":18,\"19\":19,\"20\":20,\"21\":21,\"22\":22,\"23\":23,\"24\":"
      "24}}}");

  hvac_config config2;
  config2.set_user_config(str);
  free(str);

  TSD_ChannelConfig_HVAC ds_hvac2 = {};
  config2.get_config(&ds_hvac2);

  ds_hvac1.AvailableAlgorithms = SUPLA_HVAC_ALGORITHM_ON_OFF;

  EXPECT_EQ(memcmp(&ds_hvac1, &ds_hvac2, sizeof(TSD_ChannelConfig_HVAC)), 0);

  EXPECT_EQ(ds_hvac1.Temperatures.Index, ds_hvac2.Temperatures.Index);
}

TEST_F(HvacConfigTest, selectedTemperatures) {}

TEST_F(HvacConfigTest, merge) {}

} /* namespace testing */
