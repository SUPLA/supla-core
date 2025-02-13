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

#include "CommonChannelPropertiesTest.h"

#include <string>
#include <vector>

#include "doubles/device/CommonChannelPropertiesMock.h"
#include "jsonconfig/channel/container_config.h"
#include "jsonconfig/channel/hvac_config.h"
#include "jsonconfig/channel/power_switch_config.h"
#include "jsonconfig/channel/valve_config.h"

namespace testing {

using std::string;
using std::vector;

TEST_F(CommonChannelPropertiesTest, appendRelationList) {
  vector<supla_channel_relation> rel;

  rel.push_back(supla_channel_relation(10, 20, CHANNEL_RELATION_TYPE_METER));

  {
    CommonChannelPropertiesMock mock;
    EXPECT_CALL(mock, get_func)
        .WillRepeatedly(Return(SUPLA_CHANNELFNC_CONTROLLINGTHEGATE));

    EXPECT_CALL(mock, get_id).WillRepeatedly(Return(1));
    EXPECT_CALL(mock, get_param2).WillRepeatedly(Return(2));
    EXPECT_CALL(mock, get_param3).WillRepeatedly(Return(0));

    mock.get_channel_relations(&rel, relation_with_sub_channel);
  }

  ASSERT_EQ(rel.size(), 2);

  EXPECT_EQ(rel.at(0).get_id(), 10);
  EXPECT_EQ(rel.at(0).get_parent_id(), 20);
  EXPECT_EQ(rel.at(0).get_relation_type(), CHANNEL_RELATION_TYPE_METER);

  EXPECT_EQ(rel.at(1).get_id(), 2);
  EXPECT_EQ(rel.at(1).get_parent_id(), 1);
  EXPECT_EQ(rel.at(1).get_relation_type(),
            CHANNEL_RELATION_TYPE_OPENING_SENSOR);
}

TEST_F(CommonChannelPropertiesTest, preventingDuplicateRelations) {
  vector<supla_channel_relation> rel1, rel2;

  {
    CommonChannelPropertiesMock mock;
    EXPECT_CALL(mock, get_func)
        .WillRepeatedly(Return(SUPLA_CHANNELFNC_CONTROLLINGTHEGATE));

    EXPECT_CALL(mock, get_id).WillRepeatedly(Return(1));
    EXPECT_CALL(mock, get_param2).WillRepeatedly(Return(2));
    EXPECT_CALL(mock, get_param3).WillRepeatedly(Return(3));

    mock.get_channel_relations(&rel1, relation_any);
    mock.get_channel_relations(&rel1, relation_any);
  }

  ASSERT_EQ(rel1.size(), 2);

  EXPECT_EQ(rel1.at(0).get_id(), 2);
  EXPECT_EQ(rel1.at(0).get_parent_id(), 1);
  EXPECT_EQ(rel1.at(0).get_relation_type(),
            CHANNEL_RELATION_TYPE_OPENING_SENSOR);

  EXPECT_EQ(rel1.at(1).get_id(), 3);
  EXPECT_EQ(rel1.at(1).get_parent_id(), 1);
  EXPECT_EQ(rel1.at(1).get_relation_type(),
            CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR);

  {
    CommonChannelPropertiesMock mock;
    EXPECT_CALL(mock, get_func)
        .WillRepeatedly(Return(SUPLA_CHANNELFNC_OPENINGSENSOR_GATE));

    EXPECT_CALL(mock, get_id).WillRepeatedly(Return(2));
    EXPECT_CALL(mock, get_param1).WillRepeatedly(Return(1));
    EXPECT_CALL(mock, get_param2).WillRepeatedly(Return(30));

    mock.get_channel_relations(&rel1, relation_any);
    mock.get_channel_relations(&rel1, relation_any);
    mock.get_channel_relations(&rel2, relation_any);
    mock.get_channel_relations(&rel2, relation_any);
  }

  ASSERT_EQ(rel2.size(), 2);

  EXPECT_EQ(rel2.at(0).get_id(), 2);
  EXPECT_EQ(rel2.at(0).get_parent_id(), 1);
  EXPECT_EQ(rel2.at(0).get_relation_type(),
            CHANNEL_RELATION_TYPE_OPENING_SENSOR);

  EXPECT_EQ(rel2.at(1).get_id(), 2);
  EXPECT_EQ(rel2.at(1).get_parent_id(), 30);
  EXPECT_EQ(rel2.at(1).get_relation_type(),
            CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR);

  ASSERT_EQ(rel1.size(), 3);

  EXPECT_EQ(rel1.at(0).get_id(), 2);
  EXPECT_EQ(rel1.at(0).get_parent_id(), 1);
  EXPECT_EQ(rel1.at(0).get_relation_type(),
            CHANNEL_RELATION_TYPE_OPENING_SENSOR);

  EXPECT_EQ(rel1.at(1).get_id(), 3);
  EXPECT_EQ(rel1.at(1).get_parent_id(), 1);
  EXPECT_EQ(rel1.at(1).get_relation_type(),
            CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR);

  EXPECT_EQ(rel1.at(2).get_id(), 2);
  EXPECT_EQ(rel1.at(2).get_parent_id(), 30);
  EXPECT_EQ(rel1.at(2).get_relation_type(),
            CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR);
}

TEST_F(CommonChannelPropertiesTest,
       protocolVersionDependedRelationWithSubchannel_MasterThermostat) {
  vector<int> functions = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL,
                           SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    for (auto rit = functions.cbegin(); rit != functions.cend(); ++rit) {
      CommonChannelPropertiesMock related_props_mock;

      EXPECT_CALL(related_props_mock, get_id).WillRepeatedly(Return(5001));
      EXPECT_CALL(related_props_mock, get_device_id).WillRepeatedly(Return(5));
      EXPECT_CALL(related_props_mock, get_func).WillRepeatedly(Return(*it));
      EXPECT_CALL(related_props_mock, get_channel_number)
          .WillRepeatedly(Return(10));
      CommonChannelPropertiesMock mock;
      unsigned char version = 23;

      EXPECT_CALL(mock, get_func).WillRepeatedly(Return(*rit));
      EXPECT_CALL(mock, get_protocol_version).WillRepeatedly([&version]() {
        version++;
        return version;
      });

      EXPECT_CALL(mock, get_device_id).WillRepeatedly(Return(5));
      EXPECT_CALL(mock, get_id).WillRepeatedly(Return(5000));

      EXPECT_CALL(related_props_mock, get_json_config).WillRepeatedly([]() {
        hvac_config *config = new hvac_config();
        TChannelConfig_HVAC hvac = {};
        hvac.MasterThermostatIsSet = 1;
        config->set_config(&hvac, 5);
        return config;
      });

      EXPECT_CALL(mock, for_each)
          .WillRepeatedly(
              [&](bool any_device,
                  std::function<void(supla_abstract_common_channel_properties *,
                                     bool *)>
                      on_channel_properties) {
                bool will_continue = true;
                on_channel_properties(&related_props_mock, &will_continue);
                EXPECT_FALSE(any_device);
              });

      vector<supla_channel_relation> rel1, rel2;
      mock.get_channel_relations(&rel1, relation_with_sub_channel);
      mock.get_channel_relations(&rel2, relation_with_sub_channel);

      EXPECT_EQ(rel1.size(), 0);

      ASSERT_EQ(rel2.size(), 1);
      EXPECT_EQ(rel2.at(0).get_id(), 5001);
      EXPECT_EQ(rel2.at(0).get_parent_id(), 5000);
      EXPECT_EQ(rel2.at(0).get_relation_type(),
                CHANNEL_RELATION_TYPE_MASTER_THERMOSTAT);
    }
  }
}

