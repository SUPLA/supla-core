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

#include "mqtt_channel_message_provider.h"

#include <stdlib.h>
#include <string.h>

#include "device/extended_value/channel_em_extended_value.h"
#include "device/extended_value/channel_ic_extended_value.h"
#include "jsonconfig/channel/action_trigger_config.h"
#include "log.h"

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
    case SUPLA_CHANNELTYPE_BINARYSENSOR:
      snprintf(buf, buf_size, "BINARYSENSOR");
      break;
    case SUPLA_CHANNELTYPE_SENSORNC:
      snprintf(buf, buf_size, "SENSORNC");
      break;
    case SUPLA_CHANNELTYPE_DISTANCESENSOR:
      snprintf(buf, buf_size, "DISTANCESENSOR");
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
      snprintf(buf, buf_size, "BRIDGE");
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
    case SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL:
      snprintf(buf, buf_size, "DIGIGLASS_HORIZONTAL");
      break;
    case SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL:
      snprintf(buf, buf_size, "DIGIGLASS_VERTICAL");
      break;
      buf[0] = 0;
      break;
  }
}

void supla_mqtt_channel_message_provider::get_not_empty_caption(
    int func, const char *caption_in, const char *caption_if_empty,
    char *caption_out) {
  if (!caption_out) {
    return;
  }

  if (caption_in && caption_in[0] != 0) {
    snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "%s", caption_in);
    return;
  }

  if (caption_if_empty && caption_if_empty[0] != 0) {
    snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "%s",
             caption_if_empty);

    return;
  }

  switch (func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Gateway");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Gate");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Garage door");
      break;
    case SUPLA_CHANNELFNC_THERMOMETER:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Thermometer");
      break;
    case SUPLA_CHANNELFNC_HUMIDITY:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Humidity");
      break;
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Temperature and Humidity");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Gateway opening sensor");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Gate opening sensor");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Garage door opening sensor");
      break;
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "No liquid sensor");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Door lock operation");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Door opening sensor");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Roller shutter operation");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Roof window operation");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Roller shutter opening sensor");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Roof window opening sensor");
      break;
    case SUPLA_CHANNELFNC_POWERSWITCH:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "On/Off switch");
      break;
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Light switch");
      break;
    case SUPLA_CHANNELFNC_RING:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Ring");
      break;
    case SUPLA_CHANNELFNC_ALARM:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Alarm");
      break;
    case SUPLA_CHANNELFNC_NOTIFICATION:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Notification");
      break;
    case SUPLA_CHANNELFNC_DIMMER:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Dimmer");
      break;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "RGB lighting");
      break;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Dimmer and RGB lighting");
      break;
    case SUPLA_CHANNELFNC_DEPTHSENSOR:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Depth sensor");
      break;
    case SUPLA_CHANNELFNC_DISTANCESENSOR:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Distance sensor");
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Window opening sensor");
      break;
    case SUPLA_CHANNELFNC_MAILSENSOR:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Mail sensor");
      break;
    case SUPLA_CHANNELFNC_WINDSENSOR:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Wind sensor");
      break;
    case SUPLA_CHANNELFNC_PRESSURESENSOR:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Pressure sensor");
      break;
    case SUPLA_CHANNELFNC_RAINSENSOR:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Rain sensor");
      break;
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Weight sensor");
      break;
    case SUPLA_CHANNELFNC_WEATHER_STATION:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Weather sensor");
      break;
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Staircase timer");
      break;
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Electricity meter");
      break;
    case SUPLA_CHANNELFNC_IC_GAS_METER:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Gas meter");
      break;
    case SUPLA_CHANNELFNC_IC_WATER_METER:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Water meter");
      break;
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Heat meter");
      break;
    case SUPLA_CHANNELFNC_THERMOSTAT:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Thermostat");
      break;
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Home+ Heater");
      break;
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Valve");
      break;
    case SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "General purpose measurement");
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEENGINESPEED:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               "Engine speed controller");
      break;
    case SUPLA_CHANNELFNC_ACTIONTRIGGER:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Action trigger");
      break;
    case SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL:
    case SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Digiglass");
      break;
      caption_out[0] = 0;
      break;
  }
}

void supla_mqtt_channel_message_provider::ha_json_set_name(
    cJSON *root, const char *name_if_empty, const char *name_second_segment) {
  char caption[SUPLA_CHANNEL_CAPTION_MAXSIZE];
  get_not_empty_caption(row->channel_func, row->channel_caption, name_if_empty,
                        caption);
  if (name_second_segment && name_second_segment[0] != 0) {
    char *copy = strndup(caption, SUPLA_CHANNEL_CAPTION_MAXSIZE);
    if (copy) {
      snprintf(caption, SUPLA_CHANNEL_CAPTION_MAXSIZE, "%s (%s)", copy,
               name_second_segment);
      free(copy);
    }
  }
  cJSON_AddStringToObject(root, "name", caption);
}

void supla_mqtt_channel_message_provider::ha_json_set_uniq_id(cJSON *root,
                                                              int sub_id,
                                                              bool set_sub_id) {
  char uniq_id[50];
  if (set_sub_id) {
    snprintf(uniq_id, sizeof(uniq_id), "supla_%i_%i", row->channel_id, sub_id);
  } else {
    snprintf(uniq_id, sizeof(uniq_id), "supla_%i", row->channel_id);
  }

  cJSON_AddStringToObject(root, "uniq_id", uniq_id);
}

