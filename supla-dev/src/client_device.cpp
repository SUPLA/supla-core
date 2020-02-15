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

#include "client_device.h"

client_device_channel::client_device_channel(int Id, int Number, int Type,
                                             int Func, int Param1, int Param2,
                                             int Param3, char *TextParam1,
                                             char *TextParam2, char *TextParam3,
                                             bool Hidden, bool Online,
                                             char *Caption)
    : supla_device_channel(Id, Number, Type, Func, Param1, Param2, Param3,
                           TextParam1, TextParam2, TextParam3, Hidden) {
  this->Online = Online;
  this->Caption =
      Caption ? strndup(Caption, SUPLA_CHANNEL_CAPTION_MAXSIZE) : NULL;

  memset(this->Sub_value, 0, SUPLA_CHANNELVALUE_SIZE);

  this->FileName = NULL;
  this->CommandTemplate = NULL;
  this->StateTemplate = NULL;
  this->Retain = 0;
  this->CommandTopic = NULL;
  this->StateTemplate = NULL;
  this->PayloadOff = NULL;
  this->PayloadOn = NULL;
  this->PayloadValue = NULL;
  this->StateTopic = NULL;
  this->CommandTemplate = NULL;
}

bool client_device_channel::isSensorNONC(void) {
  switch (Func) {
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
    case SUPLA_CHANNELFNC_MAILSENSOR:
      return true;
    default:
      return false;
  }
}

void client_device_channel::setValue(char value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);
  // TODO(lukbek): Remove channel type checking in future versions. Check
  // function instead of type. # 140-issue
  if ((Func == SUPLA_CHANNELFNC_IC_ELECTRICITY_METER ||
       Func == SUPLA_CHANNELFNC_IC_GAS_METER ||
       Func == SUPLA_CHANNELFNC_IC_WATER_METER ||
       (Func == SUPLA_CHANNELFNC_ELECTRICITY_METER &&
        Type == SUPLA_CHANNELTYPE_IMPULSE_COUNTER)) &&
      Param1 > 0 && Param3 > 0) {
    TDS_ImpulseCounter_Value *ic_val = (TDS_ImpulseCounter_Value *)this->value;
    ic_val->counter +=
        (unsigned _supla_int64_t)Param1 * (unsigned _supla_int64_t)Param3 / 100;

  } else if (isSensorNONC()) {
    this->value[0] = this->value[0] == 0 ? 1 : 0;

  } else {
    supla_channel_temphum *TempHum = getTempHum();

    if (TempHum) {
      if ((Param2 != 0 || Param3 != 0)) {
        if (Param2 != 0) {
          TempHum->setTemperature(TempHum->getTemperature() +
                                  (Param2 / 100.00));
        }

        if (Param3 != 0) {
          TempHum->setHumidity(TempHum->getHumidity() + (Param3 / 100.00));
        }

        TempHum->toValue(this->value);
      }

      delete TempHum;
    }
  }

  if (Param3 == 1 && (isSensorNONC())) {
    this->value[0] = this->value[0] == 0 ? 1 : 0;
  }
}

supla_channel_temphum *client_device_channel::getTempHum(void) {
  double temp;

  if (getFunc() == SUPLA_CHANNELFNC_THERMOMETER) {
    getDouble(&temp);

    if (temp > -273 && temp <= 1000) {
      return new supla_channel_temphum(0, getId(), value);
    }

  } else if (getFunc() == SUPLA_CHANNELFNC_HUMIDITY ||
             getFunc() == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE) {
    int n;
    char value[SUPLA_CHANNELVALUE_SIZE];
    double humidity;

    getValue(value);
    memcpy(&n, value, 4);
    temp = n / 1000.00;

    memcpy(&n, &value[4], 4);
    humidity = n / 1000.00;

    if (temp > -273 && temp <= 1000 && humidity >= 0 && humidity <= 100) {
      return new supla_channel_temphum(1, getId(), value);
    }
  }

  return NULL;
}

supla_channel_thermostat_measurement *
client_device_channel::getThermostatMeasurement(void) {
  switch (getFunc()) {
    case SUPLA_CHANNELFNC_THERMOSTAT:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
      char value[SUPLA_CHANNELVALUE_SIZE];
      getValue(value);
      TThermostat_Value *th_val = (TThermostat_Value *)value;

      return new supla_channel_thermostat_measurement(
          getId(), th_val->IsOn > 0, th_val->MeasuredTemperature * 0.01,
          th_val->PresetTemperature * 0.01);
    }
  }
  return NULL;
}

