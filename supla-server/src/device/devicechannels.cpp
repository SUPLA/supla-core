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

#include "devicechannels.h"

#include <math.h>
#include <string.h>

#include <memory>

#include "actions/action_gate_openclose.h"
#include "actions/action_trigger.h"
#include "db/database.h"
#include "device.h"
#include "device/channel_property_getter.h"
#include "device/extended_value/channel_hp_thermostat_ev_decorator.h"
#include "device/extended_value/channel_ic_extended_value.h"
#include "device/value/channel_hvac_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_valve_value.h"
#include "jsonconfig/channel/controlling_the_gate_config.h"
#include "jsonconfig/channel/hvac_config.h"
#include "jsonconfig/channel/roller_shutter_config.h"
#include "log.h"

using std::function;
using std::list;
using std::map;
using std::shared_ptr;
using std::vector;

supla_device_channels::supla_device_channels(
    supla_abstract_device_dao *dao, supla_device *device,
    TDS_SuplaDeviceChannel_B *schannel_b, TDS_SuplaDeviceChannel_E *schannel_e,
    int channel_count) {
  this->device = device;

  channels = dao->get_channels(device);

  for (int a = 0; a < channel_count; a++) {
    int type = 0;
    char *value = nullptr;
    unsigned char number = 0;
    unsigned int action_trigger_caps = 0;
    unsigned int flags = 0;
    supla_channel_availability_status status;

    TActionTriggerProperties at_orops = {};

    if (schannel_b != nullptr) {
      type = schannel_b[a].Type;
      value = schannel_b[a].value;
      number = schannel_b[a].Number;
    } else {
      type = schannel_e[a].Type;
      value = schannel_e[a].value;
      number = schannel_e[a].Number;
      action_trigger_caps = schannel_e[a].ActionTriggerCaps;
      at_orops = schannel_e[a].actionTriggerProperties;
      flags = schannel_e[a].Flags;
      status.set_proto_offline(schannel_e[a].Offline);
    }

    int channel_id = get_channel_id(number);

    supla_device_channel *channel = find_channel(channel_id);

    if (channel) {
      if (status.is_offline()) {
        channel->set_availability_status(status, false);
      } else {
        channel->set_value(value,
                           schannel_b == nullptr
                               ? &schannel_e[a].ValueValidityTimeSec
                               : nullptr,
                           schannel_b == nullptr ? &status : nullptr);
      }

      channel->add_init_flags(flags);

      if (type == SUPLA_CHANNELTYPE_ACTIONTRIGGER) {
        int actionTriggerRelatedChannelId = 0;
        if (at_orops.relatedChannelNumber > 0) {
          actionTriggerRelatedChannelId =
              get_channel_id(at_orops.relatedChannelNumber - 1);
        }
        channel->set_action_trigger_config(action_trigger_caps,
                                           actionTriggerRelatedChannelId,
                                           at_orops.disablesLocalOperation);
      }
    }
  }
}

supla_device_channels::~supla_device_channels() {
  for (auto it = channels.begin(); it != channels.end(); ++it) {
    delete *it;
  }
}

void *supla_device_channels::get_srpc(void) {
  if (device && device->get_connection()) {
    return device->get_connection()->get_srpc_adapter()->get_srpc();
  }

  return nullptr;
}

supla_device_channel *supla_device_channels::find_channel(int id) {
  if (id) {
    for (auto it = channels.begin(); it != channels.end(); ++it) {
      if ((*it)->get_id() == id) {
        return *it;
      }
    }
  }

  return nullptr;
}

supla_device_channel *supla_device_channels::find_channel_by_number(
    int number) {
  for (auto it = channels.begin(); it != channels.end(); ++it) {
    if ((*it)->get_channel_number() == number) {
      return *it;
    }
  }
  return nullptr;
}

void supla_device_channels::access_channel(
    int channel_id, function<void(supla_device_channel *)> on_channel) {
  supla_device_channel *channel = find_channel(channel_id);

  if (channel) {
    on_channel(channel);
  }
}

void supla_device_channels::for_each(
    function<void(supla_device_channel *, bool *)> on_channel) {
  for (auto it = channels.begin(); it != channels.end(); ++it) {
    bool will_continue = true;
    on_channel(*it, &will_continue);
    if (!will_continue) {
      break;
    }
  }
}

bool supla_device_channels::get_channel_value(
    int channel_id, char value[SUPLA_CHANNELVALUE_SIZE],
    supla_channel_availability_status *status,
    unsigned _supla_int_t *validity_time_sec,
    supla_channel_extended_value **extended_value, int *function,
    bool for_client) {
  supla_device_channel *channel = find_channel(channel_id);

  if (channel) {
    channel->get_value(value);
    if (status) {
      *status = channel->get_availability_status();
    }

    if (validity_time_sec) {
      *validity_time_sec = channel->get_value_validity_time_sec();
    }

    if (function) {
      *function = channel->get_func();
    }

    if (extended_value) {
      *extended_value =
          channel->get_extended_value<supla_channel_extended_value>(false);
    }

    if (for_client) {
      switch (channel->get_func()) {
        case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
        case SUPLA_CHANNELFNC_IC_GAS_METER:
        case SUPLA_CHANNELFNC_IC_WATER_METER:
        case SUPLA_CHANNELFNC_IC_HEAT_METER: {
          memset(value, 0, SUPLA_CHANNELVALUE_SIZE);

          supla_channel_ic_extended_value *icv =
              channel->get_extended_value<supla_channel_ic_extended_value>(
                  false);
          if (icv) {
            TSC_ImpulseCounter_Value sc;
            sc.calculated_value = icv->get_calculated_value();
            memcpy(value, &sc, sizeof(TSC_ImpulseCounter_Value));
            delete icv;
          }

          break;
        }
      }
    }

    return true;
  }

  return false;
}

bool supla_device_channels::get_channel_double_value(int channel_id,
                                                     double *value) {
  supla_device_channel *channel = find_channel(channel_id);
  if (channel) {
    channel->get_double(value);
    return true;
  }

  return false;
}

bool supla_device_channels::get_channel_char_value(int channel_id,
                                                   char *value) {
  supla_device_channel *channel = find_channel(channel_id);

  if (channel) {
    channel->get_char(value);
    return true;
  }

  return false;
}

