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

#include "OpeningSensorConfigTest.h"

#include "TestHelper.h"
#include "jsonconfig/channel/opening_sensor_config.h"
#include "proto.h"

namespace testing {

OpeningSensorConfigTest::OpeningSensorConfigTest(void) {}

OpeningSensorConfigTest::~OpeningSensorConfigTest(void) {}

TEST_F(OpeningSensorConfigTest, getRelatedChannelId) {
  opening_sensor_config *config = new opening_sensor_config();
  ASSERT_TRUE(config != NULL);

  EXPECT_EQ(config->get_related_channel_id(), 0);

  config->set_user_config("{\"openingSensorChannelId\":null}");

  EXPECT_EQ(config->get_related_channel_id(), 0);

  config->set_user_config("{\"openingSensorChannelId\":12345}");

  EXPECT_EQ(config->get_related_channel_id(), 12345);

  delete config;
}

TEST_F(OpeningSensorConfigTest, getRelatedSecondaryChannelId) {
  opening_sensor_config *config = new opening_sensor_config();
  ASSERT_TRUE(config != NULL);

  EXPECT_EQ(config->get_related_secondary_channel_id(), 0);

  config->set_user_config("{\"openingSensorSecondaryChannelId\":null}");

  EXPECT_EQ(config->get_related_secondary_channel_id(), 0);

  config->set_user_config("{\"openingSensorSecondaryChannelId\":678910}");

  EXPECT_EQ(config->get_related_secondary_channel_id(), 678910);

  delete config;
}

TEST_F(OpeningSensorConfigTest, isInvertedLogic) {
  opening_sensor_config *config = new opening_sensor_config();
  ASSERT_TRUE(config != NULL);

  EXPECT_FALSE(config->is_inverted_logic());

  config->set_user_config("{\"invertedLogic\":false}");

  EXPECT_FALSE(config->is_inverted_logic());

  config->set_user_config("{\"invertedLogic\":true}");

  EXPECT_TRUE(config->is_inverted_logic());

  delete config;
}

} /* namespace testing */