TEST_F(CommonChannelPropertiesTest,
       protocolVersionDependedRelationWithParentChannel_MasterThermostat) {
  vector<int> functions = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL,
                           SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    for (auto rit = functions.cbegin(); rit != functions.cend(); ++rit) {
      CommonChannelPropertiesMock related_props_mock;

      EXPECT_CALL(related_props_mock, get_id).WillRepeatedly(Return(5000));
      EXPECT_CALL(related_props_mock, get_device_id).WillRepeatedly(Return(5));
      EXPECT_CALL(related_props_mock, get_channel_number)
          .WillRepeatedly(Return(10));
      EXPECT_CALL(related_props_mock, get_func).WillRepeatedly(Return(*it));

      CommonChannelPropertiesMock mock;
      unsigned char version = 23;

      EXPECT_CALL(mock, get_func).WillRepeatedly(Return(*rit));
      EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(5));
      EXPECT_CALL(mock, get_protocol_version).WillRepeatedly([&version]() {
        version++;
        return version;
      });

      EXPECT_CALL(mock, get_json_config).WillRepeatedly([]() {
        hvac_config *config = new hvac_config();
        TChannelConfig_HVAC hvac = {};
        hvac.MasterThermostatIsSet = 1;
        hvac.MasterThermostatChannelNo = 10;
        config->set_config(&hvac, 5);
        return config;
      });

      EXPECT_CALL(mock, get_device_id).WillRepeatedly(Return(5));
      EXPECT_CALL(mock, get_id).WillRepeatedly(Return(5001));

      EXPECT_CALL(mock, for_each)
          .WillRepeatedly(
              [&](bool any_device,
                  std::function<void(supla_abstract_common_channel_properties *,
                                     bool *)>
                      on_channel_properties) {
                bool will_continue = true;
                on_channel_properties(&related_props_mock, &will_continue);
                EXPECT_FALSE(any_device);
              });

      vector<supla_channel_relation> rel1, rel2;
      mock.get_channel_relations(&rel1, relation_with_parent_channel);
      mock.get_channel_relations(&rel2, relation_with_parent_channel);

      EXPECT_EQ(rel1.size(), 0);

      ASSERT_EQ(rel2.size(), 1);
      EXPECT_EQ(rel2.at(0).get_id(), 5001);
      EXPECT_EQ(rel2.at(0).get_parent_id(), 5000);
      EXPECT_EQ(rel2.at(0).get_relation_type(),
                CHANNEL_RELATION_TYPE_MASTER_THERMOSTAT);
    }
  }
}

