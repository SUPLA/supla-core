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

#include "MqttClientIntegrationTest.h"
#include "log.h" // NOLINT

namespace testing {

MqttClientIntegrationTest::MqttClientIntegrationTest() : IntegrationTest() {}

MqttClientIntegrationTest::~MqttClientIntegrationTest() {}

void MqttClientIntegrationTest::SetUp() {
  iniSettings = new MqttClientSettingsMock();
  libraryAdapter = new MqttClientLibraryAdapterMock(iniSettings);
  dataSource = dsInit(iniSettings);

  client = clientInit(libraryAdapter, iniSettings, dataSource);
  client->start();
}

void MqttClientIntegrationTest::TearDown() {
  client->stop();
  delete client;
  delete dataSource;
  delete iniSettings;
  delete libraryAdapter;
}

supla_mqtt_client *MqttClientIntegrationTest::getClient(void) { return client; }

supla_mqtt_client_datasource *MqttClientIntegrationTest::getDS(void) {
  return dataSource;
}

MqttClientLibraryAdapterMock *MqttClientIntegrationTest::getLibAdapter(void) {
  return libraryAdapter;
}

MqttClientSettingsMock *MqttClientIntegrationTest::getSettings(void) {
  return iniSettings;
}

void MqttClientIntegrationTest::waitForConnection() {
  for (int a = 0; a < 5000; a++) {
    if (libraryAdapter->is_connected()) {
      return;
    }
    usleep(1000);
  }

  ASSERT_TRUE(false);
}

void MqttClientIntegrationTest::waitForPublications(int expectedTopicCount) {
  for (int a = 0; a < 5000; a++) {
    if (getLibAdapter()->published_count() == expectedTopicCount &&
        !getDS()->is_context_open()) {
      return;
    }
    usleep(1000);
  }

  EXPECT_EQ(getLibAdapter()->published_count(), expectedTopicCount);
  EXPECT_TRUE(getDS()->is_context_open());
  ASSERT_TRUE(false);
}

void MqttClientIntegrationTest::print_expected(void) {
  printf("const char *expectedData[] = {\n");
  bool first = true;
  while (getLibAdapter()->published_count()) {
    _mqtt_test_message_t m = getLibAdapter()->published_pop();
    if (first) {
      first = false;
    } else {
      printf(", ");
    }
    printf("%s%s%s, ", m.topic_name ? "\"" : "",
           m.topic_name ? m.topic_name : "NULL", m.topic_name ? "\"" : "");

    if (m.message) {
      printf("\"");

      for (size_t a = 0; a < m.message_size; a++) {
        if (m.message[a] == '"' || m.message[a] == '\\') {
          printf("\\");
        }
        printf("%c", m.message[a]);
      }

      printf("\"");
    } else {
      printf("NULL");
    }

    printf("\n");

    if (m.topic_name) {
      free(m.topic_name);
    }
    if (m.message) {
      free(m.message);
    }
  }
  printf("};\n");
}  // namespace testing

void MqttClientIntegrationTest::verify_published(const char *expectedData[],
                                                 int count) {
  ASSERT_EQ(count / 2, getLibAdapter()->published_count());

  int n = 0;
  bool ok = true;
  while (getLibAdapter()->published_count() && ok) {
    _mqtt_test_message_t m = getLibAdapter()->published_pop();

    if ((m.topic_name != NULL && expectedData[n] == NULL) ||
        (m.topic_name == NULL && expectedData[n] != NULL) ||
        (m.topic_name != NULL && strcmp(m.topic_name, expectedData[n]) != 0)) {
      supla_log(LOG_DEBUG, "TOPIC n==%i|%s|%s", n,
                m.topic_name ? m.topic_name : "NULL",
                expectedData[n] ? expectedData[n] : "NULL");
      EXPECT_TRUE(false);
      ok = false;
    }

    size_t edlen = expectedData[n + 1] ? strlen(expectedData[n + 1]) : 0;

    if (ok &&
        ((m.message != NULL && expectedData[n + 1] == NULL) ||
         (m.message == NULL && expectedData[n + 1] != NULL) ||
         (m.message_size != edlen) ||
         (m.message_size > 0 && strcmp(m.message, expectedData[n + 1]) != 0))) {
      supla_log(LOG_DEBUG, "MESSAGE n==%i|%s|%s", n,
                m.message ? m.message : "NULL",
                expectedData[n + 1] ? expectedData[n + 1] : "NULL");
      EXPECT_TRUE(false);
      ok = false;
    }

    n += 2;
    if (m.topic_name) {
      free(m.topic_name);
    }
    if (m.message) {
      free(m.message);
    }
  }
}

} /* namespace testing */
