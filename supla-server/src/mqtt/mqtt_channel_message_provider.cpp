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

#include <mqtt_channel_message_provider.h>

supla_mqtt_channel_message_provider::supla_mqtt_channel_message_provider(void)
    : supla_mqtt_message_provider() {
  row = NULL;
}

supla_mqtt_channel_message_provider::~supla_mqtt_channel_message_provider(
    void) {}

void supla_mqtt_channel_message_provider::channel_type_to_string(
    int type, char *buf, size_t buf_size) {
  // Names adapted to
  // https://github.com/SUPLA/supla-cloud/blob/b0afbfba770c4426154684668782aacbab82d0ee/src/SuplaBundle/Enums/ChannelType.php#L26
  switch (type) {
    case SUPLA_CHANNELTYPE_SENSORNO:
      snprintf(buf, buf_size, "SENSORNO");
      break;
    case SUPLA_CHANNELTYPE_SENSORNC:
      snprintf(buf, buf_size, "SENSORNC");
      break;
    case SUPLA_CHANNELTYPE_DISTANCESENSOR:
      snprintf(buf, buf_size, "SENSORNC");
      break;
    case SUPLA_CHANNELTYPE_CALLBUTTON:
      snprintf(buf, buf_size, "CALLBUTTON");
      break;
    case SUPLA_CHANNELTYPE_RELAYHFD4:
      snprintf(buf, buf_size, "RELAYHFD4");
      break;
    case SUPLA_CHANNELTYPE_RELAYG5LA1A:
      snprintf(buf, buf_size, "RELAYG5LA1A");
      break;
    case SUPLA_CHANNELTYPE_2XRELAYG5LA1A:
      snprintf(buf, buf_size, "RELAY2XG5LA1A");
      break;
    case SUPLA_CHANNELTYPE_RELAY:
      snprintf(buf, buf_size, "RELAY");
      break;
    case SUPLA_CHANNELTYPE_THERMOMETERDS18B20:
      snprintf(buf, buf_size, "THERMOMETERDS18B20");
      break;
    case SUPLA_CHANNELTYPE_DHT11:
      snprintf(buf, buf_size, "DHT11");
      break;
    case SUPLA_CHANNELTYPE_DHT22:
      snprintf(buf, buf_size, "DHT22");
      break;
    case SUPLA_CHANNELTYPE_DHT21:
      snprintf(buf, buf_size, "DHT21");
      break;
    case SUPLA_CHANNELTYPE_AM2302:
      snprintf(buf, buf_size, "AM2302");
      break;
    case SUPLA_CHANNELTYPE_AM2301:
      snprintf(buf, buf_size, "AM2301");
      break;
    case SUPLA_CHANNELTYPE_THERMOMETER:
      snprintf(buf, buf_size, "THERMOMETER");
      break;
    case SUPLA_CHANNELTYPE_HUMIDITYSENSOR:
      snprintf(buf, buf_size, "HUMIDITYSENSOR");
      break;
    case SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR:
      snprintf(buf, buf_size, "HUMIDITYANDTEMPSENSOR");
      break;
    case SUPLA_CHANNELTYPE_WINDSENSOR:
      snprintf(buf, buf_size, "WINDSENSOR");
      break;
    case SUPLA_CHANNELTYPE_PRESSURESENSOR:
      snprintf(buf, buf_size, "PRESSURESENSOR");
      break;
    case SUPLA_CHANNELTYPE_RAINSENSOR:
      snprintf(buf, buf_size, "RAINSENSOR");
      break;
    case SUPLA_CHANNELTYPE_WEIGHTSENSOR:
      snprintf(buf, buf_size, "WEIGHTSENSOR");
      break;
    case SUPLA_CHANNELTYPE_WEATHER_STATION:
      snprintf(buf, buf_size, "WEATHER_STATION");
      break;
    case SUPLA_CHANNELTYPE_DIMMER:
      snprintf(buf, buf_size, "DIMMER");
      break;
    case SUPLA_CHANNELTYPE_RGBLEDCONTROLLER:
      snprintf(buf, buf_size, "RGBLEDCONTROLLER");
      break;
    case SUPLA_CHANNELTYPE_DIMMERANDRGBLED:
      snprintf(buf, buf_size, "DIMMERANDRGBLED");
      break;
    case SUPLA_CHANNELTYPE_ELECTRICITY_METER:
      snprintf(buf, buf_size, "ELECTRICITYMETER");
      break;
    case SUPLA_CHANNELTYPE_IMPULSE_COUNTER:
      snprintf(buf, buf_size, "IMPULSECOUNTER");
      break;
    case SUPLA_CHANNELTYPE_THERMOSTAT:
      snprintf(buf, buf_size, "THERMOSTAT");
      break;
    case SUPLA_CHANNELTYPE_THERMOSTAT_HEATPOL_HOMEPLUS:
      snprintf(buf, buf_size, "THERMOSTATHEATPOLHOMEPLUS");
      break;
    case SUPLA_CHANNELTYPE_VALVE_OPENCLOSE:
      snprintf(buf, buf_size, "VALVEOPENCLOSE");
      break;
    case SUPLA_CHANNELTYPE_VALVE_PERCENTAGE:
      snprintf(buf, buf_size, "VALVEPERCENTAGE");
      break;
    case SUPLA_CHANNELTYPE_BRIDGE:
      snprintf(buf, buf_size, "VALVEBRIDGE");
      break;
    case SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT:
      snprintf(buf, buf_size, "GENERAL_PURPOSE_MEASUREMENT");
      break;
    case SUPLA_CHANNELTYPE_ENGINE:
      snprintf(buf, buf_size, "ENGINE");
      break;
    case SUPLA_CHANNELTYPE_ACTIONTRIGGER:
      snprintf(buf, buf_size, "ACTION_TRIGGER");
      break;
    case SUPLA_CHANNELTYPE_DIGIGLASS:
      snprintf(buf, buf_size, "DIGIGLASS");
      break;
    default:
      buf[0] = 0;
      break;
  }
}

