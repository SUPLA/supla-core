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

#include <actions/action_executor.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include "actions/action_gate_openclose.h"
#include "actions/action_trigger.h"
#include "channeljsonconfig/action_trigger_config.h"
#include "channeljsonconfig/electricity_meter_config.h"
#include "channeljsonconfig/impulse_counter_config.h"
#include "database.h"
#include "device/channel_gate_value.h"
#include "device/channel_onoff_value.h"
#include "device/channel_rgbw_value.h"
#include "device/channel_rs_value.h"
#include "device/value_getter.h"
#include "log.h"
#include "safearray.h"
#include "srpc/srpc.h"
#include "user/user.h"

using std::function;
using std::list;

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

  if (validity_time_sec > 0) {
    gettimeofday(&value_valid_to, NULL);
    value_valid_to.tv_sec += validity_time_sec;
  }

  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);

  switch (Type) {
    case SUPLA_CHANNELTYPE_ACTIONTRIGGER:
    case SUPLA_CHANNELTYPE_ELECTRICITY_METER:
    case SUPLA_CHANNELTYPE_IMPULSE_COUNTER:
    case SUPLA_CHANNELTYPE_RELAY:
    case SUPLA_CHANNELTYPE_RELAYHFD4:
    case SUPLA_CHANNELTYPE_RELAYG5LA1A:
    case SUPLA_CHANNELTYPE_2XRELAYG5LA1A:
      json_config = new channel_json_config(NULL);
      if (json_config) {
        json_config->set_properties(properties);
        json_config->set_user_config(user_config);
      }
      break;
  }

  if (Type == SUPLA_CHANNELTYPE_ELECTRICITY_METER ||
      SUPLA_CHANNELTYPE_IMPULSE_COUNTER) {
    logger_data = new _logger_purpose_t();
  }
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

bool supla_device_channel::getExtendedValue(TSuplaChannelExtendedValue *ev) {
  if (ev == NULL) {
    return false;
  }

  if (extendedValue == NULL) {
    memset(ev, 0, sizeof(TSuplaChannelExtendedValue));
    return false;
  }

  memcpy(ev, extendedValue, sizeof(TSuplaChannelExtendedValue));
  return true;
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

void supla_device_channel::getConfig(TSD_ChannelConfig *config,
                                     unsigned char configType,
                                     unsigned _supla_int_t flags) {
  if (configType != SUPLA_CONFIG_TYPE_DEFAULT || flags != 0) {
    return;
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
      std::shared_ptr<supla_client> client =
          user->get_clients()->get(ts_ev->SenderID);
      if (client != nullptr) {
        client->getName(ts_ev->SenderName, SUPLA_SENDER_NAME_MAXSIZE);
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

void supla_device_channel::action_trigger(int actions) {
  supla_action_executor *aexec = new supla_action_executor();
  action_trigger_config *at_config = new action_trigger_config(json_config);
  supla_value_getter *value_getter = new supla_value_getter();

  if (aexec && at_config && value_getter) {
    at_config->set_channel_id_if_subject_not_set(getId());

    supla_action_trigger *trigger =
        new supla_action_trigger(aexec, at_config, value_getter);
    if (trigger) {
      trigger->execute_actions(getId(), getUserID(), actions);
      delete trigger;
    }
  }

  if (value_getter) {
    delete value_getter;
    value_getter = NULL;
  }

  if (aexec) {
    delete aexec;
    aexec = NULL;
  }

  if (at_config) {
    delete at_config;
    at_config = NULL;
  }
}

// ---------------------------------------------
// ---------------------------------------------
// ---------------------------------------------

supla_device_channels::supla_device_channels(supla_device *device) {
  this->device = device;
  this->arr = safe_array_init();
}

supla_device_channels::~supla_device_channels() {
  arr_clean();
  safe_array_free(arr);
}

char supla_device_channels::arr_findcmp(void *ptr, void *id) {
  return ((supla_device_channel *)ptr)->getId() == *((int *)id) ? 1 : 0;
}

char supla_device_channels::arr_findncmp(void *ptr, void *n) {
  return ((supla_device_channel *)ptr)->getNumber() == *((int *)n) ? 1 : 0;
}

char supla_device_channels::arr_delcnd(void *ptr) {
  delete (supla_device_channel *)ptr;
  return 1;
}

void supla_device_channels::arr_clean(void) {
  safe_array_lock(arr);
  safe_array_clean(arr, arr_delcnd);
  safe_array_unlock(arr);
}

void *supla_device_channels::get_srpc(void) {
  if (device && device->get_connection()) {
    return device->get_connection()->get_srpc_adapter()->get_srpc();
  }

  return NULL;
}

supla_device_channel *supla_device_channels::find_channel(int Id) {
  return (supla_device_channel *)safe_array_findcnd(arr, arr_findcmp, &Id);
}

supla_device_channel *supla_device_channels::find_channel_by_number(
    int Number) {
  return (supla_device_channel *)safe_array_findcnd(arr, arr_findncmp, &Number);
}

void supla_device_channels::access_channel(
    int channel_id, function<void(supla_device_channel *)> on_channel) {
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(channel_id);

  if (channel) {
    on_channel(channel);
  }

  safe_array_unlock(arr);
}

void supla_device_channels::add_channel(
    int Id, int Number, int Type, int Func, int Param1, int Param2, int Param3,
    int Param4, const char *TextParam1, const char *TextParam2,
    const char *TextParam3, bool Hidden, unsigned int Flags,
    const char value[SUPLA_CHANNELVALUE_SIZE],
    unsigned _supla_int_t validity_time_sec, const char *user_config,
    const char *properties) {
  safe_array_lock(arr);

  if (find_channel(Id) == 0) {
    supla_device_channel *c = new supla_device_channel(
        device, Id, Number, Type, Func, Param1, Param2, Param3, Param4,
        TextParam1, TextParam2, TextParam3, Hidden, Flags, value,
        validity_time_sec, user_config, properties);

    if (c != NULL && safe_array_add(arr, c) == -1) {
      delete c;
      c = NULL;
    }
  }

  safe_array_unlock(arr);
}

void supla_device_channels::load(int UserID, int DeviceID) {
  database *db = new database();

  if (db->connect() == true) {
    safe_array_lock(arr);
    arr_clean();

    db->get_device_channels(UserID, DeviceID, this);

    safe_array_unlock(arr);
  }

  delete db;
}

bool supla_device_channels::get_channel_value(
    int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE], char *online,
    unsigned _supla_int_t *validity_time_sec, bool for_client) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel) {
      channel->getValue(value);
      if (online) {
        *online = channel->isOffline() ? 0 : 1;
      }

      if (validity_time_sec) {
        *validity_time_sec = channel->getValueValidityTimeSec();
      }

      if (for_client) {
        switch (channel->getFunc()) {
          case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
          case SUPLA_CHANNELFNC_IC_GAS_METER:
          case SUPLA_CHANNELFNC_IC_WATER_METER:
          case SUPLA_CHANNELFNC_IC_HEAT_METER: {
            TDS_ImpulseCounter_Value ds;
            memcpy(&ds, value, sizeof(TDS_ImpulseCounter_Value));
            memset(value, 0, SUPLA_CHANNELVALUE_SIZE);

            TSC_ImpulseCounter_Value sc;
            sc.calculated_value =
                supla_channel_ic_measurement::get_calculated_i(
                    channel->getParam3(), ds.counter);

            memcpy(value, &sc, sizeof(TSC_ImpulseCounter_Value));
            break;
          }
        }
      }

      result = true;
    }

    safe_array_unlock(arr);
  }

  return result;
}

