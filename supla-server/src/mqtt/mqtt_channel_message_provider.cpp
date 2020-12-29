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
#include <stdlib.h>
#include <string.h>
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
    case SUPLA_CHANNELTYPE_SENSORNO:
      snprintf(buf, buf_size, "SENSORNO");
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

void supla_mqtt_channel_message_provider::get_not_empty_caption(
    int func, const char *caption_in, char *caption_out) {
  if (!caption_out) {
    return;
  }

  if (caption_in && caption_in[0] != 0) {
    memcpy(caption_out, caption_in, SUPLA_CHANNEL_CAPTION_MAXSIZE);
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
    case SUPLA_CHANNELFNC_DIGIGLASS:
      snprintf(caption_out, SUPLA_CHANNEL_CAPTION_MAXSIZE, "Digiglass");
      break;
      caption_out[0] = 0;
      break;
  }
}

void supla_mqtt_channel_message_provider::ha_json_set_name(cJSON *root) {
  char caption[SUPLA_CHANNEL_CAPTION_MAXSIZE];
  get_not_empty_caption(row->channel_func, row->channel_caption, caption);
  cJSON_AddStringToObject(root, "name", caption);
}

void supla_mqtt_channel_message_provider::ha_json_set_uniq_id(cJSON *root,
                                                              int sub_id,
                                                              bool set_sub_id) {
  char uniq_id[50];
  if (set_sub_id) {
    snprintf(uniq_id, sizeof(uniq_id), "supla-%i-%i", row->channel_id, sub_id);
  } else {
    snprintf(uniq_id, sizeof(uniq_id), "supla-%i", row->channel_id);
  }

  cJSON_AddStringToObject(root, "uniq_id", uniq_id);
}

void supla_mqtt_channel_message_provider::ha_json_set_topic_base(
    cJSON *root, const char *topic_prefix) {
  char *topic_name = NULL;

  create_message(topic_prefix, row->user_email, &topic_name, NULL, NULL, NULL,
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

  create_message(topic_prefix, row->user_email, &topic_name, NULL, NULL, NULL,
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

void supla_mqtt_channel_message_provider::ha_json_set_availability(
    cJSON *root, const char *topic_prefix, const char *avil,
    const char *notavil) {
  cJSON *avty = cJSON_CreateObject();
  if (avty) {
    ha_json_set_full_topic(avty, "topic", topic_prefix, "state/connected");

    cJSON_AddStringToObject(avty, "payload_available", avil);
    cJSON_AddStringToObject(avty, "payload_not_available", notavil);
    cJSON_AddItemToObject(root, "avty", avty);
  }
}

cJSON *supla_mqtt_channel_message_provider::ha_json_create_root(
    const char *topic_prefix, int sub_id, bool set_sub_id) {
  cJSON *root = cJSON_CreateObject();
  if (!root) {
    return NULL;
  }

  ha_json_set_topic_base(root, topic_prefix);
  ha_json_set_name(root);
  ha_json_set_uniq_id(root, sub_id, set_sub_id);
  ha_json_set_qos(root);
  ha_json_set_availability(root, topic_prefix, "true", "false");

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

  char *node_id = homeassistant_get_node_id(row->user_email);
  if (node_id) {
    char object_id[50];
    if (set_sub_id) {
      snprintf(object_id, sizeof(object_id), "%i-%i", row->channel_id, sub_id);
    } else {
      snprintf(object_id, sizeof(object_id), "%i", row->channel_id);
    }

    const char fmt[] = "homeassistant/%s/%s/%s/config";
    int len = snprintf(NULL, 0, fmt, component, node_id, object_id);

    if (len) {
      len++;
      *topic_name = (char *)malloc(len);
      if (*topic_name) {
        snprintf(*topic_name, len, fmt, component, node_id, object_id);
        free(node_id);
        return true;
      }
    }
  }

  free(node_id);

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
  ha_json_set_availability(root, topic_prefix, "true", "false");

  return ha_get_message(root, light ? "light" : "switch", 0, false, topic_name,
                        message, message_size);
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
    const char *state_topic, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  // https://www.home-assistant.io/integrations/sensor.mqtt

  cJSON *root = ha_json_create_root(topic_prefix);
  if (!root) {
    return false;
  }

  ha_json_set_string_param(root, "unit_of_meas", unit);
  ha_json_set_short_topic(root, "stat_t", state_topic);

  char tpl[50];
  snprintf(tpl, sizeof(tpl), "{{ value | round(%i) }}", precision);

  ha_json_set_string_param(root, "val_tpl", tpl);

  return ha_get_message(root, "sensor", sub_id, set_sub_id, topic_name, message,
                        message_size);
}

bool supla_mqtt_channel_message_provider::ha_sensor_temperature(
    int sub_id, bool set_sub_id, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  return ha_sensor("°C", 1, sub_id, set_sub_id, "state/temperature",
                   topic_prefix, topic_name, message, message_size);
}

bool supla_mqtt_channel_message_provider::ha_sensor_humidity(
    int sub_id, bool set_sub_id, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  return ha_sensor("%", 1, sub_id, set_sub_id, "state/humidity", topic_prefix,
                   topic_name, message, message_size);
}

bool supla_mqtt_channel_message_provider::get_home_assistant_cfgitem(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (!row->device_enabled || row->channel_hidden || topic_name == NULL) {
    return false;
  }

  switch (row->channel_func) {
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      if (index > 1) {
        return false;
      }
      break;
    default:
      if (index != 0) {
        return false;
      }
  }

  switch (row->channel_func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      break;
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
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
      return ha_light_or_powerswitch(true, topic_prefix, topic_name, message,
                                     message_size);
    case SUPLA_CHANNELFNC_DIMMER:
      break;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      break;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      break;
    case SUPLA_CHANNELFNC_DEPTHSENSOR:
      break;
    case SUPLA_CHANNELFNC_DISTANCESENSOR:
      break;
    case SUPLA_CHANNELFNC_WINDSENSOR:
      break;
    case SUPLA_CHANNELFNC_PRESSURESENSOR:
      break;
    case SUPLA_CHANNELFNC_RAINSENSOR:
      break;
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
      break;
    case SUPLA_CHANNELFNC_WEATHER_STATION:
      break;
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      break;
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
      break;
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
      break;
    case SUPLA_CHANNELFNC_IC_GAS_METER:
      break;
    case SUPLA_CHANNELFNC_IC_WATER_METER:
      break;
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      break;
    case SUPLA_CHANNELFNC_THERMOSTAT:
      break;
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
      break;
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
      break;
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
      break;
    case SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT:
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEENGINESPEED:
      break;
    case SUPLA_CHANNELFNC_ACTIONTRIGGER:
      break;
    case SUPLA_CHANNELFNC_DIGIGLASS:
      break;
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

        return create_message(topic_prefix, row->user_email, topic_name,
                              message, message_size, channel_type, false,
                              "devices/%i/channels/%i/type", row->device_id,
                              row->channel_id);
      }
      case 1: {
        char channel_func[35];
        channel_function_to_string(row->channel_func, channel_func,
                                   sizeof(channel_func));
        return create_message(topic_prefix, row->user_email, topic_name,
                              message, message_size, channel_func, false,
                              "devices/%i/channels/%i/function", row->device_id,
                              row->channel_id);
      }
      case 2:
        return create_message(topic_prefix, row->user_email, topic_name,
                              message, message_size, row->channel_caption,
                              false, "devices/%i/channels/%i/caption",
                              row->device_id, row->channel_id);
      case 3:
        return create_message(
            topic_prefix, row->user_email, topic_name, message, message_size,
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
