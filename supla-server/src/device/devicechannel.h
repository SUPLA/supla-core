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

#ifndef DEVICECHANNEL_H_
#define DEVICECHANNEL_H_

#include <functional>
#include <list>
#include <map>
#include <vector>

#include "analyzer/voltage_analyzers.h"
#include "caller.h"
#include "channel_address.h"
#include "channel_electricity_measurement.h"
#include "channel_ic_measurement.h"
#include "channel_temphum.h"
#include "channel_thermostat_measurement.h"
#include "channel_value.h"
#include "commontypes.h"
#include "proto.h"

class supla_user;
class supla_device;
class channel_json_config;

enum rsAction {
  rsActionStop,
  rsActionDown,
  rsActionUp,
  rsActionDownOrStop,
  rsActionUpOrStop,
  rsActionStepByStep,
  rsActionShut,
  rsActionReveal
};

typedef struct {
  union {
    char value[SUPLA_CHANNELVALUE_SIZE];
    TSuplaChannelExtendedValue *extendedValue;
  };
} _logger_purpose_t;

class supla_device_channel {
 private:
  supla_device *Device;
  int Id;
  unsigned char Number;
  int Type;
  int Func;
  int Param1;
  int Param2;
  int Param3;
  int Param4;
  char *TextParam1;
  char *TextParam2;
  char *TextParam3;
  bool Hidden;
  bool Offline;
  unsigned int Flags;
  TDSC_ChannelState *state;
  char value[SUPLA_CHANNELVALUE_SIZE];
  struct timeval value_valid_to;  // during offline
  TSuplaChannelExtendedValue *extendedValue;
  channel_json_config *json_config;
  _logger_purpose_t *logger_data;
  supla_voltage_analyzers voltage_analyzers;

  void db_set_properties(channel_json_config *config);
  void db_set_params(int Param1, int Param2, int Param3, int Param4);
  void updateTimerState(void);
  void updateExtendedElectricityMeterValue(void);

 public:
  supla_device_channel(supla_device *Device, int Id, int Number, int Type,
                       int Func, int Param1, int Param2, int Param3, int Param4,
                       const char *TextParam1, const char *TextParam2,
                       const char *TextParam3, bool Hidden, unsigned int Flags,
                       const char value[SUPLA_CHANNELVALUE_SIZE],
                       unsigned _supla_int_t validity_time_sec,
                       const char *user_config, const char *properties);
  virtual ~supla_device_channel();

  static void getDefaults(int Type, int Func, int *Param1, int *Param2);
  static int funcListFilter(int FuncList, int Type);

  int getId(void);
  int getNumber(void);
  int getUserID(void);
  supla_user *getUser();
  supla_device *getDevice();
  int getFunc(void);
  void setFunc(int Func);
  int getType(void);
  int getParam1(void);
  int getParam2(void);
  int getParam3(void);
  int getParam4(void);
  const char *getTextParam1(void);
  const char *getTextParam2(void);
  const char *getTextParam3(void);
  bool getHidden(void);
  unsigned int getFlags();
  void addFlags(unsigned int flags);
  bool isOffline(void);
  bool setOffline(bool Offline);
  bool isValueWritable(void);
  bool isCharValueWritable(void);
  bool isRgbwValueWritable(void);
  unsigned int getValueDuration(void);
  unsigned _supla_int_t getValueValidityTimeSec(void);
  void getValue(char value[SUPLA_CHANNELVALUE_SIZE]);
  bool setValue(const char value[SUPLA_CHANNELVALUE_SIZE],
                const unsigned _supla_int_t *validity_time_sec,
                bool *significantChange, unsigned char proto_version);
  bool getExtendedValue(TSuplaChannelExtendedValue *ev, bool em_update);
  void setExtendedValue(TSuplaChannelExtendedValue *ev);
  void assignRgbwValue(char value[SUPLA_CHANNELVALUE_SIZE], int color,
                       char color_brightness, char brightness, char on_off);
  void assignCharValue(char value[SUPLA_CHANNELVALUE_SIZE], char cvalue);
  void getDouble(double *Value);
  void getChar(char *Value);
  bool getRGBW(int *color, char *color_brightness, char *brightness,
               char *on_off);
  bool getValveValue(TValve_Value *Value);
  bool getConfig(TSD_ChannelConfig *config, unsigned char configType,
                 unsigned _supla_int_t flags);
  void setActionTriggerConfig(unsigned int capabilities, int relatedChannelId,
                              unsigned int disablesLocalOperation);

  std::list<int> master_channel(void);
  std::list<int> related_channel(void);
  supla_channel_temphum *getTempHum(void);
  supla_channel_electricity_measurement *getElectricityMeasurement(
      bool for_data_logger_purposes);
  supla_channel_ic_measurement *getImpulseCounterMeasurement(
      bool for_data_logger_purposes);
  supla_channel_thermostat_measurement *getThermostatMeasurement(void);
  channel_json_config *getJSONConfig(void);
  bool converValueToExtended(void);
  unsigned int get_value_validity_time_left_msec(void);
  void set_state(TDSC_ChannelState *state);
  bool get_state(TDSC_ChannelState *state);
  supla_voltage_analyzers &get_voltage_analyzers(void);
};

#endif /* DEVICECHANNEL_H_ */