supla_channel_value *supla_device_channels::get_channel_value(int ChannelID) {
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  int func = 0;
  int param2 = 0;
  int param3 = 0;

  access_channel(
      ChannelID,
      [&value, &func, &param2, &param3](supla_device_channel *channel) -> void {
        func = channel->getFunc();
        param2 = channel->getParam2();
        param3 = channel->getParam3();
        channel->getValue(value);
      });

  if (!func) {
    return NULL;
  }

  switch (func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      return new supla_channel_rs_value(value);

    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR: {
      supla_channel_gate_value *gate_value =
          new supla_channel_gate_value(value);

      gate_value->update_sensors(device->get_user(), param2, param3);
      return gate_value;
    }
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      return new supla_channel_onoff_value(value);

    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      return new supla_channel_rgbw_value(value);
  }

  return new supla_channel_value(value);
}

bool supla_device_channels::get_channel_extendedvalue(
    int ChannelID, TSuplaChannelExtendedValue *value) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel) {
      result = channel->getExtendedValue(value);
    }

    safe_array_unlock(arr);
  }

  return result;
}

bool supla_device_channels::get_channel_extendedvalue(
    int ChannelID, TSC_SuplaChannelExtendedValue *cev) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel) {
      result = channel->getExtendedValue(&cev->value);
      if (result) {
        cev->Id = channel->getId();

        switch (cev->value.type) {
          case EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2:
            result = supla_channel_electricity_measurement::update_cev(
                cev, channel->getParam2(), channel->getTextParam1());
            break;
          case EV_TYPE_IMPULSE_COUNTER_DETAILS_V1:
            result = supla_channel_ic_measurement::update_cev(
                cev, channel->getFunc(), channel->getParam2(),
                channel->getParam3(), channel->getTextParam1(),
                channel->getTextParam2());
            break;
        }
      }
    }

    safe_array_unlock(arr);
  }

  return result;
}

bool supla_device_channels::get_channel_double_value(int ChannelID,
                                                     double *Value) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel) {
      channel->getDouble(Value);
      result = true;
    }

    safe_array_unlock(arr);
  }

  return result;
}

supla_channel_temphum *
supla_device_channels::get_channel_temp_and_humidity_value(int ChannelID) {
  supla_channel_temphum *result = NULL;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel) {
      result = channel->getTempHum();
    }

    safe_array_unlock(arr);
  }

  return result;
}

bool supla_device_channels::get_channel_temperature_value(int ChannelID,
                                                          double *Value) {
  supla_channel_temphum *result =
      get_channel_temp_and_humidity_value(ChannelID);
  if (result) {
    *Value = result->getTemperature();
    delete result;
    return true;
  }

  return false;
}

bool supla_device_channels::get_channel_humidity_value(int ChannelID,
                                                       double *Value) {
  supla_channel_temphum *result =
      get_channel_temp_and_humidity_value(ChannelID);
  if (result) {
    if (result->isTempAndHumidity() == 1) {
      *Value = result->getHumidity();
      delete result;
      return true;

    } else {
      delete result;
    }
  }

  return false;
}

bool supla_device_channels::get_channel_char_value(int ChannelID, char *Value) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel) {
      channel->getChar(Value);
      result = true;
    }

    safe_array_unlock(arr);
  }

  return result;
}

bool supla_device_channels::get_channel_valve_value(int ChannelID,
                                                    TValve_Value *Value) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    result = channel && channel->getValveValue(Value);

    safe_array_unlock(arr);
  }

  return result;
}

bool supla_device_channels::get_dgf_transparency(int ChannelID,
                                                 unsigned short *mask) {
  bool result = false;

  if (mask && ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel &&
        (channel->getFunc() == SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL ||
         channel->getFunc() == SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL)) {
      char value[SUPLA_CHANNELVALUE_SIZE];
      channel->getValue(value);
      TDigiglass_Value *dgf_val = (TDigiglass_Value *)value;
      *mask = dgf_val->mask;
      result = true;
    }

    safe_array_unlock(arr);
  }

  return result;
}

bool supla_device_channels::get_relay_value(int ChannelID,
                                            TRelayChannel_Value *relay_value) {
  bool result = false;

  if (relay_value && ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel && (channel->getFunc() == SUPLA_CHANNELFNC_POWERSWITCH ||
                    channel->getFunc() == SUPLA_CHANNELFNC_LIGHTSWITCH ||
                    channel->getFunc() == SUPLA_CHANNELFNC_STAIRCASETIMER)) {
      char value[SUPLA_CHANNELVALUE_SIZE];
      channel->getValue(value);
      memcpy(relay_value, value, sizeof(TRelayChannel_Value));
      result = true;
    }

    safe_array_unlock(arr);
  }

  return result;
}

