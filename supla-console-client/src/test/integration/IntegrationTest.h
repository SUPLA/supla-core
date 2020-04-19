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

#ifndef INTEGRATIONTEST_H_
#define INTEGRATIONTEST_H_

#include "gtest/gtest.h"
#include "supla-client.h"

#define TESTUSER_EMAIL "test@supla.org"
#define TESTUSER_PASSWD "supla!test"

namespace testing {

class IntegrationTest : public Test {
 private:
  static char *sqlDir;
  static char *dbName;
  static char *dbHost;
  static char *dbUser;
  static char defaultDbName[];
  static char defaultDbHost[];
  static char defaultDbUser[];
  bool iterationCancelled;

  void clientInit();
  void clientFree();

 protected:
  void *sclient;
  void iterateUntilTimeout(bool doRegister, unsigned int timeoutMS);
  void iterateUntilTimeout(unsigned int timeoutMS);
  void iterateUntilDefaultTimeout();
  void cancelIteration(void);
  void reconnect();
  void fillArrayWithOrdinalNumbers(char *arr, int arr_size, char start);
  virtual void beforeClientInit(TSuplaClientCfg *scc);
  void runSqlScript(const char *script);
  void initTestDatabase();

 public:
  static void Init(int argc, char **argv);
  IntegrationTest();
  virtual ~IntegrationTest();
  virtual void onConnected();
  virtual void onDisconnected();
  virtual void onConnectionError(int code);
  virtual void onRegistered(TSC_SuplaRegisterClientResult_B *result);
  virtual void onRegistrationError(int code);
  virtual void onSuperuserAuthorizationResult(bool authorized, int code);
  virtual void onChannelFunctionSetResult(TSC_SetChannelFunctionResult *result);
  virtual void onChannelCaptionSetResult(TSC_SetChannelCaptionResult *result);
  virtual void onChannelBasicCfg(TSC_ChannelBasicCfg *cfg);
  virtual void channelUpdate(TSC_SuplaChannel_C *channel);
  virtual void onRegistrationEnabled(TSDC_RegistrationEnabled *reg_enabled);
  virtual void onSetRegistrationEnabledResult(
      TSC_SetRegistrationEnabledResult *result);
  virtual void onGetVersionResult(TSDC_SuplaGetVersionResult *result);
  virtual void onOAuthTokenRequestResult(TSC_OAuthTokenRequestResult *result);
};

} /* namespace testing */

#endif /* INTEGRATIONTEST_H_ */
