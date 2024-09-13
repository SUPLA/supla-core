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

#include "abstract_common_channel_properties.h"

#include <string.h>

#include "db/db_access_provider.h"
#include "device/device_dao.h"
#include "jsonconfig/channel/action_trigger_config.h"
#include "jsonconfig/channel/alt_weekly_schedule_config.h"
#include "jsonconfig/channel/binary_sensor_config.h"
#include "jsonconfig/channel/electricity_meter_config.h"
#include "jsonconfig/channel/facade_blind_config.h"
#include "jsonconfig/channel/general_purpose_measurement_config.h"
#include "jsonconfig/channel/general_purpose_meter_config.h"
#include "jsonconfig/channel/hvac_config.h"
#include "jsonconfig/channel/impulse_counter_config.h"
#include "jsonconfig/channel/ocr_config.h"
#include "jsonconfig/channel/power_switch_config.h"
#include "jsonconfig/channel/roller_shutter_config.h"
#include "jsonconfig/channel/temp_hum_config.h"
#include "proto.h"

using std::vector;

supla_abstract_common_channel_properties::
    supla_abstract_common_channel_properties(void) {}

supla_abstract_common_channel_properties::
    ~supla_abstract_common_channel_properties(void) {}

void supla_abstract_common_channel_properties::add_relation(
    std::vector<supla_channel_relation> *relations, int channel_id,
    int parent_id, short relation_type) {
  if (!channel_id || !parent_id) {
    return;
  }

  for (auto it = relations->begin(); it != relations->end(); ++it) {
    if (((it->get_id() == channel_id && it->get_parent_id() == parent_id) ||
         (it->get_id() == parent_id && it->get_parent_id() == channel_id)) &&
        it->get_relation_type() == relation_type) {
      return;
    }
  }

  relations->push_back(
      supla_channel_relation(channel_id, parent_id, relation_type));
}

