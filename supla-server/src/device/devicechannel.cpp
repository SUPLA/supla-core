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

#include "channeljsonconfig/action_trigger_config.h"
#include "channeljsonconfig/impulse_counter_config.h"
#include "db/database.h"
#include "device.h"
#include "user/user.h"

using std::list;
using std::shared_ptr;

supla_device_channel::supla_device_channel(
    supla_device *Device, int Id, int Number, int Type, int Func, int Param1,
    int Param2, int Param3, int Param4, const char *TextParam1,
    const char *TextParam2, const char *TextParam3, bool Hidden,
    unsigned int Flags, const char value[SUPLA_CHANNELVALUE_SIZE],
    unsigned _supla_int_t validity_time_sec, const char *user_config,
    const char *properties) {
  this->Id = Id;
  this->Number = Number;
  this->Device = Device;
  this->Type = Type;
  this->Func = Func;
  this->Param1 = Param1;
  this->Param2 = Param2;
  this->Param3 = Param3;
  this->Param4 = Param4;
  this->TextParam1 = TextParam1 ? strndup(TextParam1, 255) : NULL;
  this->TextParam2 = TextParam2 ? strndup(TextParam2, 255) : NULL;
  this->TextParam3 = TextParam3 ? strndup(TextParam3, 255) : NULL;
  this->Hidden = Hidden;
  this->Flags = Flags;
  this->Offline = Flags & SUPLA_CHANNEL_FLAG_OFFLINE_DURING_REGISTRATION;
  this->extendedValue = NULL;
  this->value_valid_to.tv_sec = 0;
  this->value_valid_to.tv_usec = 0;
  this->json_config = NULL;
  this->logger_data = NULL;
  this->state = nullptr;

  if (validity_time_sec > 0) {
    gettimeofday(&value_valid_to, NULL);
    value_valid_to.tv_sec += validity_time_sec;
  }

  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);

  bool load_json_config = false;

  switch (Type) {
    case SUPLA_CHANNELTYPE_ACTIONTRIGGER:
    case SUPLA_CHANNELTYPE_ELECTRICITY_METER:
    case SUPLA_CHANNELTYPE_IMPULSE_COUNTER:
    case SUPLA_CHANNELTYPE_RELAY:
    case SUPLA_CHANNELTYPE_RELAYHFD4:
    case SUPLA_CHANNELTYPE_RELAYG5LA1A:
    case SUPLA_CHANNELTYPE_2XRELAYG5LA1A:
      load_json_config = true;
      break;
  }

  switch (Func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      load_json_config = true;
      break;
  }

  if (load_json_config) {
    json_config = new channel_json_config(NULL);
    if (json_config) {
      json_config->set_properties(properties);
      json_config->set_user_config(user_config);
      if (Type == SUPLA_CHANNELTYPE_ELECTRICITY_METER) {
        electricity_meter_config *config =
            new electricity_meter_config(json_config);
        this->Flags |= config->get_channel_user_flags();
        delete config;
      }
    }
  }

  if (Type == SUPLA_CHANNELTYPE_ELECTRICITY_METER ||
      SUPLA_CHANNELTYPE_IMPULSE_COUNTER) {
    logger_data = new _logger_purpose_t();
  }

  voltage_analyzers.set_channel_id(Id);
}

supla_device_channel::~supla_device_channel() {
  setExtendedValue(NULL);

  if (this->TextParam1) {
    free(this->TextParam1);
    this->TextParam1 = NULL;
  }

  if (this->TextParam2) {
    free(this->TextParam2);
    this->TextParam2 = NULL;
  }

  if (this->TextParam3) {
    free(this->TextParam3);
    this->TextParam3 = NULL;
  }

  if (json_config) {
    delete json_config;
  }

  if (logger_data) {
    if (Type == SUPLA_CHANNELTYPE_ELECTRICITY_METER &&
        logger_data->extendedValue) {
      delete logger_data->extendedValue;
    }
    delete logger_data;
    logger_data = NULL;
  }

  if (state) {
    delete state;
  }
}

