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

#include "actions/abstract_action_executor.h"

#include "actions/action_rgbw_parameters.h"
#include "actions/action_rs_parameters.h"
#include "converter/any_value_to_action_converter.h"

using std::function;
using std::map;
using std::shared_ptr;
using std::string;

supla_abstract_action_executor::supla_abstract_action_executor(void) {
  this->user = NULL;
  this->device_id = 0;
  this->subject_id = 0;
  this->subject_type = stUnknown;
}

supla_abstract_action_executor::~supla_abstract_action_executor(void) {}

void supla_abstract_action_executor::set_unknown_subject_type(void) {
  this->device_id = 0;
  this->subject_id = 0;
  this->subject_type = stUnknown;
}

void supla_abstract_action_executor::set_caller(const supla_caller &caller) {
  this->caller = caller;
}

void supla_abstract_action_executor::set_channel_id(supla_user *user,
                                                    int device_id,
                                                    int channel_id) {
  this->user = user;
  this->device_id = device_id;
  this->subject_id = channel_id;
  this->subject_type = stChannel;
}

void supla_abstract_action_executor::set_channel_id(int user_id, int device_id,
                                                    int channel_id) {
  this->user = user_id ? supla_user::find(user_id, false) : NULL;
  this->device_id = device_id;
  this->subject_id = channel_id;
  this->subject_type = stChannel;
}

void supla_abstract_action_executor::set_group_id(supla_user *user,
                                                  int group_id) {
  this->user = user;
  this->device_id = 0;
  this->subject_id = group_id;
  this->subject_type = stChannelGroup;
}

void supla_abstract_action_executor::set_group_id(int user_id, int group_id) {
  this->user = user_id ? supla_user::find(user_id, false) : NULL;
  this->device_id = 0;
  this->subject_id = group_id;
  this->subject_type = stChannelGroup;
}

void supla_abstract_action_executor::set_scene_id(supla_user *user,
                                                  int scene_id) {
  this->user = user;
  this->device_id = 0;
  this->subject_id = scene_id;
  this->subject_type = stScene;
}

void supla_abstract_action_executor::set_scene_id(int user_id, int scene_id) {
  this->user = user_id ? supla_user::find(user_id, false) : NULL;
  this->device_id = 0;
  this->subject_id = scene_id;
  this->subject_type = stScene;
}

void supla_abstract_action_executor::set_schedule_id(supla_user *user,
                                                     int schedule_id) {
  this->user = user;
  this->device_id = 0;
  this->subject_id = schedule_id;
  this->subject_type = stSchedule;
}

void supla_abstract_action_executor::set_schedule_id(int user_id,
                                                     int schedule_id) {
  this->user = user_id ? supla_user::find(user_id, false) : NULL;
  this->device_id = 0;
  this->subject_id = schedule_id;
  this->subject_type = stSchedule;
}

void supla_abstract_action_executor::set_push_notification_id(supla_user *user,
                                                              int push_id) {
  this->user = user;
  this->device_id = 0;
  this->subject_id = push_id;
  this->subject_type = stPushNotification;
}

void supla_abstract_action_executor::set_push_notification_id(int user_id,
                                                              int push_id) {
  this->user = user_id ? supla_user::find(user_id, false) : NULL;
  this->device_id = 0;
  this->subject_id = push_id;
  this->subject_type = stPushNotification;
}

shared_ptr<supla_device> supla_abstract_action_executor::get_device(void) {
  if (user && (device_id || (subject_id && subject_type == stChannel))) {
    return user->get_devices()->get(
        device_id, subject_type == stChannelGroup ? 0 : subject_id);
  }

  return nullptr;
}