bool supla_device_channels::get_dgf_transparency(int channel_id,
                                                 unsigned short *mask) {
  if (mask) {
    supla_device_channel *channel = find_channel(channel_id);

    if (channel &&
        (channel->get_func() == SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL ||
         channel->get_func() == SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL)) {
      char value[SUPLA_CHANNELVALUE_SIZE];
      channel->get_value(value);
      TDigiglass_Value *dgf_val = (TDigiglass_Value *)value;
      *mask = dgf_val->mask;
      return true;
    }
  }

  return false;
}

bool supla_device_channels::get_relay_value(int channel_id,
                                            TRelayChannel_Value *relay_value) {
  if (relay_value) {
    supla_device_channel *channel = find_channel(channel_id);

    if (channel && (channel->get_func() == SUPLA_CHANNELFNC_POWERSWITCH ||
                    channel->get_func() == SUPLA_CHANNELFNC_LIGHTSWITCH ||
                    channel->get_func() == SUPLA_CHANNELFNC_STAIRCASETIMER)) {
      char value[SUPLA_CHANNELVALUE_SIZE];
      channel->get_value(value);
      memcpy(relay_value, value, sizeof(TRelayChannel_Value));
      return true;
    }
  }

  return false;
}

bool supla_device_channels::calcfg_cmd(int channel_id,
                                       unsigned _supla_int64_t flag,
                                       bool superuser_authorized,
                                       _supla_int_t cmd) {
  supla_device_channel *channel = find_channel(channel_id);

  if (channel && ((!flag || (channel->get_flags() & flag)))) {
    TSD_DeviceCalCfgRequest request = {};

    request.ChannelNumber = channel->get_channel_number();
    request.Command = cmd;
    request.SuperUserAuthorized = superuser_authorized;

    srpc_sd_async_device_calcfg_request(get_srpc(), &request);
    return true;
  }

  return false;
}

bool supla_device_channels::reset_counters(int channel_id) {
  return calcfg_cmd(channel_id, SUPLA_CHANNEL_FLAG_CALCFG_RESET_COUNTERS, true,
                    SUPLA_CALCFG_CMD_RESET_COUNTERS);
}

bool supla_device_channels::take_ocr_photo(int channel_id) {
  return calcfg_cmd(channel_id, 0, true, SUPLA_CALCFG_CMD_TAKE_OCR_PHOTO);
}

bool supla_device_channels::restart_subdevice(int channel_id) {
  return calcfg_cmd(channel_id, SUPLA_CHANNEL_FLAG_CALCFG_RESTART_SUBDEVICE,
                    true, SUPLA_CALCFG_CMD_RESTART_SUBDEVICE);
}

bool supla_device_channels::identify_subdevice(int channel_id) {
  return calcfg_cmd(channel_id, SUPLA_CHANNEL_FLAG_CALCFG_IDENTIFY_SUBDEVICE,
                    true, SUPLA_CALCFG_CMD_IDENTIFY_SUBDEVICE);
}

bool supla_device_channels::recalibrate(int channel_id,
                                        const supla_caller &caller,
                                        bool superuser_authorized) {
  supla_device_channel *channel = find_channel(channel_id);

  if (channel &&
      (channel->get_flags() & SUPLA_CHANNEL_FLAG_CALCFG_RECALIBRATE)) {
    TSD_DeviceCalCfgRequest request = {};

    request.ChannelNumber = channel->get_channel_number();
    request.Command = SUPLA_CALCFG_CMD_RECALIBRATE;
    request.SenderID = caller.convert_to_sender_id();
    request.SuperUserAuthorized = superuser_authorized;

    int func = channel->get_func();

    if (func == SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER ||
        func == SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW ||
        func == SUPLA_CHANNELFNC_TERRACE_AWNING ||
        func == SUPLA_CHANNELFNC_PROJECTOR_SCREEN ||
        func == SUPLA_CHANNELFNC_CURTAIN ||
        func == SUPLA_CHANNELFNC_ROLLER_GARAGE_DOOR) {
      TCalCfg_RollerShutterSettings *settings =
          (TCalCfg_RollerShutterSettings *)request.Data;

      request.DataSize = sizeof(TCalCfg_RollerShutterSettings);
      request.DataType = SUPLA_CALCFG_DATATYPE_RS_SETTINGS;

      supla_json_config *config = channel->get_json_config();
      if (config) {
        roller_shutter_config rs_cfg(config);
        TChannelConfig_RollerShutter raw_cfg = {};
        rs_cfg.get_config(&raw_cfg);
        settings->FullOpeningTimeMS = raw_cfg.OpeningTimeMS;
        settings->FullClosingTimeMS = raw_cfg.ClosingTimeMS;
        delete config;
      }
    }

    srpc_sd_async_device_calcfg_request(get_srpc(), &request);
    return true;
  }

  return false;
}

bool supla_device_channels::set_channel_value(
    int channel_id, char value[SUPLA_CHANNELVALUE_SIZE],
    const unsigned _supla_int_t *validity_time_sec,
    supla_channel_availability_status *status) {
  supla_device_channel *channel = find_channel(channel_id);
  if (channel) {
    return channel->set_value(value, validity_time_sec, status);
  }

  return false;
}

bool supla_device_channels::set_channel_availability_status(
    int channel_id, const supla_channel_availability_status &status) {
  bool result = false;
  access_channel(channel_id,
                 [&result, status](supla_device_channel *channel) -> void {
                   result = channel->set_availability_status(status, true);
                 });

  return result;
}

void supla_device_channels::set_channel_extendedvalue(
    int channel_id, TSuplaChannelExtendedValue *ev) {
  supla_device_channel *channel = find_channel(channel_id);
  if (channel) {
    channel->set_extended_value(ev);
  }
}

unsigned int supla_device_channels::get_channel_value_duration(int channel_id) {
  supla_device_channel *channel = find_channel(channel_id);

  if (channel) {
    return channel->get_value_duration();
  }

  return 0;
}

int supla_device_channels::get_channel_func(int channel_id) {
  supla_device_channel *channel = find_channel(channel_id);

  if (channel) {
    return channel->get_func();
  }

  return 0;
}

