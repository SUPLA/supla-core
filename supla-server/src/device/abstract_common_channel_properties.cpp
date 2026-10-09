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

#include <vector>

#include "db/mariadb_access_provider.h"
#include "device/device_dao.h"
#include "jsonconfig/channel/action_trigger_config.h"
#include "jsonconfig/channel/alt_weekly_schedule_config.h"
#include "jsonconfig/channel/binary_sensor_config.h"
#include "jsonconfig/channel/container_config.h"
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
#include "jsonconfig/channel/valve_config.h"
#include "log.h"
#include "proto.h"
#include "suplan/peer_dao.h"
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

template <typename config_class_T, typename raw_config_T,
          typename sensor_class_T>
void supla_abstract_common_channel_properties::get_sensor_relations(
    vector<supla_channel_relation> *relations, e_relation_kind kind,
    unsigned char protocol_version, int type, int parent_channel_type, int func,
    int parent_channel_func, int related_channel_func) {
  if (protocol_version < 27) {
    return;
  }

  if (((parent_channel_func && parent_channel_func == func) ||
       (parent_channel_type && parent_channel_type == type)) &&
      (kind == relation_any || kind == relation_with_sub_channel)) {
    supla_json_config *json_config = get_json_config();
    if (json_config) {
      config_class_T config(json_config);
      raw_config_T raw_cfg = {};
      if (config.get_config(&raw_cfg)) {
        for (size_t a = 0;
             a < sizeof(raw_cfg.SensorInfo) / sizeof(sensor_class_T); a++) {
          if (raw_cfg.SensorInfo[a].ChannelId > 0) {
            for_each(false,
                     [&](supla_abstract_common_channel_properties *props,
                         bool *will_continue) -> void {
                       if (raw_cfg.SensorInfo[a].ChannelId == props->get_id() &&
                           props->get_func() == related_channel_func) {
                         add_relation(relations, props->get_id(), get_id(),
                                      CHANNEL_RELATION_TYPE_DEFAULT);
                         *will_continue = false;
                       }
                     });
          }
        }
      }
      delete json_config;
    }
  }

  if (func == related_channel_func &&
      (kind == relation_any || kind == relation_with_parent_channel)) {
    for_each(
        false,
        [&](supla_abstract_common_channel_properties *props,
            bool *will_continue) -> void {
          if ((parent_channel_func &&
               parent_channel_func == props->get_func()) ||
              (parent_channel_type &&
               parent_channel_type == props->get_type())) {
            supla_json_config *json_config = props->get_json_config();
            if (json_config) {
              config_class_T config(json_config);
              raw_config_T raw_cfg = {};
              if (config.get_config(&raw_cfg)) {
                for (size_t a = 0;
                     a < sizeof(raw_cfg.SensorInfo) / sizeof(sensor_class_T);
                     a++) {
                  if (raw_cfg.SensorInfo[a].ChannelId > 0 &&
                      raw_cfg.SensorInfo[a].ChannelId == get_id()) {
                    add_relation(relations, get_id(), props->get_id(),
                                 CHANNEL_RELATION_TYPE_DEFAULT);
                    *will_continue = false;
                    break;
                  }
                }
              }
              delete json_config;
            }
          }
        });
  }
}

