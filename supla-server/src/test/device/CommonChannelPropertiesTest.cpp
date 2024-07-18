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

#include <vector>

#include "doubles/device/CommonChannelPropertiesMock.h"
#include "jsonconfig/channel/hvac_config.h"
namespace testing {

using std::vector;

TEST_F(CommonChannelPropertiesTest, append) {
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

TEST_F(CommonChannelPropertiesTest, duplicates) {
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
  CommonChannelPropertiesMock related_props_mock;

  EXPECT_CALL(related_props_mock, get_id).WillRepeatedly(Return(5001));
  EXPECT_CALL(related_props_mock, get_device_id).WillRepeatedly(Return(5));
  EXPECT_CALL(related_props_mock, get_func)
      .WillRepeatedly(Return(SUPLA_CHANNELFNC_HVAC_THERMOSTAT));

  CommonChannelPropertiesMock mock;
  unsigned char version = 23;

  EXPECT_CALL(mock, get_func)
      .WillRepeatedly(Return(SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
  EXPECT_CALL(mock, get_protocol_version).WillRepeatedly([&version]() {
    version++;
    return version;
  });
  EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(10));
  EXPECT_CALL(mock, get_device_id).WillRepeatedly(Return(5));
  EXPECT_CALL(mock, get_id).WillRepeatedly(Return(5000));

  hvac_config config;
  TChannelConfig_HVAC hvac = {};
  hvac.MasterThermostatIsSet = 1;
  config.set_config(&hvac, 10);

  EXPECT_CALL(mock, get_json_config)
      .WillRepeatedly(Return(new hvac_config(&config)));

  EXPECT_CALL(mock, for_each)
      .WillRepeatedly(
          [&](std::function<void(supla_abstract_common_channel_properties *,
                                 bool *)>
                  on_channel_properties) {
            bool will_continue = true;
            on_channel_properties(&related_props_mock, &will_continue);
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

TEST_F(CommonChannelPropertiesTest,
       protocolVersionDependedRelationWithParentChannel_MasterThermostat) {
  CommonChannelPropertiesMock related_props_mock;

  EXPECT_CALL(related_props_mock, get_id).WillRepeatedly(Return(5000));
  EXPECT_CALL(related_props_mock, get_device_id).WillRepeatedly(Return(5));
  EXPECT_CALL(related_props_mock, get_channel_number)
      .WillRepeatedly(Return(10));
  EXPECT_CALL(related_props_mock, get_func)
      .WillRepeatedly(Return(SUPLA_CHANNELFNC_HVAC_THERMOSTAT));

  hvac_config config;
  TChannelConfig_HVAC hvac = {};
  hvac.MasterThermostatIsSet = 1;
  config.set_config(&hvac, 10);

  EXPECT_CALL(related_props_mock, get_json_config)
      .WillRepeatedly(Return(new hvac_config(&config)));

  CommonChannelPropertiesMock mock;
  unsigned char version = 23;

  EXPECT_CALL(mock, get_func)
      .WillRepeatedly(Return(SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
  EXPECT_CALL(mock, get_protocol_version).WillRepeatedly([&version]() {
    version++;
    return version;
  });

  EXPECT_CALL(mock, get_device_id).WillRepeatedly(Return(5));
  EXPECT_CALL(mock, get_id).WillRepeatedly(Return(5001));

  EXPECT_CALL(mock, for_each)
      .WillRepeatedly(
          [&](std::function<void(supla_abstract_common_channel_properties *,
                                 bool *)>
                  on_channel_properties) {
            bool will_continue = true;
            on_channel_properties(&related_props_mock, &will_continue);
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

TEST_F(CommonChannelPropertiesTest,
       protocolVersionDependedRelationWithSubchannel_HeatOrColdSource) {
  CommonChannelPropertiesMock related_props_mock;

  EXPECT_CALL(related_props_mock, get_id).WillRepeatedly(Return(5001));
  EXPECT_CALL(related_props_mock, get_device_id).WillRepeatedly(Return(5));
  EXPECT_CALL(related_props_mock, get_func)
      .WillRepeatedly(Return(SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH));

  CommonChannelPropertiesMock mock;
  unsigned char version = 23;

  EXPECT_CALL(mock, get_func)
      .WillRepeatedly(Return(SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
  EXPECT_CALL(mock, get_protocol_version).WillRepeatedly([&version]() {
    version++;
    return version;
  });
  EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(10));
  EXPECT_CALL(mock, get_device_id).WillRepeatedly(Return(5));
  EXPECT_CALL(mock, get_id).WillRepeatedly(Return(5000));

  hvac_config config;
  TChannelConfig_HVAC hvac = {};
  hvac.HeatOrColdSourceSwitchIsSet = 1;
  config.set_config(&hvac, 10);

  EXPECT_CALL(mock, get_json_config)
      .WillRepeatedly(Return(new hvac_config(&config)));

  EXPECT_CALL(mock, for_each)
      .WillRepeatedly(
          [&](std::function<void(supla_abstract_common_channel_properties *,
                                 bool *)>
                  on_channel_properties) {
            bool will_continue = true;
            on_channel_properties(&related_props_mock, &will_continue);
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

TEST_F(CommonChannelPropertiesTest,
       protocolVersionDependedRelationWithParentChannel_HeatOrColdSource) {
  CommonChannelPropertiesMock related_props_mock;

  EXPECT_CALL(related_props_mock, get_id).WillRepeatedly(Return(5000));
  EXPECT_CALL(related_props_mock, get_device_id).WillRepeatedly(Return(5));
  EXPECT_CALL(related_props_mock, get_channel_number)
      .WillRepeatedly(Return(10));
  EXPECT_CALL(related_props_mock, get_func)
      .WillRepeatedly(Return(SUPLA_CHANNELFNC_HVAC_THERMOSTAT));

  hvac_config config;
  TChannelConfig_HVAC hvac = {};
  hvac.HeatOrColdSourceSwitchIsSet = 1;
  config.set_config(&hvac, 10);

  EXPECT_CALL(related_props_mock, get_json_config)
      .WillRepeatedly(Return(new hvac_config(&config)));

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
          [&](std::function<void(supla_abstract_common_channel_properties *,
                                 bool *)>
                  on_channel_properties) {
            bool will_continue = true;
            on_channel_properties(&related_props_mock, &will_continue);
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

TEST_F(CommonChannelPropertiesTest,
       protocolVersionDependedRelationWithSubchannel_PumpSwitch) {
  CommonChannelPropertiesMock related_props_mock;

  EXPECT_CALL(related_props_mock, get_id).WillRepeatedly(Return(5001));
  EXPECT_CALL(related_props_mock, get_device_id).WillRepeatedly(Return(5));
  EXPECT_CALL(related_props_mock, get_func)
      .WillRepeatedly(Return(SUPLA_CHANNELFNC_PUMPSWITCH));

  CommonChannelPropertiesMock mock;
  unsigned char version = 23;

  EXPECT_CALL(mock, get_func)
      .WillRepeatedly(Return(SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
  EXPECT_CALL(mock, get_protocol_version).WillRepeatedly([&version]() {
    version++;
    return version;
  });
  EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(10));
  EXPECT_CALL(mock, get_device_id).WillRepeatedly(Return(5));
  EXPECT_CALL(mock, get_id).WillRepeatedly(Return(5000));

  hvac_config config;
  TChannelConfig_HVAC hvac = {};
  hvac.PumpSwitchIsSet = 1;
  config.set_config(&hvac, 10);

  EXPECT_CALL(mock, get_json_config)
      .WillRepeatedly(Return(new hvac_config(&config)));

  EXPECT_CALL(mock, for_each)
      .WillRepeatedly(
          [&](std::function<void(supla_abstract_common_channel_properties *,
                                 bool *)>
                  on_channel_properties) {
            bool will_continue = true;
            on_channel_properties(&related_props_mock, &will_continue);
          });

  vector<supla_channel_relation> rel1, rel2;
  mock.get_channel_relations(&rel1, relation_with_sub_channel);
  mock.get_channel_relations(&rel2, relation_with_sub_channel);

  EXPECT_EQ(rel1.size(), 0);

  ASSERT_EQ(rel2.size(), 1);
  EXPECT_EQ(rel2.at(0).get_id(), 5001);
  EXPECT_EQ(rel2.at(0).get_parent_id(), 5000);
  EXPECT_EQ(rel2.at(0).get_relation_type(), CHANNEL_RELATION_TYPE_PUMP_SWITCH);
}

TEST_F(CommonChannelPropertiesTest,
       protocolVersionDependedRelationWithParentChannel_PumpSwitch) {
  CommonChannelPropertiesMock related_props_mock;

  EXPECT_CALL(related_props_mock, get_id).WillRepeatedly(Return(5000));
  EXPECT_CALL(related_props_mock, get_device_id).WillRepeatedly(Return(5));
  EXPECT_CALL(related_props_mock, get_channel_number)
      .WillRepeatedly(Return(10));
  EXPECT_CALL(related_props_mock, get_func)
      .WillRepeatedly(Return(SUPLA_CHANNELFNC_HVAC_THERMOSTAT));

  hvac_config config;
  TChannelConfig_HVAC hvac = {};
  hvac.PumpSwitchIsSet = 1;
  config.set_config(&hvac, 10);

  EXPECT_CALL(related_props_mock, get_json_config)
      .WillRepeatedly(Return(new hvac_config(&config)));

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
          [&](std::function<void(supla_abstract_common_channel_properties *,
                                 bool *)>
                  on_channel_properties) {
            bool will_continue = true;
            on_channel_properties(&related_props_mock, &will_continue);
          });

  vector<supla_channel_relation> rel1, rel2;
  mock.get_channel_relations(&rel1, relation_with_parent_channel);
  mock.get_channel_relations(&rel2, relation_with_parent_channel);

  EXPECT_EQ(rel1.size(), 0);

  ASSERT_EQ(rel2.size(), 1);
  EXPECT_EQ(rel2.at(0).get_id(), 5001);
  EXPECT_EQ(rel2.at(0).get_parent_id(), 5000);
  EXPECT_EQ(rel2.at(0).get_relation_type(), CHANNEL_RELATION_TYPE_PUMP_SWITCH);
}

} /* namespace testing */