TEST_F(CommonChannelPropertiesTest,
       protocolVersionDependedRelationWithSubchannel_HeatOrColdSource) {
  vector<int> functions = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL,
                           SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    CommonChannelPropertiesMock related_props_mock;

    EXPECT_CALL(related_props_mock, get_id).WillRepeatedly(Return(5001));
    EXPECT_CALL(related_props_mock, get_device_id).WillRepeatedly(Return(5));
    EXPECT_CALL(related_props_mock, get_func)
        .WillRepeatedly(Return(SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH));

    CommonChannelPropertiesMock mock;
    unsigned char version = 23;

    EXPECT_CALL(mock, get_func).WillRepeatedly(Return(*it));
    EXPECT_CALL(mock, get_protocol_version).WillRepeatedly([&version]() {
      version++;
      return version;
    });
    EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(10));
    EXPECT_CALL(mock, get_device_id).WillRepeatedly(Return(5));
    EXPECT_CALL(mock, get_id).WillRepeatedly(Return(5000));

    EXPECT_CALL(mock, get_json_config).WillRepeatedly([]() {
      hvac_config *config = new hvac_config();
      TChannelConfig_HVAC hvac = {};
      hvac.HeatOrColdSourceSwitchIsSet = 1;
      config->set_config(&hvac, 10);
      return config;
    });

    EXPECT_CALL(mock, for_each)
        .WillRepeatedly(
            [&](bool any_device,
                std::function<void(supla_abstract_common_channel_properties *,
                                   bool *)>
                    on_channel_properties) {
              bool will_continue = true;
              on_channel_properties(&related_props_mock, &will_continue);
              EXPECT_FALSE(any_device);
            });

    vector<supla_channel_relation> rel1, rel2;
    mock.get_channel_relations(&rel1, relation_with_sub_channel);
    mock.get_channel_relations(&rel2, relation_with_sub_channel);

    EXPECT_EQ(rel1.size(), 0);

    ASSERT_EQ(rel2.size(), 1);
    EXPECT_EQ(rel2.at(0).get_id(), 5001);
    EXPECT_EQ(rel2.at(0).get_parent_id(), 5000);
    EXPECT_EQ(rel2.at(0).get_relation_type(),
              CHANNEL_RELATION_TYPE_HEAT_OR_COLD_SOURCE_SWITCH);
  }
}

TEST_F(CommonChannelPropertiesTest,
       protocolVersionDependedRelationWithParentChannel_HeatOrColdSource) {
  vector<int> functions = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL,
                           SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    CommonChannelPropertiesMock related_props_mock;

    EXPECT_CALL(related_props_mock, get_id).WillRepeatedly(Return(5000));
    EXPECT_CALL(related_props_mock, get_device_id).WillRepeatedly(Return(5));
    EXPECT_CALL(related_props_mock, get_channel_number)
        .WillRepeatedly(Return(10));
    EXPECT_CALL(related_props_mock, get_func).WillRepeatedly(Return(*it));

    EXPECT_CALL(related_props_mock, get_json_config).WillRepeatedly([]() {
      hvac_config *config = new hvac_config();
      TChannelConfig_HVAC hvac = {};
      hvac.HeatOrColdSourceSwitchIsSet = 1;
      config->set_config(&hvac, 10);
      return config;
    });

    CommonChannelPropertiesMock mock;
    unsigned char version = 23;

    EXPECT_CALL(mock, get_func)
        .WillRepeatedly(Return(SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH));
    EXPECT_CALL(mock, get_protocol_version).WillRepeatedly([&version]() {
      version++;
      return version;
    });

    EXPECT_CALL(mock, get_device_id).WillRepeatedly(Return(5));
    EXPECT_CALL(mock, get_id).WillRepeatedly(Return(5001));

    EXPECT_CALL(mock, for_each)
        .WillRepeatedly(
            [&](bool any_device,
                std::function<void(supla_abstract_common_channel_properties *,
                                   bool *)>
                    on_channel_properties) {
              bool will_continue = true;
              on_channel_properties(&related_props_mock, &will_continue);
              EXPECT_FALSE(any_device);
            });

    vector<supla_channel_relation> rel1, rel2;
    mock.get_channel_relations(&rel1, relation_with_parent_channel);
    mock.get_channel_relations(&rel2, relation_with_parent_channel);

    EXPECT_EQ(rel1.size(), 0);

    ASSERT_EQ(rel2.size(), 1);
    EXPECT_EQ(rel2.at(0).get_id(), 5001);
    EXPECT_EQ(rel2.at(0).get_parent_id(), 5000);
    EXPECT_EQ(rel2.at(0).get_relation_type(),
              CHANNEL_RELATION_TYPE_HEAT_OR_COLD_SOURCE_SWITCH);
  }
}

TEST_F(CommonChannelPropertiesTest,
       protocolVersionDependedRelationWithSubchannel_PumpSwitch) {
  vector<int> functions = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL,
                           SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    CommonChannelPropertiesMock related_props_mock;

    EXPECT_CALL(related_props_mock, get_id).WillRepeatedly(Return(5001));
    EXPECT_CALL(related_props_mock, get_device_id).WillRepeatedly(Return(5));
    EXPECT_CALL(related_props_mock, get_func)
        .WillRepeatedly(Return(SUPLA_CHANNELFNC_PUMPSWITCH));

    CommonChannelPropertiesMock mock;
    unsigned char version = 23;

    EXPECT_CALL(mock, get_func).WillRepeatedly(Return(*it));
    EXPECT_CALL(mock, get_protocol_version).WillRepeatedly([&version]() {
      version++;
      return version;
    });
    EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(10));
    EXPECT_CALL(mock, get_device_id).WillRepeatedly(Return(5));
    EXPECT_CALL(mock, get_id).WillRepeatedly(Return(5000));

    EXPECT_CALL(mock, get_json_config).WillRepeatedly([]() {
      hvac_config *config = new hvac_config();
      TChannelConfig_HVAC hvac = {};
      hvac.PumpSwitchIsSet = 1;
      config->set_config(&hvac, 10);
      return config;
    });

    EXPECT_CALL(mock, for_each)
        .WillRepeatedly(
            [&](bool any_device,
                std::function<void(supla_abstract_common_channel_properties *,
                                   bool *)>
                    on_channel_properties) {
              bool will_continue = true;
              on_channel_properties(&related_props_mock, &will_continue);
              EXPECT_FALSE(any_device);
            });

    vector<supla_channel_relation> rel1, rel2;
    mock.get_channel_relations(&rel1, relation_with_sub_channel);
    mock.get_channel_relations(&rel2, relation_with_sub_channel);

    EXPECT_EQ(rel1.size(), 0);

    ASSERT_EQ(rel2.size(), 1);
    EXPECT_EQ(rel2.at(0).get_id(), 5001);
    EXPECT_EQ(rel2.at(0).get_parent_id(), 5000);
    EXPECT_EQ(rel2.at(0).get_relation_type(),
              CHANNEL_RELATION_TYPE_PUMP_SWITCH);
  }
}

