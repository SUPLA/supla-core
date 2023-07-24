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

#include "jsonconfig/channel/hvac_config.h"
#include "proto.h"

supla_abstract_common_channel_properties::
    supla_abstract_common_channel_properties(void) {}

supla_abstract_common_channel_properties::
    ~supla_abstract_common_channel_properties(void) {}

void supla_abstract_common_channel_properties::get_parent_channel_id(
    int *parent1, int *parent2, short *parent1_relation_type,
    short *parent2_relation_type) {
  if (parent1) {
    *parent1 = 0;
  }

  if (parent2) {
    *parent2 = 0;
  }

  if (parent1_relation_type) {
    *parent1_relation_type = 0;
  }

  if (parent2_relation_type) {
    *parent2_relation_type = 0;
  }

  int p1 = 0;
  short p1_relation_type = 0;
  int p2 = 0;
  short p2_relation_type = 0;

  switch (get_func()) {
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
      p1 = get_param1();
      p1_relation_type = RELATION_OPENING_SENSOR;
      p2 = get_param2();
      p2_relation_type = RELATION_PARTIAL_OPENING_SENSOR;
      break;
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      p1 = get_param4();
      p1_relation_type = RELATION_METER;
      break;
    case SUPLA_CHANNELFNC_THERMOMETER:
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:

      for_each([&](int id, supla_abstract_common_channel_properties *props,
                   bool *will_continue) -> void {
        switch (props->get_func()) {
          case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT:
          case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_COOL:
          case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_AUTO:

            channel_json_config *json_config = props->get_json_config();
            if (json_config) {
              hvac_config config(json_config);
              TChannelConfig_HVAC hvac = {};
              if (config.get_config(&hvac)) {
                if (hvac.MainThermometerChannelNo == get_channel_number()) {
                  p1 = id;
                  p1_relation_type = RELATION_MAIN_TERMOMETER;
                }

                if (hvac.AuxThermometerType >=
                        SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR &&
                    hvac.AuxThermometerType <=
                        SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER &&
                    hvac.AuxThermometerChannelNo == get_channel_number()) {
                  p2 = id;
                  p2_relation_type = hvac.AuxThermometerType + 3;
                }
              }
              delete json_config;
            }

            break;
        }
      });
      break;
  }

  if (p1 && parent1) {
    *parent1 = p1;
    if (parent1_relation_type) {
      *parent1_relation_type = p1_relation_type;
    }
  }

  if (p2 && parent2) {
    *parent2 = p2;

    if (parent2_relation_type) {
      *parent2_relation_type = p2_relation_type;
    }
  }
}

void supla_abstract_common_channel_properties::get_sub_channel_id(int *sub1,
                                                                  int *sub2) {
  int s1 = 0;
  int s2 = 0;

  if (sub1) {
    *sub1 = 0;
  }

  if (sub2) {
    *sub2 = 0;
  }

  switch (get_func()) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      s1 = get_param2();
      s2 = get_param3();
      break;
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH: {
      s1 = get_param1();
    } break;

    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      s1 = get_param2();
      break;

    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_COOL:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_AUTO:

      channel_json_config *json_config = get_json_config();
      if (json_config) {
        hvac_config config(json_config);
        TChannelConfig_HVAC hvac = {};
        if (config.get_config(&hvac)) {
          if (hvac.MainThermometerChannelNo != get_channel_number()) {
            s1 = get_channel_id(hvac.MainThermometerChannelNo);
          }

          if (hvac.AuxThermometerType >=
                  SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR &&
              hvac.AuxThermometerType <=
                  SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER) {
            s2 = get_channel_id(hvac.AuxThermometerChannelNo);
          }
        }
        delete json_config;
      }
      break;
  }

  if (s1 && sub1) {
    *sub1 = s1;
  }

  if (s2 && sub2) {
    *sub2 = s2;
  }
}
