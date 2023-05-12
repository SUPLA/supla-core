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

#ifndef STATE_WEBHOOK_REQUEST_TEST_H_
#define STATE_WEBHOOK_REQUEST_TEST_H_

#include "asynctask/AsyncTaskTest.h"
#include "device/extended_value/channel_extended_value.h"
#include "device/value/channel_value.h"
#include "doubles/device/ChannelPropertyGetterMock.h"
#include "doubles/http/CurlAdapterMock.h"
#include "doubles/webhook/StateWebhookCredentialsMock.h"

namespace testing {

class StateWebhookRequestTest : public AsyncTaskTest {
 protected:
  StateWebhookCredentialsMock credentials;
  ChannelPropertyGetterMock *propertyGetter;
  CurlAdapterMock *curlAdapter;
  void makeTest(int func, bool online, supla_channel_value *value,
                const char *expectedPayload);
  void makeTest(int func, bool online,
                supla_channel_extended_value *extended_value,
                const char *expectedPayload);
  void makeTest(int func, bool online,
                supla_channel_electricity_measurement *em,
                const char *expectedPayload);

 public:
  virtual void SetUp(void);
};

} /* namespace testing */

#endif /* STATE_WEBHOOK_REQUEST_TEST_H_ */
