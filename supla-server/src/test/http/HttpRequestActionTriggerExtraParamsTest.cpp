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

#include "HttpRequestActionTriggerExtraParamsTest.h"

#include "http/httprequestactiontriggerextraparams.h"
#include "proto.h"

namespace testing {

HttpRequestActionTriggerExtraParamsTest::
    HttpRequestActionTriggerExtraParamsTest(void)
    : Test() {}
HttpRequestActionTriggerExtraParamsTest::
    ~HttpRequestActionTriggerExtraParamsTest(void) {}

TEST_F(HttpRequestActionTriggerExtraParamsTest, actions) {
  supla_http_request_action_trigger_extra_params *params =
      new supla_http_request_action_trigger_extra_params();
  ASSERT_TRUE(params != NULL);
  EXPECT_EQ(params->getActions(), 0);
  delete params;

  params = new supla_http_request_action_trigger_extra_params(
      SUPLA_ACTION_CAP_TURN_ON | SUPLA_ACTION_CAP_TURN_OFF);
  EXPECT_TRUE(params != NULL);
  if (params) {
    EXPECT_EQ(params->getActions(),
              SUPLA_ACTION_CAP_TURN_ON | SUPLA_ACTION_CAP_TURN_OFF);
    delete params;
  }
}

} /* namespace testing */
