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

#ifndef ALEXA_RESPONSE_REQUEST_TEST_H_
#define ALEXA_RESPONSE_REQUEST_TEST_H_

#include <string>

#include "asynctask/AsyncTaskTest.h"
#include "caller.h"
#include "device/value/channel_value.h"
#include "doubles/amazon/AlexaCredentialsMock.h"
#include "doubles/device/ChannelPropertyGetterMock.h"
#include "doubles/http/CurlAdapterMock.h"

namespace testing {

class AlexaResponseRequestTest : public AsyncTaskTest {
 protected:
  AlexaCredentialsMock credentials;
  CurlAdapterMock *curlAdapter;
  ChannelPropertyGetterMock *propertyGetter;

 public:
  virtual void SetUp(void);
  void makeTest(int func, bool online, supla_channel_value *value,
                const char *expectedPayload,
                const std::string &correlation_token);

  void makeTest(int func, bool online, supla_channel_value *value,
                const char *expectedPayload);
};

} /* namespace testing */

#endif /* ALEXA_RESPONSE_REQUEST_TEST_H_ */
