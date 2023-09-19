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

#include "devicechannel.h"

#include <assert.h>
#include <string.h>

#include <memory>

#include "db/database.h"
#include "device.h"
#include "device/device_dao.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "device/extended_value/channel_extended_value_factory.h"
#include "device/value/channel_value_factory.h"
#include "jsonconfig/channel/action_trigger_config.h"
#include "jsonconfig/channel/hvac_config.h"
#include "jsonconfig/channel/impulse_counter_config.h"
#include "jsonconfig/channel/weekly_schedule_config.h"
#include "jsonconfig/device/device_json_config.h"
#include "lck.h"
#include "user/user.h"
#include "value/channel_openclosed_value.h"
#include "vbt/value_based_triggers.h"

using std::list;
using std::shared_ptr;

supla_device_channel::supla_device_channel(
    supla_device *device, int id, unsigned char channel_number, int type,
    int func, int param1, int param2, int param3, int param4,
    const char *text_param1, const char *text_param2, const char *text_param3,
    bool hidden, unsigned int flags, const char value[SUPLA_CHANNELVALUE_SIZE],
    unsigned _supla_int_t validity_time_sec,
    supla_channel_extended_value *extended_value, const char *user_config,
    const char *properties)
    : supla_abstract_common_channel_properties(),
      id(id),
      channel_number(channel_number),
      type(type),
      param2(param2),
      param3(param3),
      param4(param4),
      hidden(hidden) {
  lck = lck_init();
  this->device = device;
  this->func = func;
  this->param1 = param1;
  this->text_param1 = text_param1 ? strndup(text_param1, 255) : nullptr;
  this->text_param2 = text_param2 ? strndup(text_param2, 255) : nullptr;
  this->text_param3 = text_param3 ? strndup(text_param3, 255) : nullptr;
  this->flags = flags;
  this->offline = flags & SUPLA_CHANNEL_FLAG_OFFLINE_DURING_REGISTRATION;
  this->extended_value = extended_value;
  this->logger_purpose_extended_value = nullptr;
  this->value_valid_to.tv_sec = 0;
  this->value_valid_to.tv_usec = 0;
  this->json_config = nullptr;
  this->state = nullptr;

  if (validity_time_sec > 0) {
    gettimeofday(&value_valid_to, nullptr);
    value_valid_to.tv_sec += validity_time_sec;
  }

  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);

  channel_json_config *json_config = new channel_json_config(nullptr);
  if (json_config) {
    json_config->set_properties(properties);
    json_config->set_user_config(user_config);
    set_json_config(json_config);
  }

  voltage_analyzers.set_channel_id(id);
}

supla_device_channel::~supla_device_channel() {
  set_extended_value(nullptr);

  if (this->text_param1) {
    free(this->text_param1);
    this->text_param1 = nullptr;
  }

  if (this->text_param2) {
    free(this->text_param2);
    this->text_param2 = nullptr;
  }

  if (this->text_param3) {
    free(this->text_param3);
    this->text_param3 = nullptr;
  }

  if (json_config) {
    delete json_config;
  }

  if (logger_purpose_extended_value) {
    delete logger_purpose_extended_value;
    logger_purpose_extended_value = nullptr;
  }

  if (state) {
    delete state;
  }

  lck_free(lck);
}

// static
void supla_device_channel::get_defaults(int type, int func, int *param1,
                                        int *param2) {
  *param1 = 0;
  *param2 = 0;

  if (type == SUPLA_CHANNELTYPE_DIGIGLASS) {
    *param1 = 7;
    *param2 = 60;  // 1:00
  }
}

// static
int supla_device_channel::func_list_filter(int func_list, int type) {
  switch (type) {
    case SUPLA_CHANNELTYPE_ACTIONTRIGGER:
      func_list = 0;
      break;
  }

  return func_list;
}

void supla_device_channel::lock(void) { lck_lock(lck); }

void supla_device_channel::unlock(void) { lck_unlock(lck); }

int supla_device_channel::get_id(void) { return id; }

int supla_device_channel::get_device_id(void) { return get_device()->get_id(); }

unsigned char supla_device_channel::get_channel_number(void) {
  return channel_number;
}

int supla_device_channel::get_func(void) {
  lock();
  int result = func;
  unlock();
  return result;
}

supla_device *supla_device_channel::get_device() { return device; }

supla_user *supla_device_channel::get_user(void) {
  return device ? device->get_user() : nullptr;
}