bool supla_device_channels::reset_counters(int ChannelID) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel &&
        (channel->getFlags() & SUPLA_CHANNEL_FLAG_CALCFG_RESET_COUNTERS)) {
      TSD_DeviceCalCfgRequest request = {};

      request.ChannelNumber = channel->getNumber();
      request.Command = SUPLA_CALCFG_CMD_RESET_COUNTERS;
      request.SuperUserAuthorized = true;

      srpc_sd_async_device_calcfg_request(get_srpc(), &request);
      result = true;
    }

    safe_array_unlock(arr);
  }

  return result;
}

bool supla_device_channels::recalibrate(int ChannelID,
                                        const supla_caller &caller,
                                        bool SuperUserAuthorized) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel &&
        (channel->getFlags() & SUPLA_CHANNEL_FLAG_CALCFG_RECALIBRATE)) {
      TSD_DeviceCalCfgRequest request = {};

      request.ChannelNumber = channel->getNumber();
      request.Command = SUPLA_CALCFG_CMD_RECALIBRATE;
      request.SenderID = caller.convert_to_sender_id();
      request.SuperUserAuthorized = SuperUserAuthorized;

      TCalCfg_RollerShutterSettings *settings =
          (TCalCfg_RollerShutterSettings *)request.Data;
      request.DataSize = sizeof(TCalCfg_RollerShutterSettings);
      request.DataType = SUPLA_CALCFG_DATATYPE_RS_SETTINGS;

      settings->FullOpeningTimeMS = channel->getParam1() * 100;
      settings->FullClosingTimeMS = channel->getParam3() * 100;

      srpc_sd_async_device_calcfg_request(get_srpc(), &request);
      result = true;
    }

    safe_array_unlock(arr);
  }

  return result;
}

bool supla_device_channels::set_channel_value(
    supla_device_channel *channel, char value[SUPLA_CHANNELVALUE_SIZE],
    bool *converted2extended, const unsigned _supla_int_t *validity_time_sec,
    bool *significantChange) {
  if (!channel) return false;
  bool result = false;

  if (converted2extended) {
    *converted2extended = false;
  }

  unsigned char proto_version = srpc_get_proto_version(get_srpc());

  safe_array_lock(arr);

  result = channel->setValue(value, validity_time_sec, significantChange,
                             proto_version);

  if (channel->converValueToExtended()) {
    if (converted2extended) {
      *converted2extended = true;
    }
  }

  safe_array_unlock(arr);
  return result;
}

bool supla_device_channels::set_channel_value(
    int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE],
    bool *converted2extended, const unsigned _supla_int_t *validity_time_sec,
    bool *significantChange) {
  return ChannelID &&
         set_channel_value(find_channel(ChannelID), value, converted2extended,
                           validity_time_sec, significantChange);
}

bool supla_device_channels::set_channel_offline(int ChannelID, bool Offline) {
  if (ChannelID == 0) return false;
  bool result = false;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) {
    result = channel->setOffline(Offline);
  }

  safe_array_unlock(arr);
  return result;
}

void supla_device_channels::set_channel_extendedvalue(
    int ChannelID, TSuplaChannelExtendedValue *ev) {
  if (ChannelID == 0) return;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) channel->setExtendedValue(ev);

  safe_array_unlock(arr);
}

unsigned int supla_device_channels::get_channel_value_duration(int ChannelID) {
  if (ChannelID == 0) return 0;

  int Duration = 0;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) Duration = channel->getValueDuration();

  safe_array_unlock(arr);

  return Duration;
}

int supla_device_channels::get_channel_func(int ChannelID) {
  if (ChannelID == 0) return 0;

  int Func = 0;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) Func = channel->getFunc();

  safe_array_unlock(arr);

  return Func;
}

int supla_device_channels::get_channel_type(int ChannelID) {
  if (ChannelID == 0) return 0;

  int Type = 0;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) Type = channel->getType();

  safe_array_unlock(arr);

  return Type;
}

list<int> supla_device_channels::mr_channel(int ChannelID, bool Master) {
  list<int> result;
  if (ChannelID == 0) return result;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel)
    result = Master ? channel->master_channel() : channel->related_channel();

  safe_array_unlock(arr);

  return result;
}

list<int> supla_device_channels::master_channel(int ChannelID) {
  return mr_channel(ChannelID, true);
}

list<int> supla_device_channels::related_channel(int ChannelID) {
  return mr_channel(ChannelID, false);
}

bool supla_device_channels::channel_exists(int ChannelID) {
  bool result = false;

  safe_array_lock(arr);

  if (find_channel(ChannelID) != NULL) result = true;

  safe_array_unlock(arr);

  return result;
}

bool supla_device_channels::is_channel_online(int ChannelID) {
  bool result = false;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) {
    result = !channel->isOffline();
  }

  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::update_channels(
    TDS_SuplaDeviceChannel_B *schannel_b, TDS_SuplaDeviceChannel_C *schannel_c,
    int count) {
  for (int a = 0; a < count; a++) {
    int type = 0;
    char *value = NULL;
    unsigned char number = 0;
    unsigned int actionTriggerCaps = 0;
    unsigned int flags = 0;
    TActionTriggerProperties atProps = {};

    if (schannel_b != NULL) {
      type = schannel_b[a].Type;
      value = schannel_b[a].value;
      number = schannel_b[a].Number;
    } else {
      type = schannel_c[a].Type;
      value = schannel_c[a].value;
      number = schannel_c[a].Number;
      actionTriggerCaps = schannel_c[a].ActionTriggerCaps;
      atProps = schannel_c[a].actionTriggerProperties;
      flags = schannel_c[a].Flags;
    }

    int channelId = get_channel_id(number);

    safe_array_lock(arr);

    supla_device_channel *channel = find_channel(channelId);

    if (channel) {
      set_channel_value(channelId, value, NULL, 0, NULL);

      channel->addFlags(flags);

      if (type == SUPLA_CHANNELTYPE_ACTIONTRIGGER) {
        int actionTriggerRelatedChannelId = 0;
        if (atProps.relatedChannelNumber > 0) {
          actionTriggerRelatedChannelId =
              get_channel_id(atProps.relatedChannelNumber - 1);
        }
        channel->setActionTriggerConfig(actionTriggerCaps,
                                        actionTriggerRelatedChannelId,
                                        atProps.disablesLocalOperation);
      }
    }

    safe_array_unlock(arr);
  }
}