void supla_mqtt_channel_message_provider::ha_json_set_topic_base(
    cJSON *root, const char *topic_prefix) {
  char *topic_name = NULL;

  create_message(topic_prefix, row->user_suid, &topic_name, NULL, NULL, NULL,
                 false, "devices/%i/channels/%i", row->device_id,
                 row->channel_id);

  if (topic_name) {
    cJSON_AddStringToObject(root, "~", topic_name);
    free(topic_name);
  }
}

void supla_mqtt_channel_message_provider::ha_json_set_short_topic(
    cJSON *root, const char *param_name, const char *topic_suffix) {
  char *topic_name = NULL;

  const char fmt[] = "~/%s";

  int len = snprintf(NULL, 0, fmt, topic_suffix);

  if (len) {
    len++;
    topic_name = (char *)malloc(len);
    if (topic_name) {
      snprintf(topic_name, len, fmt, topic_suffix);
      cJSON_AddStringToObject(root, param_name, topic_name);
      free(topic_name);
    }
  }
}

void supla_mqtt_channel_message_provider::ha_json_set_full_topic(
    cJSON *root, const char *param_name, const char *topic_prefix,
    const char *topic_suffix) {
  char *topic_name = NULL;

  create_message(topic_prefix, row->user_suid, &topic_name, NULL, NULL, NULL,
                 false, "devices/%i/channels/%i/%s", row->device_id,
                 row->channel_id, topic_suffix);

  if (topic_name) {
    cJSON_AddStringToObject(root, param_name, topic_name);
    free(topic_name);
  }
}
void supla_mqtt_channel_message_provider::ha_json_set_qos(cJSON *root,
                                                          int qos) {
  cJSON_AddNumberToObject(root, "qos", qos);
}

void supla_mqtt_channel_message_provider::ha_json_set_retain(cJSON *root,
                                                             bool retain) {
  cJSON_AddBoolToObject(root, "ret", retain);
}

void supla_mqtt_channel_message_provider::ha_json_set_optimistic(
    cJSON *root, bool optimistic) {
  cJSON_AddBoolToObject(root, "opt", optimistic);
}

void supla_mqtt_channel_message_provider::ha_json_set_string_param(
    cJSON *root, const char *param_name, const char *value) {
  cJSON_AddStringToObject(root, param_name, value);
}

void supla_mqtt_channel_message_provider::ha_json_set_int_param(
    cJSON *root, const char *param_name, int value) {
  cJSON_AddNumberToObject(root, param_name, value);
}

void supla_mqtt_channel_message_provider::ha_json_set_availability(
    cJSON *root, const char *topic_prefix, const char *avil,
    const char *notavil) {
  cJSON *avty = cJSON_CreateObject();
  if (avty) {
    ha_json_set_full_topic(avty, "topic", topic_prefix, "state/connected");

    if (avil) {
      cJSON_AddStringToObject(avty, "payload_available", avil);
    }

    if (notavil) {
      cJSON_AddStringToObject(avty, "payload_not_available", notavil);
    }

    cJSON_AddItemToObject(root, "avty", avty);
  }
}

void supla_mqtt_channel_message_provider::ha_json_set_device_info(cJSON *root) {
  cJSON *device = cJSON_CreateObject();
  if (device) {
    char mfr_name[20];
    get_mfr_name(row->device_mfr_id, mfr_name, sizeof(mfr_name));

    char id[30];
    snprintf(id, sizeof(id), "supla-iodevice-%i", row->device_id);

    cJSON_AddStringToObject(device, "ids", id);
    cJSON_AddStringToObject(device, "mf", mfr_name);
    cJSON_AddStringToObject(device, "name", row->device_name);
    cJSON_AddStringToObject(device, "sw", row->device_softver);

    cJSON_AddItemToObject(root, "device", device);
  }
}

cJSON *supla_mqtt_channel_message_provider::ha_json_create_root(
    const char *topic_prefix, const char *name_if_empty,
    const char *name_second_segment, bool avil_topic, int sub_id,
    bool set_sub_id) {
  cJSON *root = cJSON_CreateObject();
  if (!root) {
    return NULL;
  }

  if (avil_topic) {
    ha_json_set_availability(root, topic_prefix, "true", "false");
  }
  ha_json_set_topic_base(root, topic_prefix);
  ha_json_set_device_info(root);
  ha_json_set_name(root, name_if_empty, name_second_segment);
  ha_json_set_uniq_id(root, sub_id, set_sub_id);
  ha_json_set_qos(root);

  return root;
}