// static
void supla_device_channel::getDefaults(int Type, int Func, int *Param1,
                                       int *Param2) {
  *Param1 = 0;
  *Param2 = 0;

  if (Type == SUPLA_CHANNELTYPE_DIGIGLASS) {
    *Param1 = 7;
    *Param2 = 60;  // 1:00
  }
}

// static
int supla_device_channel::funcListFilter(int FuncList, int Type) {
  switch (Type) {
    case SUPLA_CHANNELTYPE_ACTIONTRIGGER:
      FuncList = 0;
      break;
  }

  return FuncList;
}

int supla_device_channel::getId(void) { return Id; }

int supla_device_channel::getNumber(void) { return Number; }

int supla_device_channel::getFunc(void) { return Func; }

supla_device *supla_device_channel::getDevice() { return Device; }

supla_user *supla_device_channel::getUser(void) {
  return Device ? Device->get_user() : NULL;
}

int supla_device_channel::getUserID(void) {
  supla_user *User = getUser();
  if (User) {
    return User->getUserID();
  }

  return 0;
}

void supla_device_channel::setFunc(int Func) { this->Func = Func; }

int supla_device_channel::getType(void) { return Type; }

int supla_device_channel::getParam1(void) { return Param1; }

int supla_device_channel::getParam2(void) { return Param2; }

int supla_device_channel::getParam3(void) { return Param3; }

int supla_device_channel::getParam4(void) { return Param4; }

bool supla_device_channel::getHidden(void) { return Hidden; }

unsigned int supla_device_channel::getFlags() { return Flags; }

void supla_device_channel::addFlags(unsigned int flags) {
  if ((this->Flags | flags) != this->Flags) {
    this->Flags = flags;

    database *db = new database();

    if (db) {
      if (db->connect() == true) {
        db->update_channel_flags(getId(), getUserID(), Flags);
      }
      delete db;
      db = NULL;
    }
  }
}

const char *supla_device_channel::getTextParam1(void) { return TextParam1; }

const char *supla_device_channel::getTextParam2(void) { return TextParam2; }

const char *supla_device_channel::getTextParam3(void) { return TextParam3; }

bool supla_device_channel::isOffline(void) {
  if (Offline && (value_valid_to.tv_sec > 0 || value_valid_to.tv_usec)) {
    struct timeval now;
    gettimeofday(&now, NULL);

    return (now.tv_sec * 1000000 + now.tv_usec) -
                       (value_valid_to.tv_sec * 1000000 +
                        value_valid_to.tv_usec) >
                   0
               ? true
               : false;
  }

  return Offline;
}

bool supla_device_channel::setOffline(bool Offline) {
  if (this->Offline != Offline) {
    this->Offline = Offline;
    return true;
  }
  return false;
}

void supla_device_channel::getValue(char value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(value, this->value, SUPLA_CHANNELVALUE_SIZE);
}

unsigned _supla_int_t supla_device_channel::getValueValidityTimeSec(void) {
  if (value_valid_to.tv_sec > 0 || value_valid_to.tv_usec) {
    struct timeval now;
    gettimeofday(&now, NULL);

    if (now.tv_sec < value_valid_to.tv_sec) {
      return value_valid_to.tv_sec - now.tv_sec;
    }
  }

  return 0;
}

bool supla_device_channel::getExtendedValue(TSuplaChannelExtendedValue *ev,
                                            bool em_update) {
  if (ev == NULL) {
    return false;
  }

  if (extendedValue == NULL) {
    memset(ev, 0, sizeof(TSuplaChannelExtendedValue));
    return false;
  }

  memcpy(ev, extendedValue, sizeof(TSuplaChannelExtendedValue));

  bool result = true;

  if (em_update) {
    switch (ev->type) {
      case EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2:
        result = supla_channel_electricity_measurement::update_cev(
            ev, getParam2(), getTextParam1());
        break;
      case EV_TYPE_IMPULSE_COUNTER_DETAILS_V1:
        result = supla_channel_ic_measurement::update_cev(
            ev, getFunc(), getParam2(), getParam3(), getTextParam1(),
            getTextParam2());
        break;
    }
  }

  return result;
}

