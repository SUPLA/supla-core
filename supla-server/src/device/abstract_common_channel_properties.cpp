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
#include "jsonconfig/channel/hvac_config.h"
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
      case SUPLA_CHANNELFNC_LIGHTSWITCH: {
        add_relation(relations, get_param1(), get_id(),
                     CHANNEL_RELATION_TYPE_METER);
      } break;

      case SUPLA_CHANNELFNC_STAIRCASETIMER:
        add_relation(relations, get_param2(), get_id(),
                     CHANNEL_RELATION_TYPE_METER);
        break;

      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_AUTO:
      case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER: {
        channel_json_config *json_config = get_json_config();
        if (json_config) {
          hvac_config config(json_config);
          TChannelConfig_HVAC hvac = {};
          if (config.get_config(&hvac)) {
            int device_id = get_device_id();

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

            for_each([&](supla_abstract_common_channel_properties *props,
                         bool *will_continue) -> void {
              if (device_id == props->get_device_id()) {
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

                if (find_sensor &&
                    hvac.BinarySensorChannelNo == props->get_channel_number()) {
                  if (props->get_type() == SUPLA_CHANNELTYPE_BINARYSENSOR) {
                    add_relation(relations, props->get_id(), get_id(),
                                 CHANNEL_RELATION_TYPE_DEFAULT);
                  }
                  find_sensor = false;
                }
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
        add_relation(relations, get_id(), get_param4(),
                     CHANNEL_RELATION_TYPE_METER);
        break;
    }

    if (type == SUPLA_CHANNELTYPE_BINARYSENSOR ||
        func == SUPLA_CHANNELFNC_THERMOMETER ||
        func == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE) {
      int device_id = get_device_id();

      for_each([&](supla_abstract_common_channel_properties *props,
                   bool *will_continue) -> void {
        if (props->get_device_id() != device_id) {
          return;
        }
        switch (props->get_func()) {
          case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
          case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_AUTO:
          case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:

            channel_json_config *json_config = props->get_json_config();
            if (json_config) {
              hvac_config config(json_config);
              TChannelConfig_HVAC hvac = {};
              if (config.get_config(&hvac)) {
                if (type == SUPLA_CHANNELTYPE_BINARYSENSOR) {
                  if (hvac.BinarySensorChannelNo == get_channel_number()) {
                    add_relation(relations, get_id(), props->get_id(),
                                 CHANNEL_RELATION_TYPE_DEFAULT);
                  }
                } else {
                  if (hvac.MainThermometerChannelNo == get_channel_number()) {
                    add_relation(relations, get_id(), props->get_id(),
                                 CHANNEL_RELATION_TYPE_MAIN_TERMOMETER);
                  }

                  if (hvac.AuxThermometerType >=
                          SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR &&
                      hvac.AuxThermometerType <=
                          SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER &&
                      hvac.AuxThermometerChannelNo == get_channel_number()) {
                    add_relation(relations, get_id(), props->get_id(),
                                 hvac.AuxThermometerType + 3);
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
    char *config, unsigned _supla_int16_t *config_size) {
  *config_size = sizeof(sdT);

  sdT *ws_cfg = (sdT *)config;

  channel_json_config *json_config = get_json_config();

  jsonT *_json_config = new jsonT(json_config);
  if (!_json_config->get_config(ws_cfg)) {
    *config_size = 0;
  }
  delete _json_config;

  if (json_config) {
    delete json_config;
  }
}

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
      json_to_config<weekly_schedule_config, TChannelConfig_WeeklySchedule>(
          config, config_size);

      return;
    } else if (config_type == SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE) {
      json_to_config<alt_weekly_schedule_config, TChannelConfig_WeeklySchedule>(
          config, config_size);

      return;
    }
  }

  if (config_type != SUPLA_CONFIG_TYPE_DEFAULT) {
    return;
  }

  if (get_type() == SUPLA_CHANNELTYPE_HVAC) {
    json_to_config<hvac_config, TChannelConfig_HVAC>(config, config_size);

    if (resolve_channel_identifiers) {
      TChannelConfig_HVAC *hvac = (TChannelConfig_HVAC *)config;
      bool find_main = hvac->MainThermometerChannelNo != get_channel_number();
      bool find_aux =
          hvac->AuxThermometerType >= SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR &&
          hvac->AuxThermometerType <=
              SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER;
      int device_id = get_device_id();

      for_each([&](supla_abstract_common_channel_properties *props,
                   bool *will_continue) -> void {
        if (device_id == props->get_device_id()) {
          if (find_main &&
              hvac->MainThermometerChannelNo == props->get_channel_number()) {
            hvac->MainThermometerChannelId = props->get_id();
            find_main = false;
          }

          if (find_aux &&
              hvac->AuxThermometerChannelNo == props->get_channel_number()) {
            hvac->AuxThermometerChannelId = props->get_id();
            find_aux = false;
          }
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
  }

  switch (get_func()) {
    case SUPLA_CHANNELFNC_STAIRCASETIMER: {
      *config_size = sizeof(TChannelConfig_StaircaseTimer);
      TChannelConfig_StaircaseTimer *cfg =
          (TChannelConfig_StaircaseTimer *)config;
      cfg->TimeMS = get_param1() * 100;
    } break;

    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW: {
      *config_size = sizeof(TChannelConfig_Rollershutter);
      TChannelConfig_Rollershutter *cfg =
          (TChannelConfig_Rollershutter *)config;
      cfg->OpeningTimeMS = get_param1() * 100;
      cfg->ClosingTimeMS = get_param3() * 100;
    } break;

    case SUPLA_CHANNELFNC_ACTIONTRIGGER: {
      *config_size = sizeof(TChannelConfig_ActionTrigger);
      TChannelConfig_ActionTrigger *cfg =
          (TChannelConfig_ActionTrigger *)config;
      cfg->ActiveActions = 0;
      channel_json_config *json_config = get_json_config();
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

  channel_json_config *json_config = nullptr;

  if (get_type() == SUPLA_CHANNELTYPE_HVAC &&
      config_type == SUPLA_CONFIG_TYPE_DEFAULT &&
      config_size == sizeof(TChannelConfig_HVAC)) {
    json_config = new hvac_config();
    static_cast<hvac_config *>(json_config)
        ->set_config((TChannelConfig_HVAC *)config);
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

  } else {
    result = SUPLA_CONFIG_RESULT_NOT_ALLOWED;
  }

  if (json_config) {
    if (dao.set_channel_user_config(get_user_id(), get_id(), json_config)) {
      result = SUPLA_CONFIG_RESULT_TRUE;
      delete json_config;

      // Get the merged configuration.
      json_config = dao.get_channel_config(get_id(), nullptr, nullptr);
      if (json_config) {
        set_json_config(json_config);
      }
    }
  }

  return result;
}
