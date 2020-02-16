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

#ifndef CLIENT_DEVICE_H
#define CLIENT_DEVICE_H

#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>
#include "device.h"
#include "supla-client-lib/devicechannel.h"
#include "supla-client-lib/safearray.h"

class client_device_channel : public supla_device_channel {
 private:
  char *FileName;
  char *PayloadOn;
  char *PayloadOff;
  char *PayloadValue;
  char *StateTopic;
  char *CommandTopic;
  char *StateTemplate;
  char *CommandTemplate;
  char *Execute;
  char Retain;
  int intervalSec;

  char *Caption;
  char Sub_value[8];
  bool Online;

 public:
  void *lck;
  struct timeval last_tv;

  client_device_channel(int Id, int Number, int Type, int Func, int Param1,
                        int Param2, int Param3, char *TextParam1,
                        char *TextParam2, char *TextParam3, bool Hidden,
                        bool Online, char *Caption);
  ~client_device_channel();
  bool isSensorNONC(void);
  void setValue(char value[SUPLA_CHANNELVALUE_SIZE]);
  void getDouble(double *Value);
  bool getRGBW(int *color, char *color_brightness, char *brightness,
               char *on_off);
  supla_channel_thermostat_measurement *getThermostatMeasurement(void);
  supla_channel_temphum *getTempHum(void);
  bool getOnline();
  void setOnline(bool value);
  void setSubValue(char sub_value[SUPLA_CHANNELVALUE_SIZE]);
  void getSubValue(char sub_value[SUPLA_CHANNELVALUE_SIZE]);
  char *getCaption(void);
  void setCaption(char *caption);

  void setType(int type);
  void setFunction(int function);
  void setFileName(const char *filename);
  void setPayloadOn(const char *payloadOn);
  void setPayloadOff(const char *payloadOff);
  void setPayloadValue(const char *payloadValue);
  void setStateTopic(const char *stateTopic);
  void setCommandTopic(const char *commandTopic);
  void setStateTemplate(const char *stateTemplate);
  void setCommandTemplate(const char *commandTemplate);
  void setRetain(unsigned char retaint);
  void setExecute(const char *execute);
  void setInterval(int interval);

  int getTypeEx(void);
  char *getStateTopic(void);
  char *getCommandTopic(void);
  char *getCommandTemplate(void);
  char *getPayloadOn(void);
  char *getPayloadOff(void);
  char *getPayloadValue(void);
  char *getFileName(void);
  char *getExecute(void);
  int getIntervalSec(void);
};

class client_device_channels : public supla_device_channels {
 private:
  bool initialized;

  client_device_channel *add_empty_channel(int ChannelId);
  _func_channelio_valuechanged on_valuechanged;
  void *on_valuechanged_user_data;

 public:
  client_device_channels();
  void add_channel(int Id, int Number, int Type, int Func, int Param1,
                   int Param2, int Param3, char *TextParam1, char *TextParam2,
                   char *TextParam3, bool Hidden, bool Online, char *Caption);

  void set_channel_sub_value(int ChannelID,
                             char sub_value[SUPLA_CHANNELVALUE_SIZE]);

  client_device_channel *find_channel(int ChannelId);
  client_device_channel *find_channel_by_topic(const char *topic);
  client_device_channel *getChannel(int idx);

  void getMqttSubscriptionTopics(std::vector<std::string>* vect);
  void setValueChangedCallback(_func_channelio_valuechanged cb,
                               void *user_data);
  int getCount(void);
  bool getInitialized(void);
  void setInitialized(bool initialized);
};

#endif