void supla_abstract_action_executor::execute_action(
    const supla_caller &caller, int user_id, int action_id,
    _subjectType_e subject_type, int subject_id,
    supla_abstract_channel_property_getter *property_getter,
    supla_abstract_action_parameters *params, int source_device_id,
    int source_channel_id, int cap, map<string, string> *replacement_map) {
  if (action_id == 0 || subject_id == 0) {
    return;
  }

  set_caller(caller);

  switch (subject_type) {
    case stChannel:
      set_channel_id(user_id, 0, subject_id);
      break;
    case stChannelGroup:
      set_group_id(user_id, subject_id);
      break;
    case stScene:
      set_scene_id(user_id, subject_id);
      break;
    case stSchedule:
      set_schedule_id(user_id, subject_id);
      break;
    case stPushNotification:
      set_push_notification_id(user_id, subject_id);
      break;
    default:
      set_unknown_subject_type();
      break;
  }

  switch (action_id) {
    case ACTION_OPEN:
      open();
      break;
    case ACTION_CLOSE:
      close();
      break;
    case ACTION_SHUT:
      shut(NULL, false);
      break;
    case ACTION_REVEAL:
      reveal();
      break;
    case ACTION_UP_OR_STOP:
      up_or_stop();
      break;
    case ACTION_DOWN_OR_STOP:
      down_or_stop();
      break;
    case ACTION_STEP_BY_STEP:
      step_by_step();
      break;
    case ACTION_SHUT_PARTIALLY:
    case ACTION_REVEAL_PARTIALLY:
      if (params) {
        supla_action_rs_parameters *rs =
            dynamic_cast<supla_action_rs_parameters *>(params);
        if (rs) {
          char percentage = rs->get_percentage();

          if (percentage > -1) {
            if (action_id == ACTION_REVEAL_PARTIALLY) {
              percentage = 100 - percentage;
            }

            shut(&percentage, rs->is_delta());
          }
        }
      }
      break;
    case ACTION_TURN_ON:
      set_on(true, 0);
      break;
    case ACTION_TURN_OFF:
      set_on(false, 0);
      break;
    case ACTION_SET_RGBW_PARAMETERS:
      if (params) {
        supla_action_rgbw_parameters *rgbw =
            dynamic_cast<supla_action_rgbw_parameters *>(params);

        if (rgbw) {
          TAction_RGBW_Parameters rgbw_p = rgbw->get_rgbw();

          if (rgbw && (rgbw_p.Brightness > -1 || rgbw_p.ColorBrightness > -1 ||
                       rgbw_p.Color)) {
            set_rgbw(
                rgbw_p.Color ? &rgbw_p.Color : nullptr,
                rgbw_p.ColorBrightness > -1 ? &rgbw_p.ColorBrightness : nullptr,
                rgbw_p.Brightness > -1 ? &rgbw_p.Brightness : nullptr,
                rgbw_p.OnOff > -1 ? &rgbw_p.OnOff : nullptr);
          }
        }
      }
      break;
    case ACTION_OPEN_CLOSE:
      open_close();
      break;
    case ACTION_EXECUTE:
      execute();
      break;
    case ACTION_ENABLE:
      enable();
      break;
    case ACTION_DISABLE:
      disable();
      break;
    case ACTION_SEND:
      send(replacement_map);
      break;
    case ACTION_INTERRUPT:
      interrupt();
      break;
    case ACTION_INTERRUPT_AND_EXECUTE:
      interrupt_and_execute();
      break;
    case ACTION_STOP:
      stop();
      break;
    case ACTION_TOGGLE:
      toggle();
      break;
    case ACTION_COPY:
      if (property_getter) {
        copy(property_getter, source_device_id, source_channel_id);  // NOLINT
      }
      break;
    case ACTION_FORWARD_OUTSIDE:
      forward_outside(cap);
      break;
    case ACTION_HVAC_SET_PARAMETERS:
      if (params) {
        supla_action_hvac_parameters *hvac =
            dynamic_cast<supla_action_hvac_parameters *>(params);

        if (hvac && hvac->is_any_param_set()) {
          hvac_set_parameters(hvac);
        }
      }
      break;
    case ACTION_HVAC_SWITCH_TO_MANUAL_MODE:
      hvac_switch_to_manual_mode();
      break;
    case ACTION_HVAC_SWITCH_TO_PROGRAM_MODE:
      hvac_switch_to_program_mode();
      break;
    case ACTION_HVAC_SET_TEMPERATURE:
      if (params) {
        supla_action_hvac_setpoint_temperature *t =
            dynamic_cast<supla_action_hvac_setpoint_temperature *>(params);

        if (t) {
          hvac_set_temperature(t);
        }
      }
      break;
    case ACTION_HVAC_SET_TEMPERATURES:
      if (params) {
        supla_action_hvac_setpoint_temperatures *t =
            dynamic_cast<supla_action_hvac_setpoint_temperatures *>(params);

        if (t) {
          hvac_set_temperatures(t);
        }
      }
      break;
  }
}

