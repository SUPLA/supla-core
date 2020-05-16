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

#include "ConnectionIntegrationTest.h"

namespace testing {

ConnectionIntegrationTest::ConnectionIntegrationTest() {
  snprintf(host, sizeof(host), "127.0.0.1");
  sslEnabled = true;
  tcpPort = 2015;
  sslPort = 205;
  waitingForConnectionEstablishedSuccessfully = false;
  expectedCode = 0;
}

ConnectionIntegrationTest::~ConnectionIntegrationTest() {}

void ConnectionIntegrationTest::beforeClientInit(TSuplaClientCfg *scc) {
  scc->ssl_enabled = sslEnabled;
  scc->host = host;
  scc->tcp_port = tcpPort;
  scc->ssl_port = sslPort;
}

void ConnectionIntegrationTest::onConnected() {
  ASSERT_TRUE(waitingForConnectionEstablishedSuccessfully);
}

void ConnectionIntegrationTest::onConnectionError(int code) {
  ASSERT_TRUE(!waitingForConnectionEstablishedSuccessfully);
  ASSERT_EQ(code, expectedCode);
}

TEST_F(ConnectionIntegrationTest, ConnectWithSSLToBadPort) {
  sslPort = 2010;
  expectedCode = SUPLA_RESULTCODE_CANTCONNECTTOHOST;
  iterateUntilDefaultTimeout();
}

TEST_F(ConnectionIntegrationTest, ConnectWithoutSSLToBadPort) {
  tcpPort = 2010;
  sslEnabled = false;
  expectedCode = SUPLA_RESULTCODE_CANTCONNECTTOHOST;
  iterateUntilDefaultTimeout();
}

TEST_F(ConnectionIntegrationTest, ConnectToNonexistentHost) {
  expectedCode = SUPLA_RESULTCODE_HOSTNOTFOUND;
  snprintf(host, sizeof(host), "nonexistent.domain");
  iterateUntilDefaultTimeout();
}
} /* namespace testing */
