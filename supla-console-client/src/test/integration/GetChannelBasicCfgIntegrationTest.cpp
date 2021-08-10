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

#include "GetChannelBasicCfgIntegrationTest.h"
#include "log.h"

namespace testing {

GetChannelBasicCfgIntegrationTest::GetChannelBasicCfgIntegrationTest() {}

GetChannelBasicCfgIntegrationTest::~GetChannelBasicCfgIntegrationTest() {}

TEST_F(GetChannelBasicCfgIntegrationTest, GetBasicCfgForChannel167) {
  getBasicCfg(167);

  char expectedDeviceName[] = "ARDUINO";
  ASSERT_EQ(strncmp(channelBasicCfg.DeviceName, expectedDeviceName,
                    SUPLA_DEVICE_NAME_MAXSIZE),
            0);

  char expectedDeviceSoftVer[] = "1.6.1";
  ASSERT_EQ(strncmp(channelBasicCfg.DeviceSoftVer, expectedDeviceSoftVer,
                    SUPLA_SOFTVER_MAXSIZE),
            0);

  int FuncList = SUPLA_BIT_FUNC_CONTROLLINGTHEGATEWAYLOCK |
                 SUPLA_BIT_FUNC_CONTROLLINGTHEGATE |
                 SUPLA_BIT_FUNC_CONTROLLINGTHEGARAGEDOOR |
                 SUPLA_BIT_FUNC_CONTROLLINGTHEDOORLOCK |
                 SUPLA_BIT_FUNC_POWERSWITCH | SUPLA_BIT_FUNC_LIGHTSWITCH |
                 SUPLA_BIT_FUNC_STAIRCASETIMER;

  ASSERT_EQ(channelBasicCfg.DeviceID, 83);
  ASSERT_EQ(channelBasicCfg.DeviceFlags, 0);
  ASSERT_EQ(channelBasicCfg.ManufacturerID, 0);
  ASSERT_EQ(channelBasicCfg.ProductID, 0);
  ASSERT_EQ(channelBasicCfg.ID, 167);
  ASSERT_EQ(channelBasicCfg.Number, 10);
  ASSERT_EQ(channelBasicCfg.Type, SUPLA_CHANNELTYPE_RELAY);
  ASSERT_EQ(channelBasicCfg.Func, SUPLA_CHANNELFNC_CONTROLLINGTHEGATE);
  ASSERT_EQ(channelBasicCfg.FuncList, FuncList);
  ASSERT_EQ(channelBasicCfg.ChannelFlags, (unsigned int)0);

  char expectedCaption[] = "Szlaban";
  ASSERT_EQ(strncmp(channelBasicCfg.Caption, expectedCaption,
                    SUPLA_CHANNEL_CAPTION_MAXSIZE),
            0);
}

TEST_F(GetChannelBasicCfgIntegrationTest, GetBasicCfgForChannel301) {
  getBasicCfg(301);

  char expectedDeviceName[] = "PEVEKO VALVE";
  ASSERT_EQ(strncmp(channelBasicCfg.DeviceName, expectedDeviceName,
                    SUPLA_DEVICE_NAME_MAXSIZE),
            0);

  char expectedDeviceSoftVer[] = "2.7.16";
  ASSERT_EQ(strncmp(channelBasicCfg.DeviceSoftVer, expectedDeviceSoftVer,
                    SUPLA_SOFTVER_MAXSIZE),
            0);

  ASSERT_EQ(channelBasicCfg.DeviceID, 142);
  ASSERT_EQ(channelBasicCfg.DeviceFlags, 0);
  ASSERT_EQ(channelBasicCfg.ManufacturerID, SUPLA_MFR_PEVEKO);
  ASSERT_EQ(channelBasicCfg.ProductID, 1);
  ASSERT_EQ(channelBasicCfg.ID, 301);
  ASSERT_EQ(channelBasicCfg.Number, 0);
  ASSERT_EQ(channelBasicCfg.Type, SUPLA_CHANNELTYPE_VALVE_OPENCLOSE);
  ASSERT_EQ(channelBasicCfg.Func, SUPLA_CHANNELFNC_VALVE_OPENCLOSE);
  ASSERT_EQ(channelBasicCfg.FuncList, 0);
  ASSERT_EQ(channelBasicCfg.ChannelFlags,
            (unsigned int)SUPLA_CHANNEL_FLAG_CHANNELSTATE);

  char expectedCaption[] = "";
  ASSERT_EQ(strncmp(channelBasicCfg.Caption, expectedCaption,
                    SUPLA_CHANNEL_CAPTION_MAXSIZE),
            0);
}

} /* namespace testing */
