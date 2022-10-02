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

#include "ControllingTheGateConfigTest.h"

#include "TestHelper.h"
#include "channeljsonconfig/controlling_the_gate_config.h"
#include "proto.h"

namespace testing {

ControllingTheGateConfigTest::ControllingTheGateConfigTest(void) {}
ControllingTheGateConfigTest::~ControllingTheGateConfigTest(void) {}

TEST_F(ControllingTheGateConfigTest, getNumberOfAttemptsToClose) {
  controlling_the_gate_config *config = new controlling_the_gate_config();
  ASSERT_TRUE(config != NULL);

  EXPECT_EQ(config->get_number_of_attempts_to_close(), 5);

  config->set_user_config("{\"numberOfAttemptsToClose\":-1}");
  EXPECT_EQ(config->get_number_of_attempts_to_close(), 5);

  config->set_user_config("{\"numberOfAttemptsToClose\":6}");
  EXPECT_EQ(config->get_number_of_attempts_to_close(), 5);

  config->set_user_config("{\"numberOfAttemptsToClose\":1}");
  EXPECT_EQ(config->get_number_of_attempts_to_close(), 1);
  EXPECT_EQ(config->get_number_of_attempts_to_open(), 5);

  config->set_user_config("{\"numberOfAttemptsToClose\":4}");
  EXPECT_EQ(config->get_number_of_attempts_to_close(), 4);

  config->set_user_config("{\"numberOfAttemptsToClose\":5}");
  EXPECT_EQ(config->get_number_of_attempts_to_close(), 5);

  delete config;
}

TEST_F(ControllingTheGateConfigTest, getNumberOfAttemptsToOpen) {
  controlling_the_gate_config *config = new controlling_the_gate_config();
  ASSERT_TRUE(config != NULL);

  EXPECT_EQ(config->get_number_of_attempts_to_open(), 5);

  config->set_user_config("{\"numberOfAttemptsToOpen\":-1}");
  EXPECT_EQ(config->get_number_of_attempts_to_open(), 5);

  config->set_user_config("{\"numberOfAttemptsToOpen\":6}");
  EXPECT_EQ(config->get_number_of_attempts_to_open(), 5);

  config->set_user_config("{\"numberOfAttemptsToOpen\":1}");
  EXPECT_EQ(config->get_number_of_attempts_to_open(), 1);
  EXPECT_EQ(config->get_number_of_attempts_to_close(), 5);

  config->set_user_config("{\"numberOfAttemptsToOpen\":4}");
  EXPECT_EQ(config->get_number_of_attempts_to_open(), 4);

  config->set_user_config("{\"numberOfAttemptsToOpen\":5}");
  EXPECT_EQ(config->get_number_of_attempts_to_open(), 5);

  delete config;
}

TEST_F(ControllingTheGateConfigTest, getRetryIntrrupt) {
  controlling_the_gate_config *config = new controlling_the_gate_config();
  ASSERT_TRUE(config != NULL);

  EXPECT_FALSE(config->get_retry_interrupt());

  config->set_user_config("{\"retryInterrupt\":false}");

  EXPECT_FALSE(config->get_retry_interrupt());

  config->set_user_config("{\"retryInterrupt\":true}");

  EXPECT_TRUE(config->get_retry_interrupt());

  delete config;
}

} /* namespace testing */
