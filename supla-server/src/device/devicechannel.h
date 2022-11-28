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

class supla_device;
class supla_device_channels {
 private:
  void *arr;
  supla_device *device;

  static char arr_findcmp(void *ptr, void *id);
  static char arr_findncmp(void *ptr, void *n);
  static char arr_delcnd(void *ptr);
  void arr_clean(void);

  void *get_srpc(void);

  supla_device_channel *find_channel(int Id);
  supla_device_channel *find_channel_by_number(int Number);

  std::list<int> mr_channel(int ChannelID, bool Master);

  void async_set_channel_value(supla_device_channel *channel,
                               const supla_caller &caller, int GroupID,
                               unsigned char EOL,
                               const char value[SUPLA_CHANNELVALUE_SIZE],
                               unsigned int durationMS,
                               bool cancelTasks = true);
  void async_set_channel_value(supla_device_channel *channel,
                               const supla_caller &caller, int GroupID,
                               unsigned char EOL,
                               const char value[SUPLA_CHANNELVALUE_SIZE],
                               bool cancelTasks = true);
  bool set_device_channel_char_value(const supla_caller &caller,
                                     supla_device_channel *channel, int GroupID,
                                     unsigned char EOL, const char value,
                                     bool cancelTasks = true);
  bool set_on(const supla_caller &caller, int ChannelID, int GroupID,
              unsigned char EOL, bool on, bool toggle);
  bool rs_action(const supla_caller &caller, int ChannelID, int GroupID,
                 unsigned char EOL, rsAction action,
                 const char *closingPercentage, bool delta);
  bool action_open_close(const supla_caller &caller, int ChannelID, int GroupID,
                         unsigned char EOL, bool unknown, bool open,
                         bool cancelTasks = true);

 public:
  explicit supla_device_channels(supla_device *device);
  virtual ~supla_device_channels();

  void access_channel(int channel_id,
                      std::function<void(supla_device_channel *)> on_channel);

  void for_each_channel(std::function<void(supla_device_channel *)> on_channel);