bool supla_mqtt_channel_message_provider::ha_get_message(
    cJSON *root, const char *component, int sub_id, bool set_sub_id,
    char **topic_name, void **message, size_t *message_size) {
  if (message) {
    *message = cJSON_PrintUnformatted(root);

    if (*message == NULL) {
      return false;
    }

    if (message_size) {
      *message_size = strnlen((const char *)*message, MQTT_MAX_MESSAGE_SIZE);
    }
  }

  cJSON_Delete(root);

  char object_id[50];
  if (set_sub_id) {
    snprintf(object_id, sizeof(object_id), "%i_%i", row->channel_id, sub_id);
  } else {
    snprintf(object_id, sizeof(object_id), "%i", row->channel_id);
  }

  const char fmt[] = "homeassistant/%s/%s/%s/config";
  int len = snprintf(NULL, 0, fmt, component, row->user_suid, object_id);

  if (len) {
    len++;
    *topic_name = (char *)malloc(len);
    if (*topic_name) {
      snprintf(*topic_name, len, fmt, component, row->user_suid, object_id);
      return true;
    }
  }

  if (message && *message) {
    free(*message);
    message = NULL;
  }

  if (message_size) {
    *message_size = 0;
  }

  return false;
}

bool supla_mqtt_channel_message_provider::ha_light_or_powerswitch(
    bool light, const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  // https://www.home-assistant.io/integrations/switch.mqtt

  cJSON *root = ha_json_create_root(topic_prefix);
  if (!root) {
    return false;
  }

  ha_json_set_retain(root);
  ha_json_set_optimistic(root);

  ha_json_set_short_topic(root, "stat_t", "state/on");
  ha_json_set_short_topic(root, "cmd_t", "set/on");
  if (!light) {
    ha_json_set_string_param(root, "stat_on", "true");
    ha_json_set_string_param(root, "stat_off", "false");
  }
  ha_json_set_string_param(root, "pl_on", "true");
  ha_json_set_string_param(root, "pl_off", "false");

  return ha_get_message(root, light ? "light" : "switch", 0, false, topic_name,
                        message, message_size);
}

void supla_mqtt_channel_message_provider::ha_json_set_brightness(
    cJSON *root, const char *bri_cmd_t, const char *bti_stat_t) {
  ha_json_set_short_topic(root, "cmd_t", "execute_action");
  ha_json_set_string_param(root, "pl_on", "TURN_ON");
  ha_json_set_string_param(root, "pl_off", "TURN_OFF");

  ha_json_set_short_topic(root, "stat_t", "state/on");
  ha_json_set_string_param(
      root, "stat_val_tpl",
      "{% if value == \"true\" %}TURN_ON{% else %}TURN_OFF{% endif %}");

  // With "last", there were problems with RGB. Changed to "first".
  ha_json_set_string_param(root, "on_cmd_type", "first");

  ha_json_set_short_topic(root, "bri_cmd_t", bri_cmd_t);
  ha_json_set_int_param(root, "bri_scl", 100);
  ha_json_set_short_topic(root, "bri_stat_t", bti_stat_t);
}

bool supla_mqtt_channel_message_provider::ha_dimmer(int sub_id, bool set_sub_id,
                                                    const char *topic_prefix,
                                                    char **topic_name,
                                                    void **message,
                                                    size_t *message_size) {
  // https://www.home-assistant.io/integrations/light.mqtt
  cJSON *root = ha_json_create_root(topic_prefix, "Dimmer", NULL, true, sub_id,
                                    set_sub_id);
  if (!root) {
    return false;
  }

  ha_json_set_brightness(root, "set/brightness", "state/brightness");

  return ha_get_message(root, "light", sub_id, set_sub_id, topic_name, message,
                        message_size);
}

bool supla_mqtt_channel_message_provider::ha_rgb(int sub_id, bool set_sub_id,
                                                 const char *topic_prefix,
                                                 char **topic_name,
                                                 void **message,
                                                 size_t *message_size) {
  // https://www.home-assistant.io/integrations/light.mqtt
  cJSON *root = ha_json_create_root(topic_prefix, "RGB Lighting", NULL, true,
                                    sub_id, set_sub_id);
  if (!root) {
    return false;
  }

  ha_json_set_brightness(root, "set/color_brightness",
                         "state/color_brightness");
  ha_json_set_short_topic(root, "rgb_stat_t", "state/normalized_rgb");
  ha_json_set_short_topic(root, "rgb_cmd_t", "set/color");

  return ha_get_message(root, "light", sub_id, set_sub_id, topic_name, message,
                        message_size);
}

bool supla_mqtt_channel_message_provider::ha_binary_sensor(
    const char *device_class, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  // https://www.home-assistant.io/integrations/binary_sensor

  cJSON *root = ha_json_create_root(topic_prefix);
  if (!root) {
    return false;
  }

  ha_json_set_short_topic(root, "stat_t", "state/hi");
  ha_json_set_string_param(root, "pl_on", "false");
  ha_json_set_string_param(root, "pl_off", "true");
  if (device_class) {
    ha_json_set_string_param(root, "device_class", device_class);
  }

  return ha_get_message(root, "binary_sensor", 0, false, topic_name, message,
                        message_size);
}

