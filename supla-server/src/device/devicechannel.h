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

#include "analyzer/abstract_data_analyzer.h"
#include "caller.h"
#include "channel_address.h"
#include "channel_availability_status.h"
#include "device/abstract_common_channel_properties.h"
#include "device/channel_state.h"
#include "device/extended_value/channel_extended_value.h"
#include "device/value/channel_temphum_value.h"
#include "device/value/channel_value.h"
#include "proto.h"

class supla_user;
class supla_device;

class supla_device_channel : public supla_abstract_common_channel_properties {
 private:
  void *lck;
  supla_device *device;
  const int id;
  const unsigned char channel_number;
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
  supla_channel_availability_status availability_status;
  unsigned _supla_int64_t flags;
  unsigned _supla_int64_t init_flags;
  supla_channel_state *state;
  char value[SUPLA_CHANNELVALUE_SIZE];
  struct timeval value_valid_to;  // during offline
  supla_channel_extended_value *extended_value;
  supla_json_config *json_config;
  supla_channel_extended_value *logger_purpose_extended_value;
  supla_abstract_data_analyzer *data_analyzer;

  void db_set_properties(supla_json_config *config);
  void db_set_params(int param1, int param2, int param3, int param4);
  supla_channel_value *_get_value(void);

  void on_value_changed(supla_channel_value *old_value,
                        supla_channel_value *new_value, bool significant_change,
                        bool convertible2extended);
  void on_extended_value_changed(supla_channel_extended_value *old_value,
                                 supla_channel_extended_value *new_value);
  supla_channel_extended_value *_get_extended_value(
      bool for_data_logger_purposes);

  void set_extended_value(TSuplaChannelExtendedValue *ev,
                          supla_channel_extended_value *new_value);
  virtual void for_each(
      bool any_device,
      std::function<void(supla_abstract_common_channel_properties *, bool *)>
          on_channel_properties);

  virtual unsigned char get_protocol_version(void);

 public:
  supla_device_channel(supla_device *device, int id,
                       unsigned char channel_number, int type, int func,
                       int param1, int param2, int param3, int param4,
                       const char *text_param1, const char *text_param2,
                       const char *text_param3, bool hidden,
                       unsigned _supla_int64_t flags,
                       const char value[SUPLA_CHANNELVALUE_SIZE],
                       unsigned _supla_int_t validity_time_sec,
                       supla_channel_extended_value *extended_value,
                       const char *user_config, const char *properties,
                       supla_channel_state *state);
  virtual ~supla_device_channel();

  static void get_defaults(int type, int func, int *param1, int *param2);
  static void trim_alt_icon_index(int func, unsigned char *alt_icon);
  static int func_list_filter(int func_list, int type);

  void lock(void);
  void unlock(void);
  int get_id(void);
  virtual int get_device_id(void);
  virtual unsigned char get_channel_number(void);
  virtual int get_user_id(void);
  supla_user *get_user();
  supla_device *get_device();
  int get_func(void);
  void set_func(int func);
  virtual int get_type(void);
  virtual int get_param1(void);
  virtual int get_param2(void);
  virtual int get_param3(void);
  virtual int get_param4(void);
  const char *get_text_param1(void);
  const char *get_text_param2(void);
  const char *get_text_param3(void);
  bool is_hidden(void);
  virtual unsigned _supla_int64_t get_flags();
  void add_init_flags(unsigned _supla_int64_t flags);
  supla_channel_availability_status get_availability_status(void);
  bool set_availability_status(const supla_channel_availability_status &status,
                               bool raise_change_event);
  bool is_value_writable(void);
  bool is_char_value_writable(void);
  bool is_rgbw_value_writable(void);
  unsigned int get_value_duration(void);
  unsigned _supla_int_t get_value_validity_time_sec(void);
  void get_value(char value[SUPLA_CHANNELVALUE_SIZE]);
  bool set_value(const char value[SUPLA_CHANNELVALUE_SIZE],
                 const unsigned _supla_int_t *validity_time_sec,
                 supla_channel_availability_status *status);
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

  virtual supla_json_config *get_json_config(void);
  void set_json_config(supla_json_config *json_config);
  unsigned int get_value_validity_time_left_msec(void);
  void set_state(TDSC_ChannelState *state);
  bool get_state(TDSC_ChannelState *state);
  void access_data_analyzer(
      std::function<void(supla_abstract_data_analyzer *analyzer)>
          on_data_analyzer);
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
