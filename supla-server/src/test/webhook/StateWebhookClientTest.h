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

#ifndef WEBHOOK_STATEWEBHOOKCLIENTTEST_H_
#define WEBHOOK_STATEWEBHOOKCLIENTTEST_H_

#include "gtest/gtest.h"  // NOLINT
#include "user/user.h"
#include "webhook/statewebhookclient.h"

namespace testing {

class StateWebhookClientTest : public Test {
 public:
  supla_user *user;
  supla_state_webhook_client *client;

  virtual void SetUp();
  virtual void TearDown();
  StateWebhookClientTest();
};

} /* namespace testing */

#endif /* WEBHOOK_STATEWEBHOOKCLIENTTEST_H_ */