int supla_device_channels::get_channel_type(int channel_id) {
  supla_device_channel *channel = find_channel(channel_id);

  if (channel) {
    return channel->get_type();
  }

  return 0;
}

vector<supla_channel_relation> supla_device_channels::get_channel_relations(
    int channel_id, e_relation_kind kind) {
  vector<supla_channel_relation> result;

  supla_device_channel *channel = find_channel(channel_id);

  if (channel) {
    result = channel->get_channel_relations(kind);
  }

  return result;
}

bool supla_device_channels::channel_exists(int channel_id) {
  return find_channel(channel_id) != nullptr;
}

supla_channel_availability_status
supla_device_channels::get_channel_availability_status(int channel_id) {
  supla_channel_availability_status result(true);

  access_channel(channel_id, [&result](supla_device_channel *channel) -> void {
    result = channel->get_availability_status();
  });
  return result;
}

void supla_device_channels::on_device_registered(
    supla_user *user, int device_id, TDS_SuplaDeviceChannel_B *schannel_b,
    TDS_SuplaDeviceChannel_E *schannel_e, int count) {
  int channel_id = 0;

  for (int a = 0; a < count; a++) {
    if (schannel_b != nullptr) {
      channel_id = get_channel_id(schannel_b[a].Number);
    } else {
      channel_id = get_channel_id(schannel_e[a].Number);
    }

    if (channel_id > 0) {
      user->on_channel_become_online(device_id, channel_id);
    }
  }
}

int supla_device_channels::get_channel_id(unsigned char channel_number) {
  supla_device_channel *channel = find_channel_by_number(channel_number);

  if (channel) {
    return channel->get_id();
  }

  return 0;
}

void supla_device_channels::async_set_channel_value(
    supla_device_channel *channel, const supla_caller &caller, int group_id,
    unsigned char eol, const char value[SUPLA_CHANNELVALUE_SIZE],
    unsigned int duration_ms, bool cancel_tasks) {
  if (cancel_tasks) {
    switch (channel->get_func()) {
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
        supla_action_gate_openclose::cancel_tasks(
            device->get_user_id(), device->get_id(), channel->get_id());
        break;
    }
  }

  if (group_id && srpc_get_proto_version(get_srpc()) >= 13) {
    TSD_SuplaChannelGroupNewValue s;
    memset(&s, 0, sizeof(TSD_SuplaChannelGroupNewValue));

    s.ChannelNumber = channel->get_channel_number();
    s.DurationMS = duration_ms;
    s.SenderID = caller.convert_to_sender_id();
    s.GroupID = group_id;
    s.EOL = eol;
    memcpy(s.value, value, SUPLA_CHANNELVALUE_SIZE);

    srpc_sd_async_set_channelgroup_value(get_srpc(), &s);
  } else {
    TSD_SuplaChannelNewValue s;
    memset(&s, 0, sizeof(TSD_SuplaChannelNewValue));

    s.ChannelNumber = channel->get_channel_number();
    s.DurationMS = duration_ms;
    s.SenderID = group_id ? 0 : caller.convert_to_sender_id();
    memcpy(s.value, value, SUPLA_CHANNELVALUE_SIZE);

    srpc_sd_async_set_channel_value(get_srpc(), &s);
  }
}

void supla_device_channels::async_set_channel_value(
    supla_device_channel *channel, const supla_caller &caller, int group_id,
    unsigned char eol, const char value[SUPLA_CHANNELVALUE_SIZE],
    bool cancel_tasks) {
  async_set_channel_value(channel, caller, group_id, eol, value,
                          channel->get_value_duration(), cancel_tasks);
}

void supla_device_channels::set_device_channel_value(
    const supla_caller &caller, int channel_id, int group_id, unsigned char eol,
    const char value[SUPLA_CHANNELVALUE_SIZE]) {
  supla_device_channel *channel = find_channel(channel_id);

  if (channel && channel->is_value_writable()) {
    async_set_channel_value(channel, caller, group_id, eol, value);
  }
}

bool supla_device_channels::set_device_channel_char_value(
    const supla_caller &caller, supla_device_channel *channel, int group_id,
    unsigned char eol, const char value, bool cancel_tasks) {
  bool result = false;

  if (channel) {
    if (channel->is_rgbw_value_writable()) {
      int color = 0;
      char color_brightness = 0;
      char brightness = 0;
      char on_off = 0;

      supla_channel_rgbw_value *rgbw_value =
          channel->get_value<supla_channel_rgbw_value>();
      if (rgbw_value) {
        rgbw_value->get_rgbw(&color, &color_brightness, &brightness);
        delete rgbw_value;

        if (value > 0) {
          color_brightness = 100;
          brightness = 100;
        } else {
          color_brightness = 0;
          brightness = 0;
        }

        on_off = RGBW_BRIGHTNESS_ONOFF | RGBW_COLOR_ONOFF;

        result = set_device_channel_rgbw_value(
            caller, channel->get_id(), group_id, eol, color, color_brightness,
            brightness, on_off);
      }

    } else if (channel->is_char_value_writable()) {
      char v[SUPLA_CHANNELVALUE_SIZE] = {};
      channel->get_value(v);
      v[0] = value;

      async_set_channel_value(channel, caller, group_id, eol, v, cancel_tasks);

      result = true;
    }
  }

  return result;
}

bool supla_device_channels::set_device_channel_char_value(
    const supla_caller &caller, int channel_id, int group_id, unsigned char eol,
    const char value) {
  return set_device_channel_char_value(caller, find_channel(channel_id),
                                       group_id, eol, value);
}

