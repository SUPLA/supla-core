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

#include "MqttMessageProviderTest.h"

namespace testing {

MqttMessageProviderTest::MqttMessageProviderTest(void) {}
MqttMessageProviderTest::~MqttMessageProviderTest(void) {}

bool MqttMessageProviderTest::dataExists(
    supla_mqtt_message_provider *provider) {
  char *_topic_name = NULL;
  void *_message = NULL;
  size_t _message_size = 0;

  bool result = provider->fetch(NULL, NULL, &_message, &_message_size);

  if (_topic_name) {
    free(_topic_name);
    _topic_name = NULL;
  }

  if (_message) {
    free(_message);
    _message = NULL;
  }

  return result;
}

bool MqttMessageProviderTest::fetchAndCompare(
    supla_mqtt_message_provider *provider, const char *prefix,
    const char *topic_name, const char *message, bool include_zero_byte) {
  char *_topic_name = NULL;
  void *_message = NULL;
  size_t _message_size = 0;

  bool result = provider->fetch(prefix, NULL, &_message, &_message_size);

  EXPECT_FALSE(result);
  EXPECT_TRUE(_message == NULL);
  EXPECT_EQ(_message_size, (size_t)0);

  if (_message) {
    free(_message);
    _message = NULL;
  }

  _message_size = 0;

  result = provider->fetch(prefix, &_topic_name, &_message, &_message_size);

  EXPECT_TRUE(result);
  EXPECT_FALSE(_topic_name == NULL);

  if (_topic_name) {
    if (strcmp(_topic_name, topic_name) != 0) {
      EXPECT_TRUE(false);
      result = false;
    }

    free(_topic_name);
    _topic_name = NULL;
  }

  if (message == NULL && (message != NULL || _message_size > 0)) {
    EXPECT_TRUE(false);
    result = false;
  }

  if (message != NULL && (message == NULL || _message_size == 0)) {
    EXPECT_TRUE(false);
    result = false;
  }

  if (message) {
    size_t message_size = strlen(message);
    if (include_zero_byte) {
      message_size++;
    }

    if (message_size != _message_size ||
        (_message_size > 0 && memcmp(message, _message, _message_size))) {
      EXPECT_TRUE(false);
    }
  }

  if (_message) {
    free(_message);
    _message = NULL;
  }

  return result;
}

} /* namespace testing */