void supla_device_channels::on_device_registered(
    supla_user *user, int DeviceId, TDS_SuplaDeviceChannel_B *schannel_b,
    TDS_SuplaDeviceChannel_C *schannel_c, int count) {
  int ChannelId = 0;

  for (int a = 0; a < count; a++) {
    if (schannel_b != NULL) {
      ChannelId = get_channel_id(schannel_b[a].Number);
    } else {
      ChannelId = get_channel_id(schannel_c[a].Number);
    }

    if (ChannelId > 0) {
      user->on_channel_become_online(DeviceId, ChannelId);
    }
  }
}

int supla_device_channels::get_channel_id(unsigned char ChannelNumber) {
  int result = 0;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel_by_number(ChannelNumber);

  if (channel) result = channel->getId();

  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::async_set_channel_value(
    supla_device_channel *channel, const supla_caller &caller, int GroupID,
    unsigned char EOL, const char value[SUPLA_CHANNELVALUE_SIZE],
    unsigned int durationMS, bool cancelTasks) {
  if (cancelTasks) {
    switch (channel->getFunc()) {
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
        supla_action_gate_openclose::cancel_tasks(
            device->get_user_id(), device->get_id(), channel->getId());
        break;
    }
  }

  if (GroupID && srpc_get_proto_version(get_srpc()) >= 13) {
    TSD_SuplaChannelGroupNewValue s;
    memset(&s, 0, sizeof(TSD_SuplaChannelGroupNewValue));

    s.ChannelNumber = channel->getNumber();
    s.DurationMS = durationMS;
    s.SenderID = caller.convert_to_sender_id();
    s.GroupID = GroupID;
    s.EOL = EOL;
    memcpy(s.value, value, SUPLA_CHANNELVALUE_SIZE);

    srpc_sd_async_set_channelgroup_value(get_srpc(), &s);
  } else {
    TSD_SuplaChannelNewValue s;
    memset(&s, 0, sizeof(TSD_SuplaChannelNewValue));

    s.ChannelNumber = channel->getNumber();
    s.DurationMS = durationMS;
    s.SenderID = GroupID ? 0 : caller.convert_to_sender_id();
    memcpy(s.value, value, SUPLA_CHANNELVALUE_SIZE);

    srpc_sd_async_set_channel_value(get_srpc(), &s);
  }
}

void supla_device_channels::async_set_channel_value(
    supla_device_channel *channel, const supla_caller &caller, int GroupID,
    unsigned char EOL, const char value[SUPLA_CHANNELVALUE_SIZE],
    bool cancelTasks) {
  async_set_channel_value(channel, caller, GroupID, EOL, value,
                          channel->getValueDuration(), cancelTasks);
}

void supla_device_channels::set_device_channel_value(
    const supla_caller &caller, int ChannelID, int GroupID, unsigned char EOL,
    const char value[SUPLA_CHANNELVALUE_SIZE]) {
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel && channel->isValueWritable()) {
    async_set_channel_value(channel, caller, GroupID, EOL, value);
  }

  safe_array_unlock(arr);
}

bool supla_device_channels::set_device_channel_char_value(
    const supla_caller &caller, supla_device_channel *channel, int GroupID,
    unsigned char EOL, const char value, bool cancelTasks) {
  bool result = false;

  if (channel) {
    if (channel->isRgbwValueWritable()) {
      int color = 0;
      char color_brightness = 0;
      char brightness = 0;
      char on_off = 0;

      if (channel->getRGBW(&color, &color_brightness, &brightness, &on_off)) {
        if (value > 0) {
          color_brightness = 100;
          brightness = 100;
        } else {
          color_brightness = 0;
          brightness = 0;
        }

        on_off = RGBW_BRIGHTNESS_ONOFF | RGBW_COLOR_ONOFF;

        result = set_device_channel_rgbw_value(
            caller, channel->getId(), GroupID, EOL, color, color_brightness,
            brightness, on_off);
      }
    } else if (channel->isCharValueWritable()) {
      char v[SUPLA_CHANNELVALUE_SIZE];
      memset(v, 0, SUPLA_CHANNELVALUE_SIZE);
      channel->assignCharValue(v, value);

      async_set_channel_value(channel, caller, GroupID, EOL, v, cancelTasks);

      result = true;
    }
  }

  return result;
}

bool supla_device_channels::set_device_channel_char_value(
    const supla_caller &caller, int ChannelID, int GroupID, unsigned char EOL,
    const char value) {
  safe_array_lock(arr);

  bool result = set_device_channel_char_value(caller, find_channel(ChannelID),
                                              GroupID, EOL, value);

  safe_array_unlock(arr);

  return result;
}

bool supla_device_channels::set_device_channel_rgbw_value(
    const supla_caller &caller, int ChannelID, int GroupID, unsigned char EOL,
    int color, char color_brightness, char brightness, char on_off) {
  bool result = false;
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel && channel->isRgbwValueWritable()) {
    char v[SUPLA_CHANNELVALUE_SIZE];
    memset(v, 0, SUPLA_CHANNELVALUE_SIZE);

    if (on_off) {
      char mask = 0;
      if (channel->getFunc() == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
        mask = RGBW_COLOR_ONOFF | RGBW_BRIGHTNESS_ONOFF;
      } else if (channel->getFunc() == SUPLA_CHANNELFNC_DIMMER) {
        mask = RGBW_BRIGHTNESS_ONOFF;
      } else if (channel->getFunc() == SUPLA_CHANNELFNC_RGBLIGHTING) {
        mask = RGBW_COLOR_ONOFF;
      }

      on_off = mask & on_off;
    }

    channel->assignRgbwValue(v, color, color_brightness, brightness, on_off);

    async_set_channel_value(channel, caller, GroupID, EOL, v);

    result = true;
  }

  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::get_temp_and_humidity(void *tarr) {
  int a;
  safe_array_lock(arr);

  for (a = 0; a < safe_array_count(arr); a++) {
    supla_device_channel *channel =
        (supla_device_channel *)safe_array_get(arr, a);

    if (channel != NULL && !channel->isOffline()) {
      supla_channel_temphum *temphum = channel->getTempHum();

      if (temphum != NULL) safe_array_add(tarr, temphum);
    }
  }

  safe_array_unlock(arr);
}

