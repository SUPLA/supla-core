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

#include "MqttPublisherIntegrationTest.h"

#include "log.h"  // NOLINT
#include "mqtt_publisher.h"
#include "mqtt_publisher_datasource.h"

namespace testing {

MqttPublisherIntegrationTest::MqttPublisherIntegrationTest()
    : MqttClientTest(), IntegrationTest() {}

MqttPublisherIntegrationTest::~MqttPublisherIntegrationTest() {}

supla_mqtt_client *MqttPublisherIntegrationTest::clientInit(
    supla_mqtt_client_library_adapter *library_adapter,
    supla_mqtt_client_settings *settings,
    supla_mqtt_client_datasource *datasource) {
  return new supla_mqtt_publisher(library_adapter, settings, datasource);
}

supla_mqtt_client_datasource *MqttPublisherIntegrationTest::dsInit(
    supla_mqtt_client_settings *settings) {
  return new supla_mqtt_publisher_datasource(settings);
}

void MqttPublisherIntegrationTest::waitForActions(int expectedTopicCount) {
  int n = 5000;
  while (getLibAdapter()->published_count() != expectedTopicCount && n > 0) {
    n--;
    usleep(1000);
  }

  usleep(200000);

  EXPECT_EQ(getLibAdapter()->published_count(), expectedTopicCount);
}

void MqttPublisherIntegrationTest::SetUp() {
  initTestDatabase();
  runSqlScript("DataForMqttTests.sql");

  MqttClientTest::SetUp();
}

TEST_F(MqttPublisherIntegrationTest, fullScope) {
  waitForConnection();
  waitForPublications(690);
  // print_expected();

  const char *expectedData[] = {
      "supla/8ce92cb8c9f6db6b65703d2703691700/account/timezone",
      "Europe/Rome",
      "supla/8ce92cb8c9f6db6b65703d2703691700/account/email",
      "48test@supla.org",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/account/timezone",
      "Europe/Warsaw",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/account/email",
      "53test@supla.org",
      "supla/5b15b00055c044220196eb41965b3627/account/timezone",
      "Europe/Warsaw",
      "supla/5b15b00055c044220196eb41965b3627/account/email",
      "117test@supla.org",
      "supla/45004b208185b207175522c7471f8526/account/timezone",
      "Europe/Warsaw",
      "supla/45004b208185b207175522c7471f8526/account/email",
      "121test@supla.org",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/account/timezone",
      "Europe/Warsaw",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/account/email",
      "171test@supla.org",
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/account/timezone",
      "Europe/Warsaw",
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/account/email",
      "239test@supla.org",
      "supla/de1257b7824b6825c276b763db1b711f/account/timezone",
      "Europe/Warsaw",
      "supla/de1257b7824b6825c276b763db1b711f/account/email",
      "390test@supla.org",
      "supla/b72dfbdb402908e6939d7257dfed2367/account/timezone",
      "Europe/Warsaw",
      "supla/b72dfbdb402908e6939d7257dfed2367/account/email",
      "431test@supla.org",
      "supla/3281677315ff159f91471b34828f603b/account/timezone",
      "Europe/Paris",
      "supla/3281677315ff159f91471b34828f603b/account/email",
      "632test@supla.org",
      "supla/46948d474db1b5e3cccf2c89281414c2/account/timezone",
      "Europe/Warsaw",
      "supla/46948d474db1b5e3cccf2c89281414c2/account/email",
      "676test@supla.org",
      "supla/0df527b1786040fe57d6d9f43b632fbb/account/timezone",
      "Europe/Warsaw",
      "supla/0df527b1786040fe57d6d9f43b632fbb/account/email",
      "743test@supla.org",
      "supla/c2e6c948d570bfb4e8b7152d890937db/account/timezone",
      "Europe/Warsaw",
      "supla/c2e6c948d570bfb4e8b7152d890937db/account/email",
      "1009test@supla.org",
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/account/timezone",
      "Europe/Warsaw",
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/account/email",
      "1017test@supla.org",
      "supla/7450214b8189dc129301c2c922cdcfd0/account/timezone",
      "Europe/Warsaw",
      "supla/7450214b8189dc129301c2c922cdcfd0/account/email",
      "1238test@supla.org",
      "supla/bd6287c398bc81403c643410f5043d4e/account/timezone",
      "Europe/Warsaw",
      "supla/bd6287c398bc81403c643410f5043d4e/account/email",
      "1347test@supla.org",
      "supla/689eb3922683b4e4e7b9227f32f2d89b/account/timezone",
      "Europe/Warsaw",
      "supla/689eb3922683b4e4e7b9227f32f2d89b/account/email",
      "1414test@supla.org",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/account/timezone",
      "Europe/Warsaw",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/account/email",
      "1495test@supla.org",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/account/timezone",
      "Europe/Budapest",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/account/email",
      "1645test@supla.org",
      "supla/b9802383dd06c57d0c41de6805957d0f/account/timezone",
      "Europe/Prague",
      "supla/b9802383dd06c57d0c41de6805957d0f/account/email",
      "1792test@supla.org",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/account/timezone",
      "Europe/Warsaw",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/account/email",
      "1802test@supla.org",
      "supla/93158d9b795bb1c65e79199d42381706/account/timezone",
      "Europe/Warsaw",
      "supla/93158d9b795bb1c65e79199d42381706/account/email",
      "1840test@supla.org",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/account/timezone",
      "Europe/Warsaw",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/account/email",
      "1899test@supla.org",
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/account/timezone",
      "Europe/Warsaw",
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/account/email",
      "2288test@supla.org",
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/account/timezone",
      "Europe/Warsaw",
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/account/email",
      "2337test@supla.org",
      "supla/7e2182788b9214ee5b66349d5b7668cf/account/timezone",
      "Europe/Warsaw",
      "supla/7e2182788b9214ee5b66349d5b7668cf/account/email",
      "2487test@supla.org",
      "supla/6527881802ebd8c4ff2b3955bc326704/account/timezone",
      "Europe/Warsaw",
      "supla/6527881802ebd8c4ff2b3955bc326704/account/email",
      "2645test@supla.org",
      "supla/15cb17b568de00e1ddc3005e4674bbde/account/timezone",
      "Europe/Warsaw",
      "supla/15cb17b568de00e1ddc3005e4674bbde/account/email",
      "3411test@supla.org",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/account/timezone",
      "Europe/Warsaw",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/account/email",
      "3741test@supla.org",
      "supla/aae5fd3b528033b98e09e0d1582d1031/account/timezone",
      "Europe/Warsaw",
      "supla/aae5fd3b528033b98e09e0d1582d1031/account/email",
      "3748test@supla.org",
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/enabled",
      "true",
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/last_connected",
      "2019-09-13T13:42:00Z",
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/last_ipv4",
      "62.196.143.138",
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/manufacturer",
      "Zamel",
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/name",
      "ZAMEL SRW-01",
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/proto_ver",
      "7",
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/soft_ver",
      "2.7.2",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/enabled",
      "true",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/last_connected",
      "2020-11-16T13:25:47Z",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/last_ipv4",
      "91.192.145.44",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/manufacturer",
      "Itead",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/name",
      "SONOFF-DS18B20",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/proto_ver",
      "7",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/soft_ver",
      "2.5.3",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/enabled",
      "true",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/last_connected",
      "2020-11-15T13:53:01Z",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/last_ipv4",
      "91.192.145.44",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/manufacturer",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/name",
      "Impulse counter RB-03",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/proto_ver",
      "10",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/soft_ver",
      "2.7.9",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/enabled",
      "true",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/last_connected",
      "2020-11-13T18:04:10Z",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/last_ipv4",
      "178.43.12.82",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/manufacturer",
      "Zamel",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/name",
      "ZAMEL ROW-02",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/proto_ver",
      "9",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/soft_ver",
      "2.7.0",
      "supla/45004b208185b207175522c7471f8526/devices/40/enabled",
      "true",
      "supla/45004b208185b207175522c7471f8526/devices/40/last_connected",
      "2019-09-17T16:33:44Z",
      "supla/45004b208185b207175522c7471f8526/devices/40/last_ipv4",
      "5.173.144.227",
      "supla/45004b208185b207175522c7471f8526/devices/40/manufacturer",
      "Itead",
      "supla/45004b208185b207175522c7471f8526/devices/40/name",
      "SONOFF-TH16",
      "supla/45004b208185b207175522c7471f8526/devices/40/proto_ver",
      "7",
      "supla/45004b208185b207175522c7471f8526/devices/40/soft_ver",
      "2.5.3",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/enabled",
      "true",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/last_connected",
      "2020-11-16T15:03:18Z",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/last_ipv4",
      "77.112.78.126",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/manufacturer",
      "Zamel",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/name",
      "ZAMEL SBW-01",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/proto_ver",
      "9",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/soft_ver",
      "2.7.0",
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/enabled",
      "true",
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/last_connected",
      "2020-11-14T17:14:21Z",
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/last_ipv4",
      "85.14.75.96",
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/manufacturer",
      NULL,
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/name",
      "*Yunshan",
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/proto_ver",
      "7",
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/soft_ver",
      "2.7.2",
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/enabled",
      "true",
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/last_connected",
      "2020-11-13T07:18:34Z",
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/last_ipv4",
      "83.22.38.123",
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/manufacturer",
      NULL,
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/name",
      "SUPLA-GATE-MODULE",
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/proto_ver",
      "7",
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/soft_ver",
      "2.5.5",
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/enabled",
      "true",
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/last_connected",
      "2020-11-15T12:06:14Z",
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/last_ipv4",
      "31.63.6.250",
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/manufacturer",
      NULL,
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/name",
      "Eco-pea level sensor",
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/proto_ver",
      "9",
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/soft_ver",
      "1.6.1",
      "supla/3281677315ff159f91471b34828f603b/devices/563/enabled",
      "true",
      "supla/3281677315ff159f91471b34828f603b/devices/563/last_connected",
      "2020-11-10T00:28:29Z",
      "supla/3281677315ff159f91471b34828f603b/devices/563/last_ipv4",
      "178.19.179.238",
      "supla/3281677315ff159f91471b34828f603b/devices/563/manufacturer",
      "Zamel",
      "supla/3281677315ff159f91471b34828f603b/devices/563/name",
      "ZAMEL SBW-01",
      "supla/3281677315ff159f91471b34828f603b/devices/563/proto_ver",
      "9",
      "supla/3281677315ff159f91471b34828f603b/devices/563/soft_ver",
      "2.7.0",
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/enabled",
      "true",
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/last_connected",
      "2020-11-14T03:45:14Z",
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/last_ipv4",
      "185.153.35.49",
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/manufacturer",
      "Zamel",
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/name",
      "ZAMEL SLW-02",
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/proto_ver",
      "7",
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/soft_ver",
      "2.7.2",
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/enabled",
      "true",
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/last_connected",
      "2020-11-14T20:51:01Z",
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/last_ipv4",
      "79.162.2.218",
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/manufacturer",
      "Zamel",
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/name",
      "ZAMEL MEW-01",
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/proto_ver",
      "10",
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/soft_ver",
      "2.7.8",
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/enabled",
      "true",
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/last_connected",
      "2020-11-15T21:53:54Z",
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/last_ipv4",
      "185.204.193.254",
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/manufacturer",
      "Zamel",
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/name",
      "ZAMEL SLW-01",
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/proto_ver",
      "7",
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/soft_ver",
      "2.5.5",
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/enabled",
      "true",
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/last_connected",
      "2020-06-20T10:09:15Z",
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/last_ipv4",
      "185.253.235.175",
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/manufacturer",
      NULL,
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/name",
      "BME280",
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/proto_ver",
      "9",
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/soft_ver",
      "2.0.0",
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/enabled",
      "true",
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/last_connected",
      "2020-11-13T07:35:35Z",
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/last_ipv4",
      "78.133.163.203",
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/manufacturer",
      "Heatpol",
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/name",
      "HEATPOL HOME+",
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/proto_ver",
      "11",
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/soft_ver",
      "2.7.25",
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/enabled",
      "true",
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/last_connected",
      "2020-11-16T08:27:10Z",
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/last_ipv4",
      "31.0.80.46",
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/manufacturer",
      NULL,
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/name",
      "HOME",
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/proto_ver",
      "9",
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/soft_ver",
      "1.6.1",
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/enabled",
      "true",
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/last_connected",
      "2019-10-20T12:04:29Z",
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/last_ipv4",
      "83.6.48.24",
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/manufacturer",
      NULL,
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/name",
      "WEMOS-ROLLER-SHUTTER-UNI by@Zybi",
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/proto_ver",
      "7",
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/soft_ver",
      "2.5.3",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/enabled",
      "true",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/last_connected",
      "2020-11-16T14:59:31Z",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/last_ipv4",
      "5.172.236.246",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/manufacturer",
      NULL,
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/name",
      "SUPLA-GATE-MODULE",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/proto_ver",
      "7",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/soft_ver",
      "2.5.5",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/enabled",
      "true",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/last_connected",
      "2020-11-14T01:04:57Z",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/last_ipv4",
      "109.196.34.173",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/manufacturer",
      "Itead",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/name",
      "SONOFF-p4-DHT22 by@Zybi",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/proto_ver",
      "5",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/soft_ver",
      "2.0",
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/enabled",
      "true",
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/last_connected",
      "2020-11-16T04:27:38Z",
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/last_ipv4",
      "80.251.245.152",
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/manufacturer",
      "Peveko",
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/name",
      "SMART VALVE PEVEKO",
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/proto_ver",
      "11",
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/soft_ver",
      "2.7.18",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/enabled",
      "true",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/last_connected",
      "2020-11-11T12:41:52Z",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/last_ipv4",
      "188.137.114.68",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/manufacturer",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/name",
      "IMPULSE COUNTER",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/proto_ver",
      "10",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/soft_ver",
      "2.7.2",
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/enabled",
      "true",
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/last_connected",
      "2020-11-16T01:00:36Z",
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/last_ipv4",
      "31.0.69.93",
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/manufacturer",
      "Zamel",
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/name",
      "ZAMEL SBW-02",
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/proto_ver",
      "7",
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/soft_ver",
      "2.7.13",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/enabled",
      "true",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/last_connected",
      "2020-09-27T08:31:10Z",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/last_ipv4",
      "195.248.246.12",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/manufacturer",
      "Itead",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/name",
      "SONOFF-gate-p5-DS18B20 by@Zybi",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/proto_ver",
      "5",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/soft_ver",
      "2.0",
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/enabled",
      "false",
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/last_connected",
      "2019-11-05T16:39:33Z",
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/last_ipv4",
      "80.48.183.94",
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/manufacturer",
      NULL,
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/name",
      "Supla-Gate",
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/proto_ver",
      "9",
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/soft_ver",
      "1.6.1",
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/enabled",
      "true",
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/last_connected",
      "2020-11-16T09:31:56Z",
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/last_ipv4",
      "155.133.49.157",
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/manufacturer",
      NULL,
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/name",
      "GATE",
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/proto_ver",
      "12",
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/soft_ver",
      "2.3.2/1.11 @lukfud",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/enabled",
      "true",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/last_connected",
      "2020-11-11T16:23:05Z",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/last_ipv4",
      "90.156.9.222",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/manufacturer",
      NULL,
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/name",
      "SUPLA-Skalar",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/proto_ver",
      "9",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/soft_ver",
      "1.6.1",
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/enabled",
      "true",
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/last_connected",
      "2019-11-05T11:41:37Z",
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/last_ipv4",
      "77.252.61.52",
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/manufacturer",
      NULL,
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/name",
      "8xDSv3.1 by@wojtek",
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/proto_ver",
      "9",
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/soft_ver",
      "1.6.1",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/enabled",
      "true",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/last_connected",
      "2020-11-16T14:59:02Z",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/last_ipv4",
      "51.38.129.82",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/manufacturer",
      NULL,
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/name",
      "Weather",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/proto_ver",
      "10",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/soft_ver",
      "1.0.4",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/enabled",
      "true",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/last_connected",
      "2020-11-16T06:07:12Z",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/last_ipv4",
      "185.210.38.89",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/manufacturer",
      "Itead",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/name",
      "SONOFF-SV by @slawek",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/proto_ver",
      "7",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/soft_ver",
      "2.5.5s",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/enabled",
      "true",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/last_connected",
      "2022-01-21T14:30:28Z",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/last_ipv4",
      "155.133.49.157",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/manufacturer",
      NULL,
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/name",
      "ZAMEL RNW-01",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/proto_ver",
      "17",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/soft_ver",
      "2.8.47",
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/channels/20/type",
      "RELAY",
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/channels/20/function",
      "CONTROLLINGTHEROLLERSHUTTER",
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/channels/20/caption",
      NULL,
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/channels/20/hidden",
      "false",
      "homeassistant/cover/8ce92cb8c9f6db6b65703d2703691700/20/config",
      "{\"~\":\"supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/channels/"
      "20\",\"device\":{\"ids\":\"supla-iodevice-12\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL SRW-01\",\"sw\":\"2.7.2\"},\"name\":\"Roller shutter "
      "operation\",\"uniq_id\":\"supla_20\",\"qos\":0,\"ret\":false,\"opt\":"
      "false,\"cmd_t\":\"~/"
      "execute_action\",\"dev_cla\":\"shutter\",\"pl_open\":\"REVEAL\",\"pl_"
      "cls\":\"SHUT\",\"pl_stop\":\"STOP\",\"set_pos_t\":\"~/set/"
      "closing_percentage\",\"pos_t\":\"~/state/"
      "shut\",\"pos_open\":0,\"pos_clsd\":100,\"avty_t\":\"~/state/"
      "connected\",\"pl_avail\":\"true\",\"pl_not_avail\":\"false\",\"pos_"
      "tpl\":\"{% if value is defined %}{% if value | int < 0 %}0{% elif value "
      "| int > 100 %}100{% else %}{{value | int}}{% endif %}{% else %}0{% "
      "endif %}\"}",
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/channels/20/state/"
      "connected",
      "false",
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/channels/20/state/hi",
      NULL,
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/channels/20/state/"
      "is_calibrating",
      NULL,
      "supla/8ce92cb8c9f6db6b65703d2703691700/devices/12/channels/20/state/"
      "shut",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/channels/23/type",
      "THERMOMETERDS18B20",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/channels/23/function",
      "THERMOMETER",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/channels/23/caption",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/channels/23/hidden",
      "false",
      "homeassistant/sensor/61b2f9bdb22d3bd6336bb235bcb1b77e/23/config",
      "{\"avty\":{\"topic\":\"supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/"
      "14/channels/23/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/"
      "channels/"
      "23\",\"device\":{\"ids\":\"supla-iodevice-14\",\"mf\":\"Itead\","
      "\"name\":\"SONOFF-DS18B20\",\"sw\":\"2.5.3\"},\"name\":\"Temperature\","
      "\"uniq_id\":\"supla_23\",\"qos\":0,\"unit_of_meas\":\"°C\",\"stat_t\":"
      "\"~/state/temperature\",\"val_tpl\":\"{{ value | round(1,default=none) "
      "}}\",\"state_class\":\"measurement\"}",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/channels/23/state/"
      "connected",
      "false",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/14/channels/23/state/"
      "temperature",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9202/type",
      "IMPULSECOUNTER",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9202/"
      "function",
      "IC_WATERMETER",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9202/"
      "caption",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9202/"
      "hidden",
      "true",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9202/state/"
      "connected",
      "false",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9202/state/"
      "total_cost",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9202/state/"
      "price_per_unit",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9202/state/"
      "impulses_per_unit",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9202/state/"
      "counter",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9202/state/"
      "calculated_value",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9202/state/"
      "currency",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9202/state/"
      "unit",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9203/type",
      "IMPULSECOUNTER",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9203/"
      "function",
      "IC_GASMETER",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9203/"
      "caption",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9203/"
      "hidden",
      "false",
      "homeassistant/sensor/61b2f9bdb22d3bd6336bb235bcb1b77e/9203_0/config",
      "{\"avty\":{\"topic\":\"supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/"
      "6688/channels/9203/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/"
      "channels/"
      "9203\",\"device\":{\"ids\":\"supla-iodevice-6688\",\"mf\":\"\",\"name\":"
      "\"Impulse counter RB-03\",\"sw\":\"2.7.9\"},\"name\":\"Gas meter "
      "(Value)\",\"uniq_id\":\"supla_9203_0\",\"qos\":0,\"unit_of_meas\":"
      "\"m³\",\"stat_t\":\"~/state/calculated_value\",\"val_tpl\":\"{{ value | "
      "round(3,default=none) "
      "}}\",\"dev_cla\":\"gas\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/61b2f9bdb22d3bd6336bb235bcb1b77e/9203_1/config",
      "{\"avty\":{\"topic\":\"supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/"
      "6688/channels/9203/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/"
      "channels/"
      "9203\",\"device\":{\"ids\":\"supla-iodevice-6688\",\"mf\":\"\",\"name\":"
      "\"Impulse counter RB-03\",\"sw\":\"2.7.9\"},\"name\":\"Gas meter (Total "
      "cost)\",\"uniq_id\":\"supla_9203_1\",\"qos\":0,\"unit_of_meas\":\"PLN\","
      "\"stat_t\":\"~/state/total_cost\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) "
      "}}\",\"dev_cla\":\"monetary\",\"state_class\":\"measurement\"}",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9203/state/"
      "connected",
      "false",
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9203/state/"
      "total_cost",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9203/state/"
      "price_per_unit",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9203/state/"
      "impulses_per_unit",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9203/state/"
      "counter",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9203/state/"
      "calculated_value",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9203/state/"
      "currency",
      NULL,
      "supla/61b2f9bdb22d3bd6336bb235bcb1b77e/devices/6688/channels/9203/state/"
      "unit",
      NULL,
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/50/type",
      "RELAY",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/50/function",
      "LIGHTSWITCH",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/50/caption",
      NULL,
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/50/hidden",
      "false",
      "homeassistant/light/5b15b00055c044220196eb41965b3627/50/config",
      "{\"avty\":{\"topic\":\"supla/5b15b00055c044220196eb41965b3627/devices/"
      "36/channels/50/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/5b15b00055c044220196eb41965b3627/devices/36/"
      "channels/"
      "50\",\"device\":{\"ids\":\"supla-iodevice-36\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL ROW-02\",\"sw\":\"2.7.0\"},\"name\":\"Light "
      "switch\",\"uniq_id\":\"supla_50\",\"qos\":0,\"ret\":false,\"opt\":false,"
      "\"stat_t\":\"~/state/on\",\"cmd_t\":\"~/set/"
      "on\",\"pl_on\":\"true\",\"pl_off\":\"false\"}",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/50/state/"
      "connected",
      "false",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/50/state/on",
      NULL,
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/50/state/"
      "overcurrent_relay_off",
      NULL,
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/51/type",
      "RELAY",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/51/function",
      "LIGHTSWITCH",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/51/caption",
      NULL,
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/51/hidden",
      "false",
      "homeassistant/light/5b15b00055c044220196eb41965b3627/51/config",
      "{\"avty\":{\"topic\":\"supla/5b15b00055c044220196eb41965b3627/devices/"
      "36/channels/51/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/5b15b00055c044220196eb41965b3627/devices/36/"
      "channels/"
      "51\",\"device\":{\"ids\":\"supla-iodevice-36\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL ROW-02\",\"sw\":\"2.7.0\"},\"name\":\"Light "
      "switch\",\"uniq_id\":\"supla_51\",\"qos\":0,\"ret\":false,\"opt\":false,"
      "\"stat_t\":\"~/state/on\",\"cmd_t\":\"~/set/"
      "on\",\"pl_on\":\"true\",\"pl_off\":\"false\"}",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/51/state/"
      "connected",
      "false",
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/51/state/on",
      NULL,
      "supla/5b15b00055c044220196eb41965b3627/devices/36/channels/51/state/"
      "overcurrent_relay_off",
      NULL,
      "supla/45004b208185b207175522c7471f8526/devices/40/channels/56/type",
      "RELAY",
      "supla/45004b208185b207175522c7471f8526/devices/40/channels/56/function",
      "POWERSWITCH",
      "supla/45004b208185b207175522c7471f8526/devices/40/channels/56/caption",
      NULL,
      "supla/45004b208185b207175522c7471f8526/devices/40/channels/56/hidden",
      "false",
      "homeassistant/switch/45004b208185b207175522c7471f8526/56/config",
      "{\"avty\":{\"topic\":\"supla/45004b208185b207175522c7471f8526/devices/"
      "40/channels/56/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/45004b208185b207175522c7471f8526/devices/40/"
      "channels/"
      "56\",\"device\":{\"ids\":\"supla-iodevice-40\",\"mf\":\"Itead\","
      "\"name\":\"SONOFF-TH16\",\"sw\":\"2.5.3\"},\"name\":\"On/Off "
      "switch\",\"uniq_id\":\"supla_56\",\"qos\":0,\"ret\":false,\"opt\":false,"
      "\"stat_t\":\"~/state/on\",\"cmd_t\":\"~/set/"
      "on\",\"stat_on\":\"true\",\"stat_off\":\"false\",\"pl_on\":\"true\","
      "\"pl_off\":\"false\"}",
      "supla/45004b208185b207175522c7471f8526/devices/40/channels/56/state/"
      "connected",
      "false",
      "supla/45004b208185b207175522c7471f8526/devices/40/channels/56/state/on",
      NULL,
      "supla/45004b208185b207175522c7471f8526/devices/40/channels/56/state/"
      "overcurrent_relay_off",
      NULL,
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/channels/286/type",
      "RELAY",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/channels/286/"
      "function",
      "CONTROLLINGTHEGATE",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/channels/286/caption",
      NULL,
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/channels/286/hidden",
      "false",
      "homeassistant/cover/0c6beb47b1c2b14191e2bbb49e9d41c2/286/config",
      "{\"~\":\"supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/channels/"
      "286\",\"device\":{\"ids\":\"supla-iodevice-157\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL "
      "SBW-01\",\"sw\":\"2.7.0\"},\"name\":\"Gate\",\"uniq_id\":\"supla_286\","
      "\"qos\":0,\"ret\":false,\"opt\":false,\"cmd_t\":\"~/"
      "execute_action\",\"dev_cla\":\"gate\",\"pl_open\":\"OPEN\",\"pl_stop\":"
      "\"\",\"pl_cls\":\"CLOSE\",\"state_topic\":\"~/state/"
      "hi\",\"state_open\":\"false\",\"state_closed\":\"true\",\"avty_t\":\"~/"
      "state/connected\",\"pl_avail\":\"true\",\"pl_not_avail\":\"false\"}",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/channels/286/state/"
      "connected",
      "false",
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/channels/286/state/"
      "hi",
      NULL,
      "supla/0c6beb47b1c2b14191e2bbb49e9d41c2/devices/157/channels/286/state/"
      "partial_hi",
      NULL,
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/channels/1835/type",
      "RELAY",
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/channels/1835/"
      "function",
      "CONTROLLINGTHEDOORLOCK",
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/channels/1835/"
      "caption",
      NULL,
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/channels/1835/hidden",
      "false",
      "homeassistant/cover/4d3e49b9f3b202b1092fbfdc48d8d04f/1835/config",
      "{\"~\":\"supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/channels/"
      "1835\",\"device\":{\"ids\":\"supla-iodevice-983\",\"mf\":\"\",\"name\":"
      "\"*Yunshan\",\"sw\":\"2.7.2\"},\"name\":\"Door lock "
      "operation\",\"uniq_id\":\"supla_1835\",\"qos\":0,\"ret\":false,\"opt\":"
      "false,\"cmd_t\":\"~/"
      "execute_action\",\"dev_cla\":\"door\",\"pl_open\":\"OPEN\",\"pl_stop\":"
      "\"\",\"pl_cls\":\"OPEN\",\"state_topic\":\"~/state/"
      "hi\",\"state_open\":\"false\",\"state_closed\":\"true\",\"avty_t\":\"~/"
      "state/connected\",\"pl_avail\":\"true\",\"pl_not_avail\":\"false\"}",
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/channels/1835/state/"
      "connected",
      "false",
      "supla/4d3e49b9f3b202b1092fbfdc48d8d04f/devices/983/channels/1835/state/"
      "hi",
      NULL,
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/channels/404/type",
      "SENSORNO",
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/channels/404/"
      "function",
      "OPENINGSENSOR_GARAGEDOOR",
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/channels/404/caption",
      NULL,
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/channels/404/hidden",
      "false",
      "homeassistant/binary_sensor/de1257b7824b6825c276b763db1b711f/404/config",
      "{\"avty\":{\"topic\":\"supla/de1257b7824b6825c276b763db1b711f/devices/"
      "205/channels/404/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/de1257b7824b6825c276b763db1b711f/devices/205/"
      "channels/"
      "404\",\"device\":{\"ids\":\"supla-iodevice-205\",\"mf\":\"\",\"name\":"
      "\"SUPLA-GATE-MODULE\",\"sw\":\"2.5.5\"},\"name\":\"Garage door opening "
      "sensor\",\"uniq_id\":\"supla_404\",\"qos\":0,\"stat_t\":\"~/state/"
      "hi\",\"pl_on\":\"false\",\"pl_off\":\"true\",\"device_class\":\"garage_"
      "door\"}",
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/channels/404/state/"
      "connected",
      "false",
      "supla/de1257b7824b6825c276b763db1b711f/devices/205/channels/404/state/"
      "hi",
      NULL,
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/channels/10259/type",
      "DISTANCESENSOR",
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/channels/10259/"
      "function",
      "DEPTHSENSOR",
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/channels/10259/"
      "caption",
      NULL,
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/channels/10259/"
      "hidden",
      "false",
      "homeassistant/sensor/b72dfbdb402908e6939d7257dfed2367/10259/config",
      "{\"avty\":{\"topic\":\"supla/b72dfbdb402908e6939d7257dfed2367/devices/"
      "7212/channels/10259/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/"
      "channels/"
      "10259\",\"device\":{\"ids\":\"supla-iodevice-7212\",\"mf\":\"\","
      "\"name\":\"Eco-pea level sensor\",\"sw\":\"1.6.1\"},\"name\":\"Depth "
      "sensor\",\"uniq_id\":\"supla_10259\",\"qos\":0,\"unit_of_meas\":\"m\","
      "\"stat_t\":\"~/state/depth\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) }}\",\"state_class\":\"measurement\"}",
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/channels/10259/"
      "state/connected",
      "false",
      "supla/b72dfbdb402908e6939d7257dfed2367/devices/7212/channels/10259/"
      "state/depth",
      NULL,
      "supla/3281677315ff159f91471b34828f603b/devices/563/channels/1055/type",
      "RELAY",
      "supla/3281677315ff159f91471b34828f603b/devices/563/channels/1055/"
      "function",
      "CONTROLLINGTHEGARAGEDOOR",
      "supla/3281677315ff159f91471b34828f603b/devices/563/channels/1055/"
      "caption",
      "Long caption test ABCDEGFH",
      "supla/3281677315ff159f91471b34828f603b/devices/563/channels/1055/hidden",
      "false",
      "homeassistant/cover/3281677315ff159f91471b34828f603b/1055/config",
      "{\"~\":\"supla/3281677315ff159f91471b34828f603b/devices/563/channels/"
      "1055\",\"device\":{\"ids\":\"supla-iodevice-563\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL SBW-01\",\"sw\":\"2.7.0\"},\"name\":\"Long caption "
      "test "
      "ABCDEGFH\",\"uniq_id\":\"supla_1055\",\"qos\":0,\"ret\":false,\"opt\":"
      "false,\"cmd_t\":\"~/"
      "execute_action\",\"dev_cla\":\"garage\",\"pl_open\":\"OPEN\",\"pl_"
      "stop\":\"\",\"pl_cls\":\"CLOSE\",\"state_topic\":\"~/state/"
      "hi\",\"state_open\":\"false\",\"state_closed\":\"true\",\"avty_t\":\"~/"
      "state/connected\",\"pl_avail\":\"true\",\"pl_not_avail\":\"false\"}",
      "supla/3281677315ff159f91471b34828f603b/devices/563/channels/1055/state/"
      "connected",
      "false",
      "supla/3281677315ff159f91471b34828f603b/devices/563/channels/1055/state/"
      "hi",
      NULL,
      "supla/3281677315ff159f91471b34828f603b/devices/563/channels/1055/state/"
      "partial_hi",
      NULL,
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/channels/821/type",
      "DIMMER",
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/channels/821/"
      "function",
      "DIMMER",
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/channels/821/caption",
      NULL,
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/channels/821/hidden",
      "false",
      "homeassistant/light/46948d474db1b5e3cccf2c89281414c2/821/config",
      "{\"avty\":{\"topic\":\"supla/46948d474db1b5e3cccf2c89281414c2/devices/"
      "411/channels/821/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/46948d474db1b5e3cccf2c89281414c2/devices/411/"
      "channels/"
      "821\",\"device\":{\"ids\":\"supla-iodevice-411\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL "
      "SLW-02\",\"sw\":\"2.7.2\"},\"name\":\"Dimmer\",\"uniq_id\":\"supla_"
      "821\",\"qos\":0,\"cmd_t\":\"~/"
      "execute_action\",\"pl_on\":\"TURN_ON\",\"pl_off\":\"TURN_OFF\",\"stat_"
      "t\":\"~/state/on\",\"stat_val_tpl\":\"{% if value == \\\"true\\\" "
      "%}TURN_ON{% else %}TURN_OFF{% endif "
      "%}\",\"on_cmd_type\":\"first\",\"bri_cmd_t\":\"~/set/"
      "brightness\",\"bri_scl\":100,\"bri_stat_t\":\"~/state/brightness\"}",
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/channels/821/state/"
      "connected",
      "false",
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/channels/821/state/"
      "on",
      NULL,
      "supla/46948d474db1b5e3cccf2c89281414c2/devices/411/channels/821/state/"
      "brightness",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/type",
      "ELECTRICITYMETER",
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/"
      "function",
      "ELECTRICITYMETER",
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/caption",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/hidden",
      "false",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_0/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total "
      "cost)\",\"uniq_id\":\"supla_966_0\",\"qos\":0,\"unit_of_meas\":\"PLN\","
      "\"stat_t\":\"~/state/total_cost\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) "
      "}}\",\"dev_cla\":\"monetary\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_1/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total cost - "
      "balanced)\",\"uniq_id\":\"supla_966_1\",\"qos\":0,\"unit_of_meas\":"
      "\"PLN\",\"stat_t\":\"~/state/total_cost_balanced\",\"val_tpl\":\"{{ "
      "value | round(2,default=none) "
      "}}\",\"dev_cla\":\"monetary\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_2/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total forward active "
      "energy)\",\"uniq_id\":\"supla_966_2\",\"qos\":0,\"unit_of_meas\":"
      "\"kWh\",\"stat_t\":\"~/state/"
      "total_forward_active_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_3/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total reverse active "
      "energy)\",\"uniq_id\":\"supla_966_3\",\"qos\":0,\"unit_of_meas\":"
      "\"kWh\",\"stat_t\":\"~/state/"
      "total_reverse_active_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_4/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total forward active energy - "
      "balanced)\",\"uniq_id\":\"supla_966_4\",\"qos\":0,\"unit_of_meas\":"
      "\"kWh\",\"stat_t\":\"~/state/"
      "total_forward_active_energy_balanced\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_5/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total reverse active energy - "
      "balanced)\",\"uniq_id\":\"supla_966_5\",\"qos\":0,\"unit_of_meas\":"
      "\"kWh\",\"stat_t\":\"~/state/"
      "total_reverse_active_energy_balanced\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_6/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total forward active energy - Phase "
      "1)\",\"uniq_id\":\"supla_966_6\",\"qos\":0,\"unit_of_meas\":\"kWh\","
      "\"stat_t\":\"~/state/phases/1/"
      "total_forward_active_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_7/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total reverse active energy - Phase "
      "1)\",\"uniq_id\":\"supla_966_7\",\"qos\":0,\"unit_of_meas\":\"kWh\","
      "\"stat_t\":\"~/state/phases/1/"
      "total_reverse_active_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_8/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total forward reactive energy - Phase "
      "1)\",\"uniq_id\":\"supla_966_8\",\"qos\":0,\"unit_of_meas\":\"kvarh\","
      "\"stat_t\":\"~/state/phases/1/"
      "total_forward_reactive_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) }}\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_9/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total reverse reactive energy - Phase "
      "1)\",\"uniq_id\":\"supla_966_9\",\"qos\":0,\"unit_of_meas\":\"kvarh\","
      "\"stat_t\":\"~/state/phases/1/"
      "total_reverse_reactive_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) }}\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_10/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Frequency - Phase "
      "1)\",\"uniq_id\":\"supla_966_10\",\"qos\":0,\"unit_of_meas\":\"Hz\","
      "\"stat_t\":\"~/state/phases/1/frequency\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) }}\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_11/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Voltage - Phase "
      "1)\",\"uniq_id\":\"supla_966_11\",\"qos\":0,\"unit_of_meas\":\"V\","
      "\"stat_t\":\"~/state/phases/1/voltage\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) "
      "}}\",\"dev_cla\":\"voltage\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_12/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Current - Phase "
      "1)\",\"uniq_id\":\"supla_966_12\",\"qos\":0,\"unit_of_meas\":\"A\","
      "\"stat_t\":\"~/state/phases/1/current\",\"val_tpl\":\"{{ value | "
      "round(3,default=none) "
      "}}\",\"dev_cla\":\"current\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_13/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Power active - Phase "
      "1)\",\"uniq_id\":\"supla_966_13\",\"qos\":0,\"unit_of_meas\":\"W\","
      "\"stat_t\":\"~/state/phases/1/power_active\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"power\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_14/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Power reactive - Phase "
      "1)\",\"uniq_id\":\"supla_966_14\",\"qos\":0,\"unit_of_meas\":\"var\","
      "\"stat_t\":\"~/state/phases/1/power_reactive\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) }}\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_15/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Power apparent - Phase "
      "1)\",\"uniq_id\":\"supla_966_15\",\"qos\":0,\"unit_of_meas\":\"VA\","
      "\"stat_t\":\"~/state/phases/1/power_apparent\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) }}\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_16/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Power factor - Phase "
      "1)\",\"uniq_id\":\"supla_966_16\",\"qos\":0,\"unit_of_meas\":\"%\","
      "\"stat_t\":\"~/state/phases/1/power_factor\",\"val_tpl\":\"{% if "
      "float(value, default=none) == None %}None{% else %}{{float(value) * "
      "100.0 "
      "| round(5)}}{% endif "
      "%}\",\"dev_cla\":\"power_factor\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_17/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Phase angle - Phase "
      "1)\",\"uniq_id\":\"supla_966_17\",\"qos\":0,\"unit_of_meas\":\"°\","
      "\"stat_t\":\"~/state/phases/1/phase_angle\",\"val_tpl\":\"{{ value | "
      "round(1,default=none) }}\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_18/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total forward active energy - Phase "
      "2)\",\"uniq_id\":\"supla_966_18\",\"qos\":0,\"unit_of_meas\":\"kWh\","
      "\"stat_t\":\"~/state/phases/2/"
      "total_forward_active_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_19/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total reverse active energy - Phase "
      "2)\",\"uniq_id\":\"supla_966_19\",\"qos\":0,\"unit_of_meas\":\"kWh\","
      "\"stat_t\":\"~/state/phases/2/"
      "total_reverse_active_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_20/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total forward reactive energy - Phase "
      "2)\",\"uniq_id\":\"supla_966_20\",\"qos\":0,\"unit_of_meas\":\"kvarh\","
      "\"stat_t\":\"~/state/phases/2/"
      "total_forward_reactive_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) }}\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_21/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total reverse reactive energy - Phase "
      "2)\",\"uniq_id\":\"supla_966_21\",\"qos\":0,\"unit_of_meas\":\"kvarh\","
      "\"stat_t\":\"~/state/phases/2/"
      "total_reverse_reactive_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) }}\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_22/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Frequency - Phase "
      "2)\",\"uniq_id\":\"supla_966_22\",\"qos\":0,\"unit_of_meas\":\"Hz\","
      "\"stat_t\":\"~/state/phases/2/frequency\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) }}\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_23/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Voltage - Phase "
      "2)\",\"uniq_id\":\"supla_966_23\",\"qos\":0,\"unit_of_meas\":\"V\","
      "\"stat_t\":\"~/state/phases/2/voltage\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) "
      "}}\",\"dev_cla\":\"voltage\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_24/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Current - Phase "
      "2)\",\"uniq_id\":\"supla_966_24\",\"qos\":0,\"unit_of_meas\":\"A\","
      "\"stat_t\":\"~/state/phases/2/current\",\"val_tpl\":\"{{ value | "
      "round(3,default=none) "
      "}}\",\"dev_cla\":\"current\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_25/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Power active - Phase "
      "2)\",\"uniq_id\":\"supla_966_25\",\"qos\":0,\"unit_of_meas\":\"W\","
      "\"stat_t\":\"~/state/phases/2/power_active\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"power\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_26/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Power reactive - Phase "
      "2)\",\"uniq_id\":\"supla_966_26\",\"qos\":0,\"unit_of_meas\":\"var\","
      "\"stat_t\":\"~/state/phases/2/power_reactive\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) }}\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_27/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Power apparent - Phase "
      "2)\",\"uniq_id\":\"supla_966_27\",\"qos\":0,\"unit_of_meas\":\"VA\","
      "\"stat_t\":\"~/state/phases/2/power_apparent\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) }}\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_28/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Power factor - Phase "
      "2)\",\"uniq_id\":\"supla_966_28\",\"qos\":0,\"unit_of_meas\":\"%\","
      "\"stat_t\":\"~/state/phases/2/power_factor\",\"val_tpl\":\"{% if "
      "float(value, default=none) == None %}None{% else %}{{float(value) * "
      "100.0 "
      "| round(5)}}{% endif "
      "%}\",\"dev_cla\":\"power_factor\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_29/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Phase angle - Phase "
      "2)\",\"uniq_id\":\"supla_966_29\",\"qos\":0,\"unit_of_meas\":\"°\","
      "\"stat_t\":\"~/state/phases/2/phase_angle\",\"val_tpl\":\"{{ value | "
      "round(1,default=none) }}\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_30/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total forward active energy - Phase "
      "3)\",\"uniq_id\":\"supla_966_30\",\"qos\":0,\"unit_of_meas\":\"kWh\","
      "\"stat_t\":\"~/state/phases/3/"
      "total_forward_active_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_31/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total reverse active energy - Phase "
      "3)\",\"uniq_id\":\"supla_966_31\",\"qos\":0,\"unit_of_meas\":\"kWh\","
      "\"stat_t\":\"~/state/phases/3/"
      "total_reverse_active_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_32/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total forward reactive energy - Phase "
      "3)\",\"uniq_id\":\"supla_966_32\",\"qos\":0,\"unit_of_meas\":\"kvarh\","
      "\"stat_t\":\"~/state/phases/3/"
      "total_forward_reactive_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) }}\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_33/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Total reverse reactive energy - Phase "
      "3)\",\"uniq_id\":\"supla_966_33\",\"qos\":0,\"unit_of_meas\":\"kvarh\","
      "\"stat_t\":\"~/state/phases/3/"
      "total_reverse_reactive_energy\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) }}\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_34/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Frequency - Phase "
      "3)\",\"uniq_id\":\"supla_966_34\",\"qos\":0,\"unit_of_meas\":\"Hz\","
      "\"stat_t\":\"~/state/phases/3/frequency\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) }}\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_35/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Voltage - Phase "
      "3)\",\"uniq_id\":\"supla_966_35\",\"qos\":0,\"unit_of_meas\":\"V\","
      "\"stat_t\":\"~/state/phases/3/voltage\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) "
      "}}\",\"dev_cla\":\"voltage\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_36/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Current - Phase "
      "3)\",\"uniq_id\":\"supla_966_36\",\"qos\":0,\"unit_of_meas\":\"A\","
      "\"stat_t\":\"~/state/phases/3/current\",\"val_tpl\":\"{{ value | "
      "round(3,default=none) "
      "}}\",\"dev_cla\":\"current\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_37/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Power active - Phase "
      "3)\",\"uniq_id\":\"supla_966_37\",\"qos\":0,\"unit_of_meas\":\"W\","
      "\"stat_t\":\"~/state/phases/3/power_active\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) "
      "}}\",\"dev_cla\":\"power\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_38/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Power reactive - Phase "
      "3)\",\"uniq_id\":\"supla_966_38\",\"qos\":0,\"unit_of_meas\":\"var\","
      "\"stat_t\":\"~/state/phases/3/power_reactive\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) }}\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_39/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Power apparent - Phase "
      "3)\",\"uniq_id\":\"supla_966_39\",\"qos\":0,\"unit_of_meas\":\"VA\","
      "\"stat_t\":\"~/state/phases/3/power_apparent\",\"val_tpl\":\"{{ value | "
      "round(5,default=none) }}\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_40/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Power factor - Phase "
      "3)\",\"uniq_id\":\"supla_966_40\",\"qos\":0,\"unit_of_meas\":\"%\","
      "\"stat_t\":\"~/state/phases/3/power_factor\",\"val_tpl\":\"{% if "
      "float(value, default=none) == None %}None{% else %}{{float(value) * "
      "100.0 "
      "| round(5)}}{% endif "
      "%}\",\"dev_cla\":\"power_factor\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/0df527b1786040fe57d6d9f43b632fbb/966_41/config",
      "{\"avty\":{\"topic\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/"
      "506/channels/966/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/"
      "channels/"
      "966\",\"device\":{\"ids\":\"supla-iodevice-506\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL MEW-01\",\"sw\":\"2.7.8\"},\"name\":\"Electricity "
      "meter (Phase angle - Phase "
      "3)\",\"uniq_id\":\"supla_966_41\",\"qos\":0,\"unit_of_meas\":\"°\","
      "\"stat_t\":\"~/state/phases/3/phase_angle\",\"val_tpl\":\"{{ value | "
      "round(1,default=none) }}\",\"state_class\":\"measurement\"}",
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "connected",
      "false",
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "total_cost",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "total_cost_balanced",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "price_per_unit",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "currency",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "support",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "total_forward_active_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "total_reverse_active_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "total_forward_active_energy_balanced",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "total_reverse_active_energy_balanced",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/1/total_forward_active_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/1/total_reverse_active_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/1/total_forward_reactive_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/1/total_reverse_reactive_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/1/frequency",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/1/voltage",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/1/current",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/1/power_active",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/1/power_reactive",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/1/power_apparent",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/1/power_factor",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/1/phase_angle",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/2/total_forward_active_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/2/total_reverse_active_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/2/total_forward_reactive_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/2/total_reverse_reactive_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/2/frequency",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/2/voltage",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/2/current",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/2/power_active",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/2/power_reactive",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/2/power_apparent",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/2/power_factor",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/2/phase_angle",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/3/total_forward_active_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/3/total_reverse_active_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/3/total_forward_reactive_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/3/total_reverse_reactive_energy",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/3/frequency",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/3/voltage",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/3/current",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/3/power_active",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/3/power_reactive",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/3/power_apparent",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/3/power_factor",
      NULL,
      "supla/0df527b1786040fe57d6d9f43b632fbb/devices/506/channels/966/state/"
      "phases/3/phase_angle",
      NULL,
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/channels/1852/type",
      "RGBLEDCONTROLLER",
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/channels/1852/"
      "function",
      "RGBLIGHTING",
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/channels/1852/"
      "caption",
      NULL,
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/channels/1852/hidden",
      "false",
      "homeassistant/light/c2e6c948d570bfb4e8b7152d890937db/1852/config",
      "{\"avty\":{\"topic\":\"supla/c2e6c948d570bfb4e8b7152d890937db/devices/"
      "989/channels/1852/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/"
      "channels/"
      "1852\",\"device\":{\"ids\":\"supla-iodevice-989\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL SLW-01\",\"sw\":\"2.5.5\"},\"name\":\"RGB "
      "Lighting\",\"uniq_id\":\"supla_1852\",\"qos\":0,\"cmd_t\":\"~/"
      "execute_action\",\"pl_on\":\"TURN_ON\",\"pl_off\":\"TURN_OFF\",\"stat_"
      "t\":\"~/state/on\",\"stat_val_tpl\":\"{% if value == \\\"true\\\" "
      "%}TURN_ON{% else %}TURN_OFF{% endif "
      "%}\",\"on_cmd_type\":\"first\",\"bri_cmd_t\":\"~/set/"
      "color_brightness\",\"bri_scl\":100,\"bri_stat_t\":\"~/state/"
      "color_brightness\",\"rgb_stat_t\":\"~/state/"
      "normalized_rgb\",\"rgb_cmd_t\":\"~/set/color\"}",
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/channels/1852/state/"
      "connected",
      "false",
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/channels/1852/state/"
      "on",
      NULL,
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/channels/1852/state/"
      "color",
      NULL,
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/channels/1852/state/"
      "normalized_rgb",
      NULL,
      "supla/c2e6c948d570bfb4e8b7152d890937db/devices/989/channels/1852/state/"
      "color_brightness",
      NULL,
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/channels/12048/type",
      "PRESSURESENSOR",
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/channels/12048/"
      "function",
      "PRESSURESENSOR",
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/channels/12048/"
      "caption",
      NULL,
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/channels/12048/"
      "hidden",
      "false",
      "homeassistant/sensor/8588fb099c7f6f32d0b5dc43cbc3318b/12048/config",
      "{\"avty\":{\"topic\":\"supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/"
      "7878/channels/12048/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/"
      "channels/"
      "12048\",\"device\":{\"ids\":\"supla-iodevice-7878\",\"mf\":\"\","
      "\"name\":\"BME280\",\"sw\":\"2.0.0\"},\"name\":\"Pressure "
      "sensor\",\"uniq_id\":\"supla_12048\",\"qos\":0,\"unit_of_meas\":\"hPa\","
      "\"stat_t\":\"~/state/value\",\"val_tpl\":\"{{ value | "
      "round(0,default=none) }}\",\"state_class\":\"measurement\"}",
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/channels/12048/"
      "state/connected",
      "false",
      "supla/8588fb099c7f6f32d0b5dc43cbc3318b/devices/7878/channels/12048/"
      "state/value",
      NULL,
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/channels/16708/type",
      "THERMOSTATHEATPOLHOMEPLUS",
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/channels/16708/"
      "function",
      "THERMOSTATHEATPOLHOMEPLUS",
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/channels/16708/"
      "caption",
      NULL,
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/channels/16708/"
      "hidden",
      "false",
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/channels/16708/"
      "state/connected",
      "false",
      "supla/7450214b8189dc129301c2c922cdcfd0/devices/9934/channels/16708/"
      "state/on",
      NULL,
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/channels/15039/type",
      "SENSORNO",
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/channels/15039/"
      "function",
      "NOLIQUIDSENSOR",
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/channels/15039/"
      "caption",
      NULL,
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/channels/15039/"
      "hidden",
      "false",
      "homeassistant/binary_sensor/bd6287c398bc81403c643410f5043d4e/15039/"
      "config",
      "{\"avty\":{\"topic\":\"supla/bd6287c398bc81403c643410f5043d4e/devices/"
      "9236/channels/15039/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/bd6287c398bc81403c643410f5043d4e/devices/9236/"
      "channels/"
      "15039\",\"device\":{\"ids\":\"supla-iodevice-9236\",\"mf\":\"\","
      "\"name\":\"HOME\",\"sw\":\"1.6.1\"},\"name\":\"No liquid "
      "sensor\",\"uniq_id\":\"supla_15039\",\"qos\":0,\"stat_t\":\"~/state/"
      "hi\",\"pl_on\":\"false\",\"pl_off\":\"true\"}",
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/channels/15039/"
      "state/connected",
      "false",
      "supla/bd6287c398bc81403c643410f5043d4e/devices/9236/channels/15039/"
      "state/hi",
      NULL,
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/channels/7341/type",
      "SENSORNO",
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/channels/7341/"
      "function",
      "OPENINGSENSOR_ROLLERSHUTTER",
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/channels/7341/"
      "caption",
      NULL,
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/channels/7341/"
      "hidden",
      "false",
      "homeassistant/binary_sensor/689eb3922683b4e4e7b9227f32f2d89b/7341/"
      "config",
      "{\"avty\":{\"topic\":\"supla/689eb3922683b4e4e7b9227f32f2d89b/devices/"
      "5823/channels/7341/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/"
      "channels/"
      "7341\",\"device\":{\"ids\":\"supla-iodevice-5823\",\"mf\":\"\",\"name\":"
      "\"WEMOS-ROLLER-SHUTTER-UNI "
      "by@Zybi\",\"sw\":\"2.5.3\"},\"name\":\"Roller shutter opening "
      "sensor\",\"uniq_id\":\"supla_7341\",\"qos\":0,\"stat_t\":\"~/state/"
      "hi\",\"pl_on\":\"false\",\"pl_off\":\"true\",\"device_class\":"
      "\"opening\"}",
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/channels/7341/state/"
      "connected",
      "false",
      "supla/689eb3922683b4e4e7b9227f32f2d89b/devices/5823/channels/7341/state/"
      "hi",
      NULL,
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/channels/2009/type",
      "RELAY",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/channels/2009/"
      "function",
      "CONTROLLINGTHEGATEWAYLOCK",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/channels/2009/"
      "caption",
      "Gateway II",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/channels/2009/"
      "hidden",
      "false",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/channels/2009/state/"
      "connected",
      "false",
      "supla/113fb17f5b300bfbbfb667eb6d4d1ef8/devices/1068/channels/2009/state/"
      "hi",
      NULL,
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/channels/3068/type",
      "DHT22",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/channels/3068/"
      "function",
      "HUMIDITYANDTEMPERATURE",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/channels/3068/"
      "caption",
      NULL,
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/channels/3068/"
      "hidden",
      "false",
      "homeassistant/sensor/1d43b8d35e31f1508c70b9b77e03e293/3068_0/config",
      "{\"avty\":{\"topic\":\"supla/1d43b8d35e31f1508c70b9b77e03e293/devices/"
      "3195/channels/3068/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/"
      "channels/"
      "3068\",\"device\":{\"ids\":\"supla-iodevice-3195\",\"mf\":\"Itead\","
      "\"name\":\"SONOFF-p4-DHT22 "
      "by@Zybi\",\"sw\":\"2.0\"},\"name\":\"Humidity\",\"uniq_id\":\"supla_"
      "3068_0\",\"qos\":0,\"unit_of_meas\":\"%\",\"stat_t\":\"~/state/"
      "humidity\",\"val_tpl\":\"{{ value | round(1,default=none) "
      "}}\",\"state_class\":\"measurement\"}",
      "homeassistant/sensor/1d43b8d35e31f1508c70b9b77e03e293/3068_1/config",
      "{\"avty\":{\"topic\":\"supla/1d43b8d35e31f1508c70b9b77e03e293/devices/"
      "3195/channels/3068/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/"
      "channels/"
      "3068\",\"device\":{\"ids\":\"supla-iodevice-3195\",\"mf\":\"Itead\","
      "\"name\":\"SONOFF-p4-DHT22 "
      "by@Zybi\",\"sw\":\"2.0\"},\"name\":\"Temperature\",\"uniq_id\":\"supla_"
      "3068_1\",\"qos\":0,\"unit_of_meas\":\"°C\",\"stat_t\":\"~/state/"
      "temperature\",\"val_tpl\":\"{{ value | round(1,default=none) "
      "}}\",\"state_class\":\"measurement\"}",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/channels/3068/state/"
      "connected",
      "false",
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/channels/3068/state/"
      "humidity",
      NULL,
      "supla/1d43b8d35e31f1508c70b9b77e03e293/devices/3195/channels/3068/state/"
      "temperature",
      NULL,
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/channels/15596/type",
      "VALVEOPENCLOSE",
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/channels/15596/"
      "function",
      "VALVEOPENCLOSE",
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/channels/15596/"
      "caption",
      NULL,
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/channels/15596/"
      "hidden",
      "false",
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/channels/15596/"
      "state/connected",
      "false",
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/channels/15596/"
      "state/closed",
      NULL,
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/channels/15596/"
      "state/manually_closed",
      NULL,
      "supla/b9802383dd06c57d0c41de6805957d0f/devices/9480/channels/15596/"
      "state/flooding",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/type",
      "IMPULSECOUNTER",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "function",
      "IC_ELECTRICITYMETER",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "caption",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "hidden",
      "false",
      "homeassistant/sensor/64d3d5bd59eb5fe2b67bf4fe4d222905/16166_0/config",
      "{\"avty\":{\"topic\":\"supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/"
      "9717/channels/16166/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/"
      "channels/"
      "16166\",\"device\":{\"ids\":\"supla-iodevice-9717\",\"mf\":\"\","
      "\"name\":\"IMPULSE COUNTER\",\"sw\":\"2.7.2\"},\"name\":\"Electricity "
      "meter "
      "(Value)\",\"uniq_id\":\"supla_16166_0\",\"qos\":0,\"unit_of_meas\":"
      "\"kWh\",\"stat_t\":\"~/state/calculated_value\",\"val_tpl\":\"{{ value "
      "| round(3,default=none) "
      "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/64d3d5bd59eb5fe2b67bf4fe4d222905/16166_1/config",
      "{\"avty\":{\"topic\":\"supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/"
      "9717/channels/16166/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/"
      "channels/"
      "16166\",\"device\":{\"ids\":\"supla-iodevice-9717\",\"mf\":\"\","
      "\"name\":\"IMPULSE COUNTER\",\"sw\":\"2.7.2\"},\"name\":\"Electricity "
      "meter (Total "
      "cost)\",\"uniq_id\":\"supla_16166_1\",\"qos\":0,\"unit_of_meas\":"
      "\"PLN\",\"stat_t\":\"~/state/total_cost\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) "
      "}}\",\"dev_cla\":\"monetary\",\"state_class\":\"measurement\"}",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/connected",
      "false",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/total_cost",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/price_per_unit",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/impulses_per_unit",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/counter",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/calculated_value",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/currency",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/unit",
      NULL,
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/channels/15283/type",
      "SENSORNO",
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/channels/15283/"
      "function",
      "OPENINGSENSOR_WINDOW",
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/channels/15283/"
      "caption",
      NULL,
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/channels/15283/"
      "hidden",
      "false",
      "homeassistant/binary_sensor/93158d9b795bb1c65e79199d42381706/15283/"
      "config",
      "{\"avty\":{\"topic\":\"supla/93158d9b795bb1c65e79199d42381706/devices/"
      "9344/channels/15283/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/93158d9b795bb1c65e79199d42381706/devices/9344/"
      "channels/"
      "15283\",\"device\":{\"ids\":\"supla-iodevice-9344\",\"mf\":\"Zamel\","
      "\"name\":\"ZAMEL SBW-02\",\"sw\":\"2.7.13\"},\"name\":\"Window opening "
      "sensor\",\"uniq_id\":\"supla_15283\",\"qos\":0,\"stat_t\":\"~/state/"
      "hi\",\"pl_on\":\"false\",\"pl_off\":\"true\",\"device_class\":"
      "\"window\"}",
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/channels/15283/"
      "state/connected",
      "false",
      "supla/93158d9b795bb1c65e79199d42381706/devices/9344/channels/15283/"
      "state/hi",
      NULL,
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/channels/7212/type",
      "SENSORNO",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/channels/7212/"
      "function",
      "MAILSENSOR",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/channels/7212/"
      "caption",
      NULL,
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/channels/7212/"
      "hidden",
      "false",
      "homeassistant/binary_sensor/4b2bb30d2db31cf5809ede445862f6d7/7212/"
      "config",
      "{\"avty\":{\"topic\":\"supla/4b2bb30d2db31cf5809ede445862f6d7/devices/"
      "5761/channels/7212/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/"
      "channels/"
      "7212\",\"device\":{\"ids\":\"supla-iodevice-5761\",\"mf\":\"Itead\","
      "\"name\":\"SONOFF-gate-p5-DS18B20 "
      "by@Zybi\",\"sw\":\"2.0\"},\"name\":\"Mail "
      "sensor\",\"uniq_id\":\"supla_7212\",\"qos\":0,\"stat_t\":\"~/state/"
      "hi\",\"pl_on\":\"false\",\"pl_off\":\"true\"}",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/channels/7212/state/"
      "connected",
      "false",
      "supla/4b2bb30d2db31cf5809ede445862f6d7/devices/5761/channels/7212/state/"
      "hi",
      NULL,
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/channels/5177/type",
      "SENSORNO",
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/channels/5177/"
      "function",
      "OPENINGSENSOR_GATEWAY",
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/channels/5177/"
      "caption",
      NULL,
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/channels/5177/"
      "hidden",
      "false",
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/channels/5177/state/"
      "connected",
      "false",
      "supla/879b9d8688bc4b7108801c5c6bff4b3b/devices/4567/channels/5177/state/"
      "hi",
      NULL,
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/channels/18023/"
      "type",
      "SENSORNO",
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/channels/18023/"
      "function",
      "OPENINGSENSOR_GATE",
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/channels/18023/"
      "caption",
      NULL,
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/channels/18023/"
      "hidden",
      "false",
      "homeassistant/binary_sensor/ee2611c4632f53261cf3fb8d9d2f182d/18023/"
      "config",
      "{\"avty\":{\"topic\":\"supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/"
      "10286/channels/18023/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/"
      "channels/"
      "18023\",\"device\":{\"ids\":\"supla-iodevice-10286\",\"mf\":\"\","
      "\"name\":\"GATE\",\"sw\":\"2.3.2/1.11 @lukfud\"},\"name\":\"Gate "
      "opening "
      "sensor\",\"uniq_id\":\"supla_18023\",\"qos\":0,\"stat_t\":\"~/state/"
      "hi\",\"pl_on\":\"false\",\"pl_off\":\"true\",\"device_class\":"
      "\"opening\"}",
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/channels/18023/"
      "state/connected",
      "false",
      "supla/ee2611c4632f53261cf3fb8d9d2f182d/devices/10286/channels/18023/"
      "state/hi",
      NULL,
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17748/"
      "type",
      "DISTANCESENSOR",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17748/"
      "function",
      "DISTANCESENSOR",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17748/"
      "caption",
      NULL,
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17748/"
      "hidden",
      "false",
      "homeassistant/sensor/7e2182788b9214ee5b66349d5b7668cf/17748/config",
      "{\"avty\":{\"topic\":\"supla/7e2182788b9214ee5b66349d5b7668cf/devices/"
      "10190/channels/17748/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/"
      "channels/"
      "17748\",\"device\":{\"ids\":\"supla-iodevice-10190\",\"mf\":\"\","
      "\"name\":\"SUPLA-Skalar\",\"sw\":\"1.6.1\"},\"name\":\"Distance "
      "sensor\",\"uniq_id\":\"supla_17748\",\"qos\":0,\"unit_of_meas\":\"m\","
      "\"stat_t\":\"~/state/distance\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) }}\",\"state_class\":\"measurement\"}",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17748/"
      "state/connected",
      "false",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17748/"
      "state/distance",
      NULL,
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17749/"
      "type",
      "DISTANCESENSOR",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17749/"
      "function",
      "DISTANCESENSOR",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17749/"
      "caption",
      NULL,
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17749/"
      "hidden",
      "false",
      "homeassistant/sensor/7e2182788b9214ee5b66349d5b7668cf/17749/config",
      "{\"avty\":{\"topic\":\"supla/7e2182788b9214ee5b66349d5b7668cf/devices/"
      "10190/channels/17749/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/"
      "channels/"
      "17749\",\"device\":{\"ids\":\"supla-iodevice-10190\",\"mf\":\"\","
      "\"name\":\"SUPLA-Skalar\",\"sw\":\"1.6.1\"},\"name\":\"Distance "
      "sensor\",\"uniq_id\":\"supla_17749\",\"qos\":0,\"unit_of_meas\":\"m\","
      "\"stat_t\":\"~/state/distance\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) }}\",\"state_class\":\"measurement\"}",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17749/"
      "state/connected",
      "false",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17749/"
      "state/distance",
      NULL,
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17750/"
      "type",
      "DISTANCESENSOR",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17750/"
      "function",
      "DISTANCESENSOR",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17750/"
      "caption",
      NULL,
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17750/"
      "hidden",
      "false",
      "homeassistant/sensor/7e2182788b9214ee5b66349d5b7668cf/17750/config",
      "{\"avty\":{\"topic\":\"supla/7e2182788b9214ee5b66349d5b7668cf/devices/"
      "10190/channels/17750/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/"
      "channels/"
      "17750\",\"device\":{\"ids\":\"supla-iodevice-10190\",\"mf\":\"\","
      "\"name\":\"SUPLA-Skalar\",\"sw\":\"1.6.1\"},\"name\":\"Distance "
      "sensor\",\"uniq_id\":\"supla_17750\",\"qos\":0,\"unit_of_meas\":\"m\","
      "\"stat_t\":\"~/state/distance\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) }}\",\"state_class\":\"measurement\"}",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17750/"
      "state/connected",
      "false",
      "supla/7e2182788b9214ee5b66349d5b7668cf/devices/10190/channels/17750/"
      "state/distance",
      NULL,
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/channels/4688/type",
      "DIMMERANDRGBLED",
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/channels/4688/"
      "function",
      "DIMMERANDRGBLIGHTING",
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/channels/4688/"
      "caption",
      NULL,
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/channels/4688/"
      "hidden",
      "false",
      "homeassistant/light/6527881802ebd8c4ff2b3955bc326704/4688_0/config",
      "{\"avty\":{\"topic\":\"supla/6527881802ebd8c4ff2b3955bc326704/devices/"
      "4124/channels/4688/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/"
      "channels/"
      "4688\",\"device\":{\"ids\":\"supla-iodevice-4124\",\"mf\":\"\",\"name\":"
      "\"8xDSv3.1 "
      "by@wojtek\",\"sw\":\"1.6.1\"},\"name\":\"Dimmer\",\"uniq_id\":\"supla_"
      "4688_0\",\"qos\":0,\"cmd_t\":\"~/"
      "execute_action\",\"pl_on\":\"TURN_ON\",\"pl_off\":\"TURN_OFF\",\"stat_"
      "t\":\"~/state/on\",\"stat_val_tpl\":\"{% if value == \\\"true\\\" "
      "%}TURN_ON{% else %}TURN_OFF{% endif "
      "%}\",\"on_cmd_type\":\"first\",\"bri_cmd_t\":\"~/set/"
      "brightness\",\"bri_scl\":100,\"bri_stat_t\":\"~/state/brightness\"}",
      "homeassistant/light/6527881802ebd8c4ff2b3955bc326704/4688_1/config",
      "{\"avty\":{\"topic\":\"supla/6527881802ebd8c4ff2b3955bc326704/devices/"
      "4124/channels/4688/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/"
      "channels/"
      "4688\",\"device\":{\"ids\":\"supla-iodevice-4124\",\"mf\":\"\",\"name\":"
      "\"8xDSv3.1 by@wojtek\",\"sw\":\"1.6.1\"},\"name\":\"RGB "
      "Lighting\",\"uniq_id\":\"supla_4688_1\",\"qos\":0,\"cmd_t\":\"~/"
      "execute_action\",\"pl_on\":\"TURN_ON\",\"pl_off\":\"TURN_OFF\",\"stat_"
      "t\":\"~/state/on\",\"stat_val_tpl\":\"{% if value == \\\"true\\\" "
      "%}TURN_ON{% else %}TURN_OFF{% endif "
      "%}\",\"on_cmd_type\":\"first\",\"bri_cmd_t\":\"~/set/"
      "color_brightness\",\"bri_scl\":100,\"bri_stat_t\":\"~/state/"
      "color_brightness\",\"rgb_stat_t\":\"~/state/"
      "normalized_rgb\",\"rgb_cmd_t\":\"~/set/color\"}",
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/channels/4688/state/"
      "connected",
      "false",
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/channels/4688/state/"
      "on",
      NULL,
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/channels/4688/state/"
      "color",
      NULL,
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/channels/4688/state/"
      "color_brightness",
      NULL,
      "supla/6527881802ebd8c4ff2b3955bc326704/devices/4124/channels/4688/state/"
      "brightness",
      NULL,
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/channels/16054/type",
      "RAINSENSOR",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/channels/16054/"
      "function",
      "RAINSENSOR",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/channels/16054/"
      "caption",
      NULL,
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/channels/16054/"
      "hidden",
      "false",
      "homeassistant/sensor/15cb17b568de00e1ddc3005e4674bbde/16054/config",
      "{\"avty\":{\"topic\":\"supla/15cb17b568de00e1ddc3005e4674bbde/devices/"
      "9665/channels/16054/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/"
      "channels/"
      "16054\",\"device\":{\"ids\":\"supla-iodevice-9665\",\"mf\":\"\","
      "\"name\":\"Weather\",\"sw\":\"1.0.4\"},\"name\":\"Rain "
      "sensor\",\"uniq_id\":\"supla_16054\",\"qos\":0,\"unit_of_meas\":\"l/"
      "m\",\"stat_t\":\"~/state/value\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) }}\",\"state_class\":\"measurement\"}",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/channels/16054/"
      "state/connected",
      "false",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/channels/16054/"
      "state/value",
      NULL,
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/channels/16055/type",
      "WINDSENSOR",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/channels/16055/"
      "function",
      "WINDSENSOR",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/channels/16055/"
      "caption",
      NULL,
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/channels/16055/"
      "hidden",
      "false",
      "homeassistant/sensor/15cb17b568de00e1ddc3005e4674bbde/16055/config",
      "{\"avty\":{\"topic\":\"supla/15cb17b568de00e1ddc3005e4674bbde/devices/"
      "9665/channels/16055/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/"
      "channels/"
      "16055\",\"device\":{\"ids\":\"supla-iodevice-9665\",\"mf\":\"\","
      "\"name\":\"Weather\",\"sw\":\"1.0.4\"},\"name\":\"Wind "
      "sensor\",\"uniq_id\":\"supla_16055\",\"qos\":0,\"unit_of_meas\":\"m/"
      "s\",\"stat_t\":\"~/state/value\",\"val_tpl\":\"{{ value | "
      "round(1,default=none) }}\",\"state_class\":\"measurement\"}",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/channels/16055/"
      "state/connected",
      "false",
      "supla/15cb17b568de00e1ddc3005e4674bbde/devices/9665/channels/16055/"
      "state/value",
      NULL,
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/channels/8453/type",
      "SENSORNO",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/channels/8453/"
      "function",
      "OPENINGSENSOR_DOOR",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/channels/8453/"
      "caption",
      NULL,
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/channels/8453/"
      "hidden",
      "false",
      "homeassistant/binary_sensor/c8e5fd3b528033b98e09e0d1582d1031/8453/"
      "config",
      "{\"avty\":{\"topic\":\"supla/c8e5fd3b528033b98e09e0d1582d1031/devices/"
      "6361/channels/8453/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/"
      "channels/"
      "8453\",\"device\":{\"ids\":\"supla-iodevice-6361\",\"mf\":\"Itead\","
      "\"name\":\"SONOFF-SV by @slawek\",\"sw\":\"2.5.5s\"},\"name\":\"Door "
      "opening "
      "sensor\",\"uniq_id\":\"supla_8453\",\"qos\":0,\"stat_t\":\"~/state/"
      "hi\",\"pl_on\":\"false\",\"pl_off\":\"true\",\"device_class\":\"door\"}",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/channels/8453/state/"
      "connected",
      "false",
      "supla/c8e5fd3b528033b98e09e0d1582d1031/devices/6361/channels/8453/state/"
      "hi",
      NULL,
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18100/"
      "type",
      "ACTION_TRIGGER",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18100/"
      "function",
      "ACTION_TRIGGER",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18100/"
      "caption",
      NULL,
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18100/"
      "hidden",
      "false",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18100_0/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18100\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18100_0\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "turn_on\",\"type\":\"button_turn_on\",\"stype\":\"button_1\",\"pl\":"
      "\"turn_on\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18100_1/config",
      NULL,
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18100_2/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18100\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18100_2\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x1\",\"type\":\"button_short_press\",\"stype\":\"button_1\","
      "\"pl\":\"toggle_x1\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18100_3/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18100\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18100_3\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x2\",\"type\":\"button_double_press\",\"stype\":\"button_1\","
      "\"pl\":\"toggle_x2\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18100_4/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18100\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18100_4\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x3\",\"type\":\"button_triple_press\",\"stype\":\"button_1\","
      "\"pl\":\"toggle_x3\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18100_5/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18100\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18100_5\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x4\",\"type\":\"button_quadruple_press\",\"stype\":\"button_1\","
      "\"pl\":\"toggle_x4\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18100_6/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18100\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18100_6\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x5\",\"type\":\"button_quintuple_press\",\"stype\":\"button_1\","
      "\"pl\":\"toggle_x5\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18100_7/config",
      NULL,
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18100/"
      "state/connected",
      "false",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18101/"
      "type",
      "ACTION_TRIGGER",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18101/"
      "function",
      "ACTION_TRIGGER",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18101/"
      "caption",
      NULL,
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18101/"
      "hidden",
      "false",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18101_0/config",
      NULL,
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18101_1/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18101\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18101_1\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "turn_off\",\"type\":\"button_turn_off\",\"stype\":\"button_2\",\"pl\":"
      "\"turn_off\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18101_2/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18101\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18101_2\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x1\",\"type\":\"button_short_press\",\"stype\":\"button_2\","
      "\"pl\":\"toggle_x1\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18101_3/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18101\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18101_3\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x2\",\"type\":\"button_double_press\",\"stype\":\"button_2\","
      "\"pl\":\"toggle_x2\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18101_4/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18101\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18101_4\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x3\",\"type\":\"button_triple_press\",\"stype\":\"button_2\","
      "\"pl\":\"toggle_x3\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18101_5/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18101\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18101_5\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x4\",\"type\":\"button_quadruple_press\",\"stype\":\"button_2\","
      "\"pl\":\"toggle_x4\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18101_6/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18101\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18101_6\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x5\",\"type\":\"button_quintuple_press\",\"stype\":\"button_2\","
      "\"pl\":\"toggle_x5\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18101_7/config",
      NULL,
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18101/"
      "state/connected",
      "false",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18102/"
      "type",
      "ACTION_TRIGGER",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18102/"
      "function",
      "ACTION_TRIGGER",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18102/"
      "caption",
      NULL,
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18102/"
      "hidden",
      "false",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18102_0/config",
      NULL,
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18102_1/config",
      NULL,
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18102_2/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18102\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18102_2\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "press_x1\",\"type\":\"button_short_press\",\"stype\":\"button_3\","
      "\"pl\":\"press_x1\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18102_3/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18102\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18102_3\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "press_x2\",\"type\":\"button_double_press\",\"stype\":\"button_3\","
      "\"pl\":\"press_x2\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18102_4/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18102\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18102_4\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "press_x3\",\"type\":\"button_triple_press\",\"stype\":\"button_3\","
      "\"pl\":\"press_x3\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18102_5/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18102\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18102_5\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "press_x4\",\"type\":\"button_quadruple_press\",\"stype\":\"button_3\","
      "\"pl\":\"press_x4\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18102_6/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18102\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18102_6\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "press_x5\",\"type\":\"button_quintuple_press\",\"stype\":\"button_3\","
      "\"pl\":\"press_x5\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18102_7/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18102\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18102_7\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "hold\",\"type\":\"button_long_press\",\"stype\":\"button_3\",\"pl\":"
      "\"hold\"}",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18102/"
      "state/connected",
      "false",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18103/"
      "type",
      "ACTION_TRIGGER",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18103/"
      "function",
      "ACTION_TRIGGER",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18103/"
      "caption",
      NULL,
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18103/"
      "hidden",
      "false",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18103_0/config",
      NULL,
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18103_1/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18103\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18103_1\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "turn_off\",\"type\":\"button_turn_off\",\"stype\":\"button_4\",\"pl\":"
      "\"turn_off\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18103_2/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18103\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18103_2\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x1\",\"type\":\"button_short_press\",\"stype\":\"button_4\","
      "\"pl\":\"toggle_x1\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18103_3/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18103\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18103_3\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x2\",\"type\":\"button_double_press\",\"stype\":\"button_4\","
      "\"pl\":\"toggle_x2\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18103_4/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18103\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18103_4\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x3\",\"type\":\"button_triple_press\",\"stype\":\"button_4\","
      "\"pl\":\"toggle_x3\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18103_5/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18103\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18103_5\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x4\",\"type\":\"button_quadruple_press\",\"stype\":\"button_4\","
      "\"pl\":\"toggle_x4\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18103_6/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18103\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18103_6\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "toggle_x5\",\"type\":\"button_quintuple_press\",\"stype\":\"button_4\","
      "\"pl\":\"toggle_x5\"}",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18103_7/config",
      "{\"~\":\"supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/"
      "18103\",\"device\":{\"ids\":\"supla-iodevice-10300\",\"mf\":\"\","
      "\"name\":\"ZAMEL RNW-01\",\"sw\":\"2.8.47\"},\"name\":\"Action "
      "trigger\",\"uniq_id\":\"supla_18103_7\",\"qos\":0,\"atype\":\"trigger\","
      "\"t\":\"~/action/"
      "hold\",\"type\":\"button_long_press\",\"stype\":\"button_4\",\"pl\":"
      "\"hold\"}",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18103/"
      "state/connected",
      "false",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18104/"
      "type",
      "ACTION_TRIGGER",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18104/"
      "function",
      "ACTION_TRIGGER",
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18104/"
      "caption",
      NULL,
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18104/"
      "hidden",
      "false",
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18104_0/config",
      NULL,
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18104_1/config",
      NULL,
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18104_2/config",
      NULL,
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18104_3/config",
      NULL,
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18104_4/config",
      NULL,
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18104_5/config",
      NULL,
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18104_6/config",
      NULL,
      "homeassistant/device_automation/aae5fd3b528033b98e09e0d1582d1031/"
      "18104_7/config",
      NULL,
      "supla/aae5fd3b528033b98e09e0d1582d1031/devices/10300/channels/18104/"
      "state/connected",
      "false"};

  verify_published(expectedData, sizeof(expectedData) / sizeof(void *));
}

TEST_F(MqttPublisherIntegrationTest, deviceScope) {
  waitForConnection();
  waitForPublications(690);
  getLibAdapter()->published_clear();

  getDS()->on_userdata_changed(2487);
  waitForPublications(30);
  getLibAdapter()->published_clear();

  getDS()->on_devicedata_changed(1802, 9717);
  waitForPublications(21);

  // print_expected();

  const char *expectedData[] = {
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/enabled",
      "true",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/last_connected",
      "2020-11-11T12:41:52Z",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/last_ipv4",
      "188.137.114.68",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/manufacturer",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/name",
      "IMPULSE COUNTER",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/proto_ver",
      "10",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/soft_ver",
      "2.7.2",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/type",
      "IMPULSECOUNTER",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "function",
      "IC_ELECTRICITYMETER",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "caption",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "hidden",
      "false",
      "homeassistant/sensor/64d3d5bd59eb5fe2b67bf4fe4d222905/16166_0/config",
      "{\"avty\":{\"topic\":\"supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/"
      "9717/channels/16166/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/"
      "channels/"
      "16166\",\"device\":{\"ids\":\"supla-iodevice-9717\",\"mf\":\"\","
      "\"name\":\"IMPULSE COUNTER\",\"sw\":\"2.7.2\"},\"name\":\"Electricity "
      "meter "
      "(Value)\",\"uniq_id\":\"supla_16166_0\",\"qos\":0,\"unit_of_meas\":"
      "\"kWh\",\"stat_t\":\"~/state/calculated_value\",\"val_tpl\":\"{{ value "
      "| round(3,default=none) "
      "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}",
      "homeassistant/sensor/64d3d5bd59eb5fe2b67bf4fe4d222905/16166_1/config",
      "{\"avty\":{\"topic\":\"supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/"
      "9717/channels/16166/state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/"
      "channels/"
      "16166\",\"device\":{\"ids\":\"supla-iodevice-9717\",\"mf\":\"\","
      "\"name\":\"IMPULSE COUNTER\",\"sw\":\"2.7.2\"},\"name\":\"Electricity "
      "meter (Total "
      "cost)\",\"uniq_id\":\"supla_16166_1\",\"qos\":0,\"unit_of_meas\":"
      "\"PLN\",\"stat_t\":\"~/state/total_cost\",\"val_tpl\":\"{{ value | "
      "round(2,default=none) "
      "}}\",\"dev_cla\":\"monetary\",\"state_class\":\"measurement\"}",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/connected",
      "false",
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/total_cost",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/price_per_unit",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/impulses_per_unit",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/counter",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/calculated_value",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/currency",
      NULL,
      "supla/64d3d5bd59eb5fe2b67bf4fe4d222905/devices/9717/channels/16166/"
      "state/unit",
      NULL};

  verify_published(expectedData, sizeof(expectedData) / sizeof(void *));
}

TEST_F(MqttPublisherIntegrationTest, actionTrigger) {
  if (!supla_user::find(3748, false)) {
    new supla_user(3748, "aa4f2278a5dbdcdb4fa52278dbdca54f", "");
  }

  waitForConnection();
  waitForPublications(690);
  getLibAdapter()->published_clear();

  dynamic_cast<supla_mqtt_publisher_datasource *>(getDS())
      ->on_actions_triggered(3748, 10300, 18103, 0xFFFFFFFF);

  waitForActions(13);
  // print_expected();

  {
    const char *expectedData[] = {
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/turn_on",
        "turn_on",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/turn_off",
        "turn_off",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/toggle_x1",
        "toggle_x1",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/toggle_x2",
        "toggle_x2",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/toggle_x3",
        "toggle_x3",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/toggle_x4",
        "toggle_x4",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/toggle_x5",
        "toggle_x5",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/hold",
        "hold",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/press_x1",
        "press_x1",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/press_x2",
        "press_x2",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/press_x3",
        "press_x3",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/press_x4",
        "press_x4",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/18103/"
        "action/press_x5",
        "press_x5"};

    verify_published(expectedData, sizeof(expectedData) / sizeof(void *));
  }

  getLibAdapter()->published_clear();

  dynamic_cast<supla_mqtt_publisher_datasource *>(getDS())
      ->on_actions_triggered(3748, 10300, 5, SUPLA_ACTION_CAP_SHORT_PRESS_x2);
  dynamic_cast<supla_mqtt_publisher_datasource *>(getDS())
      ->on_actions_triggered(3748, 10300, 6, SUPLA_ACTION_CAP_HOLD);
  dynamic_cast<supla_mqtt_publisher_datasource *>(getDS())
      ->on_actions_triggered(3748, 10300, 5, SUPLA_ACTION_CAP_SHORT_PRESS_x3);

  waitForActions(3);
  // print_expected();

  {
    const char *expectedData[] = {
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/5/"
        "action/press_x2",
        "press_x2",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/5/"
        "action/press_x3",
        "press_x3",
        "supla/aa4f2278a5dbdcdb4fa52278dbdca54f/devices/10300/channels/6/"
        "action/hold",
        "hold"};

    verify_published(expectedData, sizeof(expectedData) / sizeof(void *));
  }
}

} /* namespace testing */
