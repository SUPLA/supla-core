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

#include "MqttChannelMessageProviderTest.h"

namespace testing {

MqttChannelMessageProviderTest::MqttChannelMessageProviderTest(void)
    : MqttMessageProviderTest() {
  provider = NULL;
}
MqttChannelMessageProviderTest::~MqttChannelMessageProviderTest(void) {}

void MqttChannelMessageProviderTest::SetUp() {
  provider = new supla_mqtt_channel_message_provider();
}

void MqttChannelMessageProviderTest::TearDown() { delete provider; }

void MqttChannelMessageProviderTest::fillChannelData(
    _mqtt_db_data_row_channel_t *row_channel) {
  *row_channel = {};

  snprintf(row_channel->user_suid, SHORT_UNIQUEID_MAXSIZE,
           "7720767494dd87196e1896c7cbab707c");
  row_channel->device_id = 555;

  row_channel->device_mfr_id = SUPLA_MFR_ACSOFTWARE;
  snprintf(row_channel->device_name, SUPLA_DEVICE_NAME_MAXSIZE, "SMART SWITCH");
  snprintf(row_channel->device_softver, SUPLA_SOFTVER_MAXSIZE, "1.0");

  row_channel->device_enabled = true;
  row_channel->channel_id = 754;
  row_channel->channel_type = SUPLA_CHANNELTYPE_RELAY;
  row_channel->channel_func = SUPLA_CHANNELFNC_POWERSWITCH;
  snprintf(row_channel->channel_location, SUPLA_LOCATION_CAPTION_MAXSIZE,
           "Second floor");
  snprintf(row_channel->channel_caption, SUPLA_CHANNEL_CAPTION_MAXSIZE,
           "Socket");
  row_channel->channel_hidden = false;
}

TEST_F(MqttChannelMessageProviderTest, powerSwitch) {
  _mqtt_db_data_row_channel_t row_channel;
  fillChannelData(&row_channel);
  provider->set_data_row(&row_channel);

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "RELAY", false,
      "supla/7720767494dd87196e1896c7cbab707c/devices/%i/channels/%i/type",
      row_channel.device_id, row_channel.channel_id));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "POWERSWITCH", false,
      "supla/7720767494dd87196e1896c7cbab707c/devices/%i/channels/%i/function",
      row_channel.device_id, row_channel.channel_id));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "Socket", false,
      "supla/7720767494dd87196e1896c7cbab707c/devices/%i/channels/%i/caption",
      row_channel.device_id, row_channel.channel_id));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "false", false,
      "supla/7720767494dd87196e1896c7cbab707c/devices/%i/channels/%i/hidden",
      row_channel.device_id, row_channel.channel_id));

  const char haConfig[] =
      "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/devices/"
      "555/channels/754/"
      "state/"
      "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
      "\"false\"},\"~\":\"supla/7720767494dd87196e1896c7cbab707c/devices/555/"
      "channels/"
      "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
      "SOFTWARE\",\"name\":\"SMART "
      "SWITCH\",\"sw\":\"1.0\"},\"name\":\"Socket\",\"uniq_id\":\"supla_754\","
      "\"qos\":0,\"ret\":false,\"opt\":false,\"stat_t\":\"~/state/"
      "on\",\"cmd_t\":\"~/set/"
      "on\",\"stat_on\":\"true\",\"stat_off\":\"false\",\"pl_on\":\"true\","
      "\"pl_off\":\"false\"}";

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, haConfig, false,
      "homeassistant/switch/7720767494dd87196e1896c7cbab707c/754/config"));

  ASSERT_FALSE(dataExists(provider));
}