bool supla_device_channels::set_device_channel_rgbw_value(
    const supla_caller &caller, int channel_id, int group_id, unsigned char eol,
    int color, char color_brightness, char brightness, char on_off) {
  supla_device_channel *channel = find_channel(channel_id);

  if (channel && channel->is_rgbw_value_writable()) {
    char v[SUPLA_CHANNELVALUE_SIZE];
    memset(v, 0, SUPLA_CHANNELVALUE_SIZE);

    if (on_off) {
      char mask = 0;
      if (channel->get_func() == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
        mask = RGBW_COLOR_ONOFF | RGBW_BRIGHTNESS_ONOFF;
      } else if (channel->get_func() == SUPLA_CHANNELFNC_DIMMER) {
        mask = RGBW_BRIGHTNESS_ONOFF;
      } else if (channel->get_func() == SUPLA_CHANNELFNC_RGBLIGHTING) {
        mask = RGBW_COLOR_ONOFF;
      }

      on_off = mask & on_off;
    }

    channel->assign_rgbw_value(v, color, color_brightness, brightness, on_off);

    async_set_channel_value(channel, caller, group_id, eol, v);
    return true;
  }

  return false;
}

void supla_device_channels::get_channel_values(
    vector<supla_channel_value_envelope *> *result,
    function<bool(supla_device_channel *channel, supla_channel_value *)>
        filter) {
  for (auto it = channels.begin(); it != channels.end(); ++it) {
    supla_channel_value *value = (*it)->get_value<supla_channel_value>();
    if (value) {
      if (filter(*it, value)) {
        result->push_back(
            new supla_channel_value_envelope((*it)->get_id(), value));
      } else {
        delete value;
      }
    }
  }
}

void supla_device_channels::get_channel_extended_values(
    vector<supla_channel_extended_value_envelope *> *result,
    function<bool(supla_channel_extended_value *)> filter,
    bool for_data_logger_purposes) {
  for (auto it = channels.begin(); it != channels.end(); ++it) {
    supla_channel_extended_value *value =
        (*it)->get_extended_value<supla_channel_extended_value>(
            for_data_logger_purposes);
    if (value) {
      if (filter(value)) {
        result->push_back(
            new supla_channel_extended_value_envelope((*it)->get_id(), value));
      } else {
        delete value;
      }
    }
  }
}

bool supla_device_channels::calcfg_request(const supla_caller &caller,
                                           int channel_id,
                                           bool superuser_authorized,
                                           TCS_DeviceCalCfgRequest_B *request) {
  if (request == nullptr) {
    return false;
  }

  if (request->Command == SUPLA_CALCFG_CMD_RECALIBRATE) {
    return recalibrate(channel_id, caller, superuser_authorized);
  }

  supla_device_channel *channel = find_channel(channel_id);

  if (channel) {
    TSD_DeviceCalCfgRequest drequest;
    memset(&drequest, 0, sizeof(TSD_DeviceCalCfgRequest));

    drequest.SenderID = caller.convert_to_sender_id();
    drequest.ChannelNumber = channel->get_channel_number();
    drequest.Command = request->Command;
    drequest.SuperUserAuthorized = superuser_authorized;
    drequest.DataType = request->DataType;
    drequest.DataSize = request->DataSize > SUPLA_CALCFG_DATA_MAXSIZE
                            ? SUPLA_CALCFG_DATA_MAXSIZE
                            : request->DataSize;
    memcpy(drequest.Data, request->Data, SUPLA_CALCFG_DATA_MAXSIZE);

    srpc_sd_async_device_calcfg_request(get_srpc(), &drequest);
    return true;
  }

  return false;
}

void supla_device_channels::set_channel_state(int channel_id,
                                              TDSC_ChannelState *state) {
  access_channel(channel_id, [state](supla_device_channel *channel) -> void {
    channel->set_state(state);
  });
}

bool supla_device_channels::get_channel_state(int channel_id,
                                              TDSC_ChannelState *state) {
  bool result = false;
  access_channel(channel_id,
                 [state, &result](supla_device_channel *channel) -> void {
                   result = channel->get_state(state);
                 });

  return result;
}

bool supla_device_channels::get_channel_state_async(
    const supla_caller &caller, TCSD_ChannelStateRequest *request) {
  if (request == nullptr) {
    return false;
  }

  supla_device_channel *channel = find_channel(request->ChannelID);

  if (channel) {
    TCSD_ChannelStateRequest drequest;
    memcpy(&drequest, request, sizeof(TCSD_ChannelStateRequest));

    drequest.SenderID = caller.convert_to_sender_id();
    drequest.ChannelNumber = channel->get_channel_number();

    srpc_csd_async_get_channel_state(get_srpc(), &drequest);

    return true;
  }

  return false;
}

list<int> supla_device_channels::get_all_ids(void) {
  list<int> result;

  for (auto it = channels.begin(); it != channels.end(); ++it) {
    result.push_back((*it)->get_id());
  }

  return result;
}

void supla_device_channels::set_channel_function(int channel_id, int func) {
  supla_device_channel *channel = find_channel(channel_id);

  if (channel) {
    channel->set_func(func);
  }
}

map<int, int> supla_device_channels::get_functions(void) {
  map<int, int> result;

  for (auto it = channels.begin(); it != channels.end(); ++it) {
    result.insert({(*it)->get_channel_number(), (*it)->get_func()});
  }

  return result;
}

bool supla_device_channels::get_channel_config(unsigned char channel_number,
                                               unsigned char type,
                                               unsigned _supla_int_t flags,
                                               TSD_ChannelConfig *config) {
  if (config == nullptr) {
    return false;
  }

  supla_device_channel *channel = find_channel_by_number(channel_number);

  if (channel) {
    channel->get_config(config, type, flags);
    return true;
  }

  return false;
}

void supla_device_channels::action_trigger(TDS_ActionTrigger *at) {
  if (!at) {
    return;
  }

  int channel_id = 0;
  int user_id = 0;
  supla_json_config *json_config = nullptr;

  supla_device_channel *channel = find_channel_by_number(at->ChannelNumber);
  if (channel) {
    channel_id = channel->get_id();
    user_id = channel->get_user_id();
    json_config = channel->get_json_config();
  }

  if (channel_id) {
    supla_action_executor *aexec = new supla_action_executor();
    action_trigger_config *at_config = new action_trigger_config(json_config);
    supla_channel_property_getter *property_getter =
        new supla_channel_property_getter();

    if (aexec && at_config && property_getter) {
      at_config->set_channel_id_if_subject_not_set(channel_id);

      supla_action_trigger *trigger =
          new supla_action_trigger(aexec, at_config, property_getter);
      if (trigger) {
        trigger->execute_actions(channel_id, user_id, at->ActionTrigger);
        delete trigger;
      }
    }

    if (property_getter) {
      delete property_getter;
      property_getter = nullptr;
    }

    if (aexec) {
      delete aexec;
      aexec = nullptr;
    }

    if (at_config) {
      delete at_config;
      at_config = nullptr;
    }
  }

  if (json_config) {
    delete json_config;
  }
}