void supla_abstract_action_executor::execute_action(
    const supla_caller &caller, int user_id, abstract_action_config *config,
    supla_abstract_channel_property_getter *property_getter,
    map<string, string> *replacement_map) {
  int action_id = 0;
  int subject_id = 0;

  if (!config || (action_id = config->get_action_id()) == 0 ||
      (subject_id = config->get_subject_id()) == 0) {
    return;
  }

  int source_device_id = 0;
  int source_channel_id = 0;
  int cap = 0;

  switch (action_id) {
    case ACTION_COPY:
      source_device_id = config->get_source_device_id();
      source_channel_id = config->get_source_channel_id();
      break;
    case ACTION_FORWARD_OUTSIDE:
      cap = config->get_cap();
      break;
  }

  supla_abstract_action_parameters *params = config->get_parameters();

  execute_action(caller, user_id, action_id, config->get_subject_type(),
                 subject_id, property_getter, params, source_device_id,
                 source_channel_id, cap, replacement_map);

  if (params) {
    delete params;
  }
}

void supla_abstract_action_executor::execute_action(
    const supla_caller &caller, int user_id, abstract_action_config *config,
    supla_abstract_channel_property_getter *property_getter) {
  execute_action(caller, user_id, config, property_getter, nullptr);
}

void supla_abstract_action_executor::execute_action(
    function<void(supla_user_channelgroups *, supla_device_channels *)> f) {
  supla_user_channelgroups *channel_groups = get_channel_groups();
  if (channel_groups) {
    f(channel_groups, NULL);
  } else {
    std::shared_ptr<supla_device> device = get_device();
    if (device != nullptr) {
      supla_device_channels *channels = device->get_channels();
      if (channels) {
        f(NULL, channels);
      }
    }
  }
}

const supla_caller &supla_abstract_action_executor::get_caller(void) {
  return caller;
}

supla_user_channelgroups *supla_abstract_action_executor::get_channel_groups(
    void) {
  return subject_type == stChannelGroup && user ? user->get_channel_groups()
                                                : NULL;
}

supla_user *supla_abstract_action_executor::get_user(void) { return user; }

int supla_abstract_action_executor::get_user_id(void) {
  return user ? user->getUserID() : 0;
}

int supla_abstract_action_executor::get_device_id(void) { return device_id; }

int supla_abstract_action_executor::get_channel_id(void) {
  return subject_type == stChannel ? subject_id : 0;
}

int supla_abstract_action_executor::get_group_id(void) {
  return subject_type == stChannelGroup ? subject_id : 0;
}

int supla_abstract_action_executor::get_scene_id(void) {
  return subject_type == stScene ? subject_id : 0;
}

int supla_abstract_action_executor::get_schedule_id(void) {
  return subject_type == stSchedule ? subject_id : 0;
}

int supla_abstract_action_executor::get_push_notification_id(void) {
  return subject_type == stPushNotification ? subject_id : 0;
}

void supla_abstract_action_executor::copy(
    supla_abstract_channel_property_getter *property_getter,
    int source_device_id, int source_channel_id) {
  if (property_getter) {
    supla_channel_value *value = NULL;
    if ((value = property_getter->get_value(get_user_id(), source_device_id,
                                            source_channel_id))) {
      any_value_to_action_converter *converter =
          new any_value_to_action_converter();
      if (converter) {
        converter->convert(value, this);
        delete converter;
      }

      delete value;
      value = NULL;
    }
  }
}