bool supla_mqtt_channel_message_provider::ha_sensor(
    const char *unit, int precision, int sub_id, bool set_sub_id,
    const char *state_topic, const char *name_if_empty,
    const char *name_second_segment, const char *value_tmpl,
    const char *device_class, bool total_increasing, const char *topic_prefix,
    char **topic_name, void **message, size_t *message_size) {
  // https://www.home-assistant.io/integrations/sensor.mqtt

  cJSON *root =
      ha_json_create_root(topic_prefix, name_if_empty, name_second_segment,
                          true, sub_id, set_sub_id);
  if (!root) {
    return false;
  }

  ha_json_set_string_param(root, "unit_of_meas",
                           !unit || unit[0] == 0 ? "" : unit);
  ha_json_set_short_topic(root, "stat_t", state_topic);

  if (value_tmpl) {
    ha_json_set_string_param(root, "val_tpl", value_tmpl);
  } else {
    char tpl[50];
    snprintf(tpl, sizeof(tpl), "{{ value | round(%i,default=none) }}",
             precision);

    ha_json_set_string_param(root, "val_tpl", tpl);
  }

  if (device_class) {
    ha_json_set_string_param(root, "dev_cla", device_class);
  }

  ha_json_set_string_param(
      root, "state_class",
      total_increasing ? "total_increasing" : "measurement");

  return ha_get_message(root, "sensor", sub_id, set_sub_id, topic_name, message,
                        message_size);
}

bool supla_mqtt_channel_message_provider::ha_sensor_temperature(
    int sub_id, bool set_sub_id, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  return ha_sensor("°C", 1, sub_id, set_sub_id, "state/temperature",
                   "Temperature", NULL, NULL, NULL, false, topic_prefix,
                   topic_name, message, message_size);
}

bool supla_mqtt_channel_message_provider::ha_sensor_humidity(
    int sub_id, bool set_sub_id, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  return ha_sensor("%", 1, sub_id, set_sub_id, "state/humidity", "Humidity",
                   NULL, NULL, NULL, false, topic_prefix, topic_name, message,
                   message_size);
}

bool supla_mqtt_channel_message_provider::ha_gate(const char *topic_prefix,
                                                  char **topic_name,
                                                  void **message,
                                                  size_t *message_size,
                                                  const char *device_class) {
  cJSON *root = ha_json_create_root(topic_prefix, NULL, NULL, false);
  if (!root) {
    return false;
  }

  ha_json_set_retain(root);
  ha_json_set_optimistic(root);

  ha_json_set_short_topic(root, "cmd_t", "execute_action");
  ha_json_set_string_param(root, "dev_cla", device_class);
  ha_json_set_string_param(root, "pl_open", "OPEN");
  ha_json_set_string_param(root, "pl_stop", "");
  ha_json_set_string_param(root, "pl_cls", "CLOSE");

  ha_json_set_short_topic(root, "state_topic", "state/hi");
  ha_json_set_string_param(root, "state_open", "false");
  ha_json_set_string_param(root, "state_closed", "true");

  ha_json_set_short_topic(root, "avty_t", "state/connected");
  ha_json_set_string_param(root, "pl_avail", "true");
  ha_json_set_string_param(root, "pl_not_avail", "false");

  return ha_get_message(root, "cover", 0, false, topic_name, message,
                        message_size);
}

bool supla_mqtt_channel_message_provider::ha_door(const char *topic_prefix,
                                                  char **topic_name,
                                                  void **message,
                                                  size_t *message_size) {
  cJSON *root = ha_json_create_root(topic_prefix, NULL, NULL, false);
  if (!root) {
    return false;
  }

  ha_json_set_retain(root);
  ha_json_set_optimistic(root);

  ha_json_set_short_topic(root, "cmd_t", "execute_action");
  ha_json_set_string_param(root, "dev_cla", "door");
  ha_json_set_string_param(root, "pl_open", "OPEN");
  ha_json_set_string_param(root, "pl_stop", "");
  ha_json_set_string_param(root, "pl_cls", "OPEN");

  ha_json_set_short_topic(root, "state_topic", "state/hi");
  ha_json_set_string_param(root, "state_open", "false");
  ha_json_set_string_param(root, "state_closed", "true");

  ha_json_set_short_topic(root, "avty_t", "state/connected");
  ha_json_set_string_param(root, "pl_avail", "true");
  ha_json_set_string_param(root, "pl_not_avail", "false");

  return ha_get_message(root, "cover", 0, false, topic_name, message,
                        message_size);
}

bool supla_mqtt_channel_message_provider::ha_roller_shutter(
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  // https://www.home-assistant.io/integrations/cover.mqtt

  cJSON *root = ha_json_create_root(topic_prefix, NULL, NULL, false);
  if (!root) {
    return false;
  }

  ha_json_set_retain(root);
  ha_json_set_optimistic(root);

  ha_json_set_short_topic(root, "cmd_t", "execute_action");
  ha_json_set_string_param(root, "dev_cla", "shutter");
  ha_json_set_string_param(root, "pl_open", "REVEAL");
  ha_json_set_string_param(root, "pl_cls", "SHUT");
  ha_json_set_string_param(root, "pl_stop", "STOP");

  ha_json_set_short_topic(root, "set_pos_t", "set/closing_percentage");
  ha_json_set_short_topic(root, "pos_t", "state/shut");
  ha_json_set_int_param(root, "pos_open", 0);
  ha_json_set_int_param(root, "pos_clsd", 100);

  ha_json_set_short_topic(root, "avty_t", "state/connected");
  ha_json_set_string_param(root, "pl_avail", "true");
  ha_json_set_string_param(root, "pl_not_avail", "false");

  ha_json_set_string_param(
      root, "pos_tpl",
      "{% if value is defined %}{% if value | int < 0 %}0{% elif value | int > "
      "100 %}100{% else %}{{value | int}}{% endif %}{% else %}0{% endif %}");

  return ha_get_message(root, "cover", 0, false, topic_name, message,
                        message_size);
}