bool supla_device_channels::set_on(const supla_caller &caller, int channel_id,
                                   int group_id, unsigned char eol, bool on,
                                   bool toggle) {
  bool result = false;
  supla_device_channel *channel = find_channel(channel_id);
  if (channel) {
    switch (channel->get_func()) {
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER:
      case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
        char c = on ? 1 : 0;
        if (toggle) {
          channel->get_char(&c);
          c = c ? 0 : 1;
        }
        result =
            set_device_channel_char_value(caller, channel, group_id, eol, c);
        break;
      }

      case SUPLA_CHANNELFNC_DIMMER:
      case SUPLA_CHANNELFNC_RGBLIGHTING:
      case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING: {
        int color = 0;
        char color_brightness = 0;
        char brightness = 0;
        char on_off = 0;

        supla_channel_rgbw_value *rgbw_value =
            channel->get_value<supla_channel_rgbw_value>();
        if (rgbw_value) {
          rgbw_value->get_rgbw(&color, &color_brightness, &brightness);
          delete rgbw_value;

          if (toggle) {
            if (!color_brightness && !brightness) {
              color_brightness = 100;
              brightness = 100;
            } else {
              color_brightness = 0;
              brightness = 0;
            }
          } else {
            color_brightness = on ? 100 : 0;
            brightness = on ? 100 : 0;
          }

          on_off = RGBW_BRIGHTNESS_ONOFF | RGBW_COLOR_ONOFF;

          result = set_device_channel_rgbw_value(
              caller, channel->get_id(), group_id, eol, color, color_brightness,
              brightness, on_off);
        }
        break;
      }

      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
      case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER: {
        supla_channel_hvac_value *hvac_value =
            channel->get_value<supla_channel_hvac_value>();
        if (hvac_value) {
          if (toggle) {
            hvac_value->toggle();
          } else if (on) {
            hvac_value->turn_on();
          } else {
            hvac_value->turn_off();
          }

          char v[SUPLA_CHANNELVALUE_SIZE] = {};
          hvac_value->get_raw_value(v);
          async_set_channel_value(channel, caller, group_id, eol, v, false);
          result = true;

          delete hvac_value;
        }
      } break;
    }
  }

  return result;
}

bool supla_device_channels::is_on(int channel_id) {
  bool result = false;

  supla_device_channel *channel = find_channel(channel_id);
  if (channel) {
    switch (channel->get_func()) {
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER:
      case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
        char c = 0;
        channel->get_char(&c);
        result = c;
        break;
      }

      case SUPLA_CHANNELFNC_DIMMER:
      case SUPLA_CHANNELFNC_RGBLIGHTING:
      case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING: {
        int color = 0;
        char color_brightness = 0;
        char brightness = 0;

        supla_channel_rgbw_value *rgbw_value =
            channel->get_value<supla_channel_rgbw_value>();
        if (rgbw_value) {
          rgbw_value->get_rgbw(&color, &color_brightness, &brightness);
          delete rgbw_value;
          result = color_brightness > 0 || brightness > 0;
        }

        break;
      }
    }
  }

  return result;
}

bool supla_device_channels::set_on(const supla_caller &caller, int channel_id,
                                   int group_id, unsigned char eol, bool on) {
  return set_on(caller, channel_id, group_id, eol, on, false);
}

bool supla_device_channels::set_rgbw(const supla_caller &caller, int channel_id,
                                     int group_id, unsigned char eol,
                                     unsigned int *color,
                                     char *color_brightness, char *brightness,
                                     char *on_off) {
  supla_device_channel *channel = find_channel(channel_id);
  if (channel && channel->is_rgbw_value_writable()) {
    int _color = 0;
    char _color_brightness = 0;
    char _brightness = 0;

    supla_channel_rgbw_value *rgbw_value =
        channel->get_value<supla_channel_rgbw_value>();
    if (rgbw_value) {
      rgbw_value->get_rgbw(&_color, &_color_brightness, &_brightness);
      delete rgbw_value;
      return set_device_channel_rgbw_value(
          caller, channel_id, group_id, eol, color ? *color : _color,
          color_brightness ? *color_brightness : _color_brightness,
          brightness ? *brightness : _brightness, on_off ? *on_off : 0);
    }
  }

  return false;
}

bool supla_device_channels::set_color(const supla_caller &caller,
                                      int channel_id, int group_id,
                                      unsigned char eol, unsigned int color) {
  return set_rgbw(caller, channel_id, group_id, eol, &color, nullptr, nullptr,
                  nullptr);
}

bool supla_device_channels::set_color_brightness(const supla_caller &caller,
                                                 int channel_id, int group_id,
                                                 unsigned char eol,
                                                 char brightness) {
  return set_rgbw(caller, channel_id, group_id, eol, nullptr, &brightness,
                  nullptr, nullptr);
}

bool supla_device_channels::set_brightness(const supla_caller &caller,
                                           int channel_id, int group_id,
                                           unsigned char eol, char brightness) {
  return set_rgbw(caller, channel_id, group_id, eol, nullptr, nullptr,
                  &brightness, nullptr);
}

bool supla_device_channels::set_dgf_transparency(const supla_caller &caller,
                                                 int channel_id,
                                                 unsigned short active_bits,
                                                 unsigned short mask) {
  supla_device_channel *channel = find_channel(channel_id);
  if (channel &&
      (channel->get_func() == SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL ||
       channel->get_func() == SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL)) {
    unsigned short scope = (unsigned short)(pow(2, channel->get_param1()) - 1);
    active_bits &= scope;
    mask &= scope;

    char value[SUPLA_CHANNELVALUE_SIZE];
    memset(value, 0, SUPLA_CHANNELVALUE_SIZE);

    TCSD_Digiglass_NewValue *new_value = (TCSD_Digiglass_NewValue *)value;
    new_value->active_bits = active_bits;
    new_value->mask = mask;
    async_set_channel_value(channel, caller, 0, 0, value);
    return true;
  }

  return false;
}