void supla_abstract_common_channel_properties::get_channel_relations(
    vector<supla_channel_relation> *relations, e_relation_kind kind) {
  if (!relations) {
    return;
  }

  unsigned char protocol_version = get_protocol_version();
  int func = get_func();
  int type = get_type();

  get_sensor_relations<valve_config, TChannelConfig_Valve, TValve_SensorInfo>(
      relations, kind, protocol_version, 0, 0, func,
      SUPLA_CHANNELFNC_VALVE_OPENCLOSE, SUPLA_CHANNELFNC_FLOOD_SENSOR);

  get_sensor_relations<container_config, TChannelConfig_Container,
                       TContainer_SensorInfo>(
      relations, kind, protocol_version, type, SUPLA_CHANNELTYPE_CONTAINER,
      func, 0, SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR);

  if (kind == relation_any || kind == relation_with_sub_channel) {
    switch (func) {
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
        TChannelConfig_HVAC hvac = {};
        bool hvac_config_get_success = false;
        supla_json_config *json_config = get_json_config();
        if (json_config) {
          hvac_config config(json_config);
          hvac_config_get_success = config.get_config(&hvac);
          delete json_config;
        }

        for_each(
            false,
            [&](supla_abstract_common_channel_properties *props,
                bool *will_continue) -> void {
              if (hvac_config_get_success) {
                if (hvac.MainThermometerChannelId != get_id() &&
                    hvac.MainThermometerChannelId == props->get_id() &&
                    (props->get_func() == SUPLA_CHANNELFNC_THERMOMETER ||
                     props->get_func() ==
                         SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE)) {
                  add_relation(relations, props->get_id(), get_id(),
                               CHANNEL_RELATION_TYPE_MAIN_TERMOMETER);
                }

                if (hvac.AuxThermometerType >=
                        SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR &&
                    hvac.AuxThermometerType <=
                        SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER &&
                    hvac.AuxThermometerChannelId != get_id() &&
                    hvac.AuxThermometerChannelId == props->get_id() &&
                    (props->get_func() == SUPLA_CHANNELFNC_THERMOMETER ||
                     props->get_func() ==
                         SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE)) {
                  add_relation(relations, props->get_id(), get_id(),
                               hvac.AuxThermometerType + 3);
                }

                if (hvac.BinarySensorChannelId != get_id() &&
                    hvac.BinarySensorChannelId == props->get_id() &&
                    props->get_type() == SUPLA_CHANNELTYPE_BINARYSENSOR) {
                  add_relation(relations, props->get_id(), get_id(),
                               CHANNEL_RELATION_TYPE_DEFAULT);
                }

                if (protocol_version >= 25) {
                  if ((hvac.HeatOrColdSourceSwitchChannelId != 0) &&
                      hvac.HeatOrColdSourceSwitchChannelId == props->get_id() &&
                      props->get_func() ==
                          SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH) {
                    add_relation(
                        relations, props->get_id(), get_id(),
                        CHANNEL_RELATION_TYPE_HEAT_OR_COLD_SOURCE_SWITCH);
                  }

                  if ((hvac.PumpSwitchChannelId != 0) &&
                      hvac.PumpSwitchChannelId == props->get_id() &&
                      props->get_func() == SUPLA_CHANNELFNC_PUMPSWITCH) {
                    add_relation(relations, props->get_id(), get_id(),
                                 CHANNEL_RELATION_TYPE_PUMP_SWITCH);
                  }
                }
              }

              if (protocol_version >= 25) {
                switch (props->get_func()) {
                  case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
                  case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
                  case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
                  case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER: {
                    supla_json_config *props_json_config =
                        props->get_json_config();
                    if (props_json_config) {
                      hvac_config props_config(props_json_config);
                      TChannelConfig_HVAC props_hvac = {};

                      if (props_config.get_config(&props_hvac)) {
                        if ((props_hvac.MasterThermostatChannelId != 0) &&
                            props_hvac.MasterThermostatChannelId == get_id()) {
                          add_relation(relations, props->get_id(), get_id(),
                                       CHANNEL_RELATION_TYPE_MASTER_THERMOSTAT);
                        }
                      }

                      delete props_json_config;
                    }
                  }

                  break;
                }
              }
            });
      } break;
    }
  }

  if (kind == relation_any || kind == relation_with_parent_channel) {
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

    if (protocol_version >= 25 &&
        (func == SUPLA_CHANNELFNC_HVAC_THERMOSTAT ||
         func == SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL ||
         func == SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL ||
         func == SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER)) {
      TChannelConfig_HVAC hvac = {};
      supla_json_config *json_config = get_json_config();
      if (json_config) {
        hvac_config config(json_config);
        if (config.get_config(&hvac) && (hvac.MasterThermostatChannelId != 0)) {
          for_each(
              false,
              [&](supla_abstract_common_channel_properties *props,
                  bool *will_continue) -> void {
                switch (func) {
                  case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
                  case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
                  case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
                  case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:

                    if (hvac.MasterThermostatChannelId == props->get_id()) {
                      add_relation(relations, get_id(), props->get_id(),
                                   CHANNEL_RELATION_TYPE_MASTER_THERMOSTAT);
                      *will_continue = false;
                    }

                         break;
                     }
                   });
        }
        delete json_config;
      }
    }

    if (type == SUPLA_CHANNELTYPE_BINARYSENSOR ||
        func == SUPLA_CHANNELFNC_THERMOMETER ||
        func == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE ||
        func == SUPLA_CHANNELFNC_PUMPSWITCH ||
        func == SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH) {
      for_each(
          false,
          [&](supla_abstract_common_channel_properties *props,
              bool *will_continue) -> void {
            switch (props->get_func()) {
              case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
              case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
              case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
              case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:

                supla_json_config *props_json_config = props->get_json_config();
                if (props_json_config) {
                  hvac_config props_config(props_json_config);
                  TChannelConfig_HVAC hvac = {};
                  if (props_config.get_config(&hvac)) {
                    if (type == SUPLA_CHANNELTYPE_BINARYSENSOR) {
                      if (hvac.BinarySensorChannelId == get_id()) {
                        add_relation(relations, get_id(), props->get_id(),
                                     CHANNEL_RELATION_TYPE_DEFAULT);
                      }
                    } else {
                      switch (func) {
                        case SUPLA_CHANNELFNC_THERMOMETER:
                        case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
                          if (hvac.MainThermometerChannelId == get_id()) {
                            add_relation(relations, get_id(), props->get_id(),
                                         CHANNEL_RELATION_TYPE_MAIN_TERMOMETER);
                          }

                          if (hvac.AuxThermometerType >=
                                  SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR &&
                              hvac.AuxThermometerType <=
                                  SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER &&  // NOLINT
                              hvac.AuxThermometerChannelId == get_id()) {
                            add_relation(relations, get_id(), props->get_id(),
                                         hvac.AuxThermometerType + 3);
                          }
                          break;
                        case SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH:
                          if (protocol_version >= 25 &&
                              (hvac.HeatOrColdSourceSwitchChannelId != 0) &&
                              hvac.HeatOrColdSourceSwitchChannelId ==
                                  get_id()) {
                            add_relation(
                                relations, get_id(), props->get_id(),
                                CHANNEL_RELATION_TYPE_HEAT_OR_COLD_SOURCE_SWITCH);  // NOLINT
                          }
                          break;
                        case SUPLA_CHANNELFNC_PUMPSWITCH:
                          if (protocol_version >= 25 &&
                              (hvac.PumpSwitchChannelId != 0) &&
                              hvac.PumpSwitchChannelId == get_id()) {
                            add_relation(relations, get_id(), props->get_id(),
                                         CHANNEL_RELATION_TYPE_PUMP_SWITCH);
                          }
                          break;
                      }
                    }
                  }
                  delete props_json_config;
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

template <typename configT, typename sensorT>
void supla_abstract_common_channel_properties::resolve_sensor_identifiers(
    configT *config) {
  // Business state is always IDs. Legacy encoding clears a remote/unknown
  // reference instead of aliasing its low byte to a local ChannelNumber.
  for (auto &entry : config->SensorInfo) {
    unsigned int id = entry.ChannelId;
    entry.ChannelId = 0;
    if (!id) {
      continue;
    }
    for_each(false,
             [&](supla_abstract_common_channel_properties *props, bool *stop) {
               if (props->get_device_id() == get_device_id() &&
                   props->get_id() == static_cast<int>(id)) {
                 entry.IsSet = 1;
                 entry.ChannelNo = props->get_channel_number();
                 *stop = false;
               }
             });
  }
}

void supla_abstract_common_channel_properties::get_config(
    char *config, unsigned _supla_int16_t *config_size,
    unsigned char config_type, unsigned _supla_int_t flags,
    ChannelReferenceEncoding encoding) {
  *config_size = 0;

  if (flags != 0) {
    return;
  }

  memset(config, 0, SUPLA_CHANNEL_CONFIG_MAXSIZE);

  if (get_flags() & SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE) {
    if (config_type == SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE) {
      json_to_config<weekly_schedule_config, TChannelConfig_WeeklySchedule>(
          config, config_size,
          [&](weekly_schedule_config *json_config,
              TChannelConfig_WeeklySchedule *ws_cfg) -> bool {
            return json_config->get_config(ws_cfg, get_func());
          });

      return;
    } else if (config_type == SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE) {
      json_to_config<alt_weekly_schedule_config,
                     TChannelConfig_WeeklySchedule>(
          config, config_size,
          [&](alt_weekly_schedule_config *json_config,
              TChannelConfig_WeeklySchedule *ws_cfg) -> bool {
            return json_config->get_config(ws_cfg, get_func());
          });

      return;
    }
  }

  if (get_type() == SUPLA_CHANNELTYPE_IMPULSE_COUNTER &&
      config_type == SUPLA_CONFIG_TYPE_OCR) {
    JSON_TO_CONFIG(ocr_config, TChannelConfig_OCR, config, config_size);
  } else if (config_type == SUPLA_CONFIG_TYPE_EXTENDED &&
             get_func() == SUPLA_CHANNELFNC_STAIRCASETIMER) {
    JSON_TO_CONFIG(power_switch_config, TChannelConfig_PowerSwitch, config,
                   config_size);

    return;
  }

  if (config_type != SUPLA_CONFIG_TYPE_DEFAULT) {
    return;
  }

  if (get_type() == SUPLA_CHANNELTYPE_HVAC) {
    json_to_config<hvac_config, TChannelConfig_HVAC>(
        config, config_size,
        [&](hvac_config *json_config, TChannelConfig_HVAC *ws_cfg) -> bool {
          return json_config->get_config(ws_cfg);
        });

    if (*config_size &&
        encoding == ChannelReferenceEncoding::LocalChannelNumber) {
      TChannelConfig_HVAC *hvac = (TChannelConfig_HVAC *)config;
      auto number = [&](unsigned int id) -> unsigned int {
        unsigned int result = get_channel_number();
        bool found = !id;
        if (id) {
          for_each(false, [&](supla_abstract_common_channel_properties *props,
                              bool *will_continue) {
            if (props->get_id() == static_cast<int>(id) &&
                props->get_device_id() == get_device_id()) {
              result = props->get_channel_number();
              found = true;
              *will_continue = false;
            }
          });
        }
        if (!found) *config_size = 0;
        return result;
      };
      hvac->MainThermometerChannelId = number(hvac->MainThermometerChannelId);
      hvac->AuxThermometerChannelId = number(hvac->AuxThermometerChannelId);
      hvac->BinarySensorChannelId = number(hvac->BinarySensorChannelId);
      auto legacy = [&](unsigned int id, unsigned char *is_set,
                        unsigned char *channel_no) {
        unsigned int resolved = number(id);
        *is_set = id && resolved != get_channel_number() ? 1 : 0;
        *channel_no = resolved;
      };
      unsigned int master = hvac->MasterThermostatChannelId;
      unsigned int pump = hvac->PumpSwitchChannelId;
      unsigned int heat = hvac->HeatOrColdSourceSwitchChannelId;
      hvac->MasterThermostatChannelId = 0;
      hvac->PumpSwitchChannelId = 0;
      hvac->HeatOrColdSourceSwitchChannelId = 0;
      legacy(master, &hvac->MasterThermostatIsSet,
             &hvac->MasterThermostatChannelNo);
      legacy(pump, &hvac->PumpSwitchIsSet, &hvac->PumpSwitchChannelNo);
      legacy(heat, &hvac->HeatOrColdSourceSwitchIsSet,
             &hvac->HeatOrColdSourceSwitchChannelNo);
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
  } else if (get_type() == SUPLA_CHANNELTYPE_CONTAINER) {
    JSON_TO_CONFIG(container_config, TChannelConfig_Container, config,
                   config_size);

    if (encoding == ChannelReferenceEncoding::LocalChannelNumber) {
      resolve_sensor_identifiers<TChannelConfig_Container,
                                 TContainer_SensorInfo>(
          (TChannelConfig_Container *)config);
    }

    return;
  } else if ((get_type() == SUPLA_CHANNELTYPE_VALVE_OPENCLOSE ||
              get_type() == SUPLA_CHANNELTYPE_VALVE_PERCENTAGE)) {
    JSON_TO_CONFIG(valve_config, TChannelConfig_Valve, config, config_size);

    if (encoding == ChannelReferenceEncoding::LocalChannelNumber) {
      resolve_sensor_identifiers<TChannelConfig_Valve, TValve_SensorInfo>(
          (TChannelConfig_Valve *)config);
    }

    return;
  }

  switch (get_func()) {
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      JSON_TO_CONFIG(power_switch_config, TChannelConfig_StaircaseTimer, config,
                     config_size);
      break;

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

  // Several branches below cast the buffer to a TChannelConfig_* structure
  // without comparing config_size with its size. Work on a zero-padded copy
  // so that bytes not sent by the peer are never interpreted (and stored in
  // the JSON config) as configuration values.
  char zero_padded_config[SUPLA_CHANNEL_CONFIG_MAXSIZE] = {};
  if (config_size) {
    memcpy(zero_padded_config, config, config_size);
  }
  config = zero_padded_config;

  int result = SUPLA_CONFIG_RESULT_FALSE;

  supla_mariadb_access_provider dba;
  supla_device_dao dao(&dba);

  supla_json_config *json_config = nullptr;

  int type = get_type();
  int func = get_func();

  if (type == SUPLA_CHANNELTYPE_HVAC &&
      config_type == SUPLA_CONFIG_TYPE_DEFAULT &&
      config_size == sizeof(TChannelConfig_HVAC)) {
    TChannelConfig_HVAC incoming = *(TChannelConfig_HVAC *)config;
    if (device_reference_encoding() ==
        ChannelReferenceEncoding::LocalChannelNumber) {
      auto id = [&](unsigned char number) -> unsigned int {
        return number == get_channel_number() ? 0 : get_channel_id(number);
      };
      incoming.MainThermometerChannelId = id(incoming.MainThermometerChannelNo);
      incoming.AuxThermometerChannelId = id(incoming.AuxThermometerChannelNo);
      incoming.BinarySensorChannelId = id(incoming.BinarySensorChannelNo);
      incoming.MasterThermostatChannelId =
          incoming.MasterThermostatIsSet
              ? id(incoming.MasterThermostatChannelNo)
              : 0;
      incoming.PumpSwitchChannelId =
          incoming.PumpSwitchIsSet ? id(incoming.PumpSwitchChannelNo) : 0;
      incoming.HeatOrColdSourceSwitchChannelId =
          incoming.HeatOrColdSourceSwitchIsSet
              ? id(incoming.HeatOrColdSourceSwitchChannelNo)
              : 0;
    }
    auto hvac = new hvac_config();
    hvac->set_config(&incoming);
    {
      int user = get_user_id(), device = get_device_id(), consumer = get_id();
      hvac->protect_device_references([user, device, consumer](
                                          size_t field, unsigned int id) {
        supla_suplan::PeerDao repo(user);
        supla_suplan::ChannelInfo info;
        if (!repo.channel(id, &info)) return false;
        if (!id) return true;
        if (info.device != device ||
            id == static_cast<unsigned int>(consumer)) {
          return false;
        }
        switch (field) {
          case 0:
          case 1:
            return (info.type == SUPLA_CHANNELTYPE_THERMOMETER ||
                    info.type == SUPLA_CHANNELTYPE_THERMOMETERDS18B20 ||
                    info.type == SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR) &&
                   (info.function == SUPLA_CHANNELFNC_THERMOMETER ||
                    info.function == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE);
          case 2:
            return info.type == SUPLA_CHANNELTYPE_BINARYSENSOR;
          case 3: {
            uint32_t master = 0;
            bool slaves = false;
            return info.type == SUPLA_CHANNELTYPE_HVAC && info.function != 0 &&
                   repo.reference(id, 4, &master) && !master &&
                   repo.has_master_dependents(consumer, &slaves) && !slaves;
          }
          case 4:
          case 5:
            return info.type == SUPLA_CHANNELTYPE_RELAY &&
                   info.function ==
                       (field == 4 ? SUPLA_CHANNELFNC_PUMPSWITCH
                                   : SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH);
        }
        return false;
      });
    }
    json_config = hvac;
  } else if ((config_type == SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE ||
              config_type == SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE) &&
             config_size == sizeof(TChannelConfig_WeeklySchedule) &&
             (get_flags() & SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE)) {
    if (config_type == SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE) {
      json_config = new alt_weekly_schedule_config();
      static_cast<alt_weekly_schedule_config *>(json_config)
          ->set_config((TChannelConfig_WeeklySchedule *)config, func);
    } else {
      json_config = new weekly_schedule_config();
      static_cast<weekly_schedule_config *>(json_config)
          ->set_config((TChannelConfig_WeeklySchedule *)config, func);
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
    // Probably, SUPLA_CHANNELFNC_STAIRCASETIMER cannot come at all with
    // config_type == SUPLA_CONFIG_TYPE_DEFAULT and config_size ==
    // sizeof(TChannelConfig_PowerSwitch) and it is some old bug but it does not
    // harm anything so I leave it just in case.
    json_config = new power_switch_config();
    static_cast<power_switch_config *>(json_config)
        ->set_config((TChannelConfig_PowerSwitch *)config, this);
  } else if (func == SUPLA_CHANNELFNC_STAIRCASETIMER &&
             config_type == SUPLA_CONFIG_TYPE_DEFAULT &&
             config_size == sizeof(TChannelConfig_StaircaseTimer)) {
    json_config = new power_switch_config();
    static_cast<power_switch_config *>(json_config)
        ->set_config((TChannelConfig_StaircaseTimer *)config, this);
  } else if (func == SUPLA_CHANNELFNC_STAIRCASETIMER &&
             config_type == SUPLA_CONFIG_TYPE_EXTENDED &&
             config_size == sizeof(TChannelConfig_PowerSwitch)) {
    json_config = new power_switch_config();
    static_cast<power_switch_config *>(json_config)
        ->set_config((TChannelConfig_PowerSwitch *)config, this);
  } else if ((type == SUPLA_CHANNELTYPE_CONTAINER ||
              (type == SUPLA_CHANNELTYPE_VALVE_OPENCLOSE ||
               type == SUPLA_CHANNELTYPE_VALVE_PERCENTAGE)) &&
             config_type == SUPLA_CONFIG_TYPE_DEFAULT) {
    const bool container = type == SUPLA_CHANNELTYPE_CONTAINER;
    if (config_size != (container ? sizeof(TChannelConfig_Container)
                                  : sizeof(TChannelConfig_Valve))) {
      return SUPLA_CONFIG_RESULT_FALSE;
    }
    auto translate = [&](auto *incoming) {
      if (device_reference_encoding() !=
          ChannelReferenceEncoding::LocalChannelNumber) {
        return;
      }
      for (auto &entry : incoming->SensorInfo) {
        unsigned int id = entry.IsSet ? get_channel_id(entry.ChannelNo) : 0;
        entry.ChannelId = id;
      }
    };
    int user = get_user_id(), device = get_device_id(), consumer = get_id();
    auto legal_local = [user, device, consumer](unsigned int id) {
      if (!id) {
        return true;
      }
      if (id == static_cast<unsigned int>(consumer)) {
        return false;
      }
      supla_suplan::PeerDao repo(user);
      supla_suplan::ChannelInfo info;
      return repo.channel(id, &info) && info.device == device &&
             info.type == SUPLA_CHANNELTYPE_BINARYSENSOR;
    };
    if (container) {
      auto incoming = *reinterpret_cast<TChannelConfig_Container *>(config);
      translate(&incoming);
      auto cfg = new container_config();
      cfg->set_config(&incoming);
      cfg->protect_device_references(legal_local);
      json_config = cfg;
    } else {
      auto incoming = *reinterpret_cast<TChannelConfig_Valve *>(config);
      translate(&incoming);
      auto cfg = new valve_config();
      cfg->set_config(&incoming);
      cfg->protect_device_references(legal_local);
      json_config = cfg;
    }
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