bool client_device_channel::getRGBW(int *color, char *color_brightness,
                                    char *brightness, char *on_off) {
  if (color != NULL) *color = 0;

  if (color_brightness != NULL) *color_brightness = 0;

  if (brightness != NULL) *brightness = 0;

  if (on_off != NULL) *on_off = 0;

  bool result = false;

  if (Func == SUPLA_CHANNELFNC_DIMMER ||
      Func == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (brightness != NULL) {
      *brightness = this->value[0];

      if (*brightness < 0 || *brightness > 100) *brightness = 0;
    }

    result = true;
  }

  if (Func == SUPLA_CHANNELFNC_RGBLIGHTING ||
      Func == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (color_brightness != NULL) {
      *color_brightness = this->value[1];

      if (*color_brightness < 0 || *color_brightness > 100)
        *color_brightness = 0;
    }

    if (color != NULL) {
      *color = 0;

      *color = this->value[4] & 0xFF;
      (*color) <<= 8;

      *color |= this->value[3] & 0xFF;
      (*color) <<= 8;

      (*color) |= this->value[2] & 0xFF;
    }

    result = true;
  }

  return result;
}

void client_device_channel::getDouble(double *Value) {
  if (Value == NULL) return;

  switch (Func) {
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
    case SUPLA_CHANNELFNC_MAILSENSOR:
      *Value = this->value[0] == 1 ? 1 : 0;
      break;
    case SUPLA_CHANNELFNC_THERMOMETER:
    case SUPLA_CHANNELFNC_DISTANCESENSOR:
    case SUPLA_CHANNELFNC_DEPTHSENSOR:
    case SUPLA_CHANNELFNC_WINDSENSOR:
    case SUPLA_CHANNELFNC_PRESSURESENSOR:
    case SUPLA_CHANNELFNC_RAINSENSOR:
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
      memcpy(Value, this->value, sizeof(double));
      break;
    default:
      *Value = 0;
  }
}

client_device_channel::~client_device_channel() {
  if (this->Caption) {
    free(this->Caption);
    this->Caption = NULL;
  }
}

void client_device_channel::setType(int type) { this->Type = type; }

void client_device_channel::setFunction(int function) { this->Func = function; }

void client_device_channel::setFileName(const char *filename) {
  if (this->FileName) {
    free(this->FileName);
    this->FileName = NULL;
  }

  this->FileName = strdup(filename);
}

void client_device_channel::setPayloadOn(const char *payloadOn) {
  if (this->PayloadOn) {
    free(this->PayloadOn);
    this->PayloadOn = NULL;
  }

  this->PayloadOn = strdup(payloadOn);
}
void client_device_channel::setPayloadOff(const char *payloadOff) {
  if (this->PayloadOff) {
    free(this->PayloadOff);
    this->PayloadOff = NULL;
  }

  this->PayloadOff = strdup(payloadOff);
}
void client_device_channel::setPayloadValue(const char *payloadValue) {
  if (this->PayloadValue) {
    free(this->PayloadValue);
    this->PayloadValue = NULL;
  }

  this->PayloadValue = strdup(payloadValue);
}

void client_device_channel::setRetain(unsigned char retain) {
  this->Retain = retain;
}

void client_device_channel::setStateTopic(const char *stateTopic) {
  if (this->StateTopic) {
    free(this->StateTopic);
    this->StateTopic = NULL;
  }

  this->StateTopic = strdup(stateTopic);
}
char *client_device_channel::getPayloadOn(void) { return this->PayloadOn; };

char *client_device_channel::getStateTopic(void) { return this->StateTopic; }

char *client_device_channel::getCommandTopic(void) {
  return this->CommandTopic;
}
char *client_device_channel::getCommandTemplate(void) {
  return this->CommandTemplate;
}

void client_device_channel::setCommandTopic(const char *commandTopic) {
  if (this->CommandTopic) {
    free(this->CommandTopic);
    this->CommandTopic = NULL;
  }

  this->CommandTopic = strdup(commandTopic);
}