TEST_F(CommonChannelPropertiesTest,
       protocolVersionDependedRelationWithParentChannel_PumpSwitch) {
  vector<int> functions = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL,
                           SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    CommonChannelPropertiesMock related_props_mock;

    EXPECT_CALL(related_props_mock, get_id).WillRepeatedly(Return(5000));
    EXPECT_CALL(related_props_mock, get_device_id).WillRepeatedly(Return(5));
    EXPECT_CALL(related_props_mock, get_channel_number)
        .WillRepeatedly(Return(10));
    EXPECT_CALL(related_props_mock, get_func).WillRepeatedly(Return(*it));

    EXPECT_CALL(related_props_mock, get_json_config).WillRepeatedly([]() {
      hvac_config *config = new hvac_config();
      TChannelConfig_HVAC hvac = {};
      hvac.PumpSwitchIsSet = 1;
      config->set_config(&hvac, 10);
      return config;
    });

    CommonChannelPropertiesMock mock;
    unsigned char version = 23;

    EXPECT_CALL(mock, get_func)
        .WillRepeatedly(Return(SUPLA_CHANNELFNC_PUMPSWITCH));
    EXPECT_CALL(mock, get_protocol_version).WillRepeatedly([&version]() {
      version++;
      return version;
    });

    EXPECT_CALL(mock, get_device_id).WillRepeatedly(Return(5));
    EXPECT_CALL(mock, get_id).WillRepeatedly(Return(5001));

    EXPECT_CALL(mock, for_each)
        .WillRepeatedly(
            [&](bool any_device,
                std::function<void(supla_abstract_common_channel_properties *,
                                   bool *)>
                    on_channel_properties) {
              bool will_continue = true;
              on_channel_properties(&related_props_mock, &will_continue);
              EXPECT_FALSE(any_device);
            });

    vector<supla_channel_relation> rel1, rel2;
    mock.get_channel_relations(&rel1, relation_with_parent_channel);
    mock.get_channel_relations(&rel2, relation_with_parent_channel);

    EXPECT_EQ(rel1.size(), 0);

    ASSERT_EQ(rel2.size(), 1);
    EXPECT_EQ(rel2.at(0).get_id(), 5001);
    EXPECT_EQ(rel2.at(0).get_parent_id(), 5000);
    EXPECT_EQ(rel2.at(0).get_relation_type(),
              CHANNEL_RELATION_TYPE_PUMP_SWITCH);
  }
}

TEST_F(CommonChannelPropertiesTest, relationWithSubchannel_OpeningSensor) {
  vector<int> functions = {SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK,
                           SUPLA_CHANNELFNC_CONTROLLINGTHEGATE,
                           SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR,
                           SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK,
                           SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER,
                           SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    CommonChannelPropertiesMock mock;

    bool shutter = *it == SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER ||
                   *it == SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW;

    EXPECT_CALL(mock, get_func).WillOnce(Return(*it));

    EXPECT_CALL(mock, get_id).WillRepeatedly(Return(*it + 10));
    EXPECT_CALL(mock, get_param2).WillOnce(Return(*it + 100));

    if (!shutter) {
      EXPECT_CALL(mock, get_param3).WillOnce(Return(*it + 200));
    }

    vector<supla_channel_relation> rel;
    mock.get_channel_relations(&rel, relation_with_sub_channel);

    EXPECT_EQ(rel.size(), shutter ? 1 : 2);

    EXPECT_EQ(rel.at(0).get_id(), *it + 100);
    EXPECT_EQ(rel.at(0).get_parent_id(), *it + 10);
    EXPECT_EQ(rel.at(0).get_relation_type(),
              CHANNEL_RELATION_TYPE_OPENING_SENSOR);

    if (!shutter) {
      EXPECT_EQ(rel.at(1).get_id(), *it + 200);
      EXPECT_EQ(rel.at(1).get_parent_id(), *it + 10);
      EXPECT_EQ(rel.at(1).get_relation_type(),
                CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR);
    }
  }
}

TEST_F(CommonChannelPropertiesTest, relationWithParentChannel_OpeningSensor) {
  vector<int> functions = {SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY,
                           SUPLA_CHANNELFNC_OPENINGSENSOR_GATE,
                           SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR,
                           SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR,
                           SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER,
                           SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    CommonChannelPropertiesMock mock;

    EXPECT_CALL(mock, get_func).WillOnce(Return(*it));

    EXPECT_CALL(mock, get_id).WillRepeatedly(Return(*it + 10));
    EXPECT_CALL(mock, get_param1).WillOnce(Return(*it + 100));
    EXPECT_CALL(mock, get_param2).WillOnce(Return(*it + 200));

    vector<supla_channel_relation> rel;
    mock.get_channel_relations(&rel, relation_with_parent_channel);

    EXPECT_EQ(rel.size(), 2);

    EXPECT_EQ(rel.at(0).get_id(), *it + 10);
    EXPECT_EQ(rel.at(0).get_parent_id(), *it + 100);
    EXPECT_EQ(rel.at(0).get_relation_type(),
              CHANNEL_RELATION_TYPE_OPENING_SENSOR);

    EXPECT_EQ(rel.at(1).get_id(), *it + 10);
    EXPECT_EQ(rel.at(1).get_parent_id(), *it + 200);
    EXPECT_EQ(rel.at(1).get_relation_type(),
              CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR);
  }
}