int supla_device_channel::get_user_id(void) {
  supla_user *user = get_user();
  if (user) {
    return user->getUserID();
  }

  return 0;
}

void supla_device_channel::set_func(int func) {
  lock();
  this->func = func;
  unlock();
}

int supla_device_channel::get_type(void) { return type; }

int supla_device_channel::get_param1(void) {
  lock();
  int result = param1;
  unlock();
  return result;
}

int supla_device_channel::get_param2(void) { return param2; }

int supla_device_channel::get_param3(void) { return param3; }

int supla_device_channel::get_param4(void) { return param4; }

bool supla_device_channel::is_hidden(void) { return hidden; }

unsigned int supla_device_channel::get_flags() {
  lock();
  int result = flags;
  unlock();
  return result;
}

void supla_device_channel::add_flags(unsigned int flags) {
  unsigned int current_flags = get_flags();
  if ((current_flags | flags) != current_flags) {
    current_flags |= flags;
    lock();
    this->flags = current_flags;
    unlock();

    database *db = new database();

    if (db) {
      if (db->connect() == true) {
        db->update_channel_flags(get_id(), get_user_id(), current_flags);
      }
      delete db;
      db = nullptr;
    }
  }
}

const char *supla_device_channel::get_text_param1(void) { return text_param1; }

const char *supla_device_channel::get_text_param2(void) { return text_param2; }

const char *supla_device_channel::get_text_param3(void) { return text_param3; }

bool supla_device_channel::is_offline(void) {
  bool result = offline;
  lock();
  if (offline && (value_valid_to.tv_sec > 0 || value_valid_to.tv_usec)) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    result =
        (now.tv_sec * 1000000 + now.tv_usec) -
                    (value_valid_to.tv_sec * 1000000 + value_valid_to.tv_usec) >
                0
            ? true
            : false;
  }
  unlock();

  return result;
}

bool supla_device_channel::set_offline(bool offline) {
  bool result = false;
  lock();
  if (this->offline != offline) {
    this->offline = offline;
    result = true;
  }
  unlock();
  return result;
}

void supla_device_channel::get_value(char value[SUPLA_CHANNELVALUE_SIZE]) {
  lock();
  memcpy(value, this->value, SUPLA_CHANNELVALUE_SIZE);
  unlock();
}

unsigned _supla_int_t supla_device_channel::get_value_validity_time_sec(void) {
  unsigned _supla_int_t result = 0;
  lock();
  if (value_valid_to.tv_sec > 0 || value_valid_to.tv_usec) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    if (now.tv_sec < value_valid_to.tv_sec) {
      result = value_valid_to.tv_sec - now.tv_sec;
    }
  }
  unlock();
  return result;
}

supla_channel_extended_value *supla_device_channel::_get_extended_value(
    bool for_data_logger_purposes) {
  supla_channel_extended_value *result = nullptr;
  lock();

  if (for_data_logger_purposes) {
    result = logger_purpose_extended_value
                 ? logger_purpose_extended_value->copy()
                 : nullptr;
  } else {
    result = extended_value ? extended_value->copy() : nullptr;
  }

  unlock();
  return result;
}

void supla_device_channel::get_double(double *value) {
  if (value == nullptr) return;

  char current_value[SUPLA_CHANNELVALUE_SIZE] = {};
  get_value(current_value);

  switch (type) {
    case SUPLA_CHANNELTYPE_BINARYSENSOR:
    case SUPLA_CHANNELTYPE_SENSORNC:
      *value = current_value[0] == 1 ? 1 : 0;
      break;
    case SUPLA_CHANNELTYPE_THERMOMETER:
    case SUPLA_CHANNELTYPE_THERMOMETERDS18B20:
    case SUPLA_CHANNELTYPE_DISTANCESENSOR:
    case SUPLA_CHANNELTYPE_WINDSENSOR:
    case SUPLA_CHANNELTYPE_PRESSURESENSOR:
    case SUPLA_CHANNELTYPE_RAINSENSOR:
    case SUPLA_CHANNELTYPE_WEIGHTSENSOR:
      memcpy(value, current_value, sizeof(double));
      break;
    default:
      *value = 0;
  }
}

void supla_device_channel::get_char(char *value) {
  if (value == nullptr) return;

  char current_value[SUPLA_CHANNELVALUE_SIZE] = {};
  get_value(current_value);

  switch (get_func()) {
    case SUPLA_CHANNELFNC_THERMOSTAT:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
      TThermostat_Value *tv = (TThermostat_Value *)current_value;
      *value = tv->IsOn;
    } break;
    default:
      *value = current_value[0];
      break;
  }
}

