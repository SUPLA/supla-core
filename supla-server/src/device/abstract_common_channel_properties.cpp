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

#include <memory>
#include <utility>
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
#include "user/channel_relation_index.h"
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
  if (!relations || indexed_relations(relations, kind)) return;
  // Startup/test fallback: parse each available channel once, using exactly
  // the same relation builder. No SQL and no second relation policy.
  supla_relation_snapshot snapshot;
  auto self = supla_relation_node::read(this);
  self.device = 0;
  self.number = 0;
  snapshot.nodes[self.id] = self;
  int order = 1;
  const int f = self.function;
  bool direct = supla_relation_node::opening_control(f) ||
                supla_relation_node::opening_sensor(f);
  if (!direct || (self.type == SUPLA_CHANNELTYPE_BINARYSENSOR &&
                  kind != relation_with_sub_channel)) {
    for_each(true, [&](supla_abstract_common_channel_properties *p, bool *) {
      if (snapshot.nodes.count(p->get_id())) return;
      auto node = supla_relation_node::read(p);
      // Preserve the original iterator's first-parent order, including Devices.
      node.device = 0;
      node.number = order++;
      snapshot.nodes[node.id] = std::move(node);
    });
  }
  snapshot.build();
  // Legacy param/meter accessors also report an ID whose endpoint was not
  // supplied by a partial metadata collection. Client wire projections still
  // filter BOTH endpoints against their visible collection below.
  if (kind != relation_with_parent_channel) {
    if (direct && (supla_relation_node::opening_control(f))) {
      add_relation(relations, self.param2, self.id,
                   CHANNEL_RELATION_TYPE_OPENING_SENSOR);
      if (supla_relation_node::partial_opening_control(f))
        add_relation(relations, self.param3, self.id,
                     CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR);
    }
    if (self.meter)
      add_relation(relations, self.meter, self.id, CHANNEL_RELATION_TYPE_METER);
  }
  if (kind != relation_with_sub_channel && direct &&
      (supla_relation_node::opening_sensor(f))) {
    add_relation(relations, self.id, self.param1,
                 CHANNEL_RELATION_TYPE_OPENING_SENSOR);
    add_relation(relations, self.id, self.param2,
                 CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR);
  }
  for (auto &r : snapshot.get(self.id, kind, get_protocol_version()))
    add_relation(relations, r.get_id(), r.get_parent_id(),
                 r.get_relation_type());
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
    channel_reference_encoding encoding) {
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
        encoding == channel_reference_encoding::device_channel_number) {
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

    if (encoding == channel_reference_encoding::device_channel_number) {
      resolve_sensor_identifiers<TChannelConfig_Container,
                                 TContainer_SensorInfo>(
          (TChannelConfig_Container *)config);
    }

    return;
  } else if ((get_type() == SUPLA_CHANNELTYPE_VALVE_OPENCLOSE ||
              get_type() == SUPLA_CHANNELTYPE_VALVE_PERCENTAGE)) {
    JSON_TO_CONFIG(valve_config, TChannelConfig_Valve, config, config_size);

    if (encoding == channel_reference_encoding::device_channel_number) {
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
        channel_reference_encoding::device_channel_number) {
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
      // Lazy: an all-unset upload creates no reference DAO/connection.
      hvac->protect_device_references([user, device, consumer,
                                       repo = std::shared_ptr<
                                           supla_suplan::peer_dao>()](
                                          size_t field,
                                          unsigned int id) mutable {
        if (!id) return true;
        if (!repo) repo = std::make_shared<supla_suplan::peer_dao>(user);
        auto &repository = *repo;
        supla_suplan::channel_info_t info;
        if (!repository.channel(id, &info)) return false;
        if (info.device != device ||
            id == static_cast<unsigned int>(consumer)) {
          return false;
        }
        switch (field) {
          case 0:
          case 1:
            return (info.type == SUPLA_CHANNELTYPE_THERMOMETER ||
                    info.type == SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR) &&
                   (info.function == SUPLA_CHANNELFNC_THERMOMETER ||
                    info.function == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE);
          case 2:
            return info.type == SUPLA_CHANNELTYPE_BINARYSENSOR;
          case 3: {
            uint32_t master = 0;
            bool slaves = false;
            return info.type == SUPLA_CHANNELTYPE_HVAC && info.function != 0 &&
                   repository.reference(id, 4, &master) && !master &&
                   repository.has_master_dependents(consumer, &slaves) &&
                   !slaves;
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
          channel_reference_encoding::device_channel_number) {
        return;
      }
      for (auto &entry : incoming->SensorInfo) {
        unsigned int id = entry.IsSet ? get_channel_id(entry.ChannelNo) : 0;
        entry.ChannelId = id;
      }
    };
    int user = get_user_id(), device = get_device_id(), consumer = get_id();
    auto legal_local = [user, device, consumer,
                        reference_dao =
                            std::shared_ptr<supla_suplan::peer_dao>()](
                           unsigned int id) mutable {
      if (!id) {
        return true;
      }
      if (id == static_cast<unsigned int>(consumer)) {
        return false;
      }
      if (!reference_dao)
        reference_dao = std::make_shared<supla_suplan::peer_dao>(user);
      auto &repo = *reference_dao;
      supla_suplan::channel_info_t info;
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