bool supla_mqtt_channel_message_provider::ha_impulse_counter(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size, int func) {
  bool result = false;
  if (index <= 1) {
    TDS_ImpulseCounter_Value v;
    memset(&v, 0, sizeof(TDS_ImpulseCounter_Value));

    supla_channel_ic_extended_value *icv = new supla_channel_ic_extended_value(
        row->channel_func, &v, row->channel_text_param1,
        row->channel_text_param2, row->channel_param2, row->channel_param3);

    if (icv == NULL) {
      return false;
    }

    switch (index) {
      case 0: {
        char *device_class = NULL;
        char energy[] = "energy";
        char gas[] = "gas";
        char water[] = "water";

        if (func == SUPLA_CHANNELFNC_IC_ELECTRICITY_METER &&
            (icv->get_custom_unit() == "kWh" ||
             icv->get_custom_unit() == "Wh")) {
          device_class = energy;
        } else if (func == SUPLA_CHANNELFNC_IC_GAS_METER &&
                   (icv->get_custom_unit() == "m³" ||
                    icv->get_custom_unit() == "ft³" ||
                    icv->get_custom_unit() == "CCF")) {
          device_class = gas;
        } else if (func == SUPLA_CHANNELFNC_IC_WATER_METER &&
                   (icv->get_custom_unit() == "m³" ||
                    icv->get_custom_unit() == "ft³" ||
                    icv->get_custom_unit() == "CCF" ||
                    icv->get_custom_unit() == "L" ||
                    icv->get_custom_unit() == "gal")) {
          device_class = water;
        }

        result = ha_sensor(icv->get_custom_unit().c_str(), 3, 0, true,
                           "state/calculated_value", NULL, "Value", NULL,
                           device_class, true, topic_prefix, topic_name,
                           message, message_size);
      } break;
      case 1:
        result =
            ha_sensor(icv->get_currency().c_str(), 2, 1, true,
                      "state/total_cost", NULL, "Total cost", NULL, "monetary",
                      false, topic_prefix, topic_name, message, message_size);
        break;
    }

    delete icv;
  }

  return result;
}

bool supla_mqtt_channel_message_provider::ha_phase_sensor(
    unsigned short index, unsigned short phase, const char *unit, int precision,
    const char *state_topic, const char *name_second_segment,
    const char *value_tmpl, const char *device_class, bool total_increasing,
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  int st_len = snprintf(NULL, 0, state_topic, phase);
  if (st_len == 0) {
    return false;
  }

  st_len++;

  int nss_len = snprintf(NULL, 0, name_second_segment, phase);
  if (nss_len == 0) {
    return false;
  }

  nss_len++;

  char *_state_topic = (char *)malloc(st_len);
  if (_state_topic == NULL) {
    return false;
  }

  char *_name_second_segment = (char *)malloc(nss_len);
  if (_name_second_segment == NULL) {
    free(_state_topic);
    return false;
  }

  snprintf(_state_topic, st_len, state_topic, phase);
  snprintf(_name_second_segment, nss_len, name_second_segment, phase);

  bool result = ha_sensor(unit, precision, index, true, _state_topic, NULL,
                          _name_second_segment, value_tmpl, device_class,
                          total_increasing, topic_prefix, topic_name, message,
                          message_size);

  free(_state_topic);
  free(_name_second_segment);

  return result;
}

