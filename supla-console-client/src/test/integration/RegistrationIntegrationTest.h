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

#ifndef REGISTRATIONINTEGRATIONTEST_H_
#define REGISTRATIONINTEGRATIONTEST_H_

#include "IntegrationTest.h"

namespace testing {

class RegistrationIntegrationTest : public IntegrationTest {
 protected:
  char GUID[SUPLA_GUID_SIZE];
  char AuthKey[SUPLA_AUTHKEY_SIZE];
  char Email[SUPLA_EMAIL_MAXSIZE];
  char Password[SUPLA_PASSWORD_MAXSIZE];
  bool sslEnabled;
  int expectedRegistrationErrorCode;
  int AccessID;
  char AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE];
  int ProtocolVersion;

 public:
  RegistrationIntegrationTest();
  virtual ~RegistrationIntegrationTest();
  virtual void beforeClientInit(TSuplaClientCfg *scc);
  virtual void onRegistered(TSC_SuplaRegisterClientResult_B *result);
  virtual void onRegistrationError(int code);
  virtual void onChannelFunctionSetResult(TSC_SetChannelFunctionResult *result);
};

} /* namespace testing */

#endif /* REGISTRATIONINTEGRATIONTEST_H_ */
