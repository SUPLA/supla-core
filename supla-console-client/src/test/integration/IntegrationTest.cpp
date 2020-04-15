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

#include "IntegrationTest.h"
#include "log.h"

namespace testing {

void integration_test_on_connected(void *_suplaclient, void *instance) {
  static_cast<IntegrationTest *>(instance)->onConnected();
}

void integration_test_on_disconnected(void *_suplaclient, void *instance) {
  static_cast<IntegrationTest *>(instance)->onDisconnected();
}

void integration_test_on_connerror(void *_suplaclient, void *instance,
                                   int code) {
  static_cast<IntegrationTest *>(instance)->onConnectionError(code);
}

void integration_test_on_registererror(void *_suplaclient, void *instance,
                                       int code) {
  static_cast<IntegrationTest *>(instance)->onRegistrationError(code);
}

IntegrationTest::IntegrationTest() {
  iterationCancelled = false;
  sclient = NULL;
}

IntegrationTest::~IntegrationTest() { clientFree(); }

void IntegrationTest::clientInit() {
  if (sclient != NULL) {
    return;
  }

  TSuplaClientCfg scc;
  supla_client_cfginit(&scc);

  scc.protocol_version = SUPLA_PROTO_VERSION;

  snprintf(scc.Name, SUPLA_CLIENT_NAME_MAXSIZE, "Console client test");
  snprintf(scc.SoftVer, SUPLA_SOFTVER_MAXSIZE, "1.0-Linux");
  snprintf(scc.Email, SUPLA_EMAIL_MAXSIZE, "test@supla.org");

  fillArrayWithOrdinalNumbers(scc.clientGUID, SUPLA_GUID_SIZE, 0);
  fillArrayWithOrdinalNumbers(scc.AuthKey, SUPLA_AUTHKEY_SIZE, 0);

  char host[] = "127.0.0.1";
  scc.user_data = this;
  scc.host = host;
  scc.tcp_port = 2015;
  scc.ssl_port = 2016;
  scc.ssl_enabled = true;

  scc.cb_on_connected = &integration_test_on_connected;
  scc.cb_on_connerror = &integration_test_on_connerror;
  scc.cb_on_registererror = &integration_test_on_registererror;

  beforeClientInit(&scc);
  sclient = supla_client_init(&scc);
}

void IntegrationTest::fillArrayWithOrdinalNumbers(char *arr, int arr_size,
                                                  char start) {
  for (int a = 0; a < arr_size; a++) {
    arr[a] = start + a;
  }
}

void IntegrationTest::clientFree() {
  if (sclient != NULL) {
    supla_client_free(sclient);
    sclient = NULL;
  }
}

void IntegrationTest::iterateUntilTimeout(unsigned int timeoutMS) {
  ASSERT_GT(timeoutMS, 0);
  iterationCancelled = false;

  clientFree();
  clientInit();

  struct timeval now;
  gettimeofday(&now, NULL);
  unsigned int start_ms = now.tv_sec * 1000 + now.tv_usec / 1000;

  if (0 == supla_client_connect(sclient)) {
    return;
  }

  while (!iterationCancelled) {
    supla_client_iterate(sclient, 10);

    gettimeofday(&now, NULL);
    unsigned int time_diff = now.tv_sec * 1000 + now.tv_usec / 1000 - start_ms;
    ASSERT_LT(time_diff, timeoutMS);

    usleep(1000);
  }
}

void IntegrationTest::iterateUntilDefaultTimeout() {
  iterateUntilTimeout(5000);
}

void IntegrationTest::cancelIteration(void) { iterationCancelled = true; }

void IntegrationTest::beforeClientInit(TSuplaClientCfg *scc) {}

void IntegrationTest::onConnected() {}

void IntegrationTest::onDisconnected() {}

void IntegrationTest::onConnectionError(int code) { ASSERT_TRUE(false); }

void IntegrationTest::onRegistrationError(int code) {}

} /* namespace testing */