  void add_channel(int Id, int Number, int Type, int Func, int Param1,
                   int Param2, int Param3, int Param4, const char *TextParam1,
                   const char *TextParam2, const char *TextParam3, bool Hidden,
                   unsigned int Flags,
                   const char value[SUPLA_CHANNELVALUE_SIZE],
                   unsigned _supla_int_t validity_time_sec,
                   const char *user_config, const char *properties);
  bool get_channel_value(int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE],
                         char *online, unsigned _supla_int_t *validity_time_sec,
                         TSuplaChannelExtendedValue *ev, int *function,
                         bool for_client);
  supla_channel_value *get_channel_value(int ChannelID);
  bool get_channel_extendedvalue(int ChannelID,
                                 TSuplaChannelExtendedValue *value);
  bool get_channel_extendedvalue(int ChannelID,
                                 TSC_SuplaChannelExtendedValue *cev);
  bool get_channel_double_value(int ChannelID, double *Value);
  supla_channel_temphum *get_channel_temp_and_humidity_value(int ChannelID);
  bool get_channel_temperature_value(int ChannelID, double *Value);
  bool get_channel_humidity_value(int ChannelID, double *Value);
  bool get_channel_char_value(int ChannelID, char *Value);
  bool get_channel_rgbw_value(int ChannelID, int *color, char *color_brightness,
                              char *brightness, char *on_off);

  unsigned int get_channel_value_duration(int ChannelID);
  int get_channel_func(int ChannelID);
  int get_channel_type(int ChannelID);
  bool set_channel_value(supla_device_channel *channel,
                         char value[SUPLA_CHANNELVALUE_SIZE],
                         bool *converted2extended,
                         const unsigned _supla_int_t *validity_time_sec,
                         bool *significantChange);
  bool set_channel_value(int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE],
                         bool *converted2extended,
                         const unsigned _supla_int_t *validity_time_sec,
                         bool *significantChange);
  bool set_channel_offline(int ChannelID, bool Offline);
  void set_channel_extendedvalue(int ChannelID, TSuplaChannelExtendedValue *ev);
  void update_channels(TDS_SuplaDeviceChannel_B *schannel_b,
                       TDS_SuplaDeviceChannel_C *schannel_c, int count);

  void on_device_registered(supla_user *user, int DeviceId,
                            TDS_SuplaDeviceChannel_B *schannel_b,
                            TDS_SuplaDeviceChannel_C *schannel_c, int count);

  void set_device_channel_value(const supla_caller &caller, int ChannelID,
                                int GroupID, unsigned char EOL,
                                const char value[SUPLA_CHANNELVALUE_SIZE]);
  bool set_device_channel_char_value(const supla_caller &caller, int ChannelID,
                                     int GroupID, unsigned char EOL,
                                     const char value);
  bool set_device_channel_rgbw_value(const supla_caller &caller, int ChannelID,
                                     int GroupID, unsigned char EOL, int color,
                                     char color_brightness, char brightness,
                                     char on_off);
  bool get_channel_valve_value(int ChannelID, TValve_Value *Value);
  bool get_dgf_transparency(int ChannelID, unsigned short *mask);

  std::list<int> master_channel(int ChannelID);
  std::list<int> related_channel(int ChannelID);
  std::list<int> get_channel_ids(void);
  int get_channel_id(unsigned char ChannelNumber);
  bool channel_exists(int ChannelID);
  bool is_channel_online(int ChannelID);
  void load(int UserID, int DeviceID);

  void get_temp_and_humidity(std::vector<supla_channel_temphum *> *result);
  void get_electricity_measurements(
      std::vector<supla_channel_electricity_measurement *> *result,
      bool for_data_logger_purposes);
  supla_channel_electricity_measurement *get_electricity_measurement(
      int ChannelID);
  void get_ic_measurements(std::vector<supla_channel_ic_measurement *> *result,
                           bool for_data_logger_purposes);
  supla_channel_ic_measurement *get_ic_measurement(int ChannelID);
  void get_thermostat_measurements(
      std::vector<supla_channel_thermostat_measurement *> *result);

  bool calcfg_request(const supla_caller &caller, int ChannelID,
                      bool SuperUserAuthorized,
                      TCS_DeviceCalCfgRequest_B *request);

  void set_channel_state(int channel_id, TDSC_ChannelState *state);

  bool get_channel_state(int channel_id, TDSC_ChannelState *state);

  bool get_channel_state_async(const supla_caller &caller,
                               TCSD_ChannelStateRequest *request);

  bool get_channel_complex_value(channel_complex_value *value, int ChannelID);
  void set_channel_function(int ChannelId, int Func);
  std::map<int, int> get_functions(void);
  bool get_channel_config(unsigned char channel_number, unsigned char type,
                          unsigned _supla_int_t flags,
                          TSD_ChannelConfig *config);
  void action_trigger(TDS_ActionTrigger *at);

  bool set_on(const supla_caller &caller, int ChannelID, int GroupID,
              unsigned char EOL, bool on);
  bool is_on(int ChannelID);
  bool set_rgbw(const supla_caller &caller, int ChannelID, int GroupID,
                unsigned char EOL, unsigned int *color, char *color_brightness,
                char *brightness, char *on_off);
  bool set_color(const supla_caller &caller, int ChannelID, int GroupID,
                 unsigned char EOL, unsigned int color);
  bool set_color_brightness(const supla_caller &caller, int ChannelID,
                            int GroupID, unsigned char EOL, char brightness);
  bool set_brightness(const supla_caller &caller, int ChannelID, int GroupID,
                      unsigned char EOL, char brightness);
  bool set_dgf_transparency(const supla_caller &caller, int ChannelID,
                            unsigned short activeBits, unsigned short mask);
  bool get_relay_value(int ChannelID, TRelayChannel_Value *relay_value);
  bool action_toggle(const supla_caller &caller, int ChannelID, int GroupID,
                     unsigned char EOL);
  bool action_shut(const supla_caller &caller, int ChannelID, int GroupID,
                   unsigned char EOL, const char *closingPercentage,
                   bool action);
  bool action_reveal(const supla_caller &caller, int ChannelID, int GroupID,
                     unsigned char EOL);
  bool action_up(const supla_caller &caller, int ChannelID, int GroupID,
                 unsigned char EOL);
  bool action_down(const supla_caller &caller, int ChannelID, int GroupID,
                   unsigned char EOL);
  bool action_up_or_stop(const supla_caller &caller, int ChannelID, int GroupID,
                         unsigned char EOL);
  bool action_down_or_stop(const supla_caller &caller, int ChannelID,
                           int GroupID, unsigned char EOL);
  bool action_step_by_step(const supla_caller &caller, int ChannelID,
                           int GroupID, unsigned char EOL);
  bool action_stop(const supla_caller &caller, int ChannelID, int GroupID,
                   unsigned char EOL);
  bool action_open(const supla_caller &caller, int ChannelID, int GroupID,
                   unsigned char EOL);
  bool action_close(const supla_caller &caller, int ChannelID, int GroupID,
                    unsigned char EOL);
  bool action_close(const supla_caller &caller, int ChannelID);
  bool action_open_close(const supla_caller &caller, int ChannelID, int GroupID,
                         unsigned char EOL);
  bool action_open_close_without_canceling_tasks(const supla_caller &caller,
                                                 int ChannelID, int GroupID,
                                                 unsigned char EOL);
  bool reset_counters(int ChannelID);
  bool recalibrate(int ChannelID, const supla_caller &caller,
                   bool SuperUserAuthorized);
  void timer_arm(const supla_caller &caller, int ChannelID, int GroupID,
                 unsigned char EOL, unsigned char On, unsigned int DurationMS);
  channel_json_config *get_json_config(int ChannelID);
  unsigned int get_value_validity_time_left_msec(void);
  void on_related_sensor_value_changed(int control_channel_id, int sensor_id,
                                       bool is_open);
};

#endif /* DEVICECHANNEL_H_ */