TEST_F(CommonChannelPropertiesTest, relationWithSubchannel_Meter) {
  vector<int> functions = {SUPLA_CHANNELFNC_POWERSWITCH,
                           SUPLA_CHANNELFNC_LIGHTSWITCH,
                           SUPLA_CHANNELFNC_STAIRCASETIMER};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    CommonChannelPropertiesMock mock;

    EXPECT_CALL(mock, get_func).WillOnce(Return(*it));
    EXPECT_CALL(mock, get_id).WillRepeatedly(Return(*it + 10));

    EXPECT_CALL(mock, get_json_config).WillRepeatedly([it]() {
      string json =
          "{\"relatedMeterChannelId\":" + std::to_string(*it + 100) + "}";

      power_switch_config *json_config = new power_switch_config();
      json_config->set_user_config(json.c_str());
      return json_config;
    });

    vector<supla_channel_relation> rel;
    mock.get_channel_relations(&rel, relation_with_sub_channel);

    EXPECT_EQ(rel.size(), 1);

    EXPECT_EQ(rel.at(0).get_id(), *it + 100);
    EXPECT_EQ(rel.at(0).get_parent_id(), *it + 10);
    EXPECT_EQ(rel.at(0).get_relation_type(), CHANNEL_RELATION_TYPE_METER);
  }
}

TEST_F(CommonChannelPropertiesTest, relationWithParentChannel_Meter) {
  vector<int> functions = {
      SUPLA_CHANNELFNC_ELECTRICITY_METER, SUPLA_CHANNELFNC_IC_ELECTRICITY_METER,
      SUPLA_CHANNELFNC_IC_GAS_METER, SUPLA_CHANNELFNC_IC_WATER_METER,
      SUPLA_CHANNELFNC_IC_HEAT_METER};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    CommonChannelPropertiesMock mock;

    EXPECT_CALL(mock, get_func).WillOnce(Return(*it));
    EXPECT_CALL(mock, get_id).WillRepeatedly(Return(*it + 10));

    EXPECT_CALL(mock, for_each)
        .WillRepeatedly(
            [&](bool any_device,
                std::function<void(supla_abstract_common_channel_properties *,
                                   bool *)>
                    on_channel_properties) {
              bool will_continue = true;
              vector<int> master_functions = {SUPLA_CHANNELFNC_POWERSWITCH,
                                              SUPLA_CHANNELFNC_LIGHTSWITCH,
                                              SUPLA_CHANNELFNC_STAIRCASETIMER};
              for (auto mit = master_functions.cbegin();
                   mit != master_functions.cend(); ++mit) {
                CommonChannelPropertiesMock related_props_mock;

                EXPECT_CALL(related_props_mock, get_json_config)
                    .WillRepeatedly([it]() {
                      string json = "{\"relatedMeterChannelId\":" +
                                    std::to_string(*it + 10) + "}";
                      power_switch_config *json_config =
                          new power_switch_config();
                      json_config->set_user_config(json.c_str());
                      return json_config;
                    });

                EXPECT_CALL(related_props_mock, get_id)
                    .WillRepeatedly(Return(*mit + 100));
                EXPECT_CALL(related_props_mock, get_func)
                    .WillRepeatedly(Return(*mit));

                on_channel_properties(&related_props_mock, &will_continue);
              }

              EXPECT_TRUE(any_device);
            });

    vector<supla_channel_relation> rel;
    mock.get_channel_relations(&rel, relation_with_parent_channel);

    ASSERT_EQ(rel.size(), 3);

    EXPECT_EQ(rel.at(0).get_id(), *it + 10);
    EXPECT_EQ(rel.at(0).get_parent_id(), SUPLA_CHANNELFNC_POWERSWITCH + 100);
    EXPECT_EQ(rel.at(0).get_relation_type(), CHANNEL_RELATION_TYPE_METER);

    EXPECT_EQ(rel.at(1).get_id(), *it + 10);
    EXPECT_EQ(rel.at(1).get_parent_id(), SUPLA_CHANNELFNC_LIGHTSWITCH + 100);
    EXPECT_EQ(rel.at(1).get_relation_type(), CHANNEL_RELATION_TYPE_METER);

    EXPECT_EQ(rel.at(2).get_id(), *it + 10);
    EXPECT_EQ(rel.at(2).get_parent_id(), SUPLA_CHANNELFNC_STAIRCASETIMER + 100);
    EXPECT_EQ(rel.at(2).get_relation_type(), CHANNEL_RELATION_TYPE_METER);
  }
}

TEST_F(CommonChannelPropertiesTest, relationWithSubchannel_MainThermometer) {
  vector<int> functions = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL,
                           SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER};

  vector<int> thermometer_functions = {SUPLA_CHANNELFNC_THERMOMETER,
                                       SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    for (auto tit = thermometer_functions.cbegin();
         tit != thermometer_functions.cend(); ++tit) {
      CommonChannelPropertiesMock mock;

      EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(10));
      EXPECT_CALL(mock, get_id()).WillRepeatedly(Return(*it + 100));
      EXPECT_CALL(mock, get_func()).WillRepeatedly(Return(*it));

      EXPECT_CALL(mock, get_json_config).WillRepeatedly([]() {
        hvac_config *config = new hvac_config();
        TChannelConfig_HVAC hvac = {};
        hvac.MainThermometerChannelNo = 5;
        config->set_config(&hvac, 10);
        return config;
      });

      EXPECT_CALL(mock, for_each)
          .WillRepeatedly(
              [&](bool any_device,
                  std::function<void(supla_abstract_common_channel_properties *,
                                     bool *)>
                      on_channel_properties) {
                bool will_continue = true;

                CommonChannelPropertiesMock related_props_mock;
                EXPECT_CALL(related_props_mock, get_channel_number)
                    .WillRepeatedly(Return(5));
                EXPECT_CALL(related_props_mock, get_func)
                    .WillRepeatedly(Return(*tit));
                EXPECT_CALL(related_props_mock, get_id())
                    .WillRepeatedly(Return(*tit + 50));

                on_channel_properties(&related_props_mock, &will_continue);

                EXPECT_FALSE(any_device);
              });

      vector<supla_channel_relation> rel;
      mock.get_channel_relations(&rel, relation_with_sub_channel);

      ASSERT_EQ(rel.size(), 1);

      EXPECT_EQ(rel.at(0).get_id(), *tit + 50);
      EXPECT_EQ(rel.at(0).get_parent_id(), *it + 100);
      EXPECT_EQ(rel.at(0).get_relation_type(),
                CHANNEL_RELATION_TYPE_MAIN_TERMOMETER);
    }
  }
}