void supla_abstract_common_channel_properties::get_channel_relations(
    vector<supla_channel_relation> *relations, e_relation_kind kind) {
  if (!relations) {
    return;
  }

  unsigned char protocol_version = get_protocol_version();

  if (kind == relation_any || kind == relation_with_sub_channel) {
    switch (get_func()) {
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
        add_relation(relations, get_param2(), get_id(),
                     CHANNEL_RELATION_TYPE_OPENING_SENSOR);

        add_relation(relations, get_param3(), get_id(),
                     CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR);
        break;
      case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
        add_relation(relations, get_param2(), get_id(),
                     CHANNEL_RELATION_TYPE_OPENING_SENSOR);
        break;

      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER: {
        supla_json_config *json_config = get_json_config();
        if (json_config) {
          power_switch_config config(json_config);
          add_relation(relations, config.get_related_meter_channel_id(),
                       get_id(), CHANNEL_RELATION_TYPE_METER);
          delete json_config;
        }
      } break;

      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
      case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER: {
        supla_json_config *json_config = get_json_config();
        if (json_config) {
          hvac_config config(json_config);
          TChannelConfig_HVAC hvac = {};
          if (config.get_config(&hvac, get_channel_number())) {
            bool find_main =
                hvac.MainThermometerChannelNo != get_channel_number();

            bool find_aux =
                hvac.AuxThermometerType >=
                    SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR &&
                hvac.AuxThermometerType <=
                    SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER &&
                hvac.AuxThermometerChannelNo != get_channel_number();

            bool find_sensor =
                hvac.BinarySensorChannelNo != get_channel_number();

            bool find_master = false;
            bool find_heat_or_cool_source = false;
            bool find_pump_switch = false;

            if (protocol_version >= 25) {
              find_master = hvac.MasterThermostatIsSet;
              find_heat_or_cool_source = hvac.HeatOrColdSourceSwitchIsSet;
              find_pump_switch = hvac.PumpSwitchIsSet;
            }

            for_each(
                false,
                [&](supla_abstract_common_channel_properties *props,
                    bool *will_continue) -> void {
                  if (find_main && hvac.MainThermometerChannelNo ==
                                       props->get_channel_number()) {
                    if (props->get_func() == SUPLA_CHANNELFNC_THERMOMETER ||
                        props->get_func() ==
                            SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE) {
                      add_relation(relations, props->get_id(), get_id(),
                                   CHANNEL_RELATION_TYPE_MAIN_TERMOMETER);
                    }

                    find_main = false;
                  }

                  if (find_aux && hvac.AuxThermometerChannelNo ==
                                      props->get_channel_number()) {
                    if (props->get_func() == SUPLA_CHANNELFNC_THERMOMETER ||
                        props->get_func() ==
                            SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE) {
                      add_relation(relations, props->get_id(), get_id(),
                                   hvac.AuxThermometerType + 3);
                    }

                    find_aux = false;
                  }

                  if (find_sensor && hvac.BinarySensorChannelNo ==
                                         props->get_channel_number()) {
                    if (props->get_type() == SUPLA_CHANNELTYPE_BINARYSENSOR) {
                      add_relation(relations, props->get_id(), get_id(),
                                   CHANNEL_RELATION_TYPE_DEFAULT);
                    }
                    find_sensor = false;
                  }

                  if (find_master && hvac.MasterThermostatChannelNo ==
                                         props->get_channel_number()) {
                    switch (props->get_func()) {
                      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
                      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
                      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
                      case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:
                        add_relation(relations, get_id(), props->get_id(),
                                     CHANNEL_RELATION_TYPE_MASTER_THERMOSTAT);
                        break;
                    }
                    find_master = false;
                  }

                  if (find_heat_or_cool_source &&
                      hvac.HeatOrColdSourceSwitchChannelNo ==
                          props->get_channel_number()) {
                    if (props->get_func() ==
                        SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH) {
                      add_relation(
                          relations, props->get_id(), get_id(),
                          CHANNEL_RELATION_TYPE_HEAT_OR_COLD_SOURCE_SWITCH);
                    }
                    find_heat_or_cool_source = false;
                  }

                  if (find_pump_switch &&
                      hvac.PumpSwitchChannelNo == props->get_channel_number()) {
                    if (props->get_func() == SUPLA_CHANNELFNC_PUMPSWITCH) {
                      add_relation(relations, props->get_id(), get_id(),
                                   CHANNEL_RELATION_TYPE_PUMP_SWITCH);
                    }
                    find_pump_switch = false;
                  }

                  *will_continue = find_main || find_aux;
                });
          }
          delete json_config;
        }
      } break;
    }
  }

  if (kind == relation_any || kind == relation_with_parent_channel) {
    int func = get_func();
    int type = get_type();

    switch (func) {
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
        add_relation(relations, get_id(), get_param1(),
                     CHANNEL_RELATION_TYPE_OPENING_SENSOR);

        add_relation(relations, get_id(), get_param2(),
                     CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR);
        break;
      case SUPLA_CHANNELFNC_ELECTRICITY_METER:
      case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
      case SUPLA_CHANNELFNC_IC_GAS_METER:
      case SUPLA_CHANNELFNC_IC_WATER_METER:
      case SUPLA_CHANNELFNC_IC_HEAT_METER:

        for_each(
            true,
            [&](supla_abstract_common_channel_properties *props,
                bool *will_continue) -> void {
              switch (props->get_func()) {
                case SUPLA_CHANNELFNC_POWERSWITCH:
                case SUPLA_CHANNELFNC_LIGHTSWITCH:
                case SUPLA_CHANNELFNC_STAIRCASETIMER: {
                  supla_json_config *json_config = props->get_json_config();
                  if (json_config) {
                    power_switch_config config(json_config);

                    if (config.get_related_meter_channel_id() == get_id()) {
                      add_relation(relations, get_id(), props->get_id(),
                                   CHANNEL_RELATION_TYPE_METER);
                    }

                    delete json_config;
                  }
                }

                break;
              }
            });
        break;
    }

    if (type == SUPLA_CHANNELTYPE_BINARYSENSOR ||
        func == SUPLA_CHANNELFNC_THERMOMETER ||
        func == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE ||
        func == SUPLA_CHANNELFNC_PUMPSWITCH ||
        func == SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH ||
        func == SUPLA_CHANNELFNC_HVAC_THERMOSTAT ||
        func == SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL ||
        func == SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL ||
        func == SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER) {
      for_each(
          false,
          [&](supla_abstract_common_channel_properties *props,
              bool *will_continue) -> void {
            switch (props->get_func()) {
              case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
              case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
              case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
              case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:

                supla_json_config *json_config = props->get_json_config();
                if (json_config) {
                  hvac_config config(json_config);
                  TChannelConfig_HVAC hvac = {};
                  if (config.get_config(&hvac, props->get_channel_number())) {
                    if (type == SUPLA_CHANNELTYPE_BINARYSENSOR) {
                      if (hvac.BinarySensorChannelNo == get_channel_number()) {
                        add_relation(relations, get_id(), props->get_id(),
                                     CHANNEL_RELATION_TYPE_DEFAULT);
                      }
                    } else {
                      switch (func) {
                        case SUPLA_CHANNELFNC_THERMOMETER:
                        case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
                          if (hvac.MainThermometerChannelNo ==
                              get_channel_number()) {
                            add_relation(relations, props->get_id(), get_id(),
                                         CHANNEL_RELATION_TYPE_MAIN_TERMOMETER);
                          }

                          if (hvac.AuxThermometerType >=
                                  SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR &&
                              hvac.AuxThermometerType <=
                                  SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER &&  // NOLINT
                              hvac.AuxThermometerChannelNo ==
                                  get_channel_number()) {
                            add_relation(relations, get_id(), props->get_id(),
                                         hvac.AuxThermometerType + 3);
                          }
                          break;

                        case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
                        case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
                        case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
                        case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:
                          if (protocol_version >= 25 &&
                              hvac.MasterThermostatIsSet &&
                              hvac.MasterThermostatChannelNo ==
                                  get_channel_number()) {
                            add_relation(
                                relations, get_id(), props->get_id(),
                                CHANNEL_RELATION_TYPE_MASTER_THERMOSTAT);
                          }
                          break;
                        case SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH:
                          if (protocol_version >= 25 &&
                              hvac.HeatOrColdSourceSwitchIsSet &&
                              hvac.HeatOrColdSourceSwitchChannelNo ==
                                  get_channel_number()) {
                            add_relation(
                                relations, get_id(), props->get_id(),
                                CHANNEL_RELATION_TYPE_HEAT_OR_COLD_SOURCE_SWITCH);  // NOLINT
                          }
                          break;
                        case SUPLA_CHANNELFNC_PUMPSWITCH:
                          if (protocol_version >= 25 && hvac.PumpSwitchIsSet &&
                              hvac.PumpSwitchChannelNo ==
                                  get_channel_number()) {
                            add_relation(relations, get_id(), props->get_id(),
                                         CHANNEL_RELATION_TYPE_PUMP_SWITCH);
                          }
                          break;
                      }
                    }
                  }
                  delete json_config;
                }

                break;
            }
          });
    }
  }
}