void client_device_channel::setCommandTemplate(const char *commandTemplate) {
  if (this->CommandTemplate) {
    free(this->CommandTemplate);
    this->CommandTemplate = NULL;
  }
  this->CommandTemplate = strdup(commandTemplate);
}

int client_device_channel::getTypeEx(void) {
  int type = this->getType();
  if (type == 0) {
    switch (this->getFunc()) {
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:  // ver. >= 8
      case SUPLA_CHANNELFNC_MAILSENSOR:            // ver. >= 8
        type = SUPLA_CHANNELTYPE_SENSORNO;
        break;

      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      case SUPLA_CHANNELFNC_STAIRCASETIMER:
        type = SUPLA_CHANNELTYPE_RELAYG5LA1A;
        break;

      case SUPLA_CHANNELFNC_THERMOMETER:
        type = SUPLA_CHANNELTYPE_THERMOMETER;
        break;

      case SUPLA_CHANNELFNC_DIMMER:
        type = SUPLA_CHANNELTYPE_DIMMER;
        break;

      case SUPLA_CHANNELFNC_RGBLIGHTING:
        type = SUPLA_CHANNELTYPE_RGBLEDCONTROLLER;
        break;

      case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
        type = SUPLA_CHANNELTYPE_DIMMERANDRGBLED;
        break;

      case SUPLA_CHANNELFNC_DEPTHSENSOR:     // ver. >= 5
      case SUPLA_CHANNELFNC_DISTANCESENSOR:  // ver. >= 5
        type = SUPLA_CHANNELTYPE_DISTANCESENSOR;
        break;

      case SUPLA_CHANNELFNC_HUMIDITY:
        type = SUPLA_CHANNELTYPE_HUMIDITYSENSOR;
        break;

      case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
        type = SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR;
        break;

      case SUPLA_CHANNELFNC_WINDSENSOR:
        type = SUPLA_CHANNELTYPE_WINDSENSOR;
        break;  // ver. >= 8
      case SUPLA_CHANNELFNC_PRESSURESENSOR:
        type = SUPLA_CHANNELTYPE_PRESSURESENSOR;
        break;  // ver. >= 8
      case SUPLA_CHANNELFNC_RAINSENSOR:
        type = SUPLA_CHANNELTYPE_RAINSENSOR;
        break;  // ver. >= 8
      case SUPLA_CHANNELFNC_WEIGHTSENSOR:
        type = SUPLA_CHANNELTYPE_WEIGHTSENSOR;
        break;
      case SUPLA_CHANNELFNC_WEATHER_STATION:
        type = SUPLA_CHANNELTYPE_WEATHER_STATION;
        break;

      case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:  // ver. >= 12
      case SUPLA_CHANNELFNC_IC_GAS_METER:          // ver. >= 10
      case SUPLA_CHANNELFNC_IC_WATER_METER:
        type = SUPLA_CHANNELTYPE_IMPULSE_COUNTER;
        break;  // ver. >= 10

      case SUPLA_CHANNELFNC_ELECTRICITY_METER:
        type = SUPLA_CHANNELTYPE_ELECTRICITY_METER;
        break;

      case SUPLA_CHANNELFNC_THERMOSTAT:
        return SUPLA_CHANNELTYPE_THERMOSTAT;
        break;  // ver. >= 11
      case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
        type = SUPLA_CHANNELTYPE_THERMOSTAT_HEATPOL_HOMEPLUS;
        break;
      case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:  // ver. >= 12
      case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
        type = SUPLA_CHANNELTYPE_VALVE;
        break;

      case SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT:
        break;
        type = SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT;
        break;
      case SUPLA_CHANNELFNC_CONTROLLINGTHEENGINESPEED:
        type = SUPLA_CHANNELTYPE_ENGINE;
        break;
      case SUPLA_CHANNELFNC_ACTIONTRIGGER:
        type = SUPLA_CHANNELTYPE_ACTIONTRIGGER;
    }

    this->Type = type;
  }

  return type;
}

void client_device_channel::setStateTemplate(const char *stateTemplate) {
  if (this->StateTemplate) {
    free(this->StateTemplate);
    this->StateTemplate = NULL;
  }
  this->StateTemplate = strdup(stateTemplate);
}