bool supla_device_channels::get_channel_rgbw_value(int ChannelID, int *color,
                                                   char *color_brightness,
                                                   char *brightness,
                                                   char *on_off) {
  bool result = false;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel != NULL) {
    int _color = 0;
    char _color_brightness = 0;
    char _brightness = 0;
    char _on_off = 0;

    result =
        channel->getRGBW(&_color, &_color_brightness, &_brightness, &_on_off);

    if (result == true) {
      if (color != NULL) *color = _color;

      if (color_brightness) *color_brightness = _color_brightness;

      if (brightness != NULL) *brightness = _brightness;

      if (on_off != NULL) *on_off = _on_off;
    }
  }

  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::get_electricity_measurements(
    void *emarr, bool for_data_logger_purposes) {
  int a;
  safe_array_lock(arr);

  // TODO(anyone): Remove
  struct timeval now;
  gettimeofday(&now, NULL);
  // ---

  for (a = 0; a < safe_array_count(arr); a++) {
    supla_device_channel *channel =
        (supla_device_channel *)safe_array_get(arr, a);

    // TODO(anyone): Remove
    if (channel->getFunc() == SUPLA_CHANNELFNC_ELECTRICITY_METER) {
      if (channel->isOffline()) {
        supla_log(LOG_WARNING, "Electricity Meter is offline. Channel Id: %i",
                  channel->getId());
      }
    }
    // ----

    if (channel != NULL && !channel->isOffline()) {
      supla_channel_electricity_measurement *em =
          channel->getElectricityMeasurement(for_data_logger_purposes);
      if (em) {
        safe_array_add(emarr, em);
      } else {
        // TODO(anyone): Remove
        if (channel->getFunc() == SUPLA_CHANNELFNC_ELECTRICITY_METER &&
            channel->isOffline()) {
          supla_log(LOG_WARNING,
                    "No electricity meter data available. Channel Id: %i",
                    channel->getId());
        }
        // ----
      }
    }
  }

  safe_array_unlock(arr);
}

supla_channel_electricity_measurement *
supla_device_channels::get_electricity_measurement(int ChannelID) {
  supla_channel_electricity_measurement *result = NULL;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel != NULL) {
    result = channel->getElectricityMeasurement(false);
  }

  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::get_ic_measurements(void *icarr,
                                                bool for_data_logger_purposes) {
  int a;
  safe_array_lock(arr);

  for (a = 0; a < safe_array_count(arr); a++) {
    supla_device_channel *channel =
        (supla_device_channel *)safe_array_get(arr, a);

    if (channel != NULL && !channel->isOffline()) {
      supla_channel_ic_measurement *ic =
          channel->getImpulseCounterMeasurement(for_data_logger_purposes);
      if (ic) {
        safe_array_add(icarr, ic);
      }
    }
  }

  safe_array_unlock(arr);
}

void supla_device_channels::get_thermostat_measurements(void *tharr) {
  int a;
  safe_array_lock(arr);

  for (a = 0; a < safe_array_count(arr); a++) {
    supla_device_channel *channel =
        (supla_device_channel *)safe_array_get(arr, a);

    if (channel != NULL && !channel->isOffline()) {
      supla_channel_thermostat_measurement *th =
          channel->getThermostatMeasurement();
      if (th) {
        safe_array_add(tharr, th);
      }
    }
  }

  safe_array_unlock(arr);
}

supla_channel_ic_measurement *supla_device_channels::get_ic_measurement(
    int ChannelID) {
  supla_channel_ic_measurement *result = NULL;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel != NULL) {
    result = channel->getImpulseCounterMeasurement(false);
  }

  safe_array_unlock(arr);

  return result;
}

bool supla_device_channels::calcfg_request(const supla_caller &caller,
                                           int ChannelID,
                                           bool SuperUserAuthorized,
                                           TCS_DeviceCalCfgRequest_B *request) {
  if (request == NULL) {
    return false;
  }

  if (request->Command == SUPLA_CALCFG_CMD_RECALIBRATE) {
    return recalibrate(ChannelID, caller, SuperUserAuthorized);
  }

  bool result = false;
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) {
    TSD_DeviceCalCfgRequest drequest;
    memset(&drequest, 0, sizeof(TSD_DeviceCalCfgRequest));

    drequest.SenderID = caller.convert_to_sender_id();
    drequest.ChannelNumber = channel->getNumber();
    drequest.Command = request->Command;
    drequest.SuperUserAuthorized = SuperUserAuthorized;
    drequest.DataType = request->DataType;
    drequest.DataSize = request->DataSize > SUPLA_CALCFG_DATA_MAXSIZE
                            ? SUPLA_CALCFG_DATA_MAXSIZE
                            : request->DataSize;
    memcpy(drequest.Data, request->Data, SUPLA_CALCFG_DATA_MAXSIZE);

    srpc_sd_async_device_calcfg_request(get_srpc(), &drequest);
    result = true;
  }

  safe_array_unlock(arr);

  return result;
}

bool supla_device_channels::get_channel_state(
    const supla_caller &caller, TCSD_ChannelStateRequest *request) {
  if (request == NULL) {
    return false;
  }

  bool result = false;
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(request->ChannelID);

  if (channel) {
    TCSD_ChannelStateRequest drequest;
    memcpy(&drequest, request, sizeof(TCSD_ChannelStateRequest));

    drequest.SenderID = caller.convert_to_sender_id();
    drequest.ChannelNumber = channel->getNumber();

    srpc_csd_async_get_channel_state(get_srpc(), &drequest);
    result = true;
  }

  safe_array_unlock(arr);

  return result;
}

