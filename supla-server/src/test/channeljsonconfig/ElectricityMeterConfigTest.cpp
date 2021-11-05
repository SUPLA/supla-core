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

#include "ElectricityMeterConfigTest.h"

#include "TestHelper.h"
#include "channeljsonconfig/electricity_meter_config.h"
#include "proto.h"

namespace testing {

ElectricityMeterConfigTest::ElectricityMeterConfigTest(void) {}
ElectricityMeterConfigTest::~ElectricityMeterConfigTest(void) {}

TEST_F(ElectricityMeterConfigTest, setAllAvailableCounters) {
  electricity_meter_config *config = new electricity_meter_config();
  ASSERT_TRUE(config != NULL);

  char *str1 = config->get_properties();
  EXPECT_TRUE(str1 != NULL);

  if (str1) {
    EXPECT_EQ(strncmp(str1, "{}", 5), 0);
    free(str1);
    str1 = NULL;
  }

  EXPECT_TRUE(config->update_available_counters(0xFFFFFFFF));
  EXPECT_FALSE(config->update_available_counters(0xFFFFFFFF));

  str1 = config->get_properties();
  EXPECT_TRUE(str1 != NULL);

  if (str1) {
    // TestHelper::printEscaped(str);
    EXPECT_EQ(
        strncmp(str1,
                "{\"countersAvailable\":[\"forwardActiveEnergy\","
                "\"reverseActiveEnergy\",\"forwardReactiveEnergy\","
                "\"reverseReactiveEnergy\",\"forwardActiveEnergyBalanced\","
                "\"reverseActiveEnergyBalanced\"]}",
                1000),
        0);

    EXPECT_FALSE(config->update_available_counters(0x1));

    char *str2 = config->get_properties();
    EXPECT_TRUE(str2 != NULL);

    if (str2) {
      // All counters that have ever been available are added to the list. Once
      // added, they are no longer removed.
      EXPECT_EQ(strncmp(str1, str2, 1000), 0);
      free(str2);
      str2 = NULL;
    }

    free(str1);
    str1 = NULL;
  }

  delete config;
}

} /* namespace testing */