bool supla_mqtt_channel_message_provider::ha_electricity_meter(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (index >= 6 &&
      (row->channel_flags & SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED)) {
    index += 12;
  }

  if (index >= 18 &&
      (row->channel_flags & SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED)) {
    index += 12;
  }

  if (index >= 30 &&
      (row->channel_flags & SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED)) {
    index += 12;
  }

  short phase = (index - 6) / 12 + 1;

  switch (index) {
    case 0:
    case 1: {
      bool result = false;
      supla_channel_billing_value bil;

      switch (index) {
        case 0:
          result = ha_sensor(bil.get_currency(row->channel_text_param1).c_str(),
                             2, index, true, "state/total_cost", NULL,
                             "Total cost", NULL, "monetary", false,
                             topic_prefix, topic_name, message, message_size);
          break;
        case 1:
          result = ha_sensor(bil.get_currency(row->channel_text_param1).c_str(),
                             2, index, true, "state/total_cost_balanced", NULL,
                             "Total cost - balanced", NULL, "monetary", false,
                             topic_prefix, topic_name, message, message_size);
          break;
      }

      return result;
    }
    case 2:
      return ha_sensor("kWh", 5, index, true,
                       "state/total_forward_active_energy", NULL,
                       "Total forward active energy", NULL, "energy", true,
                       topic_prefix, topic_name, message, message_size);
    case 3:
      return ha_sensor("kWh", 5, index, true,
                       "state/total_reverse_active_energy", NULL,
                       "Total reverse active energy", NULL, "energy", true,
                       topic_prefix, topic_name, message, message_size);
    case 4:
      return ha_sensor("kWh", 5, index, true,
                       "state/total_forward_active_energy_balanced", NULL,
                       "Total forward active energy - balanced", NULL, "energy",
                       true, topic_prefix, topic_name, message, message_size);
    case 5:
      return ha_sensor("kWh", 5, index, true,
                       "state/total_reverse_active_energy_balanced", NULL,
                       "Total reverse active energy - balanced", NULL, "energy",
                       true, topic_prefix, topic_name, message, message_size);

    case 6:
    case 18:
    case 30:
      return ha_phase_sensor(
          index, phase, "kWh", 5, "state/phases/%i/total_forward_active_energy",
          "Total forward active energy - Phase %i", NULL, "energy", true,
          topic_prefix, topic_name, message, message_size);

    case 7:
    case 19:
    case 31:
      return ha_phase_sensor(
          index, phase, "kWh", 5, "state/phases/%i/total_reverse_active_energy",
          "Total reverse active energy - Phase %i", NULL, "energy", true,
          topic_prefix, topic_name, message, message_size);

    case 8:
    case 20:
    case 32:
      return ha_phase_sensor(index, phase, "kvarh", 5,
                             "state/phases/%i/total_forward_reactive_energy",
                             "Total forward reactive energy - Phase %i", NULL,
                             NULL, true, topic_prefix, topic_name, message,
                             message_size);

    case 9:
    case 21:
    case 33:
      return ha_phase_sensor(index, phase, "kvarh", 5,
                             "state/phases/%i/total_reverse_reactive_energy",
                             "Total reverse reactive energy - Phase %i", NULL,
                             NULL, true, topic_prefix, topic_name, message,
                             message_size);

    case 10:
    case 22:
    case 34:
      return ha_phase_sensor(index, phase, "Hz", 2, "state/phases/%i/frequency",
                             "Frequency - Phase %i", NULL, NULL, false,
                             topic_prefix, topic_name, message, message_size);

    case 11:
    case 23:
    case 35:
      return ha_phase_sensor(index, phase, "V", 2, "state/phases/%i/voltage",
                             "Voltage - Phase %i", NULL, "voltage", false,
                             topic_prefix, topic_name, message, message_size);

    case 12:
    case 24:
    case 36:
      return ha_phase_sensor(index, phase, "A", 3, "state/phases/%i/current",
                             "Current - Phase %i", NULL, "current", false,
                             topic_prefix, topic_name, message, message_size);

    case 13:
    case 25:
    case 37:
      return ha_phase_sensor(index, phase, "W", 5,
                             "state/phases/%i/power_active",
                             "Power active - Phase %i", NULL, "power", false,
                             topic_prefix, topic_name, message, message_size);

    case 14:
    case 26:
    case 38:
      return ha_phase_sensor(index, phase, "var", 5,
                             "state/phases/%i/power_reactive",
                             "Power reactive - Phase %i", NULL, NULL, false,
                             topic_prefix, topic_name, message, message_size);

    case 15:
    case 27:
    case 39:
      return ha_phase_sensor(index, phase, "VA", 5,
                             "state/phases/%i/power_apparent",
                             "Power apparent - Phase %i", NULL, NULL, false,
                             topic_prefix, topic_name, message, message_size);

    case 16:
    case 28:
    case 40:
      return ha_phase_sensor(
          index, phase, "%", 3, "state/phases/%i/power_factor",
          "Power factor - Phase %i",
          "{% if float(value, default=none) == None %}None{% else "
          "%}{{float(value) * 100.0 | round(5)}}{% endif %}",
          "power_factor", false, topic_prefix, topic_name, message,
          message_size);

    case 17:
    case 29:
    case 41:
      return ha_phase_sensor(index, phase, "°", 1,
                             "state/phases/%i/phase_angle",
                             "Phase angle - Phase %i", NULL, NULL, false,
                             topic_prefix, topic_name, message, message_size);
  }

  return false;
}