void supla_device_channel::get_config(TSD_ChannelConfig *config,
                                      unsigned char config_type,
                                      unsigned _supla_int_t flags) {
  supla_abstract_common_channel_properties::get_config(
      config->Config, &config->ConfigSize, config_type, flags, false);

  config->Func = get_func();
  config->ChannelNumber = get_channel_number();
  config->ConfigType = config_type;
}

void supla_device_channel::set_json_config(channel_json_config *json_config) {
  lock();

  if (this->json_config) {
    delete this->json_config;
  }

  this->json_config = json_config;
  if (json_config && type == SUPLA_CHANNELTYPE_ELECTRICITY_METER) {
    electricity_meter_config *config =
        new electricity_meter_config(json_config);
    this->flags |= config->get_channel_user_flags();
    delete config;
  }

  unlock();
}

void supla_device_channel::db_set_properties(channel_json_config *config) {
  if (config) {
    supla_db_access_provider dba;
    supla_device_dao dao(&dba);
    dao.set_channel_properties(get_user_id(), get_id(), config);
  }
}

void supla_device_channel::db_set_params(int param1, int param2, int param3,
                                         int param4) {
  database *db = new database();

  if (db) {
    if (db->connect() == true) {
      db->update_channel_params(get_id(), get_user_id(), param1, param2, param3,
                                param4);
    }
    delete db;
    db = nullptr;
  }
}

void supla_device_channel::set_action_trigger_config(
    unsigned int capabilities, int related_channel_id,
    unsigned int disables_local_operation) {
  if (type != SUPLA_CHANNELTYPE_ACTIONTRIGGER) {
    return;
  }

  lock();
  action_trigger_config *at_config = new action_trigger_config(json_config);
  if (at_config) {
    if (!json_config) {
      json_config = at_config;
    }

    bool at_config_changed = at_config->set_capabilities(capabilities);

    if (at_config->set_caps_that_disables_local_operation(
            disables_local_operation)) {
      at_config_changed = true;
    }

    if (at_config_changed) {
      db_set_properties(at_config);
    }

    if (related_channel_id != get_param1()) {
      lock();
      param1 = related_channel_id;
      unlock();
      db_set_params(related_channel_id, param2, param3, param4);
    }

    if (at_config != json_config) {
      delete at_config;
      at_config = nullptr;
    }
  }
  unlock();
}

bool supla_device_channel::set_value(
    const char value[SUPLA_CHANNELVALUE_SIZE],
    const unsigned _supla_int_t *validity_time_sec, bool *offline) {
  lock();
  if (validity_time_sec == nullptr &&
      (value_valid_to.tv_usec || value_valid_to.tv_sec)) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    if ((value_valid_to.tv_sec * 1000000 + value_valid_to.tv_usec) -
            (now.tv_sec * 1000000 + now.tv_usec) >
        0) {
      unlock();
      return false;
    }

    value_valid_to.tv_sec = 0;
    value_valid_to.tv_usec = 0;
  }

  supla_channel_value *old_value = _get_value();
  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);
  supla_channel_value *new_value = _get_value();

  new_value->apply_channel_properties(
      type, get_device()->get_connection()->get_protocol_version(), param1,
      param2, param3, param4, json_config);

  new_value->get_raw_value(this->value);

  if (validity_time_sec) {
    gettimeofday(&value_valid_to, nullptr);
    value_valid_to.tv_sec += (*validity_time_sec);
  }

  bool significant_change = false;
  bool differ = new_value->is_differ(old_value, &significant_change);

  if (offline && set_offline(*offline)) {
    differ = true;
    significant_change = true;
  }

  unlock();

  if (differ || validity_time_sec) {
    char current_value[SUPLA_CHANNELVALUE_SIZE] = {};
    new_value->get_raw_value(current_value);

    supla_db_access_provider dba;
    supla_device_dao dao(&dba);
    dao.update_channel_value(get_id(), get_user_id(), current_value,
                             validity_time_sec ? *validity_time_sec : 0);
  }

  supla_channel_extended_value *eval = new_value->convert2extended(
      json_config, func, text_param1, text_param2, param2, param3,
      &logger_purpose_extended_value);

  if (eval) {
    set_extended_value(nullptr, eval);
  }

  if (differ) {
    on_value_changed(old_value, new_value, significant_change, eval != nullptr);
  }

  if (new_value) {
    delete new_value;
  }

  if (old_value) {
    delete old_value;
  }

  return differ;
}

