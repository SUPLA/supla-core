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
#include "device/extended_value/channel_extended_value.h"
#include "device/value/channel_temphum_value.h"
#include "device/value/channel_value.h"
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

class supla_device_channel {
 private:
  void *lck;
  supla_device *device;
  const int id;
  const unsigned char number;
  const int type;
  int func;
  int param1;
  const int param2;
  const int param3;
  const int param4;
  char *text_param1;
  char *text_param2;
  char *text_param3;
  const bool hidden;
  bool offline;
  unsigned int flags;
  TDSC_ChannelState *state;
  char value[SUPLA_CHANNELVALUE_SIZE];
  struct timeval value_valid_to;  // during offline
  supla_channel_extended_value *extended_value;
  channel_json_config *json_config;
  _logger_purpose_t *logger_data;
  supla_voltage_analyzers voltage_analyzers;

  void db_set_properties(channel_json_config *config);
  void db_set_params(int param1, int param2, int param3, int param4);
  supla_channel_value *_get_value(void);
  bool is_convertible2extended(void);
  void on_value_changed(supla_channel_value *old_value,
                        supla_channel_value *new_value, bool significant_change,
                        bool convertible2extended);
  void on_extended_value_changed(supla_channel_extended_value *old_value,
                                 supla_channel_extended_value *new_value);
  supla_channel_extended_value *_get_extended_value(
      bool for_data_logger_purposes);
  template <typename jsonT, typename sdT>
  void json_to_config(TSD_ChannelConfig *config);

 public:
  supla_device_channel(supla_device *device, int id, int number, int type,
                       int func, int param1, int param2, int param3, int param4,
                       const char *text_param1, const char *text_param2,
                       const char *text_param3, bool hidden, unsigned int flags,
                       const char value[SUPLA_CHANNELVALUE_SIZE],
                       unsigned _supla_int_t validity_time_sec,
                       const char *user_config, const char *properties);
  virtual ~supla_device_channel();

  static void get_defaults(int type, int func, int *param1, int *param2);
  static int func_list_filter(int func_list, int type);
  static void get_parent_channel_id(int func, int param1, int param2,
                                    int param4, int *parent1, int *parent2);

  void lock(void);
  void unlock(void);
  int get_id(void);
  int get_number(void);
  int get_user_id(void);
  supla_user *get_user();
  supla_device *get_device();
  int get_func(void);
  void set_func(int func);
  int get_type(void);
  int get_param1(void);
  int get_param2(void);
  int get_param3(void);
  int get_param4(void);
  const char *get_text_param1(void);
  const char *get_text_param2(void);
  const char *get_text_param3(void);
  bool is_hidden(void);
  unsigned int get_flags();
  void add_flags(unsigned int flags);
  bool is_offline(void);
  bool set_offline(bool Offline);
  bool is_value_writable(void);
  bool is_char_value_writable(void);
  bool is_rgbw_value_writable(void);
  unsigned int get_value_duration(void);
  unsigned _supla_int_t get_value_validity_time_sec(void);
  void get_value(char value[SUPLA_CHANNELVALUE_SIZE]);
  bool set_value(const char value[SUPLA_CHANNELVALUE_SIZE],
                 const unsigned _supla_int_t *validity_time_sec, bool *offline);
  void set_extended_value(TSuplaChannelExtendedValue *ev);
  void assign_rgbw_value(char value[SUPLA_CHANNELVALUE_SIZE], int color,
                         char color_brightness, char brightness, char on_off);
  void get_double(double *value);
  void get_char(char *value);
  void get_config(TSD_ChannelConfig *config, unsigned char config_type,
                  unsigned _supla_int_t flags);
  void set_action_trigger_config(unsigned int capabilities,
                                 int related_channel_id,
                                 unsigned int disables_local_operation);

  std::list<int> master_channel(void);
  std::list<int> related_channel(void);
  channel_json_config *get_json_config(void);
  unsigned int get_value_validity_time_left_msec(void);
  void set_state(TDSC_ChannelState *state);
  bool get_state(TDSC_ChannelState *state);
  bool get_voltage_analyzers_with_any_sample_over_threshold(
      supla_voltage_analyzers *voltage_analyzers, bool reset);
  int set_user_config(unsigned char config_type,
                      unsigned _supla_int16_t config_size, char *config);
  void send_config_to_device(unsigned char config_type);
  void send_config_to_device(void);

  template <typename T>
  T *get_value(void);

  template <typename T>
  T *get_extended_value(bool for_data_logger_purposes);
};

template <typename T>
T *supla_device_channel::get_value(void) {
  supla_channel_value *value = _get_value();
  if (value) {
    T *expected = dynamic_cast<T *>(value);
    if (!expected) {
      delete value;
    }
    return expected;
  }
  return nullptr;
}

template <typename T>
T *supla_device_channel::get_extended_value(bool for_data_logger_purposes) {
  supla_channel_extended_value *value =
      _get_extended_value(for_data_logger_purposes);
  if (value) {
    T *expected = dynamic_cast<T *>(value);
    if (!expected) {
      delete value;
    }
    return expected;
  }
  return nullptr;
}

#endif /* DEVICECHANNEL_H_ */
