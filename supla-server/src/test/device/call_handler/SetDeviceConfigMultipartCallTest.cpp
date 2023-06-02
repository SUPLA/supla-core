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

#include "SetDeviceConfigMultipartCallTest.h"

#include "TestHelper.h"
#include "device/call_handler/set_device_config_multipart_call.h"

namespace testing {

SetDeviceConfigMultipartCallTest::SetDeviceConfigMultipartCallTest() : Test() {}

SetDeviceConfigMultipartCallTest::~SetDeviceConfigMultipartCallTest() {}

TEST_F(SetDeviceConfigMultipartCallTest, pushPop) {
  TSDS_SetDeviceConfig config1 = {};
  TestHelper::randomize((char*)&config1, sizeof(TSDS_SetDeviceConfig));
  config1.ConfigSize = SUPLA_DEVICE_CONFIG_MAXSIZE;

  supla_set_device_multipart_call call;
  call.part_push(&config1);

  TSDS_SetDeviceConfig config2 = {};
  EXPECT_TRUE(call.part_pop(&config2));
  EXPECT_EQ(memcmp(&config1, &config2, sizeof(TSDS_SetDeviceConfig)), 0);
}

TEST_F(SetDeviceConfigMultipartCallTest, getCallId) {
  supla_set_device_multipart_call call;
  EXPECT_EQ(call.get_call_id(), SUPLA_DS_CALL_SET_DEVICE_CONFIG);
}

}  // namespace testing