bool supla_device_channels::action_toggle(const supla_caller &caller,
                                          int channel_id, int group_id,
                                          unsigned char eol) {
  return set_on(caller, channel_id, group_id, eol, false, true);
}

bool supla_device_channels::shading_system_action(
    const supla_caller &caller, int channel_id, int group_id, unsigned char eol,
    int action, const supla_action_shading_system_parameters *params) {
  bool result = false;

  if (!params) {
    return result;
  }

  access_channel(
      channel_id,
      [&result, this, caller, group_id, eol, action,
       params](supla_device_channel *channel) -> void {
        char value[SUPLA_CHANNELVALUE_SIZE] = {};
        channel->get_value(value);

        if (params->apply_on_value(action, value, channel->get_func(),
                                   channel->get_flags())) {
          async_set_channel_value(channel, caller, group_id, eol, value, true);
          result = true;
        }
      });

  return result;
}

bool supla_device_channels::shading_system_action(const supla_caller &caller,
                                                  int channel_id, int group_id,
                                                  unsigned char eol,
                                                  int action) {
  supla_action_shading_system_parameters params;
  return shading_system_action(caller, channel_id, group_id, eol, action,
                               &params);
}

bool supla_device_channels::action_shut(
    const supla_caller &caller, int channel_id, int group_id, unsigned char eol,
    const supla_action_shading_system_parameters *params) {
  supla_action_shading_system_parameters _params;
  _params.set_percentage(100);

  if (params == nullptr) {
    params = &_params;
  }

  return shading_system_action(caller, channel_id, group_id, eol, ACTION_SHUT,
                               params);
}

bool supla_device_channels::action_reveal(const supla_caller &caller,
                                          int channel_id, int group_id,
                                          unsigned char eol) {
  return shading_system_action(caller, channel_id, group_id, eol,
                               ACTION_REVEAL);
}

bool supla_device_channels::action_stop(const supla_caller &caller,
                                        int channel_id, int group_id,
                                        unsigned char eol) {
  return shading_system_action(caller, channel_id, group_id, eol, ACTION_STOP);
}

bool supla_device_channels::action_up(const supla_caller &caller,
                                      int channel_id, int group_id,
                                      unsigned char eol) {
  return shading_system_action(caller, channel_id, group_id, eol, ACTION_UP);
}

bool supla_device_channels::action_down(const supla_caller &caller,
                                        int channel_id, int group_id,
                                        unsigned char eol) {
  return shading_system_action(caller, channel_id, group_id, eol, ACTION_DOWN);
}

bool supla_device_channels::action_up_or_stop(const supla_caller &caller,
                                              int channel_id, int group_id,
                                              unsigned char eol) {
  return shading_system_action(caller, channel_id, group_id, eol,
                               ACTION_UP_OR_STOP);
}

bool supla_device_channels::action_down_or_stop(const supla_caller &caller,
                                                int channel_id, int group_id,
                                                unsigned char eol) {
  return shading_system_action(caller, channel_id, group_id, eol,
                               ACTION_DOWN_OR_STOP);
}

bool supla_device_channels::action_step_by_step(const supla_caller &caller,
                                                int channel_id, int group_id,
                                                unsigned char eol) {
  return shading_system_action(caller, channel_id, group_id, eol,
                               ACTION_STEP_BY_STEP);
}

bool supla_device_channels::action_open_close(const supla_caller &caller,
                                              int channel_id, int group_id,
                                              unsigned char eol, bool unknown,
                                              bool open, bool cancel_tasks) {
  bool result = false;
  supla_device_channel *channel = find_channel(channel_id);
  if (channel && channel->is_char_value_writable()) {
    if (unknown) {
      // You don't know whether you are opening or closing
      switch (channel->get_func()) {
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
          result = set_device_channel_char_value(caller, channel, group_id, eol,
                                                 1, cancel_tasks);
          break;
      }
    } else {
      switch (channel->get_func()) {
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR: {
          supla_action_gate_openclose::open_close(caller, device->get_user_id(),
                                                  device->get_id(),
                                                  channel->get_id(), open);
          result = true;
        } break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
          result = set_device_channel_char_value(caller, channel, group_id, eol,
                                                 1, cancel_tasks);
          break;
        case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
          result = set_device_channel_char_value(caller, channel, group_id, eol,
                                                 open ? 1 : 0, false);
          break;
      }
    }
  }

  return result;
}

bool supla_device_channels::action_open(const supla_caller &caller,
                                        int channel_id, int group_id,
                                        unsigned char eol) {
  return action_open_close(caller, channel_id, group_id, eol, false, true);
}

bool supla_device_channels::action_close(const supla_caller &caller,
                                         int channel_id, int group_id,
                                         unsigned char eol) {
  return action_open_close(caller, channel_id, group_id, eol, false, false);
}

bool supla_device_channels::action_close(const supla_caller &caller,
                                         int channel_id) {
  return action_open_close(caller, channel_id, 0, 0, false, false);
}

bool supla_device_channels::action_open_close(const supla_caller &caller,
                                              int channel_id, int group_id,
                                              unsigned char eol) {
  return action_open_close(caller, channel_id, group_id, eol, true, false);
}

bool supla_device_channels::action_open_close_without_canceling_tasks(
    const supla_caller &caller, int channel_id, int group_id,
    unsigned char eol) {
  return action_open_close(caller, channel_id, group_id, eol, true, false,
                           false);
}

bool supla_device_channels::action_hvac(
    const supla_caller &caller, int channel_id, int group_id, unsigned char eol,
    unsigned int duration,
    function<bool(supla_device_channel *, supla_channel_hvac_value *)>
        on_value) {
  bool result = false;

  access_channel(channel_id, [&](supla_device_channel *channel) -> void {
    supla_channel_hvac_value *hvac_value =
        channel->get_value<supla_channel_hvac_value>();
    if (hvac_value) {
      if (on_value(channel, hvac_value)) {
        char value[SUPLA_CHANNELVALUE_SIZE] = {};
        hvac_value->get_raw_value(value);

        async_set_channel_value(channel, caller, group_id, eol, value, duration,
                                false);
        result = true;
      }
      delete hvac_value;
    }
  });

  return result;
}