void supla_device_channel::getDouble(double *Value) {
  if (Value == NULL) return;

  switch (Type) {
    case SUPLA_CHANNELTYPE_SENSORNO:
    case SUPLA_CHANNELTYPE_SENSORNC:
      *Value = this->value[0] == 1 ? 1 : 0;
      break;
    case SUPLA_CHANNELTYPE_THERMOMETER:
    case SUPLA_CHANNELTYPE_THERMOMETERDS18B20:
    case SUPLA_CHANNELTYPE_DISTANCESENSOR:
    case SUPLA_CHANNELTYPE_WINDSENSOR:
    case SUPLA_CHANNELTYPE_PRESSURESENSOR:
    case SUPLA_CHANNELTYPE_RAINSENSOR:
    case SUPLA_CHANNELTYPE_WEIGHTSENSOR:
      memcpy(Value, this->value, sizeof(double));
      break;
    default:
      *Value = 0;
  }
}

void supla_device_channel::getChar(char *Value) {
  if (Value == NULL) return;
  switch (Func) {
    case SUPLA_CHANNELFNC_THERMOSTAT:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
      TThermostat_Value *tv = (TThermostat_Value *)this->value;
      *Value = tv->IsOn;
    } break;
    default:
      *Value = this->value[0];
      break;
  }
}

bool supla_device_channel::getRGBW(int *color, char *color_brightness,
                                   char *brightness, char *on_off) {
  if (color != NULL) *color = 0;

  if (color_brightness != NULL) *color_brightness = 0;

  if (brightness != NULL) *brightness = 0;

  if (on_off != NULL) *on_off = 0;

  bool result = false;

  if (Type == SUPLA_CHANNELTYPE_DIMMER ||
      Type == SUPLA_CHANNELTYPE_DIMMERANDRGBLED) {
    if (brightness != NULL) {
      *brightness = this->value[0];

      if (*brightness < 0 || *brightness > 100) *brightness = 0;
    }

    result = true;
  }

  if (Type == SUPLA_CHANNELTYPE_RGBLEDCONTROLLER ||
      Type == SUPLA_CHANNELTYPE_DIMMERANDRGBLED) {
    if (color_brightness != NULL) {
      *color_brightness = this->value[1];

      if (*color_brightness < 0 || *color_brightness > 100)
        *color_brightness = 0;
    }

    if (color != NULL) {
      *color = 0;

      *color = this->value[4] & 0xFF;
      (*color) <<= 8;

      *color |= this->value[3] & 0xFF;
      (*color) <<= 8;

      (*color) |= this->value[2] & 0xFF;
    }

    result = true;
  }

  return result;
}

bool supla_device_channel::getValveValue(TValve_Value *Value) {
  if (Value == NULL) return false;
  switch (Func) {
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
      memcpy(Value, this->value, sizeof(TValve_Value));
      return true;
  }

  return false;
}

