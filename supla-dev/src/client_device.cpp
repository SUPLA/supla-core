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

client_device_channel::client_device_channel(int number) {
   this->number = number;
   this->type = 0;
   this->function = 0;
   this->intervalSec = 10; /* default 10 seconds*/
   this->fileName = "";
   this->payloadOn = "";
   this->payloadOff = "";
   this->payloadValue = "";
   this->stateTopic = "";
   this->commandTopic = "";
   this->stateTemplate = "";
   this->commandTemplate = "";
   this->execute = "";
   this->executeOn = "";
   this->executeOff = "";
   this->retain = true;
   this->online = true;
   this->last = (struct timeval){0};	 
   
   this->lck = lck_init();
}
client_device_channel::~client_device_channel() { lck_free(lck); }

bool client_device_channel::isSensorNONC(void) {
  switch (this->function) {
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
void client_device_channel::getValue(char value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(value, this->value, SUPLA_CHANNELVALUE_SIZE);
}
void client_device_channel::getDouble(double *result) {
  
  if (result == NULL) return;

  switch (this->function) {
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
    case SUPLA_CHANNELFNC_MAILSENSOR:
      *result = this->value[0] == 1 ? 1 : 0;
      break;
    case SUPLA_CHANNELFNC_THERMOMETER:
    case SUPLA_CHANNELFNC_DISTANCESENSOR:
    case SUPLA_CHANNELFNC_DEPTHSENSOR:
    case SUPLA_CHANNELFNC_WINDSENSOR:
    case SUPLA_CHANNELFNC_PRESSURESENSOR:
    case SUPLA_CHANNELFNC_RAINSENSOR:
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
      memcpy(result, this->value, sizeof(double));
      break;
    default:
      *result = 0;
  }
}
void client_device_channel::getTempHum(double* temp, double* humidity, 
   bool* isTemperature, bool* isHumidity) {
    
   *isTemperature = false;
   *isHumidity = false;
   if (temp == NULL || humidity == NULL) return;
    
   switch (this->function) {
	 SUPLA_CHANNELFNC_THERMOMETER: {
       getDouble(temp);
 	   if (temp > -273 && temp <= 1000) 
		 isTemperature = true;
	 }; break;
	 SUPLA_CHANNELFNC_HUMIDITY:
	 SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
	 {
		int n;
		char value[SUPLA_CHANNELVALUE_SIZE];
		double humidity;

		getValue(value);
		memcpy(&n, value, 4);
		temp = n / 1000.00;

		memcpy(&n, &value[4], 4);
		humidity = n / 1000.00;
        
		if (temp > -273 && temp <= 1000)
		  *isTemperature = true;
	  
	    if (humidity >= 0 && humidity <= 100)
		  *isHumidity = true;
	  }; break;
	}
 }
bool client_device_channel::getRGBW(int *color, char *color_brightness,
                                    char *brightness, char *on_off) {
  if (color != NULL) *color = 0;

  if (color_brightness != NULL) *color_brightness = 0;

  if (brightness != NULL) *brightness = 0;

  if (on_off != NULL) *on_off = 0;

  bool result = false;

  if (this->function == SUPLA_CHANNELFNC_DIMMER ||
      this->function == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (brightness != NULL) {
      *brightness = this->value[0];

      if (*brightness < 0 || *brightness > 100) *brightness = 0;
    }

    result = true;
  }

  if (this->function == SUPLA_CHANNELFNC_RGBLIGHTING ||
      this->function == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
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
char client_device_channel::getChar(void) { return this->value[0]; }
void client_device_channel::setValue(char value[SUPLA_CHANNELVALUE_SIZE]) {
  
  lck_lock(lck);
  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);
  lck_unlock(lck);
}
void client_device_channel::setDouble(double value) {
  lck_lock(lck);
  memcpy(this->value, value, sizeof(double));
  lck_unlock(lck);
}
void client_device_channel::setTempHum(double temp, double humidity) {
  switch (this->function) {
	 case SUPLA_CHANNELFNC_THERMOMETER: {
       setDouble(temp);
	 } break;
	 case SUPLA_CHANNELFNC_HUMIDITY:
	 case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
	 {
		int n;
        char tmp_value[SUPLA_CHANNELVALUE_SIZE];
		
		n = temp * 1000;
		memcpy(tmp_value, &n, 4);

		n = humidity * 1000;
		memcpy(&tmp_value[4], &n, 4);

		setValue(tmp_value);
	  } break;
	}
}
void client_device_channel::setRGBW(int color, char color_brightness, char brightness, 
  char on_off) {
	   
  char tmp_value[SUPLA_CHANNELVALUE_SIZE];
  
  if (this->function == SUPLA_CHANNELFNC_DIMMER ||
      this->function == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (brightness < 0 || brightness > 100) brightness = 0;

    tmp_value[0] = brightness;
  }

  if (this->function == SUPLA_CHANNELFNC_RGBLIGHTING ||
      this->function == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (color_brightness < 0 || color_brightness > 100) color_brightness = 0;

    tmp_value[1] = color_brightness;
    tmp_value[2] = (char)((color & 0x000000FF));
    tmp_value[3] = (char)((color & 0x0000FF00) >> 8);
    tmp_value[4] = (char)((color & 0x00FF0000) >> 16);
    tmp_value[5] = on_off;
  }
  
  setValue(tmp_value);
}
void client_device_channel::setChar(char chr){
  value[0] = chr;
}
int client_device_channel::getType(void) {
  if (this->type == 0) {
    switch (this->function) {
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:  // ver. >= 8
      case SUPLA_CHANNELFNC_MAILSENSOR:            // ver. >= 8
        this->type = SUPLA_CHANNELTYPE_SENSORNO;
        break;

      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      case SUPLA_CHANNELFNC_STAIRCASETIMER:
        this->type = SUPLA_CHANNELTYPE_RELAYG5LA1A;
        break;

      case SUPLA_CHANNELFNC_THERMOMETER:
        this->type = SUPLA_CHANNELTYPE_THERMOMETER;
        break;

      case SUPLA_CHANNELFNC_DIMMER:
        this->type = SUPLA_CHANNELTYPE_DIMMER;
        break;

      case SUPLA_CHANNELFNC_RGBLIGHTING:
        this->type = SUPLA_CHANNELTYPE_RGBLEDCONTROLLER;
        break;

      case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
        this->type = SUPLA_CHANNELTYPE_DIMMERANDRGBLED;
        break;

      case SUPLA_CHANNELFNC_DEPTHSENSOR:     // ver. >= 5
      case SUPLA_CHANNELFNC_DISTANCESENSOR:  // ver. >= 5
        this->type = SUPLA_CHANNELTYPE_DISTANCESENSOR;
        break;

      case SUPLA_CHANNELFNC_HUMIDITY:
        this->type = SUPLA_CHANNELTYPE_HUMIDITYSENSOR;
        break;

      case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
        this->type = SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR;
        break;

      case SUPLA_CHANNELFNC_WINDSENSOR:
        this->type = SUPLA_CHANNELTYPE_WINDSENSOR;
        break;  // ver. >= 8
      case SUPLA_CHANNELFNC_PRESSURESENSOR:
        this->type = SUPLA_CHANNELTYPE_PRESSURESENSOR;
        break;  // ver. >= 8
      case SUPLA_CHANNELFNC_RAINSENSOR:
        this->type = SUPLA_CHANNELTYPE_RAINSENSOR;
        break;  // ver. >= 8
      case SUPLA_CHANNELFNC_WEIGHTSENSOR:
        this->type = SUPLA_CHANNELTYPE_WEIGHTSENSOR;
        break;
      case SUPLA_CHANNELFNC_WEATHER_STATION:
        this->type = SUPLA_CHANNELTYPE_WEATHER_STATION;
        break;

      case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:  // ver. >= 12
      case SUPLA_CHANNELFNC_IC_GAS_METER:          // ver. >= 10
      case SUPLA_CHANNELFNC_IC_WATER_METER:
        this->type = SUPLA_CHANNELTYPE_IMPULSE_COUNTER;
        break;  // ver. >= 10

      case SUPLA_CHANNELFNC_ELECTRICITY_METER:
        this->type = SUPLA_CHANNELTYPE_ELECTRICITY_METER;
        break;

      case SUPLA_CHANNELFNC_THERMOSTAT:
        this->type = SUPLA_CHANNELTYPE_THERMOSTAT;
        break;  // ver. >= 11
      case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
        this->type = SUPLA_CHANNELTYPE_THERMOSTAT_HEATPOL_HOMEPLUS;
        break;
      case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:  // ver. >= 12
      case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
        this->type = SUPLA_CHANNELTYPE_VALVE;
        break;

      case SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT:
        break;
        this->type = SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT;
        break;
      case SUPLA_CHANNELFNC_CONTROLLINGTHEENGINESPEED:
        this->type = SUPLA_CHANNELTYPE_ENGINE;
        break;
      case SUPLA_CHANNELFNC_ACTIONTRIGGER:
        this->type = SUPLA_CHANNELTYPE_ACTIONTRIGGER;
    }
  }

  return this->type;
}
int client_device_channel::getFunction(void) { return this->function; } 
int client_device_channel::getNumber(void) { return this->number; }
int client_device_channel::getIntervalSec() { return this->intervalSec; }
std::string client_device_channel::getFileName(void) { return this->fileName; }
std::string client_device_channel::getPayloadOn(void) {
  return this->payloadOn;
};
std::string client_device_channel::getPayloadOff(void) {
  return this->payloadOff;
};
std::string client_device_channel::getPayloadValue(void) {
  return this->payloadValue;
};
std::string client_device_channel::getStateTopic(void) {
  return this->stateTopic;
}
std::string client_device_channel::getCommandTopic(void) {
  return this->commandTopic;
}
std::string client_device_channel::getStateTemplate(void) {
  return this->stateTemplate;	
}
std::string client_device_channel::getCommandTemplate(void) {
  return this->commandTemplate;
}
std::string client_device_channel::getExecute(void) { return this->execute; }
std::string client_device_channel::getExecuteOn(void) {
  return this->executeOn;
}
std::string client_device_channel::getExecuteOff(void) {
  return this->executeOff;
}
bool client_device_channel::getRetain(void) { return this->retain; }
bool client_device_channel::getOnline(void) { return this->online; }
long client_device_channel::getLastSeconds(void) {
  return this->last.tv_sec;
}

void client_device_channel::setType(int type) { this->type = type; }
void client_device_channel::setFunction(int function) { this->function = function; }
void client_device_channel::setNumber(int number) {this->number = number; }
void client_device_channel::setFileName(const char *filename) {
  this->fileName = std::string(filename);
}
void client_device_channel::setPayloadOn(const char *payloadOn) {
  this->payloadOn = std::string(payloadOn);
}
void client_device_channel::setPayloadOff(const char *payloadOff) {
  this->payloadOff = std::string(payloadOff);
}
void client_device_channel::setPayloadValue(const char *payloadValue) {
  this->payloadValue = std::string(payloadValue);
}
void client_device_channel::setStateTopic(const char *stateTopic) {
  this->stateTopic = std::string(stateTopic);
}
void client_device_channel::setCommandTopic(const char *commandTopic) {
  this->commandTopic = std::string(commandTopic);
}
void client_device_channel::setStateTemplate(const char *stateTemplate) {
  this->stateTemplate = std::string(stateTemplate);
}
void client_device_channel::setCommandTemplate(const char *commandTemplate) {
  this->commandTemplate = std::string(commandTemplate);
}
void client_device_channel::setExecute(const char *execute) {
  this->execute = std::string(execute);
}
void client_device_channel::setExecuteOn(const char *execute) {
  this->executeOn = std::string(execute);
}
void client_device_channel::setExecuteOff(const char *execute) {
  this->executeOff = std::string(execute);
}
void client_device_channel::setRetain(bool retain) { this->retain = retain; }
void client_device_channel::setOnline(bool value) { this->online = value; }
void client_device_channel::setLastSeconds(void) {
  gettimeofday(&this->last, NULL);
}

client_device_channels::client_device_channels() {
  this->initialized = false;
  this->arr = safe_array_init();
}
client_device_channel* client_device_channels::add_channel(int number) {
  safe_array_lock(arr);
  
  client_device_channel *channel = find_channel(number);
  if (channel == 0) {
    client_device_channel *c = new client_device_channel(number);

    if (c != NULL && safe_array_add(arr, c) == -1) {
      delete c;
      c = NULL;
    }
  };

  safe_array_unlock(arr);
  return c;
}

void client_device_channels::getMqttSubscriptionTopics(
    std::vector<std::string> *vect) {
  client_device_channel *channel;
  int i;

  for (i = 0; i < safe_array_count(arr); i++) {
    channel = (client_device_channel *)safe_array_get(arr, i);
    if (channel->getStateTopic().length() > 0) {
      vect->push_back(channel->getStateTopic());
    }
  }
}
int client_device_channels::getChannelCount(void) { return safe_array_count(arr); }
bool client_device_channels::getInitialized(void) { return this->initialized; }
void client_device_channels::setInitialized(bool initialized) {
  this->initialized = initialized;
}
client_device_channel *client_device_channels::getChannel(int idx) {
  return (client_device_channel *)safe_array_get(arr, idx);
}
client_device_channel *client_device_channels::find_channel(int number) {
  
  int i;

  client_device_channel *channel;

  for (i = 0; i < safe_array_count(arr); i++) {
    channel = (client_device_channel *)safe_array_get(arr, i);
     
    if (channel->getNumber() == number ) {
      break;
    }
  };
  
  if (channel == NULL) 
	channel = add_channel(number); 
  
  return channel;
  
}
client_device_channel *client_device_channels::find_channel_by_topic(std::string topic) {
  
  int i;

  client_device_channel *channel;

  for (i = 0; i < safe_array_count(arr); i++) {
    channel = (client_device_channel *)safe_array_get(arr, i);
    std::string topic_str = channel->getStateTopic();

    if (topic_str.length() > 0 && (topic_str.compare(topic) == 0)) {
      return channel;
    }
  };

  return NULL;
}
