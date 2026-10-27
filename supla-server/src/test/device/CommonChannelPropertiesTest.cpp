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

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "client/client_channel_relation_remote_updater.h"
#include "client/client_channel_relations.h"
#include "client/clientchannel.h"
#include "client/clientchannels.h"
#include "doubles/SrpcAdapterMock.h"
#include "doubles/device/CommonChannelPropertiesMock.h"
#include "jsonconfig/channel/container_config.h"
#include "jsonconfig/channel/hvac_config.h"
#include "jsonconfig/channel/power_switch_config.h"
#include "jsonconfig/channel/valve_config.h"

namespace testing {

using std::string;
using std::vector;

// Exercise the real Client-visible channel collection and relation pack
// builder. Only protocol negotiation is fixed; no mocked iteration or relation
// algorithm.
class RelationClientChannel : public supla_client_channel {
 protected:
  unsigned char get_protocol_version() override { return 29; }

 public:
  using supla_client_channel::supla_client_channel;
  bool remote_update_is_possible() override { return true; }
};

static supla_client_channel *relation_channel(
    supla_client_channels *channels, int id, int device, int function,
    const char *json = "{}", int type = SUPLA_CHANNELTYPE_THERMOMETER) {
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  auto channel = new RelationClientChannel(
      channels, id, 1, device, device, type, function, 0, 0, 0, 0, nullptr,
      nullptr, nullptr, "", 0, 0, 0, 0, 29, 0, value, 0, json, "{}", false);
  EXPECT_TRUE(channels->add(channel));
  return channel;
}

TEST_F(CommonChannelPropertiesTest, RemoteHvacThermometersFromBothEnds) {
  for (int function : {SUPLA_CHANNELFNC_THERMOMETER,
                       SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE}) {
    supla_client_channels channels(nullptr);
    auto thermostat = relation_channel(
        &channels, 100, 10, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
        "{\"mainThermometerChannelId\":102,"
        "\"auxThermometerChannelId\":103,\"auxThermometerType\":\"FLOOR\"}",
        SUPLA_CHANNELTYPE_HVAC);
    auto main = relation_channel(&channels, 102, 20, function);
    auto aux = relation_channel(&channels, 103, 30, function);
    auto children =
        thermostat->get_channel_relations(relation_with_sub_channel);
    ASSERT_EQ(2u, children.size());
    EXPECT_EQ(102, children[0].get_id());
    EXPECT_EQ(100, children[0].get_parent_id());
    EXPECT_EQ(CHANNEL_RELATION_TYPE_MAIN_TERMOMETER,
              children[0].get_relation_type());
    EXPECT_EQ(103, children[1].get_id());
    EXPECT_EQ(CHANNEL_RELATION_TYPE_AUX_THERMOMETER_FLOOR,
              children[1].get_relation_type());
    for (auto sensor : {main, aux}) {
      auto parents =
          sensor->get_channel_relations(relation_with_parent_channel);
      ASSERT_EQ(1u, parents.size());
      EXPECT_EQ(sensor->get_id(), parents[0].get_id());
      EXPECT_EQ(100, parents[0].get_parent_id());
      EXPECT_EQ(sensor == main ? CHANNEL_RELATION_TYPE_MAIN_TERMOMETER
                               : CHANNEL_RELATION_TYPE_AUX_THERMOMETER_FLOOR,
                parents[0].get_relation_type());
    }
  }
}

TEST_F(CommonChannelPropertiesTest,
       RemoteHvacThermometerRequiresCompatibleFunction) {
  supla_client_channels channels(nullptr);
  relation_channel(&channels, 100, 10, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
      "{\"mainThermometerChannelId\":102,\"auxThermometerChannelId\":102,"
      "\"auxThermometerType\":\"FLOOR\"}", SUPLA_CHANNELTYPE_HVAC);
  relation_channel(&channels, 102, 20, SUPLA_CHANNELFNC_POWERSWITCH,
                   "{}", SUPLA_CHANNELTYPE_RELAY);
  channels.for_each([](supla_client_channel *channel, bool *) {
    EXPECT_TRUE(channel->get_channel_relations(relation_any).empty());
  });
}

TEST_F(CommonChannelPropertiesTest, ConfigRelationsHaveSameScopeAcrossDevices) {
  struct Case {
    int type, function;
    const char *json;
    int related_type, related_function, relation_type;
    bool master = false;
  };
  const Case cases[] = {
      {SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
       "{\"binarySensorChannelId\":102}", SUPLA_CHANNELTYPE_BINARYSENSOR,
       SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW, CHANNEL_RELATION_TYPE_DEFAULT},
      {SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
       "{\"pumpSwitchChannelId\":102}", SUPLA_CHANNELTYPE_RELAY,
       SUPLA_CHANNELFNC_PUMPSWITCH, CHANNEL_RELATION_TYPE_PUMP_SWITCH},
      {SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
       "{\"heatOrColdSourceSwitchChannelId\":102}", SUPLA_CHANNELTYPE_RELAY,
       SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH,
       CHANNEL_RELATION_TYPE_HEAT_OR_COLD_SOURCE_SWITCH},
      {SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
       "{\"masterThermostatChannelId\":102}", SUPLA_CHANNELTYPE_HVAC,
       SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
       CHANNEL_RELATION_TYPE_MASTER_THERMOSTAT, true},
      {SUPLA_CHANNELTYPE_VALVE_OPENCLOSE, SUPLA_CHANNELFNC_VALVE_OPENCLOSE,
       "{\"floodSensorChannelIds\":[102]}", SUPLA_CHANNELTYPE_BINARYSENSOR,
       SUPLA_CHANNELFNC_FLOOD_SENSOR, CHANNEL_RELATION_TYPE_DEFAULT},
      {SUPLA_CHANNELTYPE_CONTAINER, SUPLA_CHANNELFNC_CONTAINER,
       "{\"sensors\":[{\"channelId\":102,\"fillLevel\":50}]}",
       SUPLA_CHANNELTYPE_BINARYSENSOR, SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR,
       CHANNEL_RELATION_TYPE_DEFAULT}};
  for (const auto &test : cases) {
    for (int related_device : {10, 20}) {
      SCOPED_TRACE(std::to_string(test.related_function) + ":device=" +
                   std::to_string(related_device));
      supla_client_channels channels(nullptr);
      auto consumer = relation_channel(&channels, 100, 10, test.function,
                                      test.json, test.type);
      NiceMock<SrpcAdapterMock> adapter;
      ON_CALL(adapter, get_proto_version).WillByDefault(Return(29));
      supla_client_channel_relation_remote_updater updater(&adapter);
      supla_client_channel_reactions relations(&updater);
      std::vector<TSC_SuplaChannelRelation> sent;
      EXPECT_CALL(adapter, sc_async_channel_relation_pack_update)
          .WillRepeatedly([&](TSC_SuplaChannelRelationPack *pack) {
            sent.assign(pack->items, pack->items + pack->count);
            return 1;
          });
      // A configured but invisible endpoint must not enter the Client pack.
      relations.load(&channels);
      EXPECT_TRUE(relations.update_remote());
      ASSERT_EQ(1u, sent.size());
      EXPECT_EQ(0, sent[0].Id);
      auto related = relation_channel(&channels, 102, related_device,
          test.related_function, "{}", test.related_type);
      const int child = test.master ? 100 : 102;
      const int parent = test.master ? 102 : 100;
      auto from_consumer = consumer->get_channel_relations(
          test.master ? relation_with_parent_channel
                      : relation_with_sub_channel);
      auto from_related = related->get_channel_relations(
          test.master ? relation_with_sub_channel
                      : relation_with_parent_channel);
      for (auto *edges : {&from_consumer, &from_related}) {
        EXPECT_EQ(1u, edges->size());
        if (edges->size() == 1) {
          EXPECT_EQ(child, edges->at(0).get_id());
          EXPECT_EQ(parent, edges->at(0).get_parent_id());
          EXPECT_EQ(test.relation_type, edges->at(0).get_relation_type());
        }
      }
      relations.load(&channels);
      EXPECT_TRUE(relations.update_remote());
      EXPECT_EQ(2u, sent.size());
      if (sent.size() == 2) {
        EXPECT_EQ(child, sent[0].Id);
        EXPECT_EQ(parent, sent[0].ParentId);
        EXPECT_EQ(test.relation_type, sent[0].Type);
      }
    }
  }
}

TEST_F(CommonChannelPropertiesTest,
       RemoteThermometerPackContainsOnlyVisibleChannels) {
  NiceMock<SrpcAdapterMock> adapter;
  ON_CALL(adapter, get_proto_version).WillByDefault(Return(29));
  supla_client_channel_relation_remote_updater updater(&adapter);
  supla_client_channel_reactions relations(&updater);
  supla_client_channels visible(nullptr), hidden(nullptr);
  relation_channel(&visible, 100, 10, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                   "{\"mainThermometerChannelId\":102}",
                   SUPLA_CHANNELTYPE_HVAC);
  relation_channel(&hidden, 102, 20, SUPLA_CHANNELFNC_THERMOMETER);
  std::vector<TSC_SuplaChannelRelation> sent;
  EXPECT_CALL(adapter, sc_async_channel_relation_pack_update)
      .WillRepeatedly([&](TSC_SuplaChannelRelationPack *pack) {
        sent.assign(pack->items, pack->items + pack->count);
        return 1;
      });
  relations.load(&visible);
  EXPECT_TRUE(relations.update_remote());
  ASSERT_EQ(1u, sent.size());
  EXPECT_EQ(0, sent[0].Id);  // Existing list-clear sentinel, no hidden ID.
  auto thermometer =
      relation_channel(&visible, 102, 20, SUPLA_CHANNELFNC_THERMOMETER);
  relations.load(&visible);
  EXPECT_TRUE(relations.update_remote());
  ASSERT_EQ(2u, sent.size());
  EXPECT_EQ(102, sent[0].Id);
  EXPECT_EQ(100, sent[0].ParentId);
  EXPECT_EQ(CHANNEL_RELATION_TYPE_MAIN_TERMOMETER, sent[0].Type);
  // Rebuilding after a function change removes the now-invalid relation.
  thermometer->set_func(SUPLA_CHANNELFNC_POWERSWITCH);
  relations.load(&visible);
  EXPECT_TRUE(relations.update_remote());
  ASSERT_EQ(1u, sent.size());
  EXPECT_EQ(0, sent[0].Id);
}

TEST_F(CommonChannelPropertiesTest, RemoteThermometerRelationsAtAccountScale) {
  supla_client_channels channels(nullptr);
  for (int i = 0; i < 100; ++i) {
    const auto json = "{\"mainThermometerChannelId\":" +
                      std::to_string(20000 + i) + "}";
    relation_channel(&channels, 10000 + i, i + 1,
        SUPLA_CHANNELFNC_HVAC_THERMOSTAT, json.c_str(), SUPLA_CHANNELTYPE_HVAC);
    relation_channel(&channels, 20000 + i, 1000 + i,
                     SUPLA_CHANNELFNC_THERMOMETER);
  }
  for (int i = 0; i < 800; ++i) {
    relation_channel(&channels, 30000 + i, 2000 + i,
                     SUPLA_CHANNELFNC_POWERSWITCH, "{}",
                     SUPLA_CHANNELTYPE_RELAY);
  }
  std::vector<supla_channel_relation> relations;
  const auto started = std::chrono::steady_clock::now();
  channels.for_each([&](supla_client_channel *channel, bool *) {
    channel->get_channel_relations(&relations, relation_any);
  });
  EXPECT_EQ(100u, relations.size());
  std::cout << "HVAC relations: channels=1000 hvac=100 thermometers=100 us="
            << std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::steady_clock::now() - started).count()
            << std::endl;
}

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

      EXPECT_CALL(related_props_mock, get_json_config).WillRepeatedly([&]() {
        hvac_config *config = new hvac_config();
        TChannelConfig_HVAC hvac = {};
        hvac.MasterThermostatChannelId = 5000;
        config->set_config(&hvac);
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
                EXPECT_TRUE(any_device);
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

      EXPECT_CALL(mock, get_json_config).WillRepeatedly([&]() {
        hvac_config *config = new hvac_config();
        TChannelConfig_HVAC hvac = {};

        hvac.MasterThermostatChannelId = 5000;
        config->set_config(&hvac);
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
                EXPECT_TRUE(any_device);
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

      hvac.HeatOrColdSourceSwitchChannelId = 5001;
      config->set_config(&hvac);
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
              EXPECT_TRUE(any_device);
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

      hvac.HeatOrColdSourceSwitchChannelId = 5001;
      config->set_config(&hvac);
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
              EXPECT_TRUE(any_device);
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

      hvac.PumpSwitchChannelId = 5001;
      config->set_config(&hvac);
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
              EXPECT_TRUE(any_device);
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

      hvac.PumpSwitchChannelId = 5001;
      config->set_config(&hvac);
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
              EXPECT_TRUE(any_device);
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

      EXPECT_CALL(mock, get_json_config).WillRepeatedly([&]() {
        hvac_config *config = new hvac_config();
        TChannelConfig_HVAC hvac = {};
        hvac.MainThermometerChannelId = *tit + 50;
        config->set_config(&hvac);
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

                EXPECT_TRUE(any_device);
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
                    .WillRepeatedly([&]() {
                      hvac_config *config = new hvac_config();
                      TChannelConfig_HVAC hvac = {};
                      hvac.MainThermometerChannelId = *tit + 50;
                      config->set_config(&hvac);
                      return config;
                    });

                on_channel_properties(&related_props_mock, &will_continue);

                EXPECT_TRUE(any_device);
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

        EXPECT_CALL(mock, get_json_config).WillRepeatedly([&]() {
          hvac_config *config = new hvac_config();
          TChannelConfig_HVAC hvac = {};
          hvac.AuxThermometerChannelId = *tit + 50;
          hvac.AuxThermometerType = SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR + a;
          config->set_config(&hvac);
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

                  EXPECT_TRUE(any_device);
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
                      .WillRepeatedly([&]() {
                        hvac_config *config = new hvac_config();
                        TChannelConfig_HVAC hvac = {};
                        hvac.AuxThermometerChannelId = *tit + 50;
                        hvac.AuxThermometerType =
                            SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR + a;
                        config->set_config(&hvac);
                        return config;
                      });

                  on_channel_properties(&related_props_mock, &will_continue);

                  EXPECT_TRUE(any_device);
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
      hvac.BinarySensorChannelId = 50;
      config->set_config(&hvac);
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

              EXPECT_TRUE(any_device);
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
            hvac.BinarySensorChannelId = 50;
            config->set_config(&hvac);
            return config;
          });

          on_channel_properties(&related_props_mock, &will_continue);

          EXPECT_TRUE(any_device);
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
void CommonChannelPropertiesTest::relationWithSubchannel_Sensor(
    int parnet_channel_func, int parnet_channel_type,
    int related_channel_func) {
  for (char x = 0; x < 2; x++) {
    CommonChannelPropertiesMock mock;

    EXPECT_CALL(mock, get_protocol_version).WillRepeatedly(Return(26 + x));

    EXPECT_CALL(mock, get_channel_number).WillRepeatedly(Return(10));
    EXPECT_CALL(mock, get_id()).WillRepeatedly(Return(1000));

    if (parnet_channel_func) {
      EXPECT_CALL(mock, get_func()).WillRepeatedly(Return(parnet_channel_func));
    }

    if (parnet_channel_type) {
      EXPECT_CALL(mock, get_type()).WillRepeatedly(Return(parnet_channel_type));
    }

    EXPECT_CALL(mock, get_json_config).WillRepeatedly([]() {
      config_classT *config = new config_classT();
      raw_config_T raw_cfg = {};
      raw_cfg.SensorInfo[1].ChannelId = 55;
      raw_cfg.SensorInfo[5].ChannelId = 65;
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
                    .WillRepeatedly(Return(related_channel_func));
                EXPECT_CALL(related_props_mock, get_id())
                    .WillRepeatedly(Return(50 + channel_number));

                on_channel_properties(&related_props_mock, &will_continue);
                channel_number++;
              }

              EXPECT_TRUE(any_device);
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

TEST_F(CommonChannelPropertiesTest, relationWithSubchannel_Sensor) {
  relationWithSubchannel_Sensor<valve_config, TChannelConfig_Valve>(
      SUPLA_CHANNELFNC_VALVE_OPENCLOSE, 0, SUPLA_CHANNELFNC_FLOOD_SENSOR);

  relationWithSubchannel_Sensor<container_config, TChannelConfig_Container>(
      0, SUPLA_CHANNELTYPE_CONTAINER, SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR);
}

template <typename config_classT, typename raw_config_T>
void CommonChannelPropertiesTest::relationWithParentChannel_Sensor(
    int parnet_channel_func, int parnet_channel_type,
    int related_channel_func) {
  for (char x = 1; x < 2; x++) {
    CommonChannelPropertiesMock mock;

    EXPECT_CALL(mock, get_protocol_version).WillRepeatedly(Return(26 + x));

    EXPECT_CALL(mock, get_func()).WillRepeatedly(Return(related_channel_func));
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

              if (parnet_channel_func) {
                EXPECT_CALL(related_props_mock, get_func)
                    .WillRepeatedly(Return(parnet_channel_func));
              }

              if (parnet_channel_type) {
                EXPECT_CALL(related_props_mock, get_type)
                    .WillRepeatedly(Return(parnet_channel_type));
              }

              EXPECT_CALL(related_props_mock, get_json_config)
                  .WillRepeatedly([]() {
                    config_classT *config = new config_classT();
                    raw_config_T raw_cfg = {};
                    raw_cfg.SensorInfo[5].ChannelId = 65;
                    config->set_config(&raw_cfg);
                    return config;
                  });

              on_channel_properties(&related_props_mock, &will_continue);

              EXPECT_TRUE(any_device);
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

TEST_F(CommonChannelPropertiesTest, relationWithParentChannel_Sensor) {
  relationWithParentChannel_Sensor<valve_config, TChannelConfig_Valve>(
      SUPLA_CHANNELFNC_VALVE_OPENCLOSE, 0, SUPLA_CHANNELFNC_FLOOD_SENSOR);

  relationWithParentChannel_Sensor<container_config, TChannelConfig_Container>(
      0, SUPLA_CHANNELTYPE_CONTAINER, SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR);
}

} /* namespace testing */

namespace testing {
namespace {
class ReferenceEncodingProperties : public CommonChannelPropertiesMock {
 public:
  using supla_abstract_common_channel_properties::get_config;
};
}  // namespace
TEST_F(CommonChannelPropertiesTest, explicitAllSixRecipientEncoding) {
  ReferenceEncodingProperties destination;
  ON_CALL(destination, get_type).WillByDefault(Return(SUPLA_CHANNELTYPE_HVAC));
  ON_CALL(destination, get_id).WillByDefault(Return(5000));
  ON_CALL(destination, get_device_id).WillByDefault(Return(50));
  ON_CALL(destination, get_channel_number).WillByDefault(Return(9));
  ON_CALL(destination, get_json_config).WillByDefault([] {
    auto config = new hvac_config();
    TChannelConfig_HVAC wire = {};
    wire.MainThermometerChannelId = 70000;
    wire.AuxThermometerChannelId = 70001;
    wire.BinarySensorChannelId = 70002;
    wire.MasterThermostatChannelId = 70003;

    wire.PumpSwitchChannelId = 70004;

    wire.HeatOrColdSourceSwitchChannelId = 70005;

    config->set_config(&wire);
    return config;
  });
  ON_CALL(destination, for_each).WillByDefault([](bool any_device, auto visit) {
    EXPECT_FALSE(any_device);
    for (int i = 0; i < 6; ++i) {
      CommonChannelPropertiesMock source;
      ON_CALL(source, get_id).WillByDefault(Return(70000 + i));
      ON_CALL(source, get_device_id).WillByDefault(Return(50));
      ON_CALL(source, get_channel_number).WillByDefault(Return(20 + i));
      bool more = true;
      visit(&source, &more);
      if (!more) break;
    }
  });
  for (auto encoding : {channel_reference_encoding::server_channel_id,
                        channel_reference_encoding::device_channel_number}) {
    char buffer[SUPLA_CHANNEL_CONFIG_MAXSIZE] = {};
    unsigned _supla_int16_t size = 0;
    destination.get_config(buffer, &size, SUPLA_CONFIG_TYPE_DEFAULT, 0,
                           encoding);
    ASSERT_EQ(sizeof(TChannelConfig_HVAC), size);
    auto wire = reinterpret_cast<TChannelConfig_HVAC *>(buffer);
    unsigned int base =
        encoding == channel_reference_encoding::server_channel_id ? 70000 : 20;
    EXPECT_EQ(base, wire->MainThermometerChannelId);
    EXPECT_EQ(base + 1, wire->AuxThermometerChannelId);
    EXPECT_EQ(base + 2, wire->BinarySensorChannelId);
    EXPECT_EQ(encoding == channel_reference_encoding::server_channel_id
                  ? base + 3
                  : ((base + 3) << 8) | 1,
              wire->MasterThermostatChannelId);
    EXPECT_EQ(encoding == channel_reference_encoding::server_channel_id
                  ? base + 4
                  : ((base + 4) << 8) | 1,
              wire->PumpSwitchChannelId);
    EXPECT_EQ(encoding == channel_reference_encoding::server_channel_id
                  ? base + 5
                  : ((base + 5) << 8) | 1,
              wire->HeatOrColdSourceSwitchChannelId);
  }
}
}  // namespace testing