bool supla_device_channel::getConfig(TSD_ChannelConfig *config,
                                     unsigned char configType,
                                     unsigned _supla_int_t flags) {
  if (configType != SUPLA_CONFIG_TYPE_DEFAULT || flags != 0) {
    return false;
  }

  memset(config, 0, sizeof(TSD_ChannelConfig));
  config->Func = getFunc();
  config->ChannelNumber = getNumber();

  switch (config->Func) {
    case SUPLA_CHANNELFNC_STAIRCASETIMER: {
      config->ConfigSize = sizeof(TSD_ChannelConfig_StaircaseTimer);
      TSD_ChannelConfig_StaircaseTimer *cfg =
          (TSD_ChannelConfig_StaircaseTimer *)config->Config;
      cfg->TimeMS = getParam1() * 100;
    } break;

    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW: {
      config->ConfigSize = sizeof(TSD_ChannelConfig_Rollershutter);
      TSD_ChannelConfig_Rollershutter *cfg =
          (TSD_ChannelConfig_Rollershutter *)config->Config;
      cfg->OpeningTimeMS = getParam1() * 100;
      cfg->ClosingTimeMS = getParam3() * 100;
    } break;

    case SUPLA_CHANNELFNC_ACTIONTRIGGER: {
      config->ConfigSize = sizeof(TSD_ChannelConfig_ActionTrigger);
      TSD_ChannelConfig_ActionTrigger *cfg =
          (TSD_ChannelConfig_ActionTrigger *)config->Config;
      cfg->ActiveActions = 0;
      action_trigger_config *at_config = new action_trigger_config(json_config);
      if (at_config) {
        cfg->ActiveActions = at_config->get_active_actions();
        delete at_config;
        at_config = NULL;
      }
    } break;
  }

  return true;
}

void supla_device_channel::db_set_properties(channel_json_config *config) {
  database *db = new database();

  if (db) {
    if (db->connect() == true) {
      char *cfg_string = config->get_properties();

      db->update_channel_properties(getId(), getUserID(), cfg_string);

      if (cfg_string) {
        free(cfg_string);
        cfg_string = NULL;
      }
    }
    delete db;
    db = NULL;
  }
}

void supla_device_channel::db_set_params(int Param1, int Param2, int Param3,
                                         int Param4) {
  database *db = new database();

  if (db) {
    if (db->connect() == true) {
      db->update_channel_params(getId(), getUserID(), Param1, Param2, Param3,
                                Param4);
    }
    delete db;
    db = NULL;
  }
}

void supla_device_channel::setActionTriggerConfig(
    unsigned int capabilities, int relatedChannelId,
    unsigned int disablesLocalOperation) {
  if (Type != SUPLA_CHANNELTYPE_ACTIONTRIGGER) {
    return;
  }

  action_trigger_config *at_config = new action_trigger_config(json_config);
  if (!at_config) {
    return;
  }

  if (!json_config) {
    json_config = at_config;
  }

  bool at_config_changed = at_config->set_capabilities(capabilities);

  if (at_config->set_caps_that_disables_local_operation(
          disablesLocalOperation)) {
    at_config_changed = true;
  }

  if (at_config_changed) {
    db_set_properties(at_config);
  }

  if (relatedChannelId != Param1) {
    Param1 = relatedChannelId;
    db_set_params(Param1, Param2, Param3, Param4);
  }

  if (at_config != json_config) {
    delete at_config;
    at_config = NULL;
  }
}

