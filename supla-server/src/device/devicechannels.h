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

#ifndef DEVICECHANNELS_H_
#define DEVICECHANNELS_H_

#include <list>
#include <map>
#include <vector>

#include "actions/action_hvac_parameters.h"
#include "device/abstract_device_dao.h"
#include "device/channel_fragment.h"
#include "device/devicechannel.h"
#include "device/extended_value/channel_extended_value_envelope.h"
#include "device/value/channel_value_envelope.h"

class supla_device;
class supla_device_channels {
 private:
  std::vector<supla_device_channel *> channels;
  supla_device *device;

  void *get_srpc(void);

  supla_device_channel *find_channel(int id);
  supla_device_channel *find_channel_by_number(int number);

  void async_set_channel_value(supla_device_channel *channel,
                               const supla_caller &caller, int group_id,
                               unsigned char eol,
                               const char value[SUPLA_CHANNELVALUE_SIZE],
                               unsigned int durationMS,
                               bool cancel_tasks = true);
  void async_set_channel_value(supla_device_channel *channel,
                               const supla_caller &caller, int group_id,
                               unsigned char eol,
                               const char value[SUPLA_CHANNELVALUE_SIZE],
                               bool cancel_tasks = true);
  bool set_device_channel_char_value(const supla_caller &caller,
                                     supla_device_channel *channel,
                                     int group_id, unsigned char eol,
                                     const char value,
                                     bool cancel_tasks = true);
  bool set_on(const supla_caller &caller, int channel_id, int group_id,
              unsigned char eol, bool on, bool toggle);
  bool rs_action(const supla_caller &caller, int channel_id, int group_id,
                 unsigned char eol, rsAction action,
                 const char *closing_percentage, bool delta);
  bool action_open_close(const supla_caller &caller, int channel_id,
                         int group_id, unsigned char eol, bool unknown,
                         bool open, bool cancel_tasks = true);

 public:
  explicit supla_device_channels(supla_abstract_device_dao *dao,
                                 supla_device *device,
                                 TDS_SuplaDeviceChannel_B *schannel_b,
                                 TDS_SuplaDeviceChannel_C *schannel_c,
                                 int channel_count);
  virtual ~supla_device_channels();

  void access_channel(int channel_id,
                      std::function<void(supla_device_channel *)> on_channel);

  void for_each(std::function<void(supla_device_channel *, bool *)> on_channel);

  bool get_channel_value(int channel_id, char value[SUPLA_CHANNELVALUE_SIZE],
                         char *online, unsigned _supla_int_t *validity_time_sec,
                         supla_channel_extended_value **extended_value,
                         int *function, bool for_client);
  bool get_channel_double_value(int channel_id, double *value);
  bool get_channel_char_value(int channel_id, char *value);

  unsigned int get_channel_value_duration(int channel_id);
  int get_channel_func(int channel_id);
  int get_channel_type(int channel_id);
  bool set_channel_value(int channel_id, char value[SUPLA_CHANNELVALUE_SIZE],
                         const unsigned _supla_int_t *validity_time_sec,
                         bool *offline);
  bool set_channel_offlineee(int channel_id, bool offline);
  void set_channel_extendedvalue(int channel_id,
                                 TSuplaChannelExtendedValue *ev);

  void on_device_registered(supla_user *user, int device_id,
                            TDS_SuplaDeviceChannel_B *schannel_b,
                            TDS_SuplaDeviceChannel_C *schannel_c, int count);

  void set_device_channel_value(const supla_caller &caller, int channel_id,
                                int group_id, unsigned char eol,
                                const char value[SUPLA_CHANNELVALUE_SIZE]);
  bool set_device_channel_char_value(const supla_caller &caller, int channel_id,
                                     int group_id, unsigned char eol,
                                     const char value);
  bool set_device_channel_rgbw_value(const supla_caller &caller, int channel_id,
                                     int group_id, unsigned char eol, int color,
                                     char color_brightness, char brightness,
                                     char on_off);

  bool get_dgf_transparency(int channel_id, unsigned short *mask);

  std::vector<supla_channel_relation> get_channel_relations(
      int channel_id, e_relation_kind kind);
  std::list<int> get_all_ids(void);
  int get_channel_id(unsigned char channel_number);
  bool channel_exists(int channel_id);
  bool is_channel_online(int channel_id);

  bool calcfg_request(const supla_caller &caller, int channel_id,
                      bool superuser_authorized,
                      TCS_DeviceCalCfgRequest_B *request);