bool supla_device_channels::get_channel_complex_value(
    channel_complex_value *value, int ChannelID) {
  bool result = false;

  memset(value, 0, sizeof(channel_complex_value));

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) {
    value->online = !channel->isOffline();
    value->hidden_channel = channel->getHidden();
    value->function = channel->getFunc();
    value->channel_type = channel->getType();
    value->channel_flags = channel->getFlags();

    switch (value->function) {
      case SUPLA_CHANNELFNC_THERMOMETER:
      case SUPLA_CHANNELFNC_HUMIDITY:
      case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE: {
        supla_channel_temphum *tempHum = channel->getTempHum();
        if (tempHum) {
          value->temperature = tempHum->getTemperature();
          value->humidity = tempHum->getHumidity();
          delete tempHum;
        }
      } break;

      case SUPLA_CHANNELFNC_WINDSENSOR:
        channel->getDouble(&value->wind);
        break;
      case SUPLA_CHANNELFNC_PRESSURESENSOR:
        channel->getDouble(&value->pressure);
        break;
      case SUPLA_CHANNELFNC_RAINSENSOR:
        channel->getDouble(&value->rain);
        break;
      case SUPLA_CHANNELFNC_WEIGHTSENSOR:
        channel->getDouble(&value->weight);
        break;
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
      case SUPLA_CHANNELFNC_MAILSENSOR:
      case SUPLA_CHANNELFNC_NOLIQUIDSENSOR: {
        char cv[SUPLA_CHANNELVALUE_SIZE];
        channel->getChar(cv);
        value->hi = cv[0] > 0;
      } break;

      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER: {
        TRelayChannel_Value relay_value = {};
        if (get_relay_value(ChannelID, &relay_value)) {
          value->hi = relay_value.hi > 0;
          value->overcurrent_relay_off =
              relay_value.flags & SUPLA_RELAY_FLAG_OVERCURRENT_RELAY_OFF;
        }
      } break;

      case SUPLA_CHANNELFNC_DIMMER:
      case SUPLA_CHANNELFNC_RGBLIGHTING:
      case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
        channel->getRGBW(&value->color, &value->color_brightness,
                         &value->brightness, &value->on_off);
        break;

      case SUPLA_CHANNELFNC_DEPTHSENSOR:
        channel->getDouble(&value->depth);
        break;
      case SUPLA_CHANNELFNC_DISTANCESENSOR:
        channel->getDouble(&value->distance);
        break;
      case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR: {
        switch (value->function) {
          case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
          case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
            char cv[SUPLA_CHANNELVALUE_SIZE];
            channel->getChar(cv);
            value->shut = cv[0];
            break;
        }

        // TODO(anyone): This should be better implemented. Right now, that's
        // spaghetti code
        value->hi = false;
        value->partially_closed = false;

        safe_array_unlock(arr);  // Unlock the array to avoid thread deadlock.
                                 // Do not refer to the channel from here.

        supla_user *user = supla_user::find(device->get_user_id(), false);
        if (user) {
          TSuplaChannelValue cv;
          memset(&cv, 0, sizeof(TSuplaChannelValue));

          if (user->get_channel_value(device->get_id(), ChannelID, cv.value,
                                      cv.sub_value, NULL, NULL, NULL, false)) {
            if (cv.sub_value[0] > 0) {
              value->hi = true;
            } else if ((value->function ==
                            SUPLA_CHANNELFNC_CONTROLLINGTHEGATE ||
                        value->function ==
                            SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR) &&
                       cv.sub_value[1] > 0 && cv.sub_value[0] == 0) {
              value->partially_closed = true;
            }
          }
        }
      }
        return true;
      case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
      case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
        channel->getValveValue(&value->valve_value);
        break;
    }
    result = true;
  }

  safe_array_unlock(arr);

  return result;
}

list<int> supla_device_channels::get_channel_ids(void) {
  list<int> result;
  safe_array_lock(arr);
  for (int a = 0; a < safe_array_count(arr); a++) {
    supla_device_channel *channel =
        static_cast<supla_device_channel *>(safe_array_get(arr, a));
    if (channel) {
      result.push_back(channel->getId());
    }
  }
  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::set_channel_function(int ChannelId, int Func) {
  supla_device_channel *channel = find_channel(ChannelId);

  if (channel) {
    channel->setFunc(Func);
  }
}

void supla_device_channels::get_functions_request(void) {
  TSD_ChannelFunctions result;
  memset(&result, 0, sizeof(TSD_ChannelFunctions));

  safe_array_lock(arr);
  for (int a = 0; a < safe_array_count(arr); a++) {
    supla_device_channel *channel =
        static_cast<supla_device_channel *>(safe_array_get(arr, a));
    if (channel && channel->getNumber() >= 0 &&
        channel->getNumber() < SUPLA_CHANNELMAXCOUNT) {
      if (result.ChannelCount >= channel->getNumber()) {
        result.ChannelCount = channel->getNumber() + 1;
      }

      result.Functions[channel->getNumber()] = channel->getFunc();
    }
  }
  safe_array_unlock(arr);

  srpc_sd_async_get_channel_functions_result(get_srpc(), &result);
}

void supla_device_channels::get_channel_config_request(
    TDS_GetChannelConfigRequest *request) {
  if (request == NULL) {
    return;
  }

  safe_array_lock(arr);

  supla_device_channel *channel =
      find_channel_by_number(request->ChannelNumber);

  if (channel) {
    TSD_ChannelConfig config = {};
    channel->getConfig(&config, request->ConfigType, request->Flags);
    srpc_sd_async_get_channel_config_result(get_srpc(), &config);
  }

  safe_array_unlock(arr);
}

void supla_device_channels::action_trigger(TDS_ActionTrigger *at) {
  if (!at) {
    return;
  }

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel_by_number(at->ChannelNumber);
  if (channel) {
    channel->action_trigger(at->ActionTrigger);
  }

  safe_array_unlock(arr);
}

bool supla_device_channels::set_on(const supla_caller &caller, int ChannelID,
                                   int GroupID, unsigned char EOL, bool on,
                                   bool toggle) {
  safe_array_lock(arr);

  bool result = false;
  supla_device_channel *channel = find_channel(ChannelID);
  if (channel) {
    switch (channel->getFunc()) {
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER:
      case SUPLA_CHANNELFNC_THERMOSTAT:
      case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
        char c = on ? 1 : 0;
        if (toggle) {
          channel->getChar(&c);
          c = c ? 0 : 1;
        }
        result =
            set_device_channel_char_value(caller, channel, GroupID, EOL, c);
        break;
      }

      case SUPLA_CHANNELFNC_DIMMER:
      case SUPLA_CHANNELFNC_RGBLIGHTING:
      case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING: {
        int color = 0;
        char color_brightness = 0;
        char brightness = 0;
        char on_off = 0;

        if (channel->getRGBW(&color, &color_brightness, &brightness, &on_off)) {
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
              caller, channel->getId(), GroupID, EOL, color, color_brightness,
              brightness, on_off);
        }
        break;
      }
    }
  }

  safe_array_unlock(arr);

  return result;
}