bool supla_device_channel::setValue(
    const char value[SUPLA_CHANNELVALUE_SIZE],
    const unsigned _supla_int_t *validity_time_sec, bool *significantChange,
    unsigned char proto_version) {
  if (validity_time_sec == NULL &&
      (value_valid_to.tv_usec || value_valid_to.tv_sec)) {
    struct timeval now;
    gettimeofday(&now, NULL);

    if ((value_valid_to.tv_sec * 1000000 + value_valid_to.tv_usec) -
            (now.tv_sec * 1000000 + now.tv_usec) >
        0) {
      return false;
    }

    value_valid_to.tv_sec = 0;
    value_valid_to.tv_usec = 0;
  }

  char old_value[SUPLA_CHANNELVALUE_SIZE];  // Because of
                                            // TempHum->toValue(this->value)
                                            // and this->value[0] =
                                            // this->value[0]...

  supla_channel_temphum *OldTempHum = getTempHum();
  supla_channel_temphum *TempHum = NULL;

  memcpy(old_value, this->value, SUPLA_CHANNELVALUE_SIZE);
  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);

  if (Func == SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER ||
      Func == SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW) {
    TDSC_RollerShutterValue *rs_val = (TDSC_RollerShutterValue *)this->value;
    rs_val->bottom_position = Param4;
  } else if ((Func == SUPLA_CHANNELFNC_POWERSWITCH ||
              Func == SUPLA_CHANNELFNC_LIGHTSWITCH ||
              Func == SUPLA_CHANNELFNC_STAIRCASETIMER) &&
             proto_version < 15) {
    // https://forum.supla.org/viewtopic.php?f=6&t=8861
    for (short a = 1; a < SUPLA_CHANNELVALUE_SIZE; a++) {
      this->value[a] = 0;
    }
  } else if (Type == SUPLA_CHANNELTYPE_DIGIGLASS) {
    TDigiglass_Value *dgf_val = (TDigiglass_Value *)this->value;
    dgf_val->sectionCount = Param1;
  } else if (Type == SUPLA_CHANNELTYPE_ELECTRICITY_METER) {
    electricity_meter_config *config =
        new electricity_meter_config(json_config);
    config->add_initial_value((TElectricityMeter_Value *)this->value);
    delete config;
  } else if (Type == SUPLA_CHANNELTYPE_IMPULSE_COUNTER) {
    impulse_counter_config *config = new impulse_counter_config(json_config);

    if (logger_data && !config->should_be_added_to_history()) {
      memcpy(logger_data->value, this->value, SUPLA_CHANNELVALUE_SIZE);
    }

    config->add_initial_value((TDS_ImpulseCounter_Value *)this->value);

    if (logger_data && config->should_be_added_to_history()) {
      memcpy(logger_data->value, this->value, SUPLA_CHANNELVALUE_SIZE);
    }

    delete config;
  } else if (Type == SUPLA_CHANNELTYPE_SENSORNC) {
    this->value[0] = this->value[0] == 0 ? 1 : 0;
  } else {
    TempHum = getTempHum();

    if (TempHum) {
      if ((Param2 != 0 || Param3 != 0)) {
        if (Param2 != 0) {
          TempHum->setTemperature(TempHum->getTemperature() +
                                  (Param2 / 100.00));
        }

        if (Param3 != 0) {
          TempHum->setHumidity(TempHum->getHumidity() + (Param3 / 100.00));
        }

        TempHum->toValue(this->value);
      }
    }
  }

  if (Param3 == 1 && (Type == SUPLA_CHANNELTYPE_SENSORNC ||
                      Type == SUPLA_CHANNELTYPE_SENSORNO)) {
    this->value[0] = this->value[0] == 0 ? 1 : 0;
  }

  if (validity_time_sec) {
    gettimeofday(&value_valid_to, NULL);
    value_valid_to.tv_sec += (*validity_time_sec);

    database *db = new database();

    if (db->connect() == true) {
      db->update_channel_value(getId(), getUserID(), value, *validity_time_sec);
    }

    delete db;
  }

  bool differ = memcmp(this->value, old_value, SUPLA_CHANNELVALUE_SIZE) != 0;

  if (significantChange) {
    if (TempHum || OldTempHum) {
      *significantChange = (TempHum && !OldTempHum) ||
                           (!TempHum && OldTempHum) ||
                           ((int)(TempHum->getHumidity() * 100) !=
                                (int)(OldTempHum->getHumidity() * 100) ||
                            (int)(TempHum->getTemperature() * 100) !=
                                (int)(OldTempHum->getTemperature() * 100));
    } else {
      *significantChange = differ;
    }
  }

  if (TempHum) {
    delete TempHum;
    TempHum = NULL;
  }

  if (OldTempHum) {
    delete OldTempHum;
    OldTempHum = NULL;
  }

  return differ;
}