  void set_channel_state(int channel_id, TDSC_ChannelState *state);

  bool get_channel_state(int channel_id, TDSC_ChannelState *state);

  bool get_channel_state_async(const supla_caller &caller,
                               TCSD_ChannelStateRequest *request);

  void set_channel_function(int channel_id, int func);
  std::map<int, int> get_functions(void);
  bool get_channel_config(unsigned char channel_number, unsigned char type,
                          unsigned _supla_int_t flags,
                          TSD_ChannelConfig *config);
  void action_trigger(TDS_ActionTrigger *at);

  bool set_on(const supla_caller &caller, int channel_id, int group_id,
              unsigned char eol, bool on);
  bool is_on(int channel_id);
  bool set_rgbw(const supla_caller &caller, int channel_id, int group_id,
                unsigned char eol, unsigned int *color, char *color_brightness,
                char *brightness, char *on_off);
  bool set_color(const supla_caller &caller, int channel_id, int group_id,
                 unsigned char eol, unsigned int color);
  bool set_color_brightness(const supla_caller &caller, int channel_id,
                            int group_id, unsigned char eol, char brightness);
  bool set_brightness(const supla_caller &caller, int channel_id, int group_id,
                      unsigned char eol, char brightness);
  bool set_dgf_transparency(const supla_caller &caller, int channel_id,
                            unsigned short activeBits, unsigned short mask);
  bool get_relay_value(int channel_id, TRelayChannel_Value *relay_value);
  bool action_toggle(const supla_caller &caller, int channel_id, int group_id,
                     unsigned char eol);
  bool action_shut(const supla_caller &caller, int channel_id, int group_id,
                   unsigned char eol, const char *closing_percentage,
                   bool action);
  bool action_reveal(const supla_caller &caller, int channel_id, int group_id,
                     unsigned char eol);
  bool action_up(const supla_caller &caller, int channel_id, int group_id,
                 unsigned char eol);
  bool action_down(const supla_caller &caller, int channel_id, int group_id,
                   unsigned char eol);
  bool action_up_or_stop(const supla_caller &caller, int channel_id,
                         int group_id, unsigned char eol);
  bool action_down_or_stop(const supla_caller &caller, int channel_id,
                           int group_id, unsigned char eol);
  bool action_step_by_step(const supla_caller &caller, int channel_id,
                           int group_id, unsigned char eol);
  bool action_stop(const supla_caller &caller, int channel_id, int group_id,
                   unsigned char eol);
  bool action_open(const supla_caller &caller, int channel_id, int group_id,
                   unsigned char eol);
  bool action_close(const supla_caller &caller, int channel_id, int group_id,
                    unsigned char eol);
  bool action_close(const supla_caller &caller, int channel_id);
  bool action_open_close(const supla_caller &caller, int channel_id,
                         int group_id, unsigned char eol);
  bool action_open_close_without_canceling_tasks(const supla_caller &caller,
                                                 int channel_id, int group_id,
                                                 unsigned char eol);
  bool action_set_hvac_parameters(const supla_caller &caller, int channel_id,
                                  int group_id, unsigned char eol,
                                  supla_action_hvac_parameters *params);
  bool reset_counters(int channel_id);
  bool recalibrate(int channel_id, const supla_caller &caller,
                   bool superuser_authorized);
  void timer_arm(const supla_caller &caller, int channel_id, int group_id,
                 unsigned char eol, unsigned char On, unsigned int duration_ms);
  channel_json_config *get_json_config(int channel_id);
  unsigned int get_value_validity_time_left_msec(void);
  void on_related_sensor_value_changed(int control_channel_id, int sensor_id,
                                       bool is_open);

  void get_channel_values(std::vector<supla_channel_value_envelope *> *result,
                          std::function<bool(supla_channel_value *)> filter);

  void send_configs_to_device(void);

  void get_channel_extended_values(
      std::vector<supla_channel_extended_value_envelope *> *result,
      std::function<bool(supla_channel_extended_value *)> filter,
      bool for_data_logger_purposes);

  std::vector<supla_channel_fragment> get_fragments(void);

  template <typename T>
  T *get_channel_value(int channel_id);
};

template <typename T>
T *supla_device_channels::get_channel_value(int channel_id) {
  supla_device_channel *channel = find_channel(channel_id);

  if (channel) {
    return channel->get_value<T>();
  }
  return nullptr;
}

#endif /* DEVICECHANNELS_H_ */
