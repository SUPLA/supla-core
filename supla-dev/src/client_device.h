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
#include <string.h>

#include <string>
#include <vector>

#include "device.h"
#include "supla-client-lib/devicechannel.h"
#include "supla-client-lib/log.h"
#include "supla-client-lib/safearray.h"

class client_device_channel {
 private:
  int type;
  int function;
  int number;
  int intervalSec;
  int fileWriteCheckSeck;
  int toggleSec;
  std::string fileName;
  std::string payloadOn;
  std::string payloadOff;
  std::string payloadValue;
  std::string stateTopic;
  std::string commandTopic;
  std::string stateTemplate;
  std::string commandTemplate;
  std::string execute;
  std::string executeOn;
  std::string executeOff;
  bool retain;
  bool online;
  bool toggled;

  void *lck;
  struct timeval last;
  char value[SUPLA_CHANNELVALUE_SIZE];

  bool isSensorNONC(void);

 public:
  client_device_channel(int number);
  ~client_device_channel();

  /* properties */
  int getType(void);
  int getFunction(void);
  int getNumber(void);
  int getIntervalSec(void);
  int getToggleSec(void);
  std::string getFileName(void);
  std::string getPayloadOn(void);
  std::string getPayloadOff(void);
  std::string getPayloadValue(void);
  std::string getStateTopic(void);
  std::string getCommandTopic(void);
  std::string getStateTemplate(void);
  std::string getCommandTemplate(void);
  std::string getExecute(void);
  std::string getExecuteOn(void);
  std::string getExecuteOff(void);
  bool getRetain(void);
  bool getOnline(void);
  bool getToggled(void);
  long getLastSeconds(void);
  int getFileWriteCheckSec(void);

  void setType(int type);
  void setFunction(int function);
  void setNumber(int number);
  void setIntervalSec(int interval);
  void setToggleSec(int interval);
  void setFileName(const char *filename);
  void setPayloadOn(const char *payloadOn);
  void setPayloadOff(const char *payloadOff);
  void setPayloadValue(const char *payloadValue);
  void setStateTopic(const char *stateTopic);
  void setCommandTopic(const char *commandTopic);
  void setStateTemplate(const char *stateTemplate);
  void setCommandTemplate(const char *commandTemplate);
  void setExecute(const char *execute);
  void setExecuteOn(const char *execute);
  void setExecuteOff(const char *execute);
  void setRetain(bool retain);
  void setOnline(bool online);
  void setToggled(bool toggled);
  void setLastSeconds(void);
  void setFileWriteCheckSec(int value);

  /* value handler */
  void getValue(char value[SUPLA_CHANNELVALUE_SIZE]);
  void getDouble(double *result);
  void getTempHum(double *temp, double *humidity, bool *isTemperature,
                  bool *isHumidity);
  bool getRGBW(int *color, char *color_brightness, char *brightness,
               char *on_off);
  char getChar(void);

  void setValue(char value[SUPLA_CHANNELVALUE_SIZE]);
  void setDouble(double value);
  void setTempHum(double temp, double humidity);
  void setRGBW(int color, char color_brightness, char brightness, char on_off);
  void setChar(char value);

  void toggleValue(void);
};

class client_device_channels {
 private:
  bool initialized;
  void *arr;

 public:
  client_device_channels();
  client_device_channel *add_channel(int number);

  client_device_channel *find_channel(int number);
  client_device_channel *find_channel_by_topic(std::string topic);
  client_device_channel *getChannel(int idx);
  int getChannelCount(void);

  void getMqttSubscriptionTopics(std::vector<std::string> *vect);

  bool getInitialized(void);
  void setInitialized(bool initialized);

  _func_channelio_valuechanged on_valuechanged;
  void *on_valuechanged_user_data;
};

#endif