void supla_device_channel::updateTimerState(void) {
  TTimerState_ExtendedValue *ts_ev = NULL;

  if (extendedValue->type == EV_TYPE_TIMER_STATE_V1) {
    ts_ev = (TTimerState_ExtendedValue *)extendedValue->value;
  } else if (extendedValue->type == EV_TYPE_CHANNEL_AND_TIMER_STATE_V1) {
    TChannelAndTimerState_ExtendedValue *cats =
        (TChannelAndTimerState_ExtendedValue *)extendedValue->value;
    ts_ev = &cats->Timer;
  }

  if (ts_ev == NULL) {
    return;
  }

  if (ts_ev->SenderID) {
    supla_user *user = getUser();
    if (user) {
      shared_ptr<supla_client> client =
          user->get_clients()->get(ts_ev->SenderID);
      if (client != nullptr) {
        client->get_name(ts_ev->SenderName, SUPLA_SENDER_NAME_MAXSIZE);
        ts_ev->SenderNameSize =
            strnlen(ts_ev->SenderName, SUPLA_SENDER_NAME_MAXSIZE) + 1;
      } else {
        ts_ev->SenderID = 0;
        ts_ev->SenderName[0] = 0;
        ts_ev->SenderNameSize = 0;
      }
    }
  }

  if (ts_ev->RemainingTimeMs > 0) {
    struct timeval now;
    gettimeofday(&now, NULL);

    unsigned _supla_int64_t time =
        now.tv_sec * (unsigned _supla_int64_t)1000000 + now.tv_usec;
    time /= 1000;
    time += ts_ev->RemainingTimeMs;
    time /= 1000;

    ts_ev->CountdownEndsAt = time;
  }
}

void supla_device_channel::updateExtendedElectricityMeterValue(void) {
  if (Type != SUPLA_CHANNELTYPE_ELECTRICITY_METER) {
    return;
  }

  TElectricityMeter_ExtendedValue em_ev_v1 = {};
  if (srpc_evtool_v1_extended2emextended(extendedValue, &em_ev_v1)) {
    TElectricityMeter_ExtendedValue_V2 em_ev_v2 = {};

    if (!srpc_evtool_emev_v1to2(&em_ev_v1, &em_ev_v2) ||
        !srpc_evtool_v2_emextended2extended(&em_ev_v2, extendedValue)) {
      delete extendedValue;
      extendedValue = NULL;
    }
  }

  electricity_meter_config *config = new electricity_meter_config(json_config);

  if (config->update_available_counters(extendedValue)) {
    db_set_properties(config);
  }

  if (logger_data && !logger_data->extendedValue) {
    logger_data->extendedValue = new TSuplaChannelExtendedValue();
  }

  if (logger_data && logger_data->extendedValue &&
      !config->should_be_added_to_history()) {
    memcpy(logger_data->extendedValue, extendedValue,
           sizeof(TSuplaChannelExtendedValue));
  }

  config->add_initial_values(Flags, extendedValue);

  if (logger_data && logger_data->extendedValue &&
      config->should_be_added_to_history()) {
    memcpy(logger_data->extendedValue, extendedValue,
           sizeof(TSuplaChannelExtendedValue));
  }

  voltage_analyzers.add_samples(getFlags(), config, extendedValue);

  delete config;
}

void supla_device_channel::setExtendedValue(TSuplaChannelExtendedValue *ev) {
  if (ev == NULL) {
    if (extendedValue != NULL) {
      delete extendedValue;
      extendedValue = NULL;
    }
  } else {
    if (extendedValue == NULL) {
      extendedValue = new TSuplaChannelExtendedValue;
    }
    memcpy(extendedValue, ev, sizeof(TSuplaChannelExtendedValue));

    updateExtendedElectricityMeterValue();
    updateTimerState();
  }
}

void supla_device_channel::assignRgbwValue(char value[SUPLA_CHANNELVALUE_SIZE],
                                           int color, char color_brightness,
                                           char brightness, char on_off) {
  if (Func == SUPLA_CHANNELFNC_DIMMER ||
      Func == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (brightness < 0 || brightness > 100) brightness = 0;

    value[0] = brightness;
  }

  if (Func == SUPLA_CHANNELFNC_RGBLIGHTING ||
      Func == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (color_brightness < 0 || color_brightness > 100) color_brightness = 0;

    value[1] = color_brightness;
    value[2] = (char)((color & 0x000000FF));
    value[3] = (char)((color & 0x0000FF00) >> 8);
    value[4] = (char)((color & 0x00FF0000) >> 16);
  }

  value[5] = on_off;
}