TEST_F(CommonChannelPropertiesTest, relationWithParentChannel_MainThermometer) {
  vector<int> functions = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL,
                           SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER};

  vector<int> thermometer_functions = {SUPLA_CHANNELFNC_THERMOMETER,
                                       SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE};

  for (auto tit = thermometer_functions.cbegin();
       tit != thermometer_functions.cend(); ++tit) {
    CommonChannelPropertiesMock mock;
    EXPECT_CALL(mock, get_func()).WillRepeatedly(Return(*tit));
    EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(5));

    EXPECT_CALL(mock, get_id()).WillRepeatedly(Return(*tit + 50));

    for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
      EXPECT_CALL(mock, for_each)
          .WillRepeatedly(
              [&](bool any_device,
                  std::function<void(supla_abstract_common_channel_properties *,
                                     bool *)>
                      on_channel_properties) {
                bool will_continue = true;

                CommonChannelPropertiesMock related_props_mock;

                EXPECT_CALL(related_props_mock, get_id())
                    .WillRepeatedly(Return(*it + 100));

                EXPECT_CALL(related_props_mock, get_func)
                    .WillRepeatedly(Return(*it));

                EXPECT_CALL(related_props_mock, get_json_config)
                    .WillRepeatedly([]() {
                      hvac_config *config = new hvac_config();
                      TChannelConfig_HVAC hvac = {};
                      hvac.MainThermometerChannelNo = 5;
                      config->set_config(&hvac, 10);
                      return config;
                    });

                on_channel_properties(&related_props_mock, &will_continue);

                EXPECT_FALSE(any_device);
              });

      vector<supla_channel_relation> rel;
      mock.get_channel_relations(&rel, relation_with_parent_channel);

      ASSERT_EQ(rel.size(), 1);

      EXPECT_EQ(rel.at(0).get_id(), *tit + 50);
      EXPECT_EQ(rel.at(0).get_parent_id(), *it + 100);
      EXPECT_EQ(rel.at(0).get_relation_type(),
                CHANNEL_RELATION_TYPE_MAIN_TERMOMETER);
    }
  }
}

TEST_F(CommonChannelPropertiesTest, relationWithSubchannel_AuxThermometer) {
  vector<int> functions = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL,
                           SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER};

  vector<int> thermometer_functions = {SUPLA_CHANNELFNC_THERMOMETER,
                                       SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE};

  for (char a = 0; a < 4; a++) {
    for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
      for (auto tit = thermometer_functions.cbegin();
           tit != thermometer_functions.cend(); ++tit) {
        CommonChannelPropertiesMock mock;

        EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(10));
        EXPECT_CALL(mock, get_id()).WillRepeatedly(Return(*it + 100));
        EXPECT_CALL(mock, get_func()).WillRepeatedly(Return(*it));

        EXPECT_CALL(mock, get_json_config).WillRepeatedly([a]() {
          hvac_config *config = new hvac_config();
          TChannelConfig_HVAC hvac = {};
          hvac.AuxThermometerChannelNo = 5;
          hvac.AuxThermometerType = SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR + a;
          config->set_config(&hvac, 10);
          return config;
        });

        EXPECT_CALL(mock, for_each)
            .WillRepeatedly(
                [&](bool any_device,
                    std::function<void(
                        supla_abstract_common_channel_properties *, bool *)>
                        on_channel_properties) {
                  bool will_continue = true;

                  CommonChannelPropertiesMock related_props_mock;
                  EXPECT_CALL(related_props_mock, get_channel_number)
                      .WillRepeatedly(Return(5));
                  EXPECT_CALL(related_props_mock, get_func)
                      .WillRepeatedly(Return(*tit));
                  EXPECT_CALL(related_props_mock, get_id())
                      .WillRepeatedly(Return(*tit + 50));

                  on_channel_properties(&related_props_mock, &will_continue);

                  EXPECT_FALSE(any_device);
                });

        vector<supla_channel_relation> rel;
        mock.get_channel_relations(&rel, relation_with_sub_channel);

        ASSERT_EQ(rel.size(), 1);

        EXPECT_EQ(rel.at(0).get_id(), *tit + 50);
        EXPECT_EQ(rel.at(0).get_parent_id(), *it + 100);
        EXPECT_EQ(rel.at(0).get_relation_type(),
                  CHANNEL_RELATION_TYPE_AUX_THERMOMETER_FLOOR + a);
      }
    }
  }
}