void supla_device_channel::on_value_changed(supla_channel_value *old_value,
                                            supla_channel_value *new_value,
                                            bool significant_change,
                                            bool convertible2extended) {
  switch (get_func()) {
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      if (get_param1()) {
        shared_ptr<supla_device> rel_device =
            get_device()->get_user()->get_device(device->get_user_id(), 0,
                                                 get_param1());
        if (rel_device) {
          rel_device->get_channels()->on_related_sensor_value_changed(
              get_param1(), get_id(), value[0] == 0);
        }
      }
      break;
  }

  get_device()->get_user()->on_channel_value_changed(
      supla_caller(ctDevice, get_device()->get_id()), get_device()->get_id(),
      get_id(), false, significant_change);

  if (convertible2extended) {
    get_device()->get_user()->on_channel_value_changed(
        supla_caller(ctDevice, get_device()->get_id()), get_device()->get_id(),
        get_id(), true);
  }

  get_device()
      ->get_user()
      ->get_value_based_triggers()
      ->on_channel_value_changed(supla_caller(ctDevice, get_device()->get_id()),
                                 get_id(), old_value, new_value);
}

void supla_device_channel::on_extended_value_changed(
    supla_channel_extended_value *old_value,
    supla_channel_extended_value *new_value) {
  get_device()
      ->get_user()
      ->get_value_based_triggers()
      ->on_channel_value_changed(supla_caller(ctDevice, get_device()->get_id()),
                                 get_id(), old_value, new_value);
}

void supla_device_channel::set_extended_value(
    TSuplaChannelExtendedValue *ev, supla_channel_extended_value *new_value) {
  lock();

  supla_channel_extended_value *old_value = extended_value;

  if (ev) {
    new_value = supla_channel_extended_value_factory::new_value(
        ev, text_param1, param2, get_user());
    supla_channel_em_extended_value *em =
        dynamic_cast<supla_channel_em_extended_value *>(new_value);

    if (em) {
      electricity_meter_config *config =
          new electricity_meter_config(json_config);

      if (config->update_available_counters(em)) {
        db_set_properties(config);
      }

      if (!config->should_be_added_to_history()) {
        if (logger_purpose_extended_value) {
          delete logger_purpose_extended_value;
        }

        logger_purpose_extended_value = em->copy();
      }

      {
        TElectricityMeter_ExtendedValue_V2 em_ev;
        if (em->get_raw_value(&em_ev)) {
          config->add_initial_values(flags, &em_ev);

          delete em;
          em = new supla_channel_em_extended_value(&em_ev, text_param1, param2);
          new_value = em;
        }
      }

      if (config->should_be_added_to_history()) {
        if (logger_purpose_extended_value) {
          delete logger_purpose_extended_value;
        }

        logger_purpose_extended_value = em->copy();
      }

      voltage_analyzers.add_samples(flags, config, em);

      delete config;
    }
  }

  extended_value = new_value;

  if (new_value &&
      (!old_value || (old_value && new_value->is_differ(old_value)))) {
    new_value =
        new_value
            ->copy();  // We create a copy to save the data to the database or
                       // call on_extended_value_changed after leaving the lock
  } else {
    new_value = nullptr;
  }

  unlock();

  if (new_value) {  // That means there are differences
    supla_db_access_provider dba;
    supla_device_dao dao(&dba);
    dao.update_channel_extended_value(get_id(), get_user_id(), new_value);

    if (old_value) {
      on_extended_value_changed(old_value, new_value);
    }

    delete new_value;  // This is a copy of the new value
  }

  if (old_value) {
    delete old_value;
  }
}

void supla_device_channel::set_extended_value(TSuplaChannelExtendedValue *ev) {
  set_extended_value(ev, nullptr);
}

void supla_device_channel::for_each(
    std::function<void(supla_abstract_common_channel_properties *, bool *)>
        on_channel_properties) {
  get_device()->get_channels()->for_each(
      [&](supla_device_channel *channel, bool *will_continue) -> void {
        on_channel_properties(channel, will_continue);
      });
}