void supla_device_channel::assignCharValue(char value[SUPLA_CHANNELVALUE_SIZE],
                                           char cvalue) {
  memcpy(value, this->value, SUPLA_CHANNELVALUE_SIZE);
  value[0] = cvalue;
}

bool supla_device_channel::isValueWritable(void) {
  switch (Func) {
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

bool supla_device_channel::isCharValueWritable(void) {
  switch (Func) {
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

bool supla_device_channel::isRgbwValueWritable(void) {
  switch (Func) {
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      return 1;

      break;
  }

  return 0;
}

unsigned int supla_device_channel::getValueDuration(void) {
  assert(sizeof(int) == 4 && sizeof(short) == 2);

  switch (Func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      return Param1;

    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      return Param1 * 100;

    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW: {
      unsigned int result = 0;

      result = (unsigned short)Param1;
      result <<= 16;
      result |= (unsigned short)Param3;

      return result;
    }
  }

  return 0;
}

list<int> supla_device_channel::related_channel(void) {
  list<int> result;

  // Only channels associated with NO / NC sensors can return more than one
  // channel!!!
  // See supla_user::get_channel_value

  switch (Func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:

      // The order is important !!!
      // 1st Param2
      // 2nd Param3

      // Always add Param2
      result.push_back(Param2);

      if (Param3) {
        result.push_back(Param3);
      }

      break;
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:

      if (Param1) {
        result.push_back(Param1);
      }

      break;

    case SUPLA_CHANNELFNC_STAIRCASETIMER:

      if (Param2) {
        result.push_back(Param2);
      }

      break;
  }

  return result;
}

list<int> supla_device_channel::master_channel(void) {
  list<int> result;

  switch (Func) {
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:

      if (Param1) {
        result.push_back(Param1);
      }

      if (Param2) {
        result.push_back(Param2);
      }

      break;
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:

      if (Param4) {
        result.push_back(Param4);
      }

      break;
  }

  return result;
}

supla_channel_temphum *supla_device_channel::getTempHum(void) {
  supla_channel_temphum *result = NULL;

  if ((getType() == SUPLA_CHANNELTYPE_THERMOMETERDS18B20 ||
       getType() == SUPLA_CHANNELTYPE_THERMOMETER) &&
      getFunc() == SUPLA_CHANNELFNC_THERMOMETER) {
    result = new supla_channel_temphum(false, getId(), value);

  } else if ((getType() == SUPLA_CHANNELTYPE_DHT11 ||
              getType() == SUPLA_CHANNELTYPE_DHT22 ||
              getType() == SUPLA_CHANNELTYPE_DHT21 ||
              getType() == SUPLA_CHANNELTYPE_AM2301 ||
              getType() == SUPLA_CHANNELTYPE_AM2302 ||
              getType() == SUPLA_CHANNELTYPE_HUMIDITYSENSOR ||
              getType() == SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR ||
              getType() == SUPLA_CHANNELTYPE_BRIDGE) &&
             (getFunc() == SUPLA_CHANNELFNC_THERMOMETER ||
              getFunc() == SUPLA_CHANNELFNC_HUMIDITY ||
              getFunc() == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE)) {
    result = new supla_channel_temphum(true, getId(), value);
  }

  return result;
}

supla_channel_electricity_measurement *
supla_device_channel::getElectricityMeasurement(bool for_data_logger_purposes) {
  if (getFunc() == SUPLA_CHANNELFNC_ELECTRICITY_METER) {
    TSuplaChannelExtendedValue *ev =
        for_data_logger_purposes
            ? (logger_data ? logger_data->extendedValue : NULL)
            : extendedValue;

    if (ev && ev->type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2) {
      TElectricityMeter_ExtendedValue_V2 em_ev;

      if (srpc_evtool_v2_extended2emextended(ev, &em_ev) == 1) {
        return new supla_channel_electricity_measurement(getId(), &em_ev,
                                                         Param2, TextParam1);
      }
    }
  }

  return NULL;
}

supla_channel_ic_measurement *
supla_device_channel::getImpulseCounterMeasurement(
    bool for_data_logger_purposes) {
  switch (getFunc()) {
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER: {
      TDS_ImpulseCounter_Value *ic_val = NULL;
      char value[SUPLA_CHANNELVALUE_SIZE];

      if (for_data_logger_purposes) {
        ic_val =
            logger_data ? (TDS_ImpulseCounter_Value *)logger_data->value : NULL;
      } else {
        getValue(value);
        ic_val = (TDS_ImpulseCounter_Value *)value;
      }

      if (ic_val) {
        return new supla_channel_ic_measurement(
            getId(), Func, ic_val, TextParam1, TextParam2, Param2, Param3);
      }

      break;
    }
  }
  return NULL;
}

supla_channel_thermostat_measurement *
supla_device_channel::getThermostatMeasurement(void) {
  switch (getType()) {
    case SUPLA_CHANNELTYPE_THERMOSTAT:
    case SUPLA_CHANNELTYPE_THERMOSTAT_HEATPOL_HOMEPLUS:
      switch (getFunc()) {
        case SUPLA_CHANNELFNC_THERMOSTAT:
        case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
          char value[SUPLA_CHANNELVALUE_SIZE];
          getValue(value);
          TThermostat_Value *th_val = (TThermostat_Value *)value;

          return new supla_channel_thermostat_measurement(
              getId(), th_val->IsOn > 0, th_val->MeasuredTemperature * 0.01,
              th_val->PresetTemperature * 0.01);
        }
      }
      break;
  }

  return NULL;
}

channel_json_config *supla_device_channel::getJSONConfig(void) {
  if (json_config) {
    return new channel_json_config(json_config, true);
  }

  return NULL;
}

bool supla_device_channel::converValueToExtended(void) {
  bool result = false;

  switch (getFunc()) {
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      char value[SUPLA_CHANNELVALUE_SIZE];
      TSuplaChannelExtendedValue ev;
      TSC_ImpulseCounter_ExtendedValue ic_ev;
      memset(&ic_ev, 0, sizeof(TSC_ImpulseCounter_ExtendedValue));

      getValue(value);

      TDS_ImpulseCounter_Value *ic_val = (TDS_ImpulseCounter_Value *)value;
      ic_ev.counter = ic_val->counter;

      srpc_evtool_v1_icextended2extended(&ic_ev, &ev);

      setExtendedValue(&ev);
      result = true;
      break;
  }
  return result;
}

unsigned int supla_device_channel::get_value_validity_time_left_msec(void) {
  if (value_valid_to.tv_sec || value_valid_to.tv_usec) {
    struct timeval now;
    gettimeofday(&now, NULL);

    unsigned long long now_msec = (now.tv_sec * 1000 + now.tv_usec / 1000);
    unsigned long long valid_to_msec =
        (value_valid_to.tv_sec * 1000 + value_valid_to.tv_usec / 1000);

    if (now_msec < valid_to_msec) {
      return valid_to_msec - now_msec;
    }
  }

  return 0;
}

void supla_device_channel::set_state(TDSC_ChannelState *state) {
  if (this->state == nullptr) {
    this->state = new TDSC_ChannelState();
  }

  *this->state = *state;
}

bool supla_device_channel::get_state(TDSC_ChannelState *state) {
  if (this->state) {
    *state = *this->state;
    return true;
  }

  return false;
}

supla_voltage_analyzers &supla_device_channel::get_voltage_analyzers(void) {
  return voltage_analyzers;
}