TEST_F(CommonChannelPropertiesTest, relationWithParentChannel_AuxThermometer) {
  vector<int> functions = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL,
                           SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER};

  vector<int> thermometer_functions = {SUPLA_CHANNELFNC_THERMOMETER,
                                       SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE};

  for (char a = 0; a < 4; a++) {
    for (auto tit = thermometer_functions.cbegin();
         tit != thermometer_functions.cend(); ++tit) {
      CommonChannelPropertiesMock mock;
      EXPECT_CALL(mock, get_func()).WillRepeatedly(Return(*tit));
      EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(5));

      EXPECT_CALL(mock, get_id()).WillRepeatedly(Return(*tit + 50));

      for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
        EXPECT_CALL(mock, for_each)
            .WillRepeatedly(
                [&](bool any_device,
                    std::function<void(
                        supla_abstract_common_channel_properties *, bool *)>
                        on_channel_properties) {
                  bool will_continue = true;

                  CommonChannelPropertiesMock related_props_mock;

                  EXPECT_CALL(related_props_mock, get_id())
                      .WillRepeatedly(Return(*it + 100));

                  EXPECT_CALL(related_props_mock, get_func)
                      .WillRepeatedly(Return(*it));

                  EXPECT_CALL(related_props_mock, get_json_config)
                      .WillRepeatedly([a]() {
                        hvac_config *config = new hvac_config();
                        TChannelConfig_HVAC hvac = {};
                        hvac.AuxThermometerChannelNo = 5;
                        hvac.AuxThermometerType =
                            SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR + a;
                        config->set_config(&hvac, 10);
                        return config;
                      });

                  on_channel_properties(&related_props_mock, &will_continue);

                  EXPECT_FALSE(any_device);
                });

        vector<supla_channel_relation> rel;
        mock.get_channel_relations(&rel, relation_with_parent_channel);

        ASSERT_EQ(rel.size(), 1);

        EXPECT_EQ(rel.at(0).get_id(), *tit + 50);
        EXPECT_EQ(rel.at(0).get_parent_id(), *it + 100);
        EXPECT_EQ(rel.at(0).get_relation_type(),
                  CHANNEL_RELATION_TYPE_AUX_THERMOMETER_FLOOR + a);
      }
    }
  }
}

TEST_F(CommonChannelPropertiesTest, relationWithSubchannel_BinarySensor) {
  vector<int> functions = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL,
                           SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    CommonChannelPropertiesMock mock;

    EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(10));
    EXPECT_CALL(mock, get_id()).WillRepeatedly(Return(*it + 100));
    EXPECT_CALL(mock, get_func()).WillRepeatedly(Return(*it));

    EXPECT_CALL(mock, get_json_config).WillRepeatedly([]() {
      hvac_config *config = new hvac_config();
      TChannelConfig_HVAC hvac = {};
      hvac.BinarySensorChannelNo = 5;
      config->set_config(&hvac, 10);
      return config;
    });

    EXPECT_CALL(mock, for_each)
        .WillRepeatedly(
            [&](bool any_device,
                std::function<void(supla_abstract_common_channel_properties *,
                                   bool *)>
                    on_channel_properties) {
              bool will_continue = true;

              CommonChannelPropertiesMock related_props_mock;
              EXPECT_CALL(related_props_mock, get_channel_number)
                  .WillRepeatedly(Return(5));
              EXPECT_CALL(related_props_mock, get_type)
                  .WillRepeatedly(Return(SUPLA_CHANNELTYPE_BINARYSENSOR));
              EXPECT_CALL(related_props_mock, get_id())
                  .WillRepeatedly(Return(50));

              on_channel_properties(&related_props_mock, &will_continue);

              EXPECT_FALSE(any_device);
            });

    vector<supla_channel_relation> rel;
    mock.get_channel_relations(&rel, relation_with_sub_channel);

    ASSERT_EQ(rel.size(), 1);

    EXPECT_EQ(rel.at(0).get_id(), 50);
    EXPECT_EQ(rel.at(0).get_parent_id(), *it + 100);
    EXPECT_EQ(rel.at(0).get_relation_type(), CHANNEL_RELATION_TYPE_DEFAULT);
  }
}

TEST_F(CommonChannelPropertiesTest, relationWithParentChannel_BinarySensor) {
  vector<int> functions = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL,
                           SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER};

  for (auto it = functions.cbegin(); it != functions.cend(); ++it) {
    CommonChannelPropertiesMock mock;
    EXPECT_CALL(mock, get_type())
        .WillRepeatedly(Return(SUPLA_CHANNELTYPE_BINARYSENSOR));
    EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(5));

    EXPECT_CALL(mock, get_id()).WillRepeatedly(Return(50));

    EXPECT_CALL(mock, for_each)
        .WillRepeatedly([&](bool any_device,
                            std::function<void(
                                supla_abstract_common_channel_properties *,
                                bool *)>
                                on_channel_properties) {
          bool will_continue = true;

          CommonChannelPropertiesMock related_props_mock;

          EXPECT_CALL(related_props_mock, get_id())
              .WillRepeatedly(Return(*it + 100));

          EXPECT_CALL(related_props_mock, get_func).WillRepeatedly(Return(*it));

          EXPECT_CALL(related_props_mock, get_json_config).WillRepeatedly([]() {
            hvac_config *config = new hvac_config();
            TChannelConfig_HVAC hvac = {};
            hvac.BinarySensorChannelNo = 5;
            config->set_config(&hvac, 10);
            return config;
          });

          on_channel_properties(&related_props_mock, &will_continue);

          EXPECT_FALSE(any_device);
        });

    vector<supla_channel_relation> rel;
    mock.get_channel_relations(&rel, relation_with_parent_channel);

    ASSERT_EQ(rel.size(), 1);

    EXPECT_EQ(rel.at(0).get_id(), 50);
    EXPECT_EQ(rel.at(0).get_parent_id(), *it + 100);
    EXPECT_EQ(rel.at(0).get_relation_type(), CHANNEL_RELATION_TYPE_DEFAULT);
  }
}

