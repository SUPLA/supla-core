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

#include <list>
#include "commontypes.h"
#include "proto.h"

class supla_user;

class channel_address {
 private:
  int DeviceId;
  int ChannelId;

 public:
  channel_address(int DeviceId, int ChannelId);
  int getDeviceId(void);
  int getChannelId(void);
};

class supla_channel_temphum {
 private:
  int ChannelId;
  bool TempAndHumidity;
  double Temperature;
  double Humidity;

 public:
  supla_channel_temphum(bool TempAndHumidity, int ChannelId, double Temperature,
                        double Humidity);
  supla_channel_temphum(bool TempAndHumidity, int ChannelId,
                        char value[SUPLA_CHANNELVALUE_SIZE]);

  int getChannelId(void);
  bool isTempAndHumidity(void);
  double getTemperature(void);
  double getHumidity(void);
  void setTemperature(double Temperature);
  void setHumidity(double Humidity);
  void toValue(char value[SUPLA_CHANNELVALUE_SIZE]);

  static void free(void *tarr);
};

class supla_channel_electricity_measurement {
 private:
  TElectricityMeter_ExtendedValue em_ev;
  int ChannelId;

 public:
  supla_channel_electricity_measurement(int ChannelId,
                                        TElectricityMeter_ExtendedValue *em_ev,
                                        int Param2, char *TextParam1);

  int getChannelId(void);
  void getMeasurement(TElectricityMeter_ExtendedValue *em_ev);
  void getCurrency(char currency[4]);

  static void free(void *emarr);
};

class supla_channel_ic_measurement {
 private:
  int ChannelId;

  _supla_int_t totalCost;
  _supla_int_t pricePerUnit;
  char currency[4];
  char customUnit[9];
  _supla_int_t impulsesPerUnit;
  unsigned _supla_int64_t counter;
  _supla_int64_t calculatedValue;

 public:
  supla_channel_ic_measurement(int ChannelId, TDS_ImpulseCounter_Value *ic_val,
                               char *TextParam1, char *TextParam2, int Param2,
                               int Param3);

  int getChannelId(void);
  _supla_int_t getTotalCost(void);
  _supla_int_t getPricePerUnit(void);
  const char *getCurrncy(void);
  const char *getCustomUnit(void);
  _supla_int_t getImpulsesPerUnit(void);
  unsigned _supla_int64_t getCounter(void);
  unsigned _supla_int64_t getCalculatedValue(void);

  static double get_calculated_d(_supla_int_t impulses_per_unit,
                                 unsigned _supla_int64_t counter);
  static _supla_int64_t get_calculated_i(_supla_int_t impulses_per_unit,
                                         unsigned _supla_int64_t counter);
  static void get_cost_and_currency(char *TextParam1, int Param2,
                                    char currency[3], _supla_int_t *total_cost,
                                    _supla_int_t *price_per_unit, double count);
  static void free(void *icarr);
};

class supla_device_channel {
 private:
  int Id;
  unsigned char Number;
  int Type;
  int Func;
  int Param1;
  int Param2;
  int Param3;
  char *TextParam1;
  char *TextParam2;
  char *TextParam3;
  bool Hidden;

  char value[8];
  TSuplaChannelExtendedValue *extendedValue;

 public:
  supla_device_channel(int Id, int Number, int Type, int Func, int Param1,
                       int Param2, int Param3, char *TextParam1,
                       char *TextParam2, char *TextParam3, bool Hidden);
  virtual ~supla_device_channel();

  int getId(void);
  int getNumber(void);
  int getFunc(void);
  int getType(void);
  int getParam1(void);
  bool getHidden(void);
  bool isValueWritable(void);
  bool isCharValueWritable(void);
  bool isRgbwValueWritable(void);
  unsigned int getValueDuration(void);
  void getValue(char value[SUPLA_CHANNELVALUE_SIZE]);
  void setValue(char value[SUPLA_CHANNELVALUE_SIZE]);
  bool getExtendedValue(TSuplaChannelExtendedValue *ev);
  void setExtendedValue(TSuplaChannelExtendedValue *ev);
  void assignRgbwValue(char value[SUPLA_CHANNELVALUE_SIZE], int color,
                       char color_brightness, char brightness, char on_off);
  void assignCharValue(char value[SUPLA_CHANNELVALUE_SIZE], char cvalue);
  void getDouble(double *Value);
  void getChar(char *Value);
  bool getRGBW(int *color, char *color_brightness, char *brightness,
               char *on_off);