void MqttChannelMessageProviderTest::electricityMeterTest(int channel_flags) {
  _mqtt_db_data_row_channel_t row_channel;
  fillChannelData(&row_channel);
  provider->set_data_row(&row_channel);

  snprintf(row_channel.device_name, SUPLA_DEVICE_NAME_MAXSIZE,
           "Electricity Meter");
  snprintf(row_channel.device_softver, SUPLA_SOFTVER_MAXSIZE, "1.0");

  row_channel.device_enabled = true;
  row_channel.channel_id = 754;
  row_channel.channel_type = SUPLA_CHANNELTYPE_ELECTRICITY_METER;
  row_channel.channel_func = SUPLA_CHANNELFNC_ELECTRICITY_METER;
  row_channel.channel_flags = channel_flags;

  snprintf(row_channel.channel_caption, SUPLA_CHANNEL_CAPTION_MAXSIZE, "EM");

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "ELECTRICITYMETER", false,
      "supla/7720767494dd87196e1896c7cbab707c/devices/%i/channels/%i/type",
      row_channel.device_id, row_channel.channel_id));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "ELECTRICITYMETER", false,
      "supla/7720767494dd87196e1896c7cbab707c/devices/%i/channels/%i/function",
      row_channel.device_id, row_channel.channel_id));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "EM", false,
      "supla/7720767494dd87196e1896c7cbab707c/devices/%i/channels/%i/caption",
      row_channel.device_id, row_channel.channel_id));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "false", false,
      "supla/7720767494dd87196e1896c7cbab707c/devices/%i/channels/%i/hidden",
      row_channel.device_id, row_channel.channel_id));

  {
    const char haConfig[] =
        "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/devices/"
        "555/channels/754/state/"
        "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
        "\"false\"},\"~\":\"supla/7720767494dd87196e1896c7cbab707c/devices/555/"
        "channels/754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
        "SOFTWARE\",\"name\":\"Electricity "
        "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total "
        "cost)\",\"uniq_id\":\"supla_754_0\",\"qos\":0,\"unit_of_meas\":\"\","
        "\"stat_t\":\"~/state/total_cost\",\"val_tpl\":\"{{ value | "
        "round(2,default=none) "
        "}}\",\"dev_cla\":\"monetary\",\"state_class\":\"measurement\"}";

    ASSERT_TRUE(fetchAndCompare(
        provider, NULL, haConfig, false,
        "homeassistant/sensor/7720767494dd87196e1896c7cbab707c/754_0/config"));
  }

  {
    const char haConfig[] =
        "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/devices/"
        "555/channels/754/state/"
        "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
        "\"false\"},\"~\":\"supla/7720767494dd87196e1896c7cbab707c/devices/555/"
        "channels/754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
        "SOFTWARE\",\"name\":\"Electricity "
        "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total cost - "
        "balanced)\",\"uniq_id\":\"supla_754_1\",\"qos\":0,\"unit_of_meas\":"
        "\"\",\"stat_t\":\"~/state/total_cost_balanced\",\"val_tpl\":\"{{ "
        "value | round(2,default=none) "
        "}}\",\"dev_cla\":\"monetary\",\"state_class\":\"measurement\"}";

    ASSERT_TRUE(fetchAndCompare(
        provider, NULL, haConfig, false,
        "homeassistant/sensor/7720767494dd87196e1896c7cbab707c/754_1/config"));
  }

  {
    const char haConfig[] =
        "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/devices/"
        "555/channels/754/state/"
        "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
        "\"false\"},\"~\":\"supla/7720767494dd87196e1896c7cbab707c/devices/555/"
        "channels/754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
        "SOFTWARE\",\"name\":\"Electricity "
        "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total forward active "
        "energy)\",\"uniq_id\":\"supla_754_2\",\"qos\":0,\"unit_of_meas\":"
        "\"kWh\",\"stat_t\":\"~/state/"
        "total_forward_active_energy\",\"val_tpl\":\"{{ value | "
        "round(5,default=none) "
        "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}";

    ASSERT_TRUE(fetchAndCompare(
        provider, NULL, haConfig, false,
        "homeassistant/sensor/7720767494dd87196e1896c7cbab707c/754_2/config"));
  }

  {
    const char haConfig[] =
        "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/devices/"
        "555/channels/754/state/"
        "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
        "\"false\"},\"~\":\"supla/7720767494dd87196e1896c7cbab707c/devices/555/"
        "channels/754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
        "SOFTWARE\",\"name\":\"Electricity "
        "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total reverse active "
        "energy)\",\"uniq_id\":\"supla_754_3\",\"qos\":0,\"unit_of_meas\":"
        "\"kWh\",\"stat_t\":\"~/state/"
        "total_reverse_active_energy\",\"val_tpl\":\"{{ value | "
        "round(5,default=none) "
        "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}";

    ASSERT_TRUE(fetchAndCompare(
        provider, NULL, haConfig, false,
        "homeassistant/sensor/7720767494dd87196e1896c7cbab707c/754_3/config"));
  }

  {
    const char haConfig[] =
        "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/devices/"
        "555/channels/754/state/"
        "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
        "\"false\"},\"~\":\"supla/7720767494dd87196e1896c7cbab707c/devices/555/"
        "channels/754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
        "SOFTWARE\",\"name\":\"Electricity "
        "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total forward active energy - "
        "balanced)\",\"uniq_id\":\"supla_754_4\",\"qos\":0,\"unit_of_meas\":"
        "\"kWh\",\"stat_t\":\"~/state/"
        "total_forward_active_energy_balanced\",\"val_tpl\":\"{{ value | "
        "round(5,default=none) "
        "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}";

    ASSERT_TRUE(fetchAndCompare(
        provider, NULL, haConfig, false,
        "homeassistant/sensor/7720767494dd87196e1896c7cbab707c/754_4/config"));
  }

  {
    const char haConfig[] =
        "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/devices/"
        "555/channels/754/state/"
        "connected\",\"payload_available\":\"true\",\"payload_not_available\":"
        "\"false\"},\"~\":\"supla/7720767494dd87196e1896c7cbab707c/devices/555/"
        "channels/754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
        "SOFTWARE\",\"name\":\"Electricity "
        "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total reverse active energy - "
        "balanced)\",\"uniq_id\":\"supla_754_5\",\"qos\":0,\"unit_of_meas\":"
        "\"kWh\",\"stat_t\":\"~/state/"
        "total_reverse_active_energy_balanced\",\"val_tpl\":\"{{ value | "
        "round(5,default=none) "
        "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}";

    ASSERT_TRUE(fetchAndCompare(
        provider, NULL, haConfig, false,
        "homeassistant/sensor/7720767494dd87196e1896c7cbab707c/754_5/config"));
  }

  if (!(channel_flags & SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED)) {
    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total forward active energy "
          "- Phase "
          "1)\",\"uniq_id\":\"supla_754_6\",\"qos\":0,\"unit_of_meas\":\"kWh\","
          "\"stat_t\":\"~/state/phases/1/"
          "total_forward_active_energy\",\"val_tpl\":\"{{ value | "
          "round(5,default=none) "
          "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_6/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total reverse active energy "
          "- Phase "
          "1)\",\"uniq_id\":\"supla_754_7\",\"qos\":0,\"unit_of_meas\":\"kWh\","
          "\"stat_t\":\"~/state/phases/1/"
          "total_reverse_active_energy\",\"val_tpl\":\"{{ value | "
          "round(5,default=none) "
          "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_7/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total forward reactive "
          "energy - Phase "
          "1)\",\"uniq_id\":\"supla_754_8\",\"qos\":0,\"unit_of_meas\":"
          "\"kvarh\",\"stat_t\":\"~/state/phases/1/"
          "total_forward_reactive_energy\",\"val_tpl\":\"{{ value | "
          "round(5,default=none) "
          "}}\",\"state_class\":\"total_increasing\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_8/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total reverse reactive "
          "energy - Phase "
          "1)\",\"uniq_id\":\"supla_754_9\",\"qos\":0,\"unit_of_meas\":"
          "\"kvarh\",\"stat_t\":\"~/state/phases/1/"
          "total_reverse_reactive_energy\",\"val_tpl\":\"{{ value | "
          "round(5,default=none) "
          "}}\",\"state_class\":\"total_increasing\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_9/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Frequency - Phase "
          "1)\",\"uniq_id\":\"supla_754_10\",\"qos\":0,\"unit_of_meas\":\"Hz\","
          "\"stat_t\":\"~/state/phases/1/frequency\",\"val_tpl\":\"{{ value | "
          "round(2,default=none) }}\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_10/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Voltage - Phase "
          "1)\",\"uniq_id\":\"supla_754_11\",\"qos\":0,\"unit_of_meas\":\"V\","
          "\"stat_t\":\"~/state/phases/1/voltage\",\"val_tpl\":\"{{ value | "
          "round(2,default=none) "
          "}}\",\"dev_cla\":\"voltage\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_11/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Current - Phase "
          "1)\",\"uniq_id\":\"supla_754_12\",\"qos\":0,\"unit_of_meas\":\"A\","
          "\"stat_t\":\"~/state/phases/1/current\",\"val_tpl\":\"{{ value | "
          "round(3,default=none) "
          "}}\",\"dev_cla\":\"current\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_12/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Power active - Phase "
          "1)\",\"uniq_id\":\"supla_754_13\",\"qos\":0,\"unit_of_meas\":\"W\","
          "\"stat_t\":\"~/state/phases/1/power_active\",\"val_tpl\":\"{{ value "
          "| round(5,default=none) "
          "}}\",\"dev_cla\":\"power\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_13/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Power reactive - Phase "
          "1)\",\"uniq_id\":\"supla_754_14\",\"qos\":0,\"unit_of_meas\":"
          "\"var\",\"stat_t\":\"~/state/phases/1/"
          "power_reactive\",\"val_tpl\":\"{{ value | round(5,default=none) "
          "}}\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_14/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Power apparent - Phase "
          "1)\",\"uniq_id\":\"supla_754_15\",\"qos\":0,\"unit_of_meas\":\"VA\","
          "\"stat_t\":\"~/state/phases/1/power_apparent\",\"val_tpl\":\"{{ "
          "value | round(5,default=none) }}\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_15/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Power factor - Phase "
          "1)\",\"uniq_id\":\"supla_754_16\",\"qos\":0,\"unit_of_meas\":\"%\","
          "\"stat_t\":\"~/state/phases/1/"
          "power_factor\",\"val_tpl\":\"{% if float(value, default=none) == "
          "None %}None{% else %}{{float(value) * 100.0 | round(5)}}{% endif "
          "%}\",\"dev_cla\":\"power_factor\",\"state_class\":"
          "\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_16/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Phase angle - Phase "
          "1)\",\"uniq_id\":\"supla_754_17\",\"qos\":0,\"unit_of_meas\":\"°\","
          "\"stat_t\":\"~/state/phases/1/phase_angle\",\"val_tpl\":\"{{ value "
          "| round(1,default=none) }}\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_17/config"));
    }
  }

  // PHASE 2
  if (!(channel_flags & SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED)) {
    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total forward active energy "
          "- Phase "
          "2)\",\"uniq_id\":\"supla_754_18\",\"qos\":0,\"unit_of_meas\":"
          "\"kWh\",\"stat_t\":\"~/state/phases/2/"
          "total_forward_active_energy\",\"val_tpl\":\"{{ value | "
          "round(5,default=none) "
          "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_18/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total reverse active energy "
          "- Phase "
          "2)\",\"uniq_id\":\"supla_754_19\",\"qos\":0,\"unit_of_meas\":"
          "\"kWh\",\"stat_t\":\"~/state/phases/2/"
          "total_reverse_active_energy\",\"val_tpl\":\"{{ value | "
          "round(5,default=none) "
          "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_19/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total forward reactive "
          "energy - Phase "
          "2)\",\"uniq_id\":\"supla_754_20\",\"qos\":0,\"unit_of_meas\":"
          "\"kvarh\",\"stat_t\":\"~/state/phases/2/"
          "total_forward_reactive_energy\",\"val_tpl\":\"{{ value | "
          "round(5,default=none) "
          "}}\",\"state_class\":\"total_increasing\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_20/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total reverse reactive "
          "energy - Phase "
          "2)\",\"uniq_id\":\"supla_754_21\",\"qos\":0,\"unit_of_meas\":"
          "\"kvarh\",\"stat_t\":\"~/state/phases/2/"
          "total_reverse_reactive_energy\",\"val_tpl\":\"{{ value | "
          "round(5,default=none) "
          "}}\",\"state_class\":\"total_increasing\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_21/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Frequency - Phase "
          "2)\",\"uniq_id\":\"supla_754_22\",\"qos\":0,\"unit_of_meas\":\"Hz\","
          "\"stat_t\":\"~/state/phases/2/frequency\",\"val_tpl\":\"{{ value | "
          "round(2,default=none) }}\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_22/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Voltage - Phase "
          "2)\",\"uniq_id\":\"supla_754_23\",\"qos\":0,\"unit_of_meas\":\"V\","
          "\"stat_t\":\"~/state/phases/2/voltage\",\"val_tpl\":\"{{ value | "
          "round(2,default=none) "
          "}}\",\"dev_cla\":\"voltage\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_23/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Current - Phase "
          "2)\",\"uniq_id\":\"supla_754_24\",\"qos\":0,\"unit_of_meas\":\"A\","
          "\"stat_t\":\"~/state/phases/2/current\",\"val_tpl\":\"{{ value | "
          "round(3,default=none) "
          "}}\",\"dev_cla\":\"current\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_24/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Power active - Phase "
          "2)\",\"uniq_id\":\"supla_754_25\",\"qos\":0,\"unit_of_meas\":\"W\","
          "\"stat_t\":\"~/state/phases/2/power_active\",\"val_tpl\":\"{{ value "
          "| round(5,default=none) "
          "}}\",\"dev_cla\":\"power\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_25/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Power reactive - Phase "
          "2)\",\"uniq_id\":\"supla_754_26\",\"qos\":0,\"unit_of_meas\":"
          "\"var\",\"stat_t\":\"~/state/phases/2/"
          "power_reactive\",\"val_tpl\":\"{{ value | round(5,default=none) "
          "}}\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_26/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Power apparent - Phase "
          "2)\",\"uniq_id\":\"supla_754_27\",\"qos\":0,\"unit_of_meas\":\"VA\","
          "\"stat_t\":\"~/state/phases/2/power_apparent\",\"val_tpl\":\"{{ "
          "value | round(5,default=none) }}\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_27/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Power factor - Phase "
          "2)\",\"uniq_id\":\"supla_754_28\",\"qos\":0,\"unit_of_meas\":\"%\","
          "\"stat_t\":\"~/state/phases/2/"
          "power_factor\",\"val_tpl\":\"{% if float(value, default=none) == "
          "None %}None{% else %}{{float(value) * 100.0 | round(5)}}{% endif "
          "%}\",\"dev_cla\":\"power_factor\",\"state_class\":"
          "\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_28/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Phase angle - Phase "
          "2)\",\"uniq_id\":\"supla_754_29\",\"qos\":0,\"unit_of_meas\":\"°\","
          "\"stat_t\":\"~/state/phases/2/phase_angle\",\"val_tpl\":\"{{ value "
          "| round(1,default=none) }}\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_29/config"));
    }
  }

  // PHASE 3

  if (!(channel_flags & SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED)) {
    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total forward active energy "
          "- Phase "
          "3)\",\"uniq_id\":\"supla_754_30\",\"qos\":0,\"unit_of_meas\":"
          "\"kWh\",\"stat_t\":\"~/state/phases/3/"
          "total_forward_active_energy\",\"val_tpl\":\"{{ value | "
          "round(5,default=none) "
          "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_30/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total reverse active energy "
          "- Phase "
          "3)\",\"uniq_id\":\"supla_754_31\",\"qos\":0,\"unit_of_meas\":"
          "\"kWh\",\"stat_t\":\"~/state/phases/3/"
          "total_reverse_active_energy\",\"val_tpl\":\"{{ value | "
          "round(5,default=none) "
          "}}\",\"dev_cla\":\"energy\",\"state_class\":\"total_increasing\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_31/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total forward reactive "
          "energy - Phase "
          "3)\",\"uniq_id\":\"supla_754_32\",\"qos\":0,\"unit_of_meas\":"
          "\"kvarh\",\"stat_t\":\"~/state/phases/3/"
          "total_forward_reactive_energy\",\"val_tpl\":\"{{ value | "
          "round(5,default=none) "
          "}}\",\"state_class\":\"total_increasing\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_32/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Total reverse reactive "
          "energy - Phase "
          "3)\",\"uniq_id\":\"supla_754_33\",\"qos\":0,\"unit_of_meas\":"
          "\"kvarh\",\"stat_t\":\"~/state/phases/3/"
          "total_reverse_reactive_energy\",\"val_tpl\":\"{{ value | "
          "round(5,default=none) "
          "}}\",\"state_class\":\"total_increasing\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_33/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Frequency - Phase "
          "3)\",\"uniq_id\":\"supla_754_34\",\"qos\":0,\"unit_of_meas\":\"Hz\","
          "\"stat_t\":\"~/state/phases/3/frequency\",\"val_tpl\":\"{{ value | "
          "round(2,default=none) }}\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_34/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Voltage - Phase "
          "3)\",\"uniq_id\":\"supla_754_35\",\"qos\":0,\"unit_of_meas\":\"V\","
          "\"stat_t\":\"~/state/phases/3/voltage\",\"val_tpl\":\"{{ value | "
          "round(2,default=none) "
          "}}\",\"dev_cla\":\"voltage\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_35/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Current - Phase "
          "3)\",\"uniq_id\":\"supla_754_36\",\"qos\":0,\"unit_of_meas\":\"A\","
          "\"stat_t\":\"~/state/phases/3/current\",\"val_tpl\":\"{{ value | "
          "round(3,default=none) "
          "}}\",\"dev_cla\":\"current\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_36/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Power active - Phase "
          "3)\",\"uniq_id\":\"supla_754_37\",\"qos\":0,\"unit_of_meas\":\"W\","
          "\"stat_t\":\"~/state/phases/3/power_active\",\"val_tpl\":\"{{ value "
          "| round(5,default=none) "
          "}}\",\"dev_cla\":\"power\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_37/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Power reactive - Phase "
          "3)\",\"uniq_id\":\"supla_754_38\",\"qos\":0,\"unit_of_meas\":"
          "\"var\",\"stat_t\":\"~/state/phases/3/"
          "power_reactive\",\"val_tpl\":\"{{ value | round(5,default=none) "
          "}}\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_38/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Power apparent - Phase "
          "3)\",\"uniq_id\":\"supla_754_39\",\"qos\":0,\"unit_of_meas\":\"VA\","
          "\"stat_t\":\"~/state/phases/3/power_apparent\",\"val_tpl\":\"{{ "
          "value | round(5,default=none) }}\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_39/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Power factor - Phase "
          "3)\",\"uniq_id\":\"supla_754_40\",\"qos\":0,\"unit_of_meas\":\"%\","
          "\"stat_t\":\"~/state/phases/3/"
          "power_factor\",\"val_tpl\":\"{% if float(value, default=none) == "
          "None %}None{% else %}{{float(value) * 100.0 | round(5)}}{% endif "
          "%}\",\"dev_cla\":\"power_factor\",\"state_class\":"
          "\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_40/config"));
    }

    {
      const char haConfig[] =
          "{\"avty\":{\"topic\":\"supla/7720767494dd87196e1896c7cbab707c/"
          "devices/555/channels/754/state/"
          "connected\",\"payload_available\":\"true\",\"payload_not_"
          "available\":\"false\"},\"~\":\"supla/"
          "7720767494dd87196e1896c7cbab707c/devices/555/channels/"
          "754\",\"device\":{\"ids\":\"supla-iodevice-555\",\"mf\":\"AC "
          "SOFTWARE\",\"name\":\"Electricity "
          "Meter\",\"sw\":\"1.0\"},\"name\":\"EM (Phase angle - Phase "
          "3)\",\"uniq_id\":\"supla_754_41\",\"qos\":0,\"unit_of_meas\":\"°\","
          "\"stat_t\":\"~/state/phases/3/phase_angle\",\"val_tpl\":\"{{ value "
          "| round(1,default=none) }}\",\"state_class\":\"measurement\"}";

      ASSERT_TRUE(
          fetchAndCompare(provider, NULL, haConfig, false,
                          "homeassistant/sensor/"
                          "7720767494dd87196e1896c7cbab707c/754_41/config"));
    }
  }

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttChannelMessageProviderTest, electricityMeter) {
  electricityMeterTest(0);
}

TEST_F(MqttChannelMessageProviderTest, electricityMeterWithout1stPhase) {
  electricityMeterTest(SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED);
}

TEST_F(MqttChannelMessageProviderTest, electricityMeterWithout2ndPhase) {
  electricityMeterTest(SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED);
}

TEST_F(MqttChannelMessageProviderTest, electricityMeterWithout3rdPhase) {
  electricityMeterTest(SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED);
}

TEST_F(MqttChannelMessageProviderTest, electricityMeterWithout2ndAnd3rdPhase) {
  electricityMeterTest(SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
                       SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED);
}

TEST_F(MqttChannelMessageProviderTest, electricityMeterWithout1stAnd2ndPhase) {
  electricityMeterTest(SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                       SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED);
}

TEST_F(MqttChannelMessageProviderTest, electricityMeterWithoutPhases) {
  electricityMeterTest(SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED |
                       SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
                       SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED);
}

} /* namespace testing */
