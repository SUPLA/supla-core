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

#include "GoogleHomeConfigTest.h"

#include "TestHelper.h"
#include "jsonconfig/channel/google_home_config.h"
#include "proto.h"

namespace testing {

GoogleHomeConfigTest::GoogleHomeConfigTest(void) {}

GoogleHomeConfigTest::~GoogleHomeConfigTest(void) {}

TEST_F(GoogleHomeConfigTest, isIntegrationDisabled) {
  google_home_config config;

  EXPECT_FALSE(config.is_integration_disabled());

  config.set_user_config("{\"googleHome\":{\"googleHomeDisabled\":true}}");

  EXPECT_TRUE(config.is_integration_disabled());
}

} /* namespace testing */