vector<supla_channel_relation>
supla_abstract_common_channel_properties::get_channel_relations(
    e_relation_kind kind) {
  vector<supla_channel_relation> result;
  get_channel_relations(&result, kind);
  return result;
}

template <typename jsonT, typename sdT>
void supla_abstract_common_channel_properties::json_to_config(
    char *config, unsigned _supla_int16_t *config_size,
    std::function<bool(jsonT *, sdT *)> get_config) {
  *config_size = sizeof(sdT);

  sdT *ws_cfg = (sdT *)config;

  supla_json_config *json_config = get_json_config();

  jsonT *_json_config = new jsonT(json_config);
  if (!get_config(_json_config, ws_cfg)) {
    *config_size = 0;
  }
  delete _json_config;

  if (json_config) {
    delete json_config;
  }
}

#define JSON_TO_CONFIG(jsonT, sdT, config, config_size)                    \
  json_to_config<jsonT, sdT>(config, config_size,                          \
                             [](jsonT *json_config, sdT *ws_cfg) -> bool { \
                               return json_config->get_config(ws_cfg);     \
                             });

void supla_abstract_common_channel_properties::get_config(
    char *config, unsigned _supla_int16_t *config_size,
    unsigned char config_type, unsigned _supla_int_t flags,
    bool resolve_channel_identifiers) {
  *config_size = 0;

  if (flags != 0) {
    return;
  }

  memset(config, 0, SUPLA_CHANNEL_CONFIG_MAXSIZE);

  if (get_flags() & SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE) {
    if (config_type == SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE) {
      JSON_TO_CONFIG(weekly_schedule_config, TChannelConfig_WeeklySchedule,
                     config, config_size);

      return;
    } else if (config_type == SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE) {
      JSON_TO_CONFIG(alt_weekly_schedule_config, TChannelConfig_WeeklySchedule,
                     config, config_size);

      return;
    }
  } else if (get_type() == SUPLA_CHANNELTYPE_IMPULSE_COUNTER &&
             config_type == SUPLA_CONFIG_TYPE_OCR) {
    JSON_TO_CONFIG(ocr_config, TChannelConfig_OCR, config, config_size);
  }

  if (config_type != SUPLA_CONFIG_TYPE_DEFAULT) {
    return;
  }

  if (get_type() == SUPLA_CHANNELTYPE_HVAC) {
    json_to_config<hvac_config, TChannelConfig_HVAC>(
        config, config_size,
        [&](hvac_config *json_config, TChannelConfig_HVAC *ws_cfg) -> bool {
          return json_config->get_config(ws_cfg, get_channel_number());
        });

    if (resolve_channel_identifiers) {
      TChannelConfig_HVAC *hvac = (TChannelConfig_HVAC *)config;
      bool find_main = hvac->MainThermometerChannelNo != get_channel_number();
      bool find_aux =
          hvac->AuxThermometerType >= SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR &&
          hvac->AuxThermometerType <=
              SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER;
      for_each(false,
               [&](supla_abstract_common_channel_properties *props,
                   bool *will_continue) -> void {
                 if (find_main && hvac->MainThermometerChannelNo ==
                                      props->get_channel_number()) {
                   hvac->MainThermometerChannelId = props->get_id();
                   find_main = false;
                 }

                 if (find_aux && hvac->AuxThermometerChannelNo ==
                                     props->get_channel_number()) {
                   hvac->AuxThermometerChannelId = props->get_id();
                   find_aux = false;
                 }

                 *will_continue = find_main || find_aux;
               });

      if (find_main) {
        hvac->MainThermometerChannelId = 0;
      }

      if (find_aux) {
        hvac->AuxThermometerChannelId = 0;
      }
    }

    return;
  } else if (get_type() == SUPLA_CHANNELTYPE_BINARYSENSOR) {
    JSON_TO_CONFIG(binary_sensor_config, TChannelConfig_BinarySensor, config,
                   config_size);
    return;
  } else if (get_type() == SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT) {
    JSON_TO_CONFIG(general_purpose_measurement_config,
                   TChannelConfig_GeneralPurposeMeasurement, config,
                   config_size);
    return;
  } else if (get_type() == SUPLA_CHANNELTYPE_GENERAL_PURPOSE_METER) {
    JSON_TO_CONFIG(general_purpose_meter_config,
                   TChannelConfig_GeneralPurposeMeter, config, config_size);
    return;
  } else if (get_type() == SUPLA_CHANNELTYPE_IMPULSE_COUNTER) {
    JSON_TO_CONFIG(impulse_counter_config, TChannelConfig_ImpulseCounter,
                   config, config_size);
    return;
  }

  switch (get_func()) {
    case SUPLA_CHANNELFNC_STAIRCASETIMER: {
      *config_size = sizeof(TChannelConfig_StaircaseTimer);
      TChannelConfig_StaircaseTimer *cfg =
          (TChannelConfig_StaircaseTimer *)config;
      cfg->TimeMS = get_param1() * 100;
    } break;

    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
    case SUPLA_CHANNELFNC_TERRACE_AWNING:
    case SUPLA_CHANNELFNC_PROJECTOR_SCREEN:
    case SUPLA_CHANNELFNC_CURTAIN:
    case SUPLA_CHANNELFNC_ROLLER_GARAGE_DOOR:
      JSON_TO_CONFIG(roller_shutter_config, TChannelConfig_RollerShutter,
                     config, config_size);
      break;

    case SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND:
    case SUPLA_CHANNELFNC_VERTICAL_BLIND:
      JSON_TO_CONFIG(facade_blind_config, TChannelConfig_FacadeBlind, config,
                     config_size);
      break;

    case SUPLA_CHANNELFNC_ACTIONTRIGGER: {
      *config_size = sizeof(TChannelConfig_ActionTrigger);
      TChannelConfig_ActionTrigger *cfg =
          (TChannelConfig_ActionTrigger *)config;
      cfg->ActiveActions = 0;
      supla_json_config *json_config = get_json_config();
      action_trigger_config *at_config = new action_trigger_config(json_config);
      if (at_config) {
        cfg->ActiveActions = at_config->get_active_actions();
        delete at_config;
        at_config = nullptr;
      }

      if (json_config) {
        delete json_config;
      }
    } break;
    case SUPLA_CHANNELFNC_THERMOMETER:
    case SUPLA_CHANNELFNC_HUMIDITY:
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      JSON_TO_CONFIG(temp_hum_config, TChannelConfig_TemperatureAndHumidity,
                     config, config_size);
      break;
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
      JSON_TO_CONFIG(electricity_meter_config, TChannelConfig_ElectricityMeter,
                     config, config_size);
      break;
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
      JSON_TO_CONFIG(power_switch_config, TChannelConfig_PowerSwitch, config,
                     config_size);
      break;
  }
}