void client_device_channel::setCaption(char *caption) {
  if (this->Caption) {
    free(this->Caption);
    this->Caption = NULL;
  }
  this->Caption =
      caption ? strndup(caption, SUPLA_CHANNEL_CAPTION_MAXSIZE) : NULL;
}

char *client_device_channel::getCaption(void) { return Caption; }

void client_device_channel::setSubValue(
    char sub_value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(this->Sub_value, sub_value, SUPLA_CHANNELVALUE_SIZE);
}

void client_device_channel::getSubValue(
    char sub_value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(sub_value, this->Sub_value, SUPLA_CHANNELVALUE_SIZE);
}

void client_device_channel::setOnline(bool value) { this->Online = value; }

bool client_device_channel::getOnline() { return this->Online; }

void client_device_channels::add_channel(int Id, int Number, int Type, int Func,
                                         int Param1, int Param2, int Param3,
                                         char *TextParam1, char *TextParam2,
                                         char *TextParam3, bool Hidden,
                                         bool Online, char *Caption) {
  safe_array_lock(arr);

  client_device_channel *channel = find_channel(Id);
  if (channel == 0) {
    client_device_channel *c = new client_device_channel(
        Id, Number, Type, Func, Param1, Param2, Param3, TextParam1, TextParam2,
        TextParam3, Hidden, Online, Caption);

    if (c != NULL && safe_array_add(arr, c) == -1) {
      delete c;
      c = NULL;
    }
  } else {
    channel->setOnline(Online);
    channel->setCaption(Caption);
  }

  safe_array_unlock(arr);
}

void client_device_channels::set_channel_sub_value(
    int ChannelID, char sub_value[SUPLA_CHANNELVALUE_SIZE]) {
  if (ChannelID == 0) return;

  safe_array_lock(arr);

  client_device_channel *channel = find_channel(ChannelID);

  if (channel) {
    channel->setSubValue(sub_value);
  }
  safe_array_unlock(arr);
}

client_device_channel *client_device_channels::add_empty_channel(
    int ChannelId) {
  safe_array_lock(arr);

  client_device_channel *c = NULL;

  c = new client_device_channel(ChannelId, ChannelId, 0, 0, 0, 0, 0, NULL, NULL,
                                NULL, false, true, NULL);
  if (c != NULL && safe_array_add(arr, c) == -1) {
    delete c;
    c = NULL;
  }

  safe_array_unlock(arr);
  return c;
}

const char **client_device_channels::getMqttSubscriptionTopics(int *count) {
  *count = 0;

  const char **result = NULL;

  safe_array_lock(arr);

  int i;

  client_device_channel *channel;
  for (i = 0; i < safe_array_count(arr); i++) {
    channel = (client_device_channel *)safe_array_get(arr, i);

    if (channel && channel->getStateTopic()) *count = *count + 1;
  };

  result = (const char **)malloc(*count * sizeof(char *));

  for (i = 0; i < safe_array_count(arr); i++) {
    channel = (client_device_channel *)safe_array_get(arr, i);

    if (channel && channel->getStateTopic())
      result[i] = strdup(channel->getStateTopic());
  };

  safe_array_unlock(arr);

  return result;
}

void client_device_channels::setValueChangedCallback(
    _func_channelio_valuechanged cb, void *user_data) {
  this->on_valuechanged = on_valuechanged;
  this->on_valuechanged_user_data = user_data;
}

int client_device_channels::getCount(void) { return safe_array_count(arr); }

client_device_channel *client_device_channels::find_channel(int ChannelId) {
  client_device_channel *result =
      (client_device_channel *)safe_array_findcnd(arr, arr_findcmp, &ChannelId);

  if (result == NULL) result = add_empty_channel(ChannelId);
  return result;
}

client_device_channel *client_device_channels::getChannel(int idx) {
  return (client_device_channel *)safe_array_get(arr, idx);
}

client_device_channel *client_device_channels::find_channel_by_topic(
    const char *topic) {
  int i;

  safe_array_lock(arr);

  client_device_channel *channel;

  for (i = 0; i < safe_array_count(arr); i++) {
    channel = (client_device_channel *)safe_array_get(arr, i);

    if (channel && strlen(channel->getStateTopic()) > 0 &&
        strcasecmp(channel->getStateTopic(), topic) == 0) {
      safe_array_unlock(arr);
      return channel;
    }
  };

  safe_array_unlock(arr);
  return NULL;
}