bool supla_device_channels::action_hvac_set_parameters(
    const supla_caller &caller, int channel_id, int group_id, unsigned char eol,
    const supla_action_hvac_parameters *params) {
  if (!params) {
    return false;
  }

  bool hp_match = false;
  bool result = hp_action(
      channel_id, &hp_match,
      [params, this](supla_device_channel *channel,
                     TSD_DeviceCalCfgRequest *req) -> bool {
        switch (params->get_mode()) {
          case SUPLA_HVAC_MODE_HEAT:
          case SUPLA_HVAC_MODE_CMD_SWITCH_TO_MANUAL:
            req->Command = SUPLA_THERMOSTAT_CMD_SET_MODE_NORMAL;
            req->Data[0] = 1;
            req->DataSize = 1;
            break;
          case SUPLA_HVAC_MODE_OFF:
            req->Command = SUPLA_THERMOSTAT_CMD_TURNON;
            req->Data[0] = 0;  // 0 == off
            req->DataSize = 1;
            break;
          case SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE:
            req->Command = SUPLA_THERMOSTAT_CMD_SET_MODE_AUTO;
            req->Data[0] = 1;
            req->DataSize = 1;
            break;
        }

        if (params->get_flags() &
            SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET) {
          if (req->Command != SUPLA_THERMOSTAT_CMD_SET_MODE_NORMAL) {
            supla_channel_thermostat_extended_value *th =
                channel->get_extended_value<
                    supla_channel_thermostat_extended_value>(false);
            if (th) {
              supla_channel_hp_thermostat_ev_decorator decorator(th);
              if (decorator.get_state_flags() & HP_STATUS_PROGRAMMODE) {
                // When setting the temperature, force switching to manual
                // mode.
                req->Command = SUPLA_THERMOSTAT_CMD_SET_MODE_NORMAL;
                req->Data[0] = 0;  // Do not force the power on
                req->DataSize = 1;
              }
              delete th;
            }
          }

          if (req->Command && device && device->get_connection()) {
            device->get_connection()
                ->get_srpc_adapter()
                ->sd_async_device_calcfg_request(req);
          }

          req->Command = SUPLA_THERMOSTAT_CMD_SET_TEMPERATURE;
          req->DataSize = sizeof(TThermostatTemperatureCfg);

          TThermostatTemperatureCfg *cfg =
              (TThermostatTemperatureCfg *)req->Data;

          memset(cfg, 0, sizeof(TThermostatTemperatureCfg));
          cfg->Index = TEMPERATURE_INDEX1;
          cfg->Temperature[0] = params->get_setpoint_temperature_heat();
        }

        return req->Command;
      });

  if (hp_match) {
    return result;
  }

  return action_hvac(caller, channel_id, group_id, eol,
                     params->get_duration_sec(),
                     [&](supla_device_channel *channel,
                         supla_channel_hvac_value *value) -> bool {
                       value->clear();
                       params->apply_on(value);
                       return true;
                     });
}

bool supla_device_channels::hp_action(
    int channel_id, bool *function_match,
    function<bool(supla_device_channel *, TSD_DeviceCalCfgRequest *req)>
        on_calcfg) {
  bool result = false;

  access_channel(channel_id,
                 [&function_match, &result, on_calcfg,
                  this](supla_device_channel *channel) -> void {
                   if (channel->get_func() ==
                       SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS) {
                     *function_match = true;

                     TSD_DeviceCalCfgRequest req = {};
                     req.ChannelNumber = channel->get_channel_number();

                     if (device && device->get_connection() &&
                         on_calcfg(channel, &req) &&
                         device->get_connection()
                             ->get_srpc_adapter()
                             ->sd_async_device_calcfg_request(&req)) {
                       result = true;
                     }
                   }
                 });

  return result;
}

bool supla_device_channels::action_hvac_switch_to_manual_mode(
    const supla_caller &caller, int channel_id, int group_id,
    unsigned char eol) {
  bool hp_match = false;
  bool result = hp_action(
      channel_id, &hp_match,
      [](supla_device_channel *channel, TSD_DeviceCalCfgRequest *req) -> bool {
        req->Command = SUPLA_THERMOSTAT_CMD_SET_MODE_NORMAL;
        req->Data[0] = 1;
        req->DataSize = 1;
        return true;
      });

  if (hp_match) {
    return result;
  }

  return action_hvac(caller, channel_id, group_id, eol, 0,
                     [&](supla_device_channel *channel,
                         supla_channel_hvac_value *value) -> bool {
                       value->clear();
                       value->switch_to_manual();
                       return true;
                     });
}

bool supla_device_channels::action_hvac_switch_to_program_mode(
    const supla_caller &caller, int channel_id, int group_id,
    unsigned char eol) {
  bool hp_match = false;
  bool result = hp_action(
      channel_id, &hp_match,
      [](supla_device_channel *channel, TSD_DeviceCalCfgRequest *req) -> bool {
        req->Command = SUPLA_THERMOSTAT_CMD_SET_MODE_AUTO;
        req->Data[0] = 1;
        req->DataSize = 1;
        return true;
      });

  if (hp_match) {
    return result;
  }

  return action_hvac(caller, channel_id, group_id, eol, 0,
                     [&](supla_device_channel *channel,
                         supla_channel_hvac_value *value) -> bool {
                       value->clear();
                       value->switch_to_program();
                       return true;
                     });
}

