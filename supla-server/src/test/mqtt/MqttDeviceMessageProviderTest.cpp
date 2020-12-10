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

#include "MqttDeviceMessageProviderTest.h"

namespace testing {

MqttDeviceMessageProviderTest::MqttDeviceMessageProviderTest(void)
    : MqttMessageProviderTest() {}
MqttDeviceMessageProviderTest::~MqttDeviceMessageProviderTest(void) {}

void MqttDeviceMessageProviderTest::SetUp() {
  provider = new supla_mqtt_device_message_provider();
}

void MqttDeviceMessageProviderTest::TearDown() { delete provider; }

void MqttDeviceMessageProviderTest::fillDeviceData(
    _mqtt_db_data_row_device_t *row_device) {
  memset(row_device, 0, sizeof(_mqtt_db_data_row_device_t));

  snprintf(row_device->user_email, SUPLA_EMAIL_MAXSIZE, "user@supla.org");
  row_device->device_id = 555;
  row_device->device_enabled = true;
  snprintf(row_device->device_location, SUPLA_LOCATION_CAPTION_MAXSIZE,
           "First floor");
  snprintf(row_device->device_last_connected, DATE_STRING_MAXSIZE,
           "2020-11-21T21:30:42Z");
  snprintf(row_device->device_last_ipv4, IPV4_STRING_MAXSIZE,
           "179.110.192.238");
  row_device->device_mfr_id = SUPLA_MFR_ACSOFTWARE;
  snprintf(row_device->device_name, SUPLA_DEVICE_NAME_MAXSIZE,
           "GATE CONTROLLER");
  row_device->device_proto_version = 12;
  snprintf(row_device->device_softver, SUPLA_SOFTVER_MAXSIZE, "2.7.23");
}

TEST_F(MqttDeviceMessageProviderTest, fetchAll) {
  _mqtt_db_data_row_device_t row_device;
  fillDeviceData(&row_device);
  provider->set_data_row(&row_device);

  ASSERT_TRUE(fetchAndCompare(provider, NULL, "true", false,
                              "supla/user@supla.org/devices/%i/enabled",
                              row_device.device_id));

  ASSERT_TRUE(fetchAndCompare(provider, NULL, "2020-11-21T21:30:42Z", false,
                              "supla/user@supla.org/devices/%i/last_connected",
                              row_device.device_id));

  ASSERT_TRUE(fetchAndCompare(provider, NULL, "179.110.192.238", false,
                              "supla/user@supla.org/devices/%i/last_ipv4",
                              row_device.device_id));

  ASSERT_TRUE(fetchAndCompare(provider, NULL, "AC SOFTWARE", false,
                              "supla/user@supla.org/devices/%i/manufacturer",
                              row_device.device_id));

  ASSERT_TRUE(fetchAndCompare(provider, NULL, "GATE CONTROLLER", false,
                              "supla/user@supla.org/devices/%i/name",
                              row_device.device_id));

  ASSERT_TRUE(fetchAndCompare(provider, NULL, "12", false,
                              "supla/user@supla.org/devices/%i/proto_ver",
                              row_device.device_id));

  ASSERT_TRUE(fetchAndCompare(provider, NULL, "2.7.23", false,
                              "supla/user@supla.org/devices/%i/soft_ver",
                              row_device.device_id));

  ASSERT_FALSE(dataExists(provider));
}

} /* namespace testing */