bool supla_device_channels::is_on(int ChannelID) {
  bool result = false;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);
  if (channel) {
    switch (channel->getFunc()) {
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER:
      case SUPLA_CHANNELFNC_THERMOSTAT:
      case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
        char c = 0;
        channel->getChar(&c);
        result = c;
        break;
      }

      case SUPLA_CHANNELFNC_DIMMER:
      case SUPLA_CHANNELFNC_RGBLIGHTING:
      case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING: {
        int color = 0;
        char color_brightness = 0;
        char brightness = 0;
        char on_off = 0;

        if (channel->getRGBW(&color, &color_brightness, &brightness, &on_off)) {
          result = color_brightness > 0 || brightness > 0;
        }
        break;
      }
    }
  }

  safe_array_unlock(arr);

  return result;
}

bool supla_device_channels::set_on(const supla_caller &caller, int ChannelID,
                                   int GroupID, unsigned char EOL, bool on) {
  return set_on(caller, ChannelID, GroupID, EOL, on, false);
}

bool supla_device_channels::set_rgbw(const supla_caller &caller, int ChannelID,
                                     int GroupID, unsigned char EOL,
                                     unsigned int *color,
                                     char *color_brightness, char *brightness,
                                     char *on_off) {
  safe_array_lock(arr);

  bool result = false;
  supla_device_channel *channel = find_channel(ChannelID);
  if (channel && channel->isRgbwValueWritable()) {
    int _color = 0;
    char _color_brightness = 0;
    char _brightness = 0;
    char _on_off = 0;

    if (channel->getRGBW(&_color, &_color_brightness, &_brightness, &_on_off)) {
      result = set_device_channel_rgbw_value(
          caller, ChannelID, GroupID, EOL, color ? *color : _color,
          color_brightness ? *color_brightness : _color_brightness,
          brightness ? *brightness : _brightness, on_off ? *on_off : 0);
    }
  }

  safe_array_unlock(arr);
  return result;
}

bool supla_device_channels::set_color(const supla_caller &caller, int ChannelID,
                                      int GroupID, unsigned char EOL,
                                      unsigned int color) {
  return set_rgbw(caller, ChannelID, GroupID, EOL, &color, NULL, NULL, NULL);
}

bool supla_device_channels::set_color_brightness(const supla_caller &caller,
                                                 int ChannelID, int GroupID,
                                                 unsigned char EOL,
                                                 char brightness) {
  return set_rgbw(caller, ChannelID, GroupID, EOL, NULL, &brightness, NULL,
                  NULL);
}

bool supla_device_channels::set_brightness(const supla_caller &caller,
                                           int ChannelID, int GroupID,
                                           unsigned char EOL, char brightness) {
  return set_rgbw(caller, ChannelID, GroupID, EOL, NULL, NULL, &brightness,
                  NULL);
}

bool supla_device_channels::set_dgf_transparency(const supla_caller &caller,
                                                 int ChannelID,
                                                 unsigned short activeBits,
                                                 unsigned short mask) {
  safe_array_lock(arr);

  bool result = false;
  supla_device_channel *channel = find_channel(ChannelID);
  if (channel && (channel->getFunc() == SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL ||
                  channel->getFunc() == SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL)) {
    unsigned short scope = (unsigned short)(pow(2, channel->getParam1()) - 1);
    activeBits &= scope;
    mask &= scope;

    char value[SUPLA_CHANNELVALUE_SIZE];
    memset(value, 0, SUPLA_CHANNELVALUE_SIZE);

    TCSD_Digiglass_NewValue *new_value = (TCSD_Digiglass_NewValue *)value;
    new_value->active_bits = activeBits;
    new_value->mask = mask;
    async_set_channel_value(channel, caller, 0, 0, value);
    result = true;
  }

  safe_array_unlock(arr);
  return result;
}

bool supla_device_channels::action_toggle(const supla_caller &caller,
                                          int ChannelID, int GroupID,
                                          unsigned char EOL) {
  return set_on(caller, ChannelID, GroupID, EOL, false, true);
}