template <typename config_classT, typename raw_config_T>
void CommonChannelPropertiesTest::relationWithSubchannel_FloodSensor(
    int parnet_channel_func) {
  for (char x = 0; x < 2; x++) {
    CommonChannelPropertiesMock mock;

    EXPECT_CALL(mock, get_protocol_version).WillRepeatedly(Return(26 + x));

    EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(10));
    EXPECT_CALL(mock, get_id()).WillRepeatedly(Return(1000));
    EXPECT_CALL(mock, get_func()).WillRepeatedly(Return(parnet_channel_func));

    EXPECT_CALL(mock, get_json_config).WillRepeatedly([]() {
      config_classT *config = new config_classT();
      raw_config_T raw_cfg = {};
      raw_cfg.SensorInfo[1].IsSet = 1;
      raw_cfg.SensorInfo[1].ChannelNo = 5;
      raw_cfg.SensorInfo[5].IsSet = 1;
      raw_cfg.SensorInfo[5].ChannelNo = 15;
      config->set_config(&raw_cfg);
      return config;
    });

    EXPECT_CALL(mock, for_each)
        .WillRepeatedly(
            [&](bool any_device,
                std::function<void(supla_abstract_common_channel_properties *,
                                   bool *)>
                    on_channel_properties) {
              bool will_continue = true;

              char channel_number = 0;

              while (will_continue && channel_number < 100) {
                CommonChannelPropertiesMock related_props_mock;
                EXPECT_CALL(related_props_mock, get_channel_number)
                    .WillRepeatedly(Return(channel_number));
                EXPECT_CALL(related_props_mock, get_func)
                    .WillRepeatedly(Return(SUPLA_CHANNELFNC_FLOOD_SENSOR));
                EXPECT_CALL(related_props_mock, get_id())
                    .WillRepeatedly(Return(50 + channel_number));

                on_channel_properties(&related_props_mock, &will_continue);
                channel_number++;
              }

              EXPECT_FALSE(any_device);
            });

    vector<supla_channel_relation> rel;
    mock.get_channel_relations(&rel, relation_with_sub_channel);

    if (x == 0) {
      ASSERT_EQ(rel.size(), 0);
    } else {
      ASSERT_EQ(rel.size(), 2);

      EXPECT_EQ(rel.at(0).get_id(), 55);
      EXPECT_EQ(rel.at(0).get_parent_id(), 1000);
      EXPECT_EQ(rel.at(0).get_relation_type(), CHANNEL_RELATION_TYPE_DEFAULT);

      EXPECT_EQ(rel.at(1).get_id(), 65);
      EXPECT_EQ(rel.at(1).get_parent_id(), 1000);
      EXPECT_EQ(rel.at(1).get_relation_type(), CHANNEL_RELATION_TYPE_DEFAULT);
    }
  }
}

TEST_F(CommonChannelPropertiesTest, relationWithSubchannel_FloodSensor) {
  relationWithSubchannel_FloodSensor<valve_config, TChannelConfig_Valve>(
      SUPLA_CHANNELFNC_VALVE_OPENCLOSE);

  relationWithSubchannel_FloodSensor<container_config,
                                     TChannelConfig_Container>(
      SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR);
}

template <typename config_classT, typename raw_config_T>
void CommonChannelPropertiesTest::relationWithParentChannel_FloodSensor(
    int parnet_channel_func) {
  for (char x = 0; x < 2; x++) {
    CommonChannelPropertiesMock mock;

    EXPECT_CALL(mock, get_protocol_version).WillRepeatedly(Return(26 + x));

    EXPECT_CALL(mock, get_func())
        .WillRepeatedly(Return(SUPLA_CHANNELFNC_FLOOD_SENSOR));
    EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(15));

    EXPECT_CALL(mock, get_id()).WillRepeatedly(Return(65));

    EXPECT_CALL(mock, for_each)
        .WillRepeatedly(
            [&](bool any_device,
                std::function<void(supla_abstract_common_channel_properties *,
                                   bool *)>
                    on_channel_properties) {
              bool will_continue = true;

              CommonChannelPropertiesMock related_props_mock;

              EXPECT_CALL(related_props_mock, get_id())
                  .WillRepeatedly(Return(1000));

              EXPECT_CALL(related_props_mock, get_func)
                  .WillRepeatedly(Return(parnet_channel_func));

              EXPECT_CALL(related_props_mock, get_json_config)
                  .WillRepeatedly([]() {
                    config_classT *config = new config_classT();
                    raw_config_T raw_cfg = {};
                    raw_cfg.SensorInfo[5].IsSet = 1;
                    raw_cfg.SensorInfo[5].ChannelNo = 15;
                    config->set_config(&raw_cfg);
                    return config;
                  });

              on_channel_properties(&related_props_mock, &will_continue);

              EXPECT_FALSE(any_device);
            });

    vector<supla_channel_relation> rel;
    mock.get_channel_relations(&rel, relation_with_parent_channel);

    ASSERT_EQ(rel.size(), x);

    if (x == 1) {
      EXPECT_EQ(rel.at(0).get_id(), 65);
      EXPECT_EQ(rel.at(0).get_parent_id(), 1000);
      EXPECT_EQ(rel.at(0).get_relation_type(), CHANNEL_RELATION_TYPE_DEFAULT);
    }
  }
}

TEST_F(CommonChannelPropertiesTest, relationWithParentChannel_FloodSensor) {
  relationWithParentChannel_FloodSensor<valve_config, TChannelConfig_Valve>(
      SUPLA_CHANNELFNC_VALVE_OPENCLOSE);

  relationWithParentChannel_FloodSensor<container_config,
                                        TChannelConfig_Container>(
      SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR);
}

} /* namespace testing */