  std::list<int> master_channel(void);
  std::list<int> slave_channel(void);
  supla_channel_temphum *getTempHum(void);
  supla_channel_electricity_measurement *getElectricityMeasurement(void);
  supla_channel_ic_measurement *getImpulseCounterMeasurement(void);
  bool converValueToExtended(void);
};

class supla_device_channels {
 private:
  void *arr;

  static char arr_findcmp(void *ptr, void *id);
  static char arr_findncmp(void *ptr, void *n);
  static char arr_delcnd(void *ptr);
  void arr_clean(void);

  supla_device_channel *find_channel(int Id);
  supla_device_channel *find_channel_by_number(int Number);

  std::list<int> ms_channel(int ChannelID, bool Master);

 public:
  supla_device_channels();
  virtual ~supla_device_channels();
  void add_channel(int Id, int Number, int Type, int Func, int Param1,
                   int Param2, int Param3, char *TextParam1, char *TextParam2,
                   char *TextParam3, bool Hidden);
  bool get_channel_value(int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE]);
  bool get_channel_extendedvalue(int ChannelID,
                                 TSuplaChannelExtendedValue *value);
  bool get_channel_double_value(int ChannelID, double *Value);
  supla_channel_temphum *get_channel_temp_and_humidity_value(int ChannelID);
  bool get_channel_temperature_value(int ChannelID, double *Value);
  bool get_channel_humidity_value(int ChannelID, double *Value);
  bool get_channel_char_value(int ChannelID, char *Value);
  bool get_channel_rgbw_value(int ChannelID, int *color, char *color_brightness,
                              char *brightness, char *on_off);

  unsigned int get_channel_value_duration(int ChannelID);
  int get_channel_func(int ChannelID);
  void set_channel_value(int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE],
                         bool *converted2extended);
  void set_channel_extendedvalue(int ChannelID, TSuplaChannelExtendedValue *ev);
  void set_channels_value(TDS_SuplaDeviceChannel_B *schannel_b,
                          TDS_SuplaDeviceChannel_C *schannel_c, int count);

  void on_device_registered(supla_user *user, int DeviceId,
                            TDS_SuplaDeviceChannel_B *schannel_b,
                            TDS_SuplaDeviceChannel_C *schannel_c, int count);

  void set_device_channel_value(void *srpc, int SenderID, int ChannelID,
                                const char value[SUPLA_CHANNELVALUE_SIZE]);
  bool set_device_channel_char_value(void *srpc, int SenderID, int ChannelID,
                                     const char value);
  bool set_device_channel_rgbw_value(void *srpc, int SenderID, int ChannelID,
                                     int color, char color_brightness,
                                     char brightness, char on_off);

  std::list<int> master_channel(int ChannelID);
  std::list<int> slave_channel(int ChannelID);
  std::list<int> get_channel_ids(void);
  int get_channel_id(unsigned char ChannelNumber);
  bool channel_exists(int ChannelID);
  void load(int DeviceID);

  void get_temp_and_humidity(void *tarr);
  void get_electricity_measurements(void *emarr);
  supla_channel_electricity_measurement *get_electricity_measurement(
      int ChannelID);
  void get_ic_measurements(void *icarr);
  supla_channel_ic_measurement *get_ic_measurement(int ChannelID);

  bool calcfg_request(void *srpc, int SenderID, bool SuperUserAuthorized,
                      TCS_DeviceCalCfgRequest *request);

  bool get_channel_complex_value(channel_complex_value *value, int ChannelID);
};

#endif /* DEVICECHANNEL_H_ */