int supla_abstract_common_channel_properties::set_user_config(
    unsigned char config_type, unsigned _supla_int16_t config_size,
    char *config) {
  if (config_size > SUPLA_CHANNEL_CONFIG_MAXSIZE || !config) {
    return SUPLA_CONFIG_RESULT_FALSE;
  }

  int result = SUPLA_CONFIG_RESULT_FALSE;

  supla_db_access_provider dba;
  supla_device_dao dao(&dba);

  supla_json_config *json_config = nullptr;

  int type = get_type();
  int func = get_func();

  if (type == SUPLA_CHANNELTYPE_HVAC &&
      config_type == SUPLA_CONFIG_TYPE_DEFAULT &&
      config_size == sizeof(TChannelConfig_HVAC)) {
    json_config = new hvac_config();
    static_cast<hvac_config *>(json_config)
        ->set_config((TChannelConfig_HVAC *)config, get_channel_number());
  } else if ((config_type == SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE ||
              config_type == SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE) &&
             config_size == sizeof(TChannelConfig_WeeklySchedule) &&
             (get_flags() & SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE)) {
    if (config_type == SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE) {
      json_config = new alt_weekly_schedule_config();
      static_cast<alt_weekly_schedule_config *>(json_config)
          ->set_config((TChannelConfig_WeeklySchedule *)config);
    } else {
      json_config = new weekly_schedule_config();
      static_cast<weekly_schedule_config *>(json_config)
          ->set_config((TChannelConfig_WeeklySchedule *)config);
    }

  } else if (type == SUPLA_CHANNELTYPE_BINARYSENSOR) {
    json_config = new binary_sensor_config();
    static_cast<binary_sensor_config *>(json_config)
        ->set_config((TChannelConfig_BinarySensor *)config);
  } else if (func == SUPLA_CHANNELFNC_THERMOMETER ||
             func == SUPLA_CHANNELFNC_HUMIDITY ||
             func == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE) {
    json_config = new temp_hum_config();
    static_cast<temp_hum_config *>(json_config)
        ->set_config((TChannelConfig_TemperatureAndHumidity *)config);
  } else if (type == SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT) {
    json_config = new general_purpose_measurement_config();
    static_cast<general_purpose_measurement_config *>(json_config)
        ->set_config((TChannelConfig_GeneralPurposeMeasurement *)config);
  } else if (type == SUPLA_CHANNELTYPE_GENERAL_PURPOSE_METER) {
    json_config = new general_purpose_meter_config();
    static_cast<general_purpose_meter_config *>(json_config)
        ->set_config((TChannelConfig_GeneralPurposeMeter *)config);
  } else if (func == SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER ||
             func == SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW ||
             func == SUPLA_CHANNELFNC_TERRACE_AWNING ||
             func == SUPLA_CHANNELFNC_PROJECTOR_SCREEN ||
             func == SUPLA_CHANNELFNC_CURTAIN ||
             func == SUPLA_CHANNELFNC_ROLLER_GARAGE_DOOR) {
    json_config = new roller_shutter_config();
    static_cast<roller_shutter_config *>(json_config)
        ->set_config((TChannelConfig_RollerShutter *)config);
  } else if (func == SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND ||
             func == SUPLA_CHANNELFNC_VERTICAL_BLIND) {
    json_config = new facade_blind_config();
    static_cast<facade_blind_config *>(json_config)
        ->set_config((TChannelConfig_FacadeBlind *)config);
  } else if (type == SUPLA_CHANNELTYPE_ELECTRICITY_METER) {
    json_config = new electricity_meter_config(nullptr);
    static_cast<electricity_meter_config *>(json_config)
        ->set_config((TChannelConfig_ElectricityMeter *)config);
  } else if (type == SUPLA_CHANNELTYPE_IMPULSE_COUNTER &&
             config_type == SUPLA_CONFIG_TYPE_OCR) {
    json_config = new ocr_config(nullptr);
    static_cast<ocr_config *>(json_config)
        ->set_config((TChannelConfig_OCR *)config);
  } else if (type == SUPLA_CHANNELTYPE_IMPULSE_COUNTER &&
             config_type == SUPLA_CONFIG_TYPE_DEFAULT &&
             config_size == sizeof(TChannelConfig_ImpulseCounter)) {
    json_config = new impulse_counter_config();
    static_cast<impulse_counter_config *>(json_config)
        ->set_config((TChannelConfig_ImpulseCounter *)config);
  } else if ((func == SUPLA_CHANNELFNC_POWERSWITCH ||
              func == SUPLA_CHANNELFNC_LIGHTSWITCH ||
              func == SUPLA_CHANNELFNC_STAIRCASETIMER) &&
             config_type == SUPLA_CONFIG_TYPE_DEFAULT &&
             config_size == sizeof(TChannelConfig_PowerSwitch)) {
    json_config = new power_switch_config();
    static_cast<power_switch_config *>(json_config)
        ->set_config((TChannelConfig_PowerSwitch *)config, this);
  } else {
    result = SUPLA_CONFIG_RESULT_NOT_ALLOWED;
  }

  if (json_config) {
    if (dao.set_channel_config(get_user_id(), get_id(), json_config)) {
      result = SUPLA_CONFIG_RESULT_TRUE;
      delete json_config;

      // Get the merged configuration.
      json_config = dao.get_channel_config(get_id(), nullptr, nullptr);
      if (json_config) {
        set_json_config(json_config);
      }
    } else {
      delete json_config;
    }
  }

  return result;
}

int supla_abstract_common_channel_properties::get_channel_id(
    unsigned char number) {
  if (get_channel_number() == number) {
    return get_id();
  }

  int result = 0;

  for_each(
      false,
      [this, number, &result](supla_abstract_common_channel_properties *props,
                              bool *will_continue) -> void {
        if (number == props->get_channel_number()) {
          result = props->get_id();
        }

        *will_continue = !result;
      });

  return result;
}