void supla_mqtt_channel_message_provider::channel_function_to_string(
    int func, char *buf, size_t buf_size) {
  // Names adapted to
  // https://github.com/SUPLA/supla-cloud/blob/b0afbfba770c4426154684668782aacbab82d0ee/src/SuplaBundle/Enums/ChannelFunction.php#L28

  switch (func) {
    default:
    case SUPLA_CHANNELFNC_NONE:
      snprintf(buf, buf_size, "NONE");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      snprintf(buf, buf_size, "CONTROLLINGTHEGATEWAYLOCK");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      snprintf(buf, buf_size, "CONTROLLINGTHEGATE");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      snprintf(buf, buf_size, "CONTROLLINGTHEGARAGEDOOR");
      break;
    case SUPLA_CHANNELFNC_THERMOMETER:
      snprintf(buf, buf_size, "THERMOMETER");
      break;
    case SUPLA_CHANNELFNC_HUMIDITY:
      snprintf(buf, buf_size, "HUMIDITY");
      break;
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      snprintf(buf, buf_size, "HUMIDITYANDTEMPERATURE");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
      snprintf(buf, buf_size, "OPENINGSENSOR_GATEWAY");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      snprintf(buf, buf_size, "OPENINGSENSOR_GATE");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      snprintf(buf, buf_size, "OPENINGSENSOR_GARAGEDOOR");
      break;
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
      snprintf(buf, buf_size, "NOLIQUIDSENSOR");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      snprintf(buf, buf_size, "CONTROLLINGTHEDOORLOCK");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      snprintf(buf, buf_size, "OPENINGSENSOR_DOOR");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      snprintf(buf, buf_size, "CONTROLLINGTHEROLLERSHUTTER");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      snprintf(buf, buf_size, "CONTROLLINGTHEROOFWINDOW");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
      snprintf(buf, buf_size, "OPENINGSENSOR_ROLLERSHUTTER");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
      snprintf(buf, buf_size, "OPENINGSENSOR_ROOFWINDOW");
      break;
    case SUPLA_CHANNELFNC_POWERSWITCH:
      snprintf(buf, buf_size, "POWERSWITCH");
      break;
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
      snprintf(buf, buf_size, "LIGHTSWITCH");
      break;
    case SUPLA_CHANNELFNC_RING:
      snprintf(buf, buf_size, "RING");
      break;
    case SUPLA_CHANNELFNC_ALARM:
      snprintf(buf, buf_size, "ALARM");
      break;
    case SUPLA_CHANNELFNC_NOTIFICATION:
      snprintf(buf, buf_size, "NOTIFICATION");
      break;
    case SUPLA_CHANNELFNC_DIMMER:
      snprintf(buf, buf_size, "DIMMER");
      break;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      snprintf(buf, buf_size, "RGBLIGHTING");
      break;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      snprintf(buf, buf_size, "DIMMERANDRGBLIGHTING");
      break;
    case SUPLA_CHANNELFNC_DEPTHSENSOR:
      snprintf(buf, buf_size, "DEPTHSENSOR");
      break;
    case SUPLA_CHANNELFNC_DISTANCESENSOR:
      snprintf(buf, buf_size, "DISTANCESENSOR");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
      snprintf(buf, buf_size, "OPENINGSENSOR_WINDOW");
      break;
    case SUPLA_CHANNELFNC_MAILSENSOR:
      snprintf(buf, buf_size, "MAILSENSOR");
      break;
    case SUPLA_CHANNELFNC_WINDSENSOR:
      snprintf(buf, buf_size, "WINDSENSOR");
      break;
    case SUPLA_CHANNELFNC_PRESSURESENSOR:
      snprintf(buf, buf_size, "PRESSURESENSOR");
      break;
    case SUPLA_CHANNELFNC_RAINSENSOR:
      snprintf(buf, buf_size, "RAINSENSOR");
      break;
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
      snprintf(buf, buf_size, "WEIGHTSENSOR");
      break;
    case SUPLA_CHANNELFNC_WEATHER_STATION:
      snprintf(buf, buf_size, "WEATHER_STATION");
      break;
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      snprintf(buf, buf_size, "STAIRCASETIMER");
      break;
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
      snprintf(buf, buf_size, "ELECTRICITYMETER");
      break;
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
      snprintf(buf, buf_size, "IC_ELECTRICITYMETER");
      break;
    case SUPLA_CHANNELFNC_IC_GAS_METER:
      snprintf(buf, buf_size, "IC_GASMETER");
      break;
    case SUPLA_CHANNELFNC_IC_WATER_METER:
      snprintf(buf, buf_size, "IC_WATERMETER");
      break;
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      snprintf(buf, buf_size, "IC_HEATMETER");
      break;
    case SUPLA_CHANNELFNC_THERMOSTAT:
      snprintf(buf, buf_size, "THERMOSTAT");
      break;
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
      snprintf(buf, buf_size, "THERMOSTATHEATPOLHOMEPLUS");
      break;
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
      snprintf(buf, buf_size, "VALVEOPENCLOSE");
      break;
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
      snprintf(buf, buf_size, "VALVEPERCENTAGE");
      break;
    case SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT:
      snprintf(buf, buf_size, "GENERAL_PURPOSE_MEASUREMENT");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEENGINESPEED:
      snprintf(buf, buf_size, "CONTROLLINGTHEENGINESPEED");
      break;
    case SUPLA_CHANNELFNC_ACTIONTRIGGER:
      snprintf(buf, buf_size, "ACTION_TRIGGER");
      break;
    case SUPLA_CHANNELFNC_DIGIGLASS:
      snprintf(buf, buf_size, "DIGIGLASS");
      break;
      buf[0] = 0;
      break;
  }
}