bool supla_mqtt_channel_message_provider::ha_device_trigger(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size, unsigned int cap) {
  // https://www.home-assistant.io/integrations/device_trigger.mqtt/

  cJSON *root =
      ha_json_create_root(topic_prefix, NULL, NULL, false, index, true);
  if (!root) {
    return false;
  }

  char component[] = "device_automation";

  if (!cap) {
    if (message_size) {
      *message_size = 0;
    }
    return ha_get_message(root, component, index, true, topic_name, NULL, NULL);
  }

  char topic[15] = {};
  char actiontopic[30] = {};
  char type[30] = {};
  char subtype[20] = {};

  // Also note that the combination of type and subtype should be unique for a
  // device.

  snprintf(subtype, sizeof(subtype), "button_%i", row->channel_number + 1);

  switch (cap) {
    case SUPLA_ACTION_CAP_TURN_ON:
      snprintf(topic, sizeof(topic), "turn_on");
      break;
    case SUPLA_ACTION_CAP_TURN_OFF:
      snprintf(topic, sizeof(topic), "turn_off");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x1:
      snprintf(topic, sizeof(topic), "toggle_x1");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x2:
      snprintf(topic, sizeof(topic), "toggle_x2");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x3:
      snprintf(topic, sizeof(topic), "toggle_x3");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x4:
      snprintf(topic, sizeof(topic), "toggle_x4");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x5:
      snprintf(topic, sizeof(topic), "toggle_x5");
      break;
    case SUPLA_ACTION_CAP_HOLD:
      snprintf(topic, sizeof(topic), "hold");
      break;
    case SUPLA_ACTION_CAP_SHORT_PRESS_x1:
      snprintf(topic, sizeof(topic), "press_x1");
      break;
    case SUPLA_ACTION_CAP_SHORT_PRESS_x2:
      snprintf(topic, sizeof(topic), "press_x2");
      break;
    case SUPLA_ACTION_CAP_SHORT_PRESS_x3:
      snprintf(topic, sizeof(topic), "press_x3");
      break;
    case SUPLA_ACTION_CAP_SHORT_PRESS_x4:
      snprintf(topic, sizeof(topic), "press_x4");
      break;
    case SUPLA_ACTION_CAP_SHORT_PRESS_x5:
      snprintf(topic, sizeof(topic), "press_x5");
      break;
  }

  switch (cap) {
    case SUPLA_ACTION_CAP_TURN_ON:
      snprintf(type, sizeof(type), "button_turn_on");
      break;
    case SUPLA_ACTION_CAP_TURN_OFF:
      snprintf(type, sizeof(type), "button_turn_off");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x1:
    case SUPLA_ACTION_CAP_SHORT_PRESS_x1:
      snprintf(type, sizeof(type), "button_short_press");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x2:
    case SUPLA_ACTION_CAP_SHORT_PRESS_x2:
      snprintf(type, sizeof(type), "button_double_press");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x3:
    case SUPLA_ACTION_CAP_SHORT_PRESS_x3:
      snprintf(type, sizeof(type), "button_triple_press");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x4:
    case SUPLA_ACTION_CAP_SHORT_PRESS_x4:
      snprintf(type, sizeof(type), "button_quadruple_press");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x5:
    case SUPLA_ACTION_CAP_SHORT_PRESS_x5:
      snprintf(type, sizeof(type), "button_quintuple_press");
      break;
    case SUPLA_ACTION_CAP_HOLD:
      snprintf(type, sizeof(type), "button_long_press");
      break;
  }

  snprintf(actiontopic, sizeof(actiontopic), "action/%s", topic);

  ha_json_set_string_param(root, "atype", "trigger");
  ha_json_set_short_topic(root, "t", actiontopic);
  ha_json_set_string_param(root, "type", type);
  ha_json_set_string_param(root, "stype", subtype);
  ha_json_set_string_param(root, "pl", topic);

  return ha_get_message(root, component, index, true, topic_name, message,
                        message_size);
}

bool supla_mqtt_channel_message_provider::ha_action_trigger(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (index > 12) {
    return false;
  }

  unsigned int cap = 0;

  action_trigger_config at_config(&row->json_config);
  unsigned int active = at_config.get_active_actions();

  switch (index) {
    case 0:
      cap = SUPLA_ACTION_CAP_TURN_ON;
      break;
    case 1:
      cap = SUPLA_ACTION_CAP_TURN_OFF;
      break;
    case 2:
      cap = (active & SUPLA_ACTION_CAP_TOGGLE_x1)
                ? SUPLA_ACTION_CAP_TOGGLE_x1
                : SUPLA_ACTION_CAP_SHORT_PRESS_x1;
      break;
    case 3:
      cap = (active & SUPLA_ACTION_CAP_TOGGLE_x2)
                ? SUPLA_ACTION_CAP_TOGGLE_x2
                : SUPLA_ACTION_CAP_SHORT_PRESS_x2;
      break;
    case 4:
      cap = (active & SUPLA_ACTION_CAP_TOGGLE_x3)
                ? SUPLA_ACTION_CAP_TOGGLE_x3
                : SUPLA_ACTION_CAP_SHORT_PRESS_x3;
      break;
    case 5:
      cap = (active & SUPLA_ACTION_CAP_TOGGLE_x4)
                ? SUPLA_ACTION_CAP_TOGGLE_x4
                : SUPLA_ACTION_CAP_SHORT_PRESS_x4;
      break;
    case 6:
      cap = (active & SUPLA_ACTION_CAP_TOGGLE_x5)
                ? SUPLA_ACTION_CAP_TOGGLE_x5
                : SUPLA_ACTION_CAP_SHORT_PRESS_x5;
      break;
    case 7:
      cap = SUPLA_ACTION_CAP_HOLD;
      break;
  }

  at_config.set_active_cap(cap);
  if (at_config.get_action_id() != ACTION_FORWARD_OUTSIDE) {
    cap = 0;
  }

  return ha_device_trigger(index, topic_prefix, topic_name, message,
                           message_size, cap);
}