void supla_device_channel::assign_rgbw_value(
    char value[SUPLA_CHANNELVALUE_SIZE], int color, char color_brightness,
    char brightness, char on_off) {
  int func = get_func();

  if (func == SUPLA_CHANNELFNC_DIMMER ||
      func == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (brightness < 0 || brightness > 100) brightness = 0;

    value[0] = brightness;
  }

  if (func == SUPLA_CHANNELFNC_RGBLIGHTING ||
      func == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (color_brightness < 0 || color_brightness > 100) color_brightness = 0;

    value[1] = color_brightness;
    value[2] = (char)((color & 0x000000FF));
    value[3] = (char)((color & 0x0000FF00) >> 8);
    value[4] = (char)((color & 0x00FF0000) >> 16);
  }

  value[5] = on_off;
}

bool supla_device_channel::is_value_writable(void) {
  switch (get_func()) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
    case SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL:
    case SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL:
      return true;
  }

  return false;
}

bool supla_device_channel::is_char_value_writable(void) {
  switch (get_func()) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_THERMOMETER:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
      return true;
  }

  return false;
}

bool supla_device_channel::is_rgbw_value_writable(void) {
  switch (get_func()) {
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      return 1;

      break;
  }

  return 0;
}

unsigned int supla_device_channel::get_value_duration(void) {
  switch (get_func()) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      return get_param1();

    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      return get_param1() * 100;

    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW: {
      unsigned int result = 0;

      result = (unsigned short)get_param1();
      result <<= 16;
      result |= (unsigned short)param3;

      return result;
    }
  }

  return 0;
}

channel_json_config *supla_device_channel::get_json_config(void) {
  channel_json_config *result = nullptr;

  lock();
  if (json_config) {
    result = new channel_json_config(json_config, true);
  }
  unlock();

  return result;
}

void supla_device_channel::send_config_to_device(unsigned char config_type) {
  if ((get_flags() & SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE) &&
      get_device()->get_protocol_version() >= 21) {
    TSDS_SetChannelConfig config = {};

    get_config(&config, config_type, 0);

    if (config.ConfigSize > 0) {
      get_device()
          ->get_connection()
          ->get_srpc_adapter()
          ->sd_async_set_channel_config_request(
              (TSDS_SetChannelConfig *)&config);
    }
  }
}

void supla_device_channel::send_config_to_device(void) {
  send_config_to_device(SUPLA_CONFIG_TYPE_DEFAULT);

  if (get_type() == SUPLA_CHANNELTYPE_HVAC &&
      (get_flags() & SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE)) {
    send_config_to_device(SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE);

    if (get_func() == SUPLA_CHANNELFNC_HVAC_THERMOSTAT) {
      send_config_to_device(SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE);
    }
  }

  TSD_ChannelConfigFinished fin = {};
  fin.ChannelNumber = get_channel_number();

  get_device()
      ->get_connection()
      ->get_srpc_adapter()
      ->sd_async_channel_config_finished(&fin);
}

unsigned int supla_device_channel::get_value_validity_time_left_msec(void) {
  unsigned int result = 0;
  lock();
  if (value_valid_to.tv_sec || value_valid_to.tv_usec) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    unsigned long long now_msec = (now.tv_sec * 1000 + now.tv_usec / 1000);
    unsigned long long valid_to_msec =
        (value_valid_to.tv_sec * 1000 + value_valid_to.tv_usec / 1000);

    if (now_msec < valid_to_msec) {
      result = valid_to_msec - now_msec;
    }
  }

  unlock();
  return result;
}

void supla_device_channel::set_state(TDSC_ChannelState *state) {
  lock();
  if (this->state == nullptr) {
    this->state = new TDSC_ChannelState();
  }

  *this->state = *state;
  unlock();
}

bool supla_device_channel::get_state(TDSC_ChannelState *state) {
  bool result = false;
  lock();
  if (this->state) {
    *state = *this->state;
    result = true;
  }
  unlock();

  return result;
}

bool supla_device_channel::get_voltage_analyzers_with_any_sample_over_threshold(
    supla_voltage_analyzers *voltage_analyzers, bool reset) {
  bool result = false;
  lock();
  if (this->voltage_analyzers.is_any_sample_over_threshold()) {
    *voltage_analyzers = this->voltage_analyzers;
    result = true;
  }

  if (reset) {
    this->voltage_analyzers.reset();
  }
  unlock();
  return result;
}

supla_channel_value *supla_device_channel::_get_value(void) {
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  get_value(value);
  int func = get_func();

  lock();
  supla_channel_value *result = supla_channel_value_factory::new_value(
      value, type, func, get_user(), param2, param3);
  unlock();

  return result;
}