bool supla_device_channels::action_hvac_set_temperature(
    const supla_caller &caller, int channel_id, int group_id, unsigned char eol,
    const supla_action_hvac_setpoint_temperature *temperature) {
  if (!temperature) {
    return false;
  }

  bool hp_match = false;
  bool result = hp_action(
      channel_id, &hp_match,
      [temperature, this](supla_device_channel *channel,
                          TSD_DeviceCalCfgRequest *req) -> bool {
        req->Command = SUPLA_THERMOSTAT_CMD_SET_TEMPERATURE;
        req->DataSize = sizeof(TThermostatTemperatureCfg);

        TThermostatTemperatureCfg *cfg = (TThermostatTemperatureCfg *)req->Data;

        cfg->Index = TEMPERATURE_INDEX1;
        cfg->Temperature[0] = temperature->get_temperature();

        supla_channel_thermostat_extended_value *th =
            channel
                ->get_extended_value<supla_channel_thermostat_extended_value>(
                    false);
        if (th) {
          supla_channel_hp_thermostat_ev_decorator decorator(th);
          if (decorator.get_state_flags() & HP_STATUS_PROGRAMMODE) {
            // When setting the temperature, force switching to manual mode.
            if (device && device->get_connection() &&
                device->get_connection()
                    ->get_srpc_adapter()
                    ->sd_async_device_calcfg_request(req)) {
              req->Command = SUPLA_THERMOSTAT_CMD_SET_MODE_NORMAL;
              req->Data[0] = 0;  // Do not force the power on
              req->DataSize = 1;
            }
          }
          delete th;
        }

        return true;
      });

  if (hp_match) {
    return result;
  }

  return action_hvac(
      caller, channel_id, group_id, eol, 0,
      [&](supla_device_channel *channel,
          supla_channel_hvac_value *value) -> bool {
        unsigned char mode = value->get_mode();
        value->clear();
        bool result = false;
        switch (channel->get_func()) {
          case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
          case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:
            value->set_setpoint_temperature_heat(
                temperature->get_temperature());
            result = true;
            break;
          case SUPLA_CHANNELFNC_HVAC_THERMOSTAT: {
            supla_json_config *json_config = channel->get_json_config();
            if (json_config) {
              hvac_config hvac(json_config);
              TChannelConfig_HVAC raw = {};
              if (hvac.get_config(&raw, channel->get_channel_number())) {
                if (raw.Subfunction == SUPLA_HVAC_SUBFUNCTION_HEAT) {
                  value->set_setpoint_temperature_heat(
                      temperature->get_temperature());
                  result = true;
                } else if (raw.Subfunction == SUPLA_HVAC_SUBFUNCTION_COOL) {
                  value->set_setpoint_temperature_cool(
                      temperature->get_temperature());
                  result = true;
                }
              }
              delete json_config;
            }
          } break;
          case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL: {
            if (mode == SUPLA_HVAC_MODE_COOL) {
              value->set_setpoint_temperature_cool(
                  temperature->get_temperature());
              result = true;
            } else {
              value->set_setpoint_temperature_heat(
                  temperature->get_temperature());
              result = true;
            }
          } break;
        }

        return result;
      });
}

bool supla_device_channels::action_hvac_set_temperatures(
    const supla_caller &caller, int channel_id, int group_id, unsigned char eol,
    const supla_action_hvac_setpoint_temperatures *temperatures) {
  if (!temperatures) {
    return false;
  }

  return action_hvac(
      caller, channel_id, group_id, eol, 0,
      [&](supla_device_channel *channel,
          supla_channel_hvac_value *value) -> bool {
        bool result = false;
        if (channel->get_func() == SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL) {
          value->clear();
          short temperature = 0;
          if (temperatures->get_heating_temperature(&temperature)) {
            value->set_setpoint_temperature_heat(temperature);
            result = true;
          }

          if (temperatures->get_cooling_temperature(&temperature)) {
            value->set_setpoint_temperature_cool(temperature);
            result = true;
          }
        }

        return result;
      });
}

void supla_device_channels::timer_arm(const supla_caller &caller,
                                      int channel_id, int group_id,
                                      unsigned char eol, unsigned char on,
                                      unsigned int duration_ms) {
  supla_device_channel *channel = find_channel(channel_id);
  char value[SUPLA_CHANNELVALUE_SIZE] = {};

  if (channel) {
    switch (channel->get_func()) {
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER:
        value[0] = on ? 1 : 0;
        break;
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
      case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER: {
        supla_channel_hvac_value *hvac_value =
            channel->get_value<supla_channel_hvac_value>();
        if (hvac_value) {
          if (on) {
            hvac_value->turn_on();
          } else {
            hvac_value->turn_off();
          }
          hvac_value->get_raw_value(value);
          delete hvac_value;
        } else {
          return;
        }
      } break;
      default:
        return;
    }
  }

  async_set_channel_value(channel, caller, group_id, eol, value, duration_ms);
}

supla_json_config *supla_device_channels::get_json_config(int channel_id) {
  supla_json_config *result = nullptr;
  access_channel(channel_id, [&result](supla_device_channel *channel) -> void {
    result = channel->get_json_config();
  });

  return result;
}

unsigned int supla_device_channels::get_value_validity_time_left_msec(void) {
  unsigned int result = 0;
  for_each(
      [&result](supla_device_channel *channel, bool *will_continue) -> void {
        unsigned int time = channel->get_value_validity_time_left_msec();
        if (time > result) {
          result = time;
        }
      });
  return result;
}

void supla_device_channels::on_related_sensor_value_changed(
    int control_channel_id, int sensor_id, bool is_open) {
  supla_json_config *config = nullptr;
  access_channel(control_channel_id,
                 [&config](supla_device_channel *channel) -> void {
                   switch (channel->get_func()) {
                     case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
                     case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
                       config = channel->get_json_config();
                       break;
                   }
                 });

  if (config) {
    controlling_the_gate_config *gcfg = new controlling_the_gate_config(config);
    if (gcfg->is_active_state_verification_method_enabled()) {
      supla_action_gate_openclose::cancel_tasks(
          device->get_user_id(), device->get_id(), control_channel_id, is_open);
    }
    delete gcfg;
    delete config;
  }
}

vector<supla_channel_fragment> supla_device_channels::get_fragments(void) {
  vector<supla_channel_fragment> result;
  result.reserve(channels.size());

  for_each(
      [&result](supla_device_channel *channel, bool *will_continue) -> void {
        supla_channel_fragment f;
        f = channel;
        result.push_back(f);
      });

  return result;
}

void supla_device_channels::send_configs_to_device(void) {
  for_each([](supla_device_channel *channel, bool *will_continue) -> void {
    channel->send_config_to_device();
  });
}