bool supla_mqtt_channel_message_provider::get_home_assistant_cfgitem(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (!row->device_enabled || row->channel_hidden || topic_name == NULL) {
    return false;
  }

  switch (row->channel_func) {
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      break;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      if (index > 1) {
        return false;
      }
      break;
    case SUPLA_CHANNELFNC_ACTIONTRIGGER:
      if (index > 7) {
        return false;
      }
      break;
    default:
      if (index != 0) {
        return false;
      }
  }

  switch (row->channel_func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      return ha_gate(topic_prefix, topic_name, message, message_size, "gate");
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      return ha_gate(topic_prefix, topic_name, message, message_size, "garage");
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      return ha_door(topic_prefix, topic_name, message, message_size);
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      return ha_roller_shutter(topic_prefix, topic_name, message, message_size);
    case SUPLA_CHANNELFNC_THERMOMETER:
      return ha_sensor_temperature(0, false, topic_prefix, topic_name, message,
                                   message_size);

    case SUPLA_CHANNELFNC_HUMIDITY:
      return ha_sensor_humidity(0, true, topic_prefix, topic_name, message,
                                message_size);
      break;
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      switch (index) {
        case 0:
          return ha_sensor_humidity(0, true, topic_prefix, topic_name, message,
                                    message_size);
        case 1:
          return ha_sensor_temperature(1, true, topic_prefix, topic_name,
                                       message, message_size);
      }
      break;
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
    case SUPLA_CHANNELFNC_MAILSENSOR:
      return ha_binary_sensor(NULL, topic_prefix, topic_name, message,
                              message_size);
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
      return ha_binary_sensor("opening", topic_prefix, topic_name, message,
                              message_size);
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      return ha_binary_sensor("garage_door", topic_prefix, topic_name, message,
                              message_size);
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      return ha_binary_sensor("door", topic_prefix, topic_name, message,
                              message_size);
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
      return ha_binary_sensor("window", topic_prefix, topic_name, message,
                              message_size);
    case SUPLA_CHANNELFNC_POWERSWITCH:
      return ha_light_or_powerswitch(false, topic_prefix, topic_name, message,
                                     message_size);
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
      return ha_light_or_powerswitch(true, topic_prefix, topic_name, message,
                                     message_size);
    case SUPLA_CHANNELFNC_DIMMER:
      return ha_dimmer(0, false, topic_prefix, topic_name, message,
                       message_size);
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      return ha_rgb(0, false, topic_prefix, topic_name, message, message_size);
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      switch (index) {
        case 0:
          return ha_dimmer(0, true, topic_prefix, topic_name, message,
                           message_size);
        case 1:
          return ha_rgb(1, true, topic_prefix, topic_name, message,
                        message_size);
      }
      break;
    case SUPLA_CHANNELFNC_DEPTHSENSOR:
      return ha_sensor("m", 2, 0, false, "state/depth", NULL, NULL, NULL, NULL,
                       false, topic_prefix, topic_name, message, message_size);
    case SUPLA_CHANNELFNC_DISTANCESENSOR:
      return ha_sensor("m", 2, 0, false, "state/distance", NULL, NULL, NULL,
                       NULL, false, topic_prefix, topic_name, message,
                       message_size);
    case SUPLA_CHANNELFNC_WINDSENSOR:
      return ha_sensor("m/s", 1, 0, false, "state/value", NULL, NULL, NULL,
                       NULL, false, topic_prefix, topic_name, message,
                       message_size);
    case SUPLA_CHANNELFNC_PRESSURESENSOR:
      return ha_sensor("hPa", 0, 0, false, "state/value", NULL, NULL, NULL,
                       NULL, false, topic_prefix, topic_name, message,
                       message_size);
    case SUPLA_CHANNELFNC_RAINSENSOR:
      return ha_sensor("l/m", 2, 0, false, "state/value", NULL, NULL, NULL,
                       NULL, false, topic_prefix, topic_name, message,
                       message_size);
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
      return ha_sensor("g", 0, 0, false, "state/value", NULL, NULL, NULL, NULL,
                       false, topic_prefix, topic_name, message, message_size);
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
      return ha_electricity_meter(index, topic_prefix, topic_name, message,
                                  message_size);

    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      return ha_impulse_counter(index, topic_prefix, topic_name, message,
                                message_size, row->channel_func);
    case SUPLA_CHANNELFNC_ACTIONTRIGGER:
      return ha_action_trigger(index, topic_prefix, topic_name, message,
                               message_size);
  }
  return false;
}

bool supla_mqtt_channel_message_provider::get_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (row != NULL) {
    switch (index) {
      case 0: {
        char channel_type[35];
        channel_type_to_string(row->channel_type, channel_type,
                               sizeof(channel_type));

        return create_message(topic_prefix, row->user_suid, topic_name, message,
                              message_size, channel_type, false,
                              "devices/%i/channels/%i/type", row->device_id,
                              row->channel_id);
      }
      case 1: {
        char channel_func[35];
        channel_function_to_string(row->channel_func, channel_func,
                                   sizeof(channel_func));
        return create_message(topic_prefix, row->user_suid, topic_name, message,
                              message_size, channel_func, false,
                              "devices/%i/channels/%i/function", row->device_id,
                              row->channel_id);
      }
      case 2:
        return create_message(topic_prefix, row->user_suid, topic_name, message,
                              message_size, row->channel_caption, false,
                              "devices/%i/channels/%i/caption", row->device_id,
                              row->channel_id);
      case 3:
        return create_message(
            topic_prefix, row->user_suid, topic_name, message, message_size,
            row->channel_hidden ? "true" : "false", false,
            "devices/%i/channels/%i/hidden", row->device_id, row->channel_id);
    }

    if (index > 3) {
      return get_home_assistant_cfgitem(index - 4, topic_prefix, topic_name,
                                        message, message_size);
    }
  }
  return false;
}

void supla_mqtt_channel_message_provider::set_data_row(
    _mqtt_db_data_row_channel_t *row) {
  this->row = row;
  reset_index();
}
