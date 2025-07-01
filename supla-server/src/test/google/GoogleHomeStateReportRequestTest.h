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

#ifndef GOOGLE_HOME_STATE_REPORT_REQUEST_TEST_H_
#define GOOGLE_HOME_STATE_REPORT_REQUEST_TEST_H_

#include <string>

#include "asynctask/AsyncTaskTest.h"
#include "device/extended_value/abstract_channel_extended_value.h"
#include "device/value/abstract_channel_value.h"
#include "doubles/device/ChannelPropertyGetterMock.h"
#include "doubles/google/GoogleHomeCredentialsMock.h"
#include "doubles/http/AccessTokenProviderMock.h"
#include "doubles/http/CurlAdapterMock.h"

namespace testing {

class GoogleHomeStateReportRequestTest : public AsyncTaskTest {
 protected:
  GoogleHomeCredentialsMock credentials;
  CurlAdapterMock *curlAdapter;
  ChannelPropertyGetterMock *propertyGetter;
  CurlAdapterMock *tokenProviderCurlAdapter;
  AccessTokenProviderMock *tokenProvider;
  void expectToken(bool direct);

 public:
  virtual void SetUp(void);
  virtual void TearDown(void);
  void makeTest(int func, bool online, supla_abstract_channel_value *value,
                const char *expectedPayload, const std::string &request_id,
                bool direct);
  void makeTest(int func, bool online, supla_abstract_channel_value *value,
                const char *expectedPayload);
  void makeHvacThermostatTest(
      int func, bool online, supla_abstract_channel_value *hvacValue,
      supla_abstract_channel_value *tempHumValue,
      supla_abstract_channel_extended_value *extendedValue,
      const char *expectedPayload, const std::string &request_id, bool direct);
};

} /* namespace testing */

#endif /* GOOGLE_HOME_STATE_REPORT_REQUEST_TEST_H_ */