bool supla_device_channels::rs_action(const supla_caller &caller, int ChannelID,
                                      int GroupID, unsigned char EOL,
                                      rsAction action,
                                      const char *closingPercentage) {
  bool result = false;
  access_channel(ChannelID,
                 [&result, this, caller, GroupID, EOL, action,
                  closingPercentage](supla_device_channel *channel) -> void {
                   char v = -1;

                   switch (channel->getFunc()) {
                     case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
                     case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:

                       switch (action) {
                         case rsActionStop:
                           v = 0;
                           break;
                         case rsActionDown:
                           v = 1;
                           break;
                         case rsActionUp:
                           v = 2;
                           break;
                         case rsActionDownOrStop:
                           if (channel->getFlags() &
                               SUPLA_CHANNEL_FLAG_RS_SBS_AND_STOP_ACTIONS) {
                             v = 3;
                           }
                           break;
                         case rsActionUpOrStop:
                           if (channel->getFlags() &
                               SUPLA_CHANNEL_FLAG_RS_SBS_AND_STOP_ACTIONS) {
                             v = 4;
                           }
                           break;
                         case rsActionStepByStep:
                           if (channel->getFlags() &
                               SUPLA_CHANNEL_FLAG_RS_SBS_AND_STOP_ACTIONS) {
                             v = 5;
                           }
                           break;
                         case rsActionShut:
                           if (closingPercentage) {
                             if (*closingPercentage > 100) {
                               v = 110;
                             } else if (*closingPercentage < 0) {
                               v = 10;
                             } else {
                               v = *closingPercentage + 10;
                             }
                           } else {
                             v = 110;
                           }
                           break;
                         case rsActionReveal:
                           v = 10;
                           break;
                       }
                       if (v >= 0 && v <= 110) {
                         result = set_device_channel_char_value(
                             caller, channel, GroupID, EOL, v);
                       }
                       break;
                   }
                 });

  return result;
}

bool supla_device_channels::action_shut(const supla_caller &caller,
                                        int ChannelID, int GroupID,
                                        unsigned char EOL,
                                        const char *closingPercentage) {
  return rs_action(caller, ChannelID, GroupID, EOL, rsActionShut,
                   closingPercentage);
}

bool supla_device_channels::action_reveal(const supla_caller &caller,
                                          int ChannelID, int GroupID,
                                          unsigned char EOL) {
  return rs_action(caller, ChannelID, GroupID, EOL, rsActionReveal, NULL);
}

bool supla_device_channels::action_stop(const supla_caller &caller,
                                        int ChannelID, int GroupID,
                                        unsigned char EOL) {
  return rs_action(caller, ChannelID, GroupID, EOL, rsActionStop, NULL);
}

bool supla_device_channels::action_up(const supla_caller &caller, int ChannelID,
                                      int GroupID, unsigned char EOL) {
  return rs_action(caller, ChannelID, GroupID, EOL, rsActionUp, NULL);
}

bool supla_device_channels::action_down(const supla_caller &caller,
                                        int ChannelID, int GroupID,
                                        unsigned char EOL) {
  return rs_action(caller, ChannelID, GroupID, EOL, rsActionDown, NULL);
}

bool supla_device_channels::action_up_or_stop(const supla_caller &caller,
                                              int ChannelID, int GroupID,
                                              unsigned char EOL) {
  return rs_action(caller, ChannelID, GroupID, EOL, rsActionUpOrStop, NULL);
}

bool supla_device_channels::action_down_or_stop(const supla_caller &caller,
                                                int ChannelID, int GroupID,
                                                unsigned char EOL) {
  return rs_action(caller, ChannelID, GroupID, EOL, rsActionDownOrStop, NULL);
}

bool supla_device_channels::action_step_by_step(const supla_caller &caller,
                                                int ChannelID, int GroupID,
                                                unsigned char EOL) {
  return rs_action(caller, ChannelID, GroupID, EOL, rsActionStepByStep, NULL);
}

bool supla_device_channels::action_open_close(const supla_caller &caller,
                                              int ChannelID, int GroupID,
                                              unsigned char EOL, bool unknown,
                                              bool open, bool cancelTasks) {
  safe_array_lock(arr);

  bool result = false;
  supla_device_channel *channel = find_channel(ChannelID);
  if (channel && channel->isCharValueWritable()) {
    if (unknown) {
      // You don't know whether you are opening or closing
      switch (channel->getFunc()) {
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
          result = set_device_channel_char_value(caller, channel, GroupID, EOL,
                                                 1, cancelTasks);
          break;
      }
    } else {
      switch (channel->getFunc()) {
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR: {
          supla_action_gate_openclose::open_close(caller, device->get_user_id(),
                                                  device->get_id(),
                                                  channel->getId(), open);
          result = true;
        } break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
          result = set_device_channel_char_value(caller, channel, GroupID, EOL,
                                                 1, cancelTasks);
          break;
      }
    }
  }

  safe_array_unlock(arr);
  return result;
}

bool supla_device_channels::action_open(const supla_caller &caller,
                                        int ChannelID, int GroupID,
                                        unsigned char EOL) {
  return action_open_close(caller, ChannelID, GroupID, EOL, false, true);
}

bool supla_device_channels::action_close(const supla_caller &caller,
                                         int ChannelID, int GroupID,
                                         unsigned char EOL) {
  return action_open_close(caller, ChannelID, GroupID, EOL, false, false);
}

bool supla_device_channels::action_close(const supla_caller &caller,
                                         int ChannelID) {
  return action_open_close(caller, ChannelID, 0, 0, false, false);
}

bool supla_device_channels::action_open_close(const supla_caller &caller,
                                              int ChannelID, int GroupID,
                                              unsigned char EOL) {
  return action_open_close(caller, ChannelID, GroupID, EOL, true, false);
}

bool supla_device_channels::action_open_close_without_canceling_tasks(
    const supla_caller &caller, int ChannelID, int GroupID, unsigned char EOL) {
  return action_open_close(caller, ChannelID, GroupID, EOL, true, false, false);
}

void supla_device_channels::timer_arm(const supla_caller &caller, int ChannelID,
                                      int GroupID, unsigned char EOL,
                                      unsigned char On,
                                      unsigned int DurationMS) {
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) {
    switch (channel->getFunc()) {
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER: {
        char value[SUPLA_CHANNELVALUE_SIZE] = {};
        value[0] = On ? 1 : 0;
        async_set_channel_value(channel, caller, GroupID, EOL, value,
                                DurationMS);
      }
    }
  }

  safe_array_unlock(arr);
}

channel_json_config *supla_device_channels::get_json_config(int ChannelID) {
  channel_json_config *result = NULL;
  access_channel(ChannelID, [&result](supla_device_channel *channel) -> void {
    result = channel->getJSONConfig();
  });

  return result;
}