bool supla_mqtt_channel_message_provider::get_home_assistant_cfgitem(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  return false;
}

bool supla_mqtt_channel_message_provider::get_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (row != NULL) {
    switch (index) {
      case 0: {
        char device_id[15];
        snprintf(device_id, sizeof(device_id), "%i", row->device_id);

        return create_message(topic_prefix, row->user_email, topic_name,
                              message, message_size, device_id, false,
                              "/channels/%i/device_id", row->channel_id);
      }
      case 1: {
        char channel_type[35];
        channel_type_to_string(row->channel_type, channel_type,
                               sizeof(channel_type));

        return create_message(topic_prefix, row->user_email, topic_name,
                              message, message_size, channel_type, false,
                              "/channels/%i/type", row->channel_id);
      }
      case 2: {
        char channel_func[35];
        channel_function_to_string(row->channel_func, channel_func,
                                   sizeof(channel_func));

        return create_message(topic_prefix, row->user_email, topic_name,
                              message, message_size, channel_func, false,
                              "/channels/%i/function", row->channel_id);
      }

      case 3:
        return create_message(topic_prefix, row->user_email, topic_name,
                              message, message_size, row->channel_location,
                              false, "/channels/%i/location", row->channel_id);

      case 4:
        return create_message(topic_prefix, row->user_email, topic_name,
                              message, message_size, row->channel_caption,
                              false, "/channels/%i/caption", row->channel_id);
      case 5:
        return create_message(topic_prefix, row->user_email, topic_name,
                              message, message_size,
                              row->channel_hidden ? "true" : "false", false,
                              "/channels/%i/hidden", row->channel_id);
    }
  }

  if (index > 5) {
    return get_home_assistant_cfgitem(index - 5, topic_prefix, topic_name,
                                      message, message_size);
  }

  return false;
}

void supla_mqtt_channel_message_provider::set_data_row(
    _mqtt_db_data_row_channel_t *row) {
  this->row = row;
  reset_index();
}
