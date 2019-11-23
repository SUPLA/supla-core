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
#include "../client_config.h"
#include "../mqtt/mqtt_client.h"
#include "gtest/gtest.h"

#define MIN_RETRY_TIME 5
#define MIN_CHECK_TIME 1

namespace {

class MqttTest : public ::testing::Test {
 protected:
};

TEST_F(MqttTest, config_parse_test) {
  client_config* config = new client_config();
  ASSERT_TRUE(config->load("../config/config_template.yaml") == true);
  ASSERT_TRUE(config->getMqttProtocolVersion() >= 3 &&
              config->getMqttProtocolVersion() <= 5);
  ASSERT_TRUE(config->getSuplaProtocolVersion() <= 11);

  delete config;
}

}  // namespace
