// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include <gtest/gtest.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "conn/connection.h"
#include "device/device_dao.h"
#include "device/devicechannels.h"
#include "ipc/on_channel_config_changed_command.h"
#include "ipc/on_channel_deleted_command.h"
#include "ipc/on_device_deleted_command.h"
#include "jsonconfig/channel/hvac_config.h"
#include "suplan/peer_dao.h"
#include "suplan/server_peer_transport.h"
#include "test/doubles/device/DeviceStub.h"
#include "test/doubles/ipc/IpcSocketAdapterMock.h"
#include "test/doubles/suplan/PeerProvisionerTestAccess.h"
#include "test/integration/IntegrationTest.h"
#include "user.h"
#include "supla-socket.h"
#include "sthread.h"
using namespace supla_suplan;  // NOLINT
namespace {
class SupLanPeerDaoIntegrationTest : public testing::Test,
                                     public testing::IntegrationTest {
 protected:
  PeerService::Factory factory = [] { return std::make_unique<PeerDao>(2); };
  PeerService service{factory, nullptr};
  int source = 0, destination = 0;
  uint32_t resource = 0;
  void SetUp() override {
    initTestDatabase();
    runSqlScript("SupLanM2TestSchema.sql");
    // Use the standard Core fixture's actual ownership; no hardcoded keys.
    supla_mariadb_access_provider db;
    ASSERT_TRUE(db.connect());
    void *stmt = nullptr;
    ASSERT_TRUE(db.stmt_get_int(
        &stmt, &source, &destination, nullptr, nullptr,
        "SELECT MIN(id),MAX(id) FROM supla_iodevice WHERE user_id=2", nullptr,
        0, true));
    int channel = 0;
    ASSERT_TRUE(db.stmt_get_int(
        &stmt, &channel, nullptr, nullptr, nullptr,
        ("SELECT MIN(id) FROM supla_dev_channel WHERE iodevice_id=" +
         std::to_string(source))
            .c_str(),
        nullptr, 0, true));
    resource = channel;
    ASSERT_GT(source, 0);
    ASSERT_GT(destination, source);
    ASSERT_GT(resource, 0u);
    auto dao = factory();
    ASSERT_TRUE(dao->accept_identity(source, {123, {resource}}));
  }
  Association state() {
    Association a;
    auto dao = factory();
    EXPECT_TRUE(dao->find(source, destination, &a));
    return a;
  }
};
TEST_F(SupLanPeerDaoIntegrationTest,
       StableIdentityCanonicalProjectionAndPartialAckSurviveDaoRestart) {
  Grant g{1, resource, 1, 100, 1};
  ASSERT_TRUE(service.upsert(source, destination, g));
  auto a = state();
  EXPECT_EQ(1u, a.revision);
  EXPECT_EQ(1u, a.acl.size());
  g.origin_id = 200;
  ASSERT_TRUE(service.upsert(source, destination, g));
  ASSERT_TRUE(service.delete_origin(1, 100));
  EXPECT_EQ(1u, state().revision);
  g.permissions = 3;
  ASSERT_TRUE(service.upsert(source, destination, g));
  EXPECT_EQ(2u, state().revision);
  EXPECT_EQ(2, state().acl[0].Permissions);
  ASSERT_TRUE(service.provisioned(state(), 123));
  ASSERT_TRUE(service.delete_origin(1, 200));
  a = state();
  EXPECT_EQ(Lifecycle::Draining, a.lifecycle);
  EXPECT_TRUE(a.acl.empty());
  ASSERT_TRUE(service.cleanup_ack(a, true));
  auto reloaded = state();
  EXPECT_TRUE(reloaded.source_acked);
  EXPECT_FALSE(reloaded.destination_acked);
  ASSERT_TRUE(service.cleanup_ack(reloaded, false));
  EXPECT_EQ(Lifecycle::Dormant, state().lifecycle);
}
TEST_F(SupLanPeerDaoIntegrationTest,
       CurrentRootCleanupContextAndPartialAckSurviveDaoRestart) {
  Grant g{1, resource, 1, 100, 1};
  ASSERT_TRUE(service.upsert(source, destination, g));
  ASSERT_TRUE(service.provisioned(state(), 123));
  ASSERT_TRUE(factory()->accept_identity(source, {456, {resource}}));
  // A replacement may already be installed under 456, with its ACK unknown.
  ASSERT_TRUE(service.delete_origin(1, 100));
  auto a = state();
  EXPECT_EQ(456u, a.root);
  EXPECT_EQ(Lifecycle::Draining, a.lifecycle);
  ASSERT_TRUE(service.cleanup_ack(a, false));
  ASSERT_TRUE(service.observe_root(a.id));
  a = state();
  EXPECT_TRUE(a.destination_acked);
  EXPECT_FALSE(a.source_acked);
  EXPECT_EQ(Lifecycle::Draining, a.lifecycle);
  ASSERT_TRUE(service.cleanup_ack(a, true));
  EXPECT_EQ(Lifecycle::Dormant, state().lifecycle);
  ASSERT_TRUE(service.upsert(source, destination, g));
  EXPECT_EQ(2u, state().generation);
  EXPECT_EQ(1u, state().revision);
}
TEST_F(SupLanPeerDaoIntegrationTest,
       ConcurrentMutationAcrossIndependentConnectionsHasNoLostUpdate) {
  Grant g{1, resource, 1, 100, 1};
  ASSERT_TRUE(service.upsert(source, destination, g));
  std::atomic<int> successes{0};
  std::vector<std::thread> threads;
  for (int i = 0; i < 8; ++i)
    threads.emplace_back([&, i] {
      mysql_thread_init();
      Grant v = g;
      v.origin_id += i + 1;
      v.permissions = i % 2 ? 2 : 4;
      if (service.upsert(source, destination, v)) ++successes;
      mysql_thread_end();
    });
  for (auto &t : threads) t.join();
  EXPECT_EQ(8, successes);
  auto a = state();
  EXPECT_EQ(6, a.acl[0].Permissions);
  auto dao = factory();
  std::vector<Grant> grants;
  ASSERT_TRUE(dao->grants(a.id, &grants));
  EXPECT_EQ(9u, grants.size());
}
TEST_F(SupLanPeerDaoIntegrationTest,
       StaleOwnershipCleanupIsScopedToTheOldAssociation) {
  Grant g{1, resource, 1, 100, 1};
  ASSERT_TRUE(service.upsert(source, destination, g));
  auto old = state();
  auto dao = factory();
  ASSERT_TRUE(dao->owns_acl(source, destination, old.acl));
  std::string out;
  supla_mariadb_access_provider db;
  ASSERT_TRUE(db.connect());
  void *stmt = nullptr;
  int number = 0;
  ASSERT_TRUE(db.stmt_get_int(
      &stmt, &number, nullptr, nullptr, nullptr,
      ("SELECT COALESCE(MAX(channel_number),-1)+1 FROM supla_dev_channel "
       "WHERE iodevice_id=" +
       std::to_string(destination))
          .c_str(),
      nullptr, 0, true));
  const std::string query = "UPDATE supla_dev_channel SET iodevice_id=" +
                            std::to_string(destination) +
                            ",channel_number=" + std::to_string(number) +
                            " WHERE id=" + std::to_string(resource);
  sqlQuery(query.c_str(), &out);
  EXPECT_FALSE(dao->owns_acl(source, destination, old.acl));
  ASSERT_TRUE(dao->accept_identity(destination, {456, {resource}}));
  g.origin_id = 200;
  ASSERT_TRUE(service.upsert(destination, source, g));
  ASSERT_TRUE(service.delete_resource(old.id, 1, resource));
  EXPECT_EQ(Lifecycle::Draining, state().lifecycle);
  Association current;
  ASSERT_TRUE(dao->find(destination, source, &current));
  EXPECT_EQ(Lifecycle::Active, current.lifecycle);
  EXPECT_EQ(1u, current.acl.size());
  EXPECT_EQ(1u, current.revision);
  EXPECT_TRUE(dao->owns_acl(destination, source, current.acl));
}
TEST_F(SupLanPeerDaoIntegrationTest,
       RollbackAndScopedIndexesNoUnchangedProjectionWrites) {
  Grant g{1, resource, 1, 100, 1};
  ASSERT_TRUE(service.upsert(source, destination, g));
  std::string out;
  sqlQuery("CREATE TABLE suplan_test_writes (n INT NOT NULL)", &out);
  sqlQuery("INSERT INTO suplan_test_writes VALUES (0)", &out);
  sqlQuery(
      "CREATE TRIGGER suplan_test_write_counter AFTER UPDATE ON "
      "supla_suplan_peer_association "
      "FOR EACH ROW UPDATE suplan_test_writes SET n=n+1",
      &out);
  g.origin_id = 200;
  ASSERT_TRUE(service.upsert(source, destination, g));
  ASSERT_TRUE(service.delete_origin(1, 100));
  sqlQuery("SELECT n FROM suplan_test_writes", &out);
  EXPECT_EQ("n\n0\n", out);
  auto dao = factory();
  Association a = state();
  ASSERT_TRUE(dao->begin(a.id, &a));
  ASSERT_TRUE(dao->erase_grants(a.id));
  dao->rollback();
  std::vector<Grant> grants;
  ASSERT_TRUE(dao->grants(a.id, &grants));
  EXPECT_EQ(1u, grants.size());
  out.clear();
  sqlQuery(
      "SELECT COUNT(*) AS n FROM information_schema.STATISTICS WHERE "
      "TABLE_SCHEMA=DATABASE() "
      "AND TABLE_NAME='supla_suplan_grant' AND "
      "INDEX_NAME='suplan_grant_origin'",
      &out);
  EXPECT_EQ("n\n3\n", out);
}
// Exercise production IPC dispatch, including the static user lookup.
class SupLanOfflineDeleteIntegrationTest : public SupLanPeerDaoIntegrationTest {
 protected:
  void SetUp() override {
    SupLanPeerDaoIntegrationTest::SetUp();
    // Other Server tests intentionally retain runtime user 2. Use a separate
    // real DB account so randomized suite order cannot change the offline case.
    std::string out;
    sqlQuery(
        "INSERT INTO supla_user (id,short_unique_id,long_unique_id,salt,"
        "email,enabled,reg_date,timezone,home_latitude,home_longitude) "
        "SELECT 99123,REPEAT('a',32),'suplan-offline-delete-test',salt,"
        "'suplan-offline-delete@test.invalid',enabled,reg_date,timezone,"
        "home_latitude,home_longitude FROM supla_user WHERE id=2",
        &out);
    sqlQuery("UPDATE supla_iodevice SET user_id=99123 WHERE user_id=2", &out);
    sqlQuery("UPDATE supla_dev_channel SET user_id=99123 WHERE user_id=2",
             &out);
    factory = [] { return std::make_unique<PeerDao>(99123); };
    service = PeerService(factory, nullptr);
    ASSERT_TRUE(factory()->accept_identity(source, {123, {resource}}));
    ASSERT_EQ(nullptr, supla_user::find(99123, false));
    ASSERT_TRUE(service.upsert(source, destination, {1, resource, 1, 100, 1}));
    ASSERT_TRUE(service.provisioned(state(), 123));
  }
  void command(bool channel, int endpoint, int resource_id = 0) {
    testing::IpcSocketAdapterMock socket(-1);
    EXPECT_CALL(socket, send_data(std::string("OK:99123\n"))).Times(1);
    std::string input = channel ? "USER-ON-CHANNEL-DELETED:99123,"
                                : "USER-ON-DEVICE-DELETED:99123,";
    input += std::to_string(endpoint);
    if (channel) input += "," + std::to_string(resource_id);
    input += "\n";
    std::vector<char> buffer(input.begin(), input.end());
    buffer.push_back(0);
    if (channel) {
      supla_on_channel_deleted_command cmd(&socket);
      ASSERT_TRUE(
          cmd.process_command(buffer.data(), buffer.size(), input.size()));
    } else {
      supla_on_device_deleted_command cmd(&socket);
      ASSERT_TRUE(
          cmd.process_command(buffer.data(), buffer.size(), input.size()));
    }
  }
  void erase_device(int id) {
    std::string out;
    sqlQuery(
        ("DELETE FROM supla_iodevice WHERE id=" + std::to_string(id)).c_str(),
        &out);
    EXPECT_FALSE(factory()->device_exists(id));
  }
  void expect_grants(size_t count) {
    std::vector<Grant> grants;
    ASSERT_TRUE(factory()->grants(state().id, &grants));
    EXPECT_EQ(count, grants.size());
  }
};
TEST_F(SupLanOfflineDeleteIntegrationTest, ChannelDeleteFinalGrantWithoutUser) {
  ASSERT_TRUE(service.upsert(source, destination, {1, resource, 1, 200, 1}));
  std::string out;
  sqlQuery(
      ("DELETE FROM supla_dev_channel WHERE id=" + std::to_string(resource))
          .c_str(),
      &out);
  command(true, source, resource);
  EXPECT_EQ(nullptr, supla_user::find(99123, false));
  auto a = state();
  EXPECT_EQ(Lifecycle::Draining, a.lifecycle);
  EXPECT_EQ(2u, a.revision);
  EXPECT_TRUE(a.acl.empty());
  EXPECT_FALSE(a.source_acked);
  EXPECT_FALSE(a.destination_acked);
  expect_grants(0);
}
TEST_F(SupLanOfflineDeleteIntegrationTest,
       ChannelDeletePreservesUnrelatedGrant) {
  ASSERT_TRUE(service.upsert(
      source, destination,
      {SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, static_cast<uint32_t>(source), 1, 200,
       SUPLA_SUPLAN_PERMISSION_READ}));
  std::string out;
  sqlQuery(
      ("DELETE FROM supla_dev_channel WHERE id=" + std::to_string(resource))
          .c_str(),
      &out);
  command(true, source, resource);
  auto a = state();
  EXPECT_EQ(Lifecycle::Active, a.lifecycle);
  EXPECT_EQ(3u, a.revision);
  ASSERT_EQ(1u, a.acl.size());
  EXPECT_EQ(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, a.acl[0].ResourceType);
  EXPECT_EQ(static_cast<uint32_t>(source), a.acl[0].ResourceId);
  expect_grants(1);
  EXPECT_EQ(nullptr, supla_user::find(99123, false));
}
TEST_F(SupLanOfflineDeleteIntegrationTest, SourceDeletedAfterIdentityCascade) {
  erase_device(source);
  Identity identity;
  ASSERT_TRUE(factory()->identity(source, &identity));
  EXPECT_EQ(0u, identity.root);
  command(false, source);
  auto a = state();
  EXPECT_EQ(Lifecycle::Draining, a.lifecycle);
  EXPECT_TRUE(a.source_removed);
  EXPECT_TRUE(a.source_acked);
  EXPECT_FALSE(a.destination_removed);
  EXPECT_FALSE(a.destination_acked);
  EXPECT_EQ(123u, a.root);
  expect_grants(0);
  EXPECT_EQ(nullptr, supla_user::find(99123, false));
}
TEST_F(SupLanOfflineDeleteIntegrationTest, DestinationDeletedWithoutUser) {
  erase_device(destination);
  command(false, destination);
  auto a = state();
  EXPECT_EQ(Lifecycle::Draining, a.lifecycle);
  EXPECT_TRUE(a.destination_removed);
  EXPECT_TRUE(a.destination_acked);
  EXPECT_FALSE(a.source_removed);
  EXPECT_FALSE(a.source_acked);
  expect_grants(0);
  EXPECT_EQ(nullptr, supla_user::find(99123, false));
}
TEST_F(SupLanOfflineDeleteIntegrationTest,
       BothEndpointsDeletedWithoutReconnect) {
  erase_device(source);
  command(false, source);
  erase_device(destination);
  command(false, destination);
  EXPECT_EQ(0u, state().id);
  EXPECT_EQ(nullptr, supla_user::find(99123, false));
}
class DeleteTransport : public PeerTransport {
 public:
  std::vector<TSDS_SuplaSetSuplanSourceAssociation> sources;
  std::vector<TSDS_SuplaSetSuplanDestinationAssociation> destinations;
  bool ready(int) override { return true; }
  bool owner_ready(int, uint32_t) override { return true; }
  bool send(int, TSDS_SuplaSetSuplanSourceAssociation *q) override {
    sources.push_back(*q);
    return true;
  }
  bool send(int, TSDS_SuplaSetSuplanDestinationAssociation *q) override {
    destinations.push_back(*q);
    return true;
  }
  void rebootstrap(int) override { ADD_FAILURE(); }
  void diagnostic(uint64_t, int, uint8_t) override { ADD_FAILURE(); }
};
TEST_F(SupLanOfflineDeleteIntegrationTest,
       SurvivorReconnectReadsOnlyPendingCleanup) {
  erase_device(source);
  command(false, source);
  DeleteTransport transport;
  PeerProvisioner peers(factory, &transport);
  peers.reconnect(destination);
  EXPECT_TRUE(transport.sources.empty());
  ASSERT_EQ(1u, transport.destinations.size());
  const auto &q = transport.destinations.back();
  EXPECT_EQ(0, q.ResourceCount);
  EXPECT_EQ(0, q.PeerKeySize);
  TDS_SuplaSetSuplanDestinationAssociationResult reply = {};
  reply.PeerContext = q.PeerContext;
  reply.AclRevision = q.AclRevision;
  reply.Result = SUPLA_SUPLAN_RESULT_OK;
  peers.on_destination(destination, &reply);
  EXPECT_EQ(0u, state().id);
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanOfflineDeleteIntegrationTest,
       LiveUserMutatesOnceAndProvisionsImmediately) {
  DeleteTransport transport;
  // The production entry point must reuse this user's existing provisioner.
  std::unique_ptr<supla_user> user(supla_user::find(99123, true));
  ASSERT_NE(nullptr, user);
  auto *peers = user->get_suplan_peers()->peers();
  auto *previous = PeerProvisionerTestAccess::transport(*peers, &transport);
  std::string out;
  sqlQuery("CREATE TABLE suplan_test_writes (n INT NOT NULL)", &out);
  sqlQuery("INSERT INTO suplan_test_writes VALUES (0)", &out);
  sqlQuery(
      "CREATE TRIGGER suplan_test_write_counter AFTER UPDATE ON "
      "supla_suplan_peer_association FOR EACH ROW "
      "UPDATE suplan_test_writes SET n=n+1",
      &out);
  command(false, destination);
  EXPECT_EQ(1u, transport.sources.size());
  EXPECT_TRUE(transport.destinations.empty());
  sqlQuery("SELECT n FROM suplan_test_writes", &out);
  EXPECT_EQ("n\n2\n", out);  // Empty projection + removed-side flags, once.
  EXPECT_EQ(2u, state().revision);
  PeerProvisionerTestAccess::transport(*peers, previous);
}
TEST_F(SupLanOfflineDeleteIntegrationTest,
       LiveChannelDeleteUsesExistingProvisioner) {
  DeleteTransport transport;
  std::unique_ptr<supla_user> user(supla_user::find(99123, true));
  ASSERT_NE(nullptr, user);
  auto *peers = user->get_suplan_peers()->peers();
  auto *previous = PeerProvisionerTestAccess::transport(*peers, &transport);
  command(true, source, resource);
  EXPECT_EQ(1u, transport.sources.size());
  EXPECT_EQ(1u, transport.destinations.size());
  EXPECT_EQ(2u, state().revision);
  EXPECT_EQ(Lifecycle::Draining, state().lifecycle);
  PeerProvisionerTestAccess::transport(*peers, previous);
}

}  // namespace

namespace {
class SupLanHvacIntegrationTest : public SupLanPeerDaoIntegrationTest {
 protected:
  uint32_t hvac = 0, local = 0, other_resource = 0;
  int other_source = 0;
  void query(const std::string &sql) {
    supla_mariadb_access_provider db;
    ASSERT_TRUE(db.connect());
    ASSERT_EQ(0, db.query(sql.c_str(), true)) << sql;
  }
  int scalar(const std::string &sql) {
    supla_mariadb_access_provider db;
    EXPECT_TRUE(db.connect());
    void *stmt = nullptr;
    int result = 0;
    EXPECT_TRUE(db.stmt_get_int(&stmt, &result, nullptr, nullptr, nullptr,
                                sql.c_str(), nullptr, 0, true));
    return result;
  }
  uint32_t add_channel(int device, int number, int type, int function) {
    query(
        "INSERT INTO supla_dev_channel "
        "(iodevice_id,user_id,channel_number,type,func,param1,param2,param3) "
        "VALUES(" +
        std::to_string(device) + ",99124," + std::to_string(number) + "," +
        std::to_string(type) + "," + std::to_string(function) + ",0,0,0)");
    return scalar("SELECT id FROM supla_dev_channel WHERE iodevice_id=" +
                  std::to_string(device) +
                  " AND channel_number=" + std::to_string(number));
  }
  void SetUp() override {
    SupLanPeerDaoIntegrationTest::SetUp();
    query(
        "INSERT INTO supla_user (id,short_unique_id,long_unique_id,salt,"
        "email,enabled,reg_date,timezone,home_latitude,home_longitude) "
        "SELECT 99124,REPEAT('b',32),'suplan-hvac-test',salt,"
        "'suplan-hvac@test.invalid',enabled,reg_date,timezone,"
        "home_latitude,home_longitude FROM supla_user WHERE id=2");
    query("UPDATE supla_iodevice SET user_id=99124 WHERE user_id=2");
    query("UPDATE supla_dev_channel SET user_id=99124 WHERE user_id=2");
    factory = [] { return std::make_unique<PeerDao>(99124); };
    service = PeerService(factory, nullptr);
    ASSERT_TRUE(factory()->accept_identity(source, {123, {resource}}));
    query("UPDATE supla_iodevice SET flags=flags|" +
          std::to_string(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) +
          " WHERE user_id=99124");
    query("UPDATE supla_dev_channel SET type=" +
          std::to_string(SUPLA_CHANNELTYPE_THERMOMETER) +
          ",func=" + std::to_string(SUPLA_CHANNELFNC_THERMOMETER) +
          " WHERE id=" + std::to_string(resource));
    hvac = add_channel(destination, 240, SUPLA_CHANNELTYPE_HVAC,
                       SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
    local = add_channel(destination, 241, SUPLA_CHANNELTYPE_THERMOMETER,
                        SUPLA_CHANNELFNC_THERMOMETER);
    other_source = scalar(
        "SELECT MIN(id) FROM supla_iodevice WHERE user_id=99124 "
        "AND id<>" +
        std::to_string(source) + " AND id<>" + std::to_string(destination));
    ASSERT_GT(other_source, 0);
    other_resource =
        add_channel(other_source, 240, SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR,
                    SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE);
    ASSERT_TRUE(
        factory()->accept_identity(other_source, {456, {other_resource}}));
  }
  void config(uint32_t selected) {
    query(
        "UPDATE supla_dev_channel SET user_config='{"
        "\"mainThermometerChannelId\":" +
        std::to_string(selected) + "}' WHERE id=" + std::to_string(hvac));
  }
  bool reconcile() {
    return supla_suplan_server_peers::reconcile_hvac(99124, hvac);
  }
  void with_destination(
      const std::function<void(supla_device_channel *)> &visit,
      int flags = SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) {
    testing::DeviceStub device(nullptr);
    device.set_id(destination);
    device.set_flags(flags);
    device.set_user(supla_user::find(99124, true));
    supla_mariadb_access_provider db;
    ASSERT_TRUE(db.connect());
    supla_device_dao dao(&db);
    device.set_channels(
        new supla_device_channels(&dao, &device, nullptr, nullptr, 0));
    device.get_channels()->access_channel(hvac, visit);
  }
  std::vector<Grant> origin_grants() {
    auto repo = factory();
    std::vector<uint64_t> ids;
    EXPECT_TRUE(repo->for_origin(CHANNEL_CONFIG_ORIGIN,
                                 main_thermometer_origin(hvac), &ids));
    std::vector<Grant> result;
    for (auto id : ids) {
      std::vector<Grant> grants;
      EXPECT_TRUE(repo->grants(id, &grants));
      for (const auto &g : grants)
        if (g.origin_type == CHANNEL_CONFIG_ORIGIN &&
            g.origin_id == main_thermometer_origin(hvac))
          result.push_back(g);
    }
    return result;
  }
};

TEST_F(SupLanHvacIntegrationTest,
       ExactReadIdempotencyReplacementAndIndependentOrigin) {
  config(local);
  ASSERT_TRUE(reconcile());
  EXPECT_TRUE(origin_grants().empty());
  config(resource);
  ASSERT_TRUE(reconcile());
  auto grants = origin_grants();
  ASSERT_EQ(1u, grants.size());
  EXPECT_EQ(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, grants[0].resource_type);
  EXPECT_EQ(resource, grants[0].resource_id);
  EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_READ, grants[0].permissions);
  auto initial = state();
  ASSERT_TRUE(reconcile());
  EXPECT_TRUE(same_state(initial, state()));
  uint32_t same_source = add_channel(source, 240, SUPLA_CHANNELTYPE_THERMOMETER,
                                     SUPLA_CHANNELFNC_THERMOMETER);
  ASSERT_TRUE(
      factory()->accept_identity(source, {123, {resource, same_source}}));
  config(same_source);
  ASSERT_TRUE(reconcile());
  ASSERT_EQ(1u, origin_grants().size());
  EXPECT_EQ(same_source, origin_grants()[0].resource_id);
  EXPECT_EQ(initial.id, state().id);
  config(resource);
  ASSERT_TRUE(reconcile());
  Grant manual{1, resource, 99, 77, SUPLA_SUPLAN_PERMISSION_CONTROL};
  ASSERT_TRUE(service.upsert(source, destination, manual));
  config(other_resource);
  ASSERT_TRUE(reconcile());
  grants = origin_grants();
  ASSERT_EQ(1u, grants.size());
  EXPECT_EQ(other_resource, grants[0].resource_id);
  EXPECT_EQ(Lifecycle::Active, state().lifecycle);
  EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_CONTROL, state().acl[0].Permissions);
  for (auto selected : {local, 0u}) {
    config(other_resource);
    ASSERT_TRUE(reconcile());
    config(selected);
    ASSERT_TRUE(reconcile());
    EXPECT_TRUE(origin_grants().empty());
    EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_CONTROL, state().acl[0].Permissions);
  }
}

TEST_F(SupLanHvacIntegrationTest, InvalidRelationAlwaysRevokes) {
  for (int failure = 0; failure < 7; ++failure) {
    query("UPDATE supla_iodevice SET flags=flags|" +
          std::to_string(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) +
          " WHERE user_id=99124");
    query("UPDATE supla_dev_channel SET func=" +
          std::to_string(SUPLA_CHANNELFNC_THERMOMETER) +
          " WHERE id=" + std::to_string(resource));
    query("UPDATE supla_dev_channel SET type=" +
          std::to_string(SUPLA_CHANNELTYPE_THERMOMETER) +
          " WHERE id=" + std::to_string(resource));
    query("UPDATE supla_dev_channel SET func=" +
          std::to_string(SUPLA_CHANNELFNC_HVAC_THERMOSTAT) +
          " WHERE id=" + std::to_string(hvac));
    config(resource);
    ASSERT_TRUE(reconcile());
    ASSERT_EQ(1u, origin_grants().size());
    if (failure < 2) {
      query("UPDATE supla_iodevice SET flags=flags & ~" +
            std::to_string(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) +
            " WHERE id=" + std::to_string(failure ? destination : source));
    } else if (failure == 2) {
      query("UPDATE supla_dev_channel SET func=0 WHERE id=" +
            std::to_string(resource));
    } else if (failure == 3) {
      config(4000000000u);
    } else if (failure == 4) {
      query(
          "INSERT INTO supla_iodevice "
          "(location_id,user_id,guid,enabled,reg_date,protocol_version,flags) "
          "SELECT location_id,2,guid,enabled,reg_date,protocol_version,flags "
          "FROM supla_iodevice WHERE id=" +
          std::to_string(source));
      int foreign_device =
          scalar("SELECT MAX(id) FROM supla_iodevice WHERE user_id=2");
      query(
          "INSERT INTO supla_dev_channel "
          "(iodevice_id,user_id,channel_number,type,func,param1,param2,param3) "
          "VALUES(" +
          std::to_string(foreign_device) + ",2,240," +
          std::to_string(SUPLA_CHANNELTYPE_THERMOMETER) + "," +
          std::to_string(SUPLA_CHANNELFNC_THERMOMETER) + ",0,0,0)");
      int foreign =
          scalar("SELECT id FROM supla_dev_channel WHERE iodevice_id=" +
                 std::to_string(foreign_device));
      ASSERT_GT(foreign, 0);
      config(foreign);
    } else if (failure == 5) {
      query("UPDATE supla_dev_channel SET type=" +
            std::to_string(SUPLA_CHANNELTYPE_RELAY) +
            " WHERE id=" + std::to_string(resource));
    } else {
      query("UPDATE supla_dev_channel SET func=0 WHERE id=" +
            std::to_string(hvac));
    }
    ASSERT_TRUE(reconcile());
    EXPECT_TRUE(origin_grants().empty());
  }
}

TEST_F(SupLanHvacIntegrationTest, DeleteSourceAndDestinationCleanup) {
  config(resource);
  ASSERT_TRUE(reconcile());
  ASSERT_TRUE(supla_suplan_server_peers::channel_deleted(99124, resource));
  EXPECT_TRUE(origin_grants().empty());
  EXPECT_EQ(0, scalar("SELECT IFNULL(JSON_VALUE(user_config,"
                      "'$.mainThermometerChannelId'),0) FROM supla_dev_channel "
                      "WHERE id=" +
                      std::to_string(hvac)));
  config(other_resource);
  ASSERT_TRUE(reconcile());
  ASSERT_TRUE(supla_suplan_server_peers::channel_deleted(99124, hvac));
  EXPECT_TRUE(origin_grants().empty());
}

TEST_F(SupLanHvacIntegrationTest,
       ColdIpcReconcilesCommittedConfigWithOfflineSource) {
  query(
      "INSERT INTO supla_user (id,short_unique_id,long_unique_id,salt,"
      "email,enabled,reg_date,timezone,home_latitude,home_longitude) "
      "SELECT 99125,REPEAT('c',32),'suplan-hvac-cold-ipc',salt,"
      "'suplan-hvac-cold@test.invalid',enabled,reg_date,timezone,"
      "home_latitude,home_longitude FROM supla_user WHERE id=99124");
  query("UPDATE supla_iodevice SET user_id=99125 WHERE user_id=99124");
  query("UPDATE supla_dev_channel SET user_id=99125 WHERE user_id=99124");
  factory = [] { return std::make_unique<PeerDao>(99125); };
  ASSERT_TRUE(factory()->accept_identity(source, {123, {resource}}));
  EXPECT_EQ(nullptr, supla_user::find(99125, false));
  config(resource);
  testing::IpcSocketAdapterMock socket(-1);
  supla_on_channel_config_changed_command command(&socket);
  std::string notification =
      "USER-ON-CHANNEL-CONFIG-CHANGED:99125," + std::to_string(destination) +
      "," + std::to_string(hvac) + "," +
      std::to_string(SUPLA_CHANNELTYPE_HVAC) + "," +
      std::to_string(SUPLA_CHANNELFNC_HVAC_THERMOSTAT) + "," +
      std::to_string(CONFIG_CHANGE_SCOPE_JSON_DEFAULT) + "\n";
  char buffer[512] = {};
  snprintf(buffer, sizeof(buffer), "%s", notification.c_str());
  EXPECT_TRUE(
      command.process_command(buffer, sizeof(buffer), notification.size()));
  ASSERT_EQ(1u, origin_grants().size());
  EXPECT_EQ(resource, origin_grants()[0].resource_id);
}

TEST_F(SupLanHvacIntegrationTest, DeviceFlagSelectsAllSixIdsOrNumbers) {
  uint32_t binary =
      add_channel(destination, 242, SUPLA_CHANNELTYPE_BINARYSENSOR,
                  SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW);
  uint32_t master = add_channel(destination, 243, SUPLA_CHANNELTYPE_HVAC,
                                SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  uint32_t pump = add_channel(destination, 244, SUPLA_CHANNELTYPE_RELAY,
                              SUPLA_CHANNELFNC_PUMPSWITCH);
  uint32_t heat = add_channel(destination, 245, SUPLA_CHANNELTYPE_RELAY,
                              SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH);
  query(
      "UPDATE supla_dev_channel SET "
      "user_config='{\"mainThermometerChannelId\":" +
      std::to_string(local) +
      ",\"auxThermometerChannelId\":" + std::to_string(local) +
      ",\"binarySensorChannelId\":" + std::to_string(binary) +
      ",\"masterThermostatChannelId\":" + std::to_string(master) +
      ",\"pumpSwitchChannelId\":" + std::to_string(pump) +
      ",\"heatOrColdSourceSwitchChannelId\":" + std::to_string(heat) +
      "}' WHERE id=" + std::to_string(hvac));
  for (int flags : {0, SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED}) {
    with_destination(
        [&](supla_device_channel *channel) {
          TSD_ChannelConfig cfg = {};
          channel->get_config(&cfg, SUPLA_CONFIG_TYPE_DEFAULT, 0);
          ASSERT_EQ(sizeof(TChannelConfig_HVAC), cfg.ConfigSize);
          auto wire = reinterpret_cast<TChannelConfig_HVAC *>(cfg.Config);
          EXPECT_EQ(flags ? local : 241u, wire->MainThermometerChannelId);
          EXPECT_EQ(flags ? local : 241u, wire->AuxThermometerChannelId);
          EXPECT_EQ(flags ? binary : 242u, wire->BinarySensorChannelId);
          EXPECT_EQ(flags ? master : (243u << 8) | 1,
                    wire->MasterThermostatChannelId);
          EXPECT_EQ(flags ? pump : (244u << 8) | 1, wire->PumpSwitchChannelId);
          EXPECT_EQ(flags ? heat : (245u << 8) | 1,
                    wire->HeatOrColdSourceSwitchChannelId);
        },
        flags);
  }
  EXPECT_TRUE(origin_grants().empty());
}

TEST_F(SupLanHvacIntegrationTest,
       DeviceUploadProtectedMergeAndSpoofCannotAuthorize) {
  for (auto current : {0u, local, resource}) {
    config(current);
    ASSERT_TRUE(reconcile());
    for (auto incoming : {0u, local, resource, other_resource}) {
      with_destination([&](supla_device_channel *channel) {
        TChannelConfig_HVAC wire = {};
        wire.MainThermometerChannelId = incoming;
        wire.MinOnTimeS = 19;
        ASSERT_EQ(
            SUPLA_CONFIG_RESULT_TRUE,
            channel->set_user_config(SUPLA_CONFIG_TYPE_DEFAULT, sizeof(wire),
                                     reinterpret_cast<char *>(&wire)));
        TSD_ChannelConfig cfg = {};
        channel->get_config(&cfg, SUPLA_CONFIG_TYPE_DEFAULT, 0);
        ASSERT_EQ(sizeof(TChannelConfig_HVAC), cfg.ConfigSize);
        auto merged = reinterpret_cast<TChannelConfig_HVAC *>(cfg.Config);
        unsigned int expected = current == resource || incoming == resource ||
                                        incoming == other_resource
                                    ? current
                                    : incoming;
        EXPECT_EQ(expected, merged->MainThermometerChannelId);
        EXPECT_EQ(19, merged->MinOnTimeS);
      });
      auto grants = origin_grants();
      if (current == resource) {
        ASSERT_EQ(1u, grants.size());
        EXPECT_EQ(resource, grants[0].resource_id);
      } else {
        EXPECT_TRUE(grants.empty());
      }
      // Restore the trusted business state for each independent matrix cell.
      config(current);
      ASSERT_TRUE(reconcile());
    }
  }
}

TEST_F(SupLanHvacIntegrationTest,
       ConfigDeliveryWaitsForDesiredCommitAndNotOfflineSource) {
  config(resource);
  ASSERT_TRUE(reconcile());
  auto old = state();
  query(
      "UPDATE supla_suplan_peer_association SET acl_revision=4294967295,"
      "peer_generation=4294967295 WHERE id=" +
      std::to_string(old.id));
  config(other_resource);
  with_destination([&](supla_device_channel *channel) {
    TSD_ChannelConfig cfg = {};
    channel->get_config(&cfg, SUPLA_CONFIG_TYPE_DEFAULT, 0);
    EXPECT_EQ(0, cfg.ConfigSize);
  });
  // Shared origin transaction must retain the old Grant on failure.
  ASSERT_EQ(1u, origin_grants().size());
  EXPECT_EQ(resource, origin_grants()[0].resource_id);
  query(
      "UPDATE supla_suplan_peer_association SET acl_revision=1,"
      "peer_generation=1 WHERE id=" +
      std::to_string(old.id));
  with_destination([&](supla_device_channel *channel) {
    TSD_ChannelConfig cfg = {};
    channel->get_config(&cfg, SUPLA_CONFIG_TYPE_DEFAULT, 0);
    ASSERT_EQ(sizeof(TChannelConfig_HVAC), cfg.ConfigSize);
    EXPECT_EQ(other_resource,
              reinterpret_cast<TChannelConfig_HVAC *>(cfg.Config)
                  ->MainThermometerChannelId);
    ASSERT_EQ(1u, origin_grants().size());
    EXPECT_EQ(other_resource, origin_grants()[0].resource_id);
  });
}

TEST_F(SupLanHvacIntegrationTest,
       NotificationsObserveOnlyCommittedOriginReplacement) {
  int notifications = 0;
  uint32_t expected = resource;
  PeerService notifying(factory, [&](uint64_t) {
    ++notifications;
    auto grants = origin_grants();
    ASSERT_EQ(1u, grants.size());
    EXPECT_EQ(expected, grants[0].resource_id);
  });
  ASSERT_TRUE(notifying.reconcile_main_thermometer(hvac, resource));
  EXPECT_EQ(1, notifications);
  ASSERT_TRUE(notifying.reconcile_main_thermometer(hvac, resource));
  EXPECT_EQ(1, notifications);
  expected = other_resource;
  ASSERT_TRUE(notifying.reconcile_main_thermometer(hvac, other_resource));
  EXPECT_EQ(3, notifications);
}

TEST_F(SupLanHvacIntegrationTest, LegacyUploadTranslatesAllSixToOwnedIds) {
  uint32_t binary =
      add_channel(destination, 242, SUPLA_CHANNELTYPE_BINARYSENSOR,
                  SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW);
  uint32_t master = add_channel(destination, 243, SUPLA_CHANNELTYPE_HVAC,
                                SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  uint32_t pump = add_channel(destination, 244, SUPLA_CHANNELTYPE_RELAY,
                              SUPLA_CHANNELFNC_PUMPSWITCH);
  uint32_t heat = add_channel(destination, 245, SUPLA_CHANNELTYPE_RELAY,
                              SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH);
  config(0);
  with_destination(
      [&](supla_device_channel *channel) {
        TChannelConfig_HVAC incoming = {};
        incoming.MainThermometerChannelNo = 241;
        incoming.AuxThermometerChannelNo = 241;
        incoming.BinarySensorChannelNo = 242;
        incoming.MasterThermostatChannelNo = 243;
        incoming.MasterThermostatIsSet = 1;
        incoming.PumpSwitchChannelNo = 244;
        incoming.PumpSwitchIsSet = 1;
        incoming.HeatOrColdSourceSwitchChannelNo = 245;
        incoming.HeatOrColdSourceSwitchIsSet = 1;
        ASSERT_EQ(SUPLA_CONFIG_RESULT_TRUE,
                  channel->set_user_config(
                      SUPLA_CONFIG_TYPE_DEFAULT, sizeof(incoming),
                      reinterpret_cast<char *>(&incoming)));
        std::unique_ptr<supla_json_config> current(channel->get_json_config());
        hvac_config cfg(current.get());
        EXPECT_EQ(local, cfg.reference(0));
        EXPECT_EQ(local, cfg.reference(1));
        EXPECT_EQ(binary, cfg.reference(2));
        EXPECT_EQ(master, cfg.reference(3));
        EXPECT_EQ(pump, cfg.reference(4));
        EXPECT_EQ(heat, cfg.reference(5));
      },
      0);
  EXPECT_TRUE(origin_grants().empty());
  EXPECT_EQ(0, scalar("SELECT LOCATE('ChannelNo',user_config) FROM "
                      "supla_dev_channel WHERE id=" + std::to_string(hvac)));
  for (const char *key : hvac_config::reference_keys) {
    EXPECT_GT(scalar("SELECT IFNULL(JSON_VALUE(user_config,'$." +
                     std::string(key) + "'),0) FROM supla_dev_channel "
                     "WHERE id=" +
                     std::to_string(hvac)), 0);
  }
}

TEST_F(SupLanHvacIntegrationTest,
       HistoricalDbReadDoesNotWriteOrResolveLegacyKeys) {
  query(
      "UPDATE supla_dev_channel SET user_config='{"
      "\"mainThermometerChannelNo\":241,\"mainThermometerChannelId\":" +
      std::to_string(resource) +
      ",\"auxThermometerChannelNo\":241,"
      "\"binarySensorChannelNo\":241,\"masterThermostatChannelNo\":241,"
      "\"pumpSwitchChannelNo\":241,\"heatOrColdSourceSwitchChannelNo\":241}',"
      "properties='{\"readOnlyConfigFields\":[\"mainThermometerChannelNo\"]}' "
      "WHERE id=" +
      std::to_string(hvac));
  supla_mariadb_access_provider db;
  supla_device_dao dao(&db);
  std::unique_ptr<supla_json_config> current(
      dao.get_channel_config(hvac, nullptr, nullptr));
  ASSERT_NE(nullptr, current);
  hvac_config cfg(current.get());
  EXPECT_EQ(resource, cfg.reference(0));
  for (size_t i = 1; i < 6; ++i) EXPECT_EQ(0u, cfg.reference(i));
  EXPECT_GT(scalar("SELECT LOCATE('ChannelNo',user_config) FROM "
                   "supla_dev_channel WHERE id=" + std::to_string(hvac)),
            0);
  EXPECT_GT(scalar("SELECT LOCATE('ChannelNo',properties) FROM "
                   "supla_dev_channel WHERE id=" + std::to_string(hvac)),
            0);
  std::string user_md5, properties_md5;
  std::unique_ptr<supla_json_config> again(
      dao.get_channel_config(hvac, &user_md5, &properties_md5));
  ASSERT_NE(nullptr, again);
  for (int read = 0; read < 3; ++read) {
    std::string next_user_md5, next_properties_md5;
    std::unique_ptr<supla_json_config> next(
        dao.get_channel_config(hvac, &next_user_md5, &next_properties_md5));
    ASSERT_NE(nullptr, next);
    EXPECT_EQ(user_md5, next_user_md5);
    EXPECT_EQ(properties_md5, next_properties_md5);
  }
}

TEST_F(SupLanHvacIntegrationTest, DeleteDeviceClearsOrphansAfterCloudDeletion) {
  config(resource);
  ASSERT_TRUE(reconcile());
  query("DELETE FROM supla_dev_channel WHERE iodevice_id=" +
        std::to_string(source));
  query("DELETE FROM supla_iodevice WHERE id=" + std::to_string(source));
  ASSERT_TRUE(supla_suplan_server_peers::device_deleted(99124, source));
  EXPECT_TRUE(origin_grants().empty());
  EXPECT_EQ(0, scalar("SELECT IFNULL(JSON_VALUE(user_config,"
                      "'$.mainThermometerChannelId'),0) FROM supla_dev_channel "
                      "WHERE id=" +
                      std::to_string(hvac)));
}

TEST_F(SupLanHvacIntegrationTest, SourceFunctionChangeReconcilesDependents) {
  config(resource);
  ASSERT_TRUE(reconcile());
  query("UPDATE supla_dev_channel SET func=0 WHERE id=" +
        std::to_string(resource));
  ASSERT_TRUE(
      supla_suplan_server_peers::reconcile_dependencies(99124, resource));
  EXPECT_TRUE(origin_grants().empty());
  query("UPDATE supla_dev_channel SET func=" +
        std::to_string(SUPLA_CHANNELFNC_THERMOMETER) + " WHERE id=" +
        std::to_string(resource));
  ASSERT_TRUE(
      supla_suplan_server_peers::reconcile_dependencies(99124, resource));
  ASSERT_EQ(1u, origin_grants().size());
  EXPECT_EQ(resource, origin_grants()[0].resource_id);
  EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_READ, origin_grants()[0].permissions);
}

TEST_F(SupLanHvacIntegrationTest,
       MissingDestinationReconciliationRemovesOrigin) {
  config(resource);
  ASSERT_TRUE(reconcile());
  with_destination([&](supla_device_channel *channel) {
    query("DELETE FROM supla_dev_channel WHERE id=" + std::to_string(hvac));
    TSD_ChannelConfig config = {};
    EXPECT_FALSE(channel->get_config(&config, SUPLA_CONFIG_TYPE_DEFAULT, 0));
    EXPECT_EQ(0, config.ConfigSize);
  });
  ASSERT_TRUE(supla_suplan_server_peers::reconcile_hvac(99124, hvac, nullptr));
  EXPECT_TRUE(origin_grants().empty());
}

TEST_F(SupLanHvacIntegrationTest,
       ReconcileReloadsInsteadOfTrustingOldSnapshot) {
  config(resource);
  supla_mariadb_access_provider db;
  supla_device_dao dao(&db);
  std::unique_ptr<supla_json_config> current(
      dao.get_channel_config(hvac, nullptr, nullptr));
  config(other_resource);
  ASSERT_TRUE(reconcile());
  ASSERT_TRUE(
      supla_suplan_server_peers::reconcile_hvac(99124, hvac, &current));
  ASSERT_NE(nullptr, current);
  EXPECT_EQ(other_resource, hvac_config(current.get()).reference(0));
  ASSERT_EQ(1u, origin_grants().size());
  EXPECT_EQ(other_resource, origin_grants()[0].resource_id);
}

class ReplayProtocolChannel : public supla_device_channel {
 public:
  using supla_device_channel::supla_device_channel;
  unsigned char get_protocol_version() override { return 29; }
};
TEST_F(SupLanHvacIntegrationTest, DesiredCommitFailurePropagatesToReplay) {
  config(resource);
  ASSERT_TRUE(reconcile());
  auto old = state();
  query("UPDATE supla_suplan_peer_association SET acl_revision=4294967295,"
        "peer_generation=4294967295 WHERE id=" + std::to_string(old.id));
  config(other_resource);
  with_destination([&](supla_device_channel *channel) {
    char value[SUPLA_CHANNELVALUE_SIZE] = {};
    ReplayProtocolChannel replay(channel->get_device(), hvac, 240,
        SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
        0, 0, 0, 0, nullptr, nullptr, nullptr, false,
        SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE,
        value, 0, nullptr, nullptr, nullptr, nullptr);
    std::vector<TSDS_SetChannelConfig> configs;
    bool failed = false;
    EXPECT_FALSE(replay.prepare_config_for_device(&configs, &failed));
    EXPECT_TRUE(failed);
    EXPECT_TRUE(configs.empty());
  });
}

TEST_F(SupLanHvacIntegrationTest, OriginGateCoversAuthoritativeRead) {
  config(resource);
  std::promise<void> first_read, release_first, second_started, second_read;
  auto release = release_first.get_future().share();
  auto second_entered = second_read.get_future();
  PeerService first(factory, nullptr), second(factory, nullptr);
  auto one = std::async(std::launch::async, [&] {
    return first.reconcile_main_thermometer(hvac, [&](uint32_t *source) {
      *source = resource;
      first_read.set_value();
      release.wait();
      return true;
    });
  });
  first_read.get_future().wait();
  config(other_resource);
  auto two = std::async(std::launch::async, [&] {
    second_started.set_value();
    return second.reconcile_main_thermometer(hvac, [&](uint32_t *source) {
      second_read.set_value();
      supla_mariadb_access_provider db;
      supla_device_dao dao(&db);
      std::unique_ptr<supla_json_config> current(
          dao.get_channel_config(hvac, nullptr, nullptr));
      if (!current) return false;
      *source = hvac_config(current.get()).reference(0);
      return true;
    });
  });
  second_started.get_future().wait();
  EXPECT_EQ(std::future_status::timeout,
            second_entered.wait_for(std::chrono::milliseconds(50)));
  release_first.set_value();
  EXPECT_TRUE(one.get());
  EXPECT_TRUE(two.get());
  ASSERT_EQ(1u, origin_grants().size());
  EXPECT_EQ(other_resource, origin_grants()[0].resource_id);
}

TEST_F(SupLanHvacIntegrationTest,
       LegacyWireCannotCreateOrEncodeRemoteRelation) {
  add_channel(source, 239, SUPLA_CHANNELTYPE_THERMOMETER,
              SUPLA_CHANNELFNC_THERMOMETER);
  config(0);
  with_destination([&](supla_device_channel *channel) {
    TChannelConfig_HVAC incoming = {};
    incoming.MainThermometerChannelNo = 239;
    ASSERT_EQ(SUPLA_CONFIG_RESULT_TRUE,
              channel->set_user_config(SUPLA_CONFIG_TYPE_DEFAULT,
                  sizeof(incoming), reinterpret_cast<char *>(&incoming)));
    std::unique_ptr<supla_json_config> current(channel->get_json_config());
    EXPECT_EQ(0u, hvac_config(current.get()).reference(0));
  }, 0);
  EXPECT_TRUE(origin_grants().empty());
  config(resource);
  ASSERT_TRUE(reconcile());
  with_destination([&](supla_device_channel *channel) {
    TSD_ChannelConfig wire = {};
    EXPECT_FALSE(channel->get_config(&wire, SUPLA_CONFIG_TYPE_DEFAULT, 0));
    EXPECT_EQ(0u, wire.ConfigSize);
  }, 0);
}
}  // namespace

namespace {
// Only the remote Device and IPC socket are test peers. Registration, DB reads,
// reconciliation, dispatch, publication and SRPC run in production code.
class SupLanHvacWireIntegrationTest : public SupLanHvacIntegrationTest {
 protected:
  void *listener = nullptr, *worker = nullptr, *wire = nullptr;
  int client = -1;
  supla_connection *connection = nullptr;

  void SetUp() override {
    SupLanHvacIntegrationTest::SetUp();
    ASSERT_FALSE(HasFatalFailure());
    // Isolate the live user from runtime users retained by other suites.
    query(
        "INSERT INTO supla_user (id,short_unique_id,long_unique_id,salt,"
        "email,enabled,reg_date,timezone,home_latitude,home_longitude) "
        "SELECT 99126,REPEAT('d',32),'suplan-hvac-wire-test',salt,"
        "'hvac-wire@test.invalid',enabled,reg_date,timezone,"
        "home_latitude,home_longitude FROM supla_user WHERE id=99124");
    query("UPDATE supla_iodevice SET user_id=99126 WHERE user_id=99124");
    query("UPDATE supla_dev_channel SET user_id=99126 WHERE user_id=99124");
    query("DELETE FROM supla_dev_channel WHERE iodevice_id=" +
          std::to_string(destination) + " AND id<>" + std::to_string(hvac));
    query("UPDATE supla_iodevice SET guid=UNHEX(REPEAT('ab',16)),"
          "auth_key=NULL,enabled=1 WHERE id=" + std::to_string(destination));
    query("UPDATE supla_dev_channel SET user_config='{\"minOnTimeS\":17}',"
          "flist=" + std::to_string(SUPLA_BIT_FUNC_HVAC_THERMOSTAT) +
          " WHERE id=" + std::to_string(hvac));
    supla_connection::init();
    listener = ssocket_server_init(nullptr, nullptr, 0, 0);
    ASSERT_NE(nullptr, listener);
    ASSERT_TRUE(ssocket_openlistener(listener));
    sockaddr_in address = {};
    socklen_t size = sizeof(address);
    ASSERT_EQ(0, getsockname(ssocket_get_fd(listener),
                            reinterpret_cast<sockaddr *>(&address), &size));
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    client = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(client, 0);
    ASSERT_EQ(0, connect(client, reinterpret_cast<sockaddr *>(&address), size));
    timeval timeout = {2, 0};
    ASSERT_EQ(0, setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                            sizeof(timeout)));
    void *accepted = nullptr;
    unsigned int ip = 0;
    ASSERT_TRUE(ssocket_accept(listener, &ip, &accepted));
    connection = new supla_connection(listener, accepted, ip);
    wire = sproto_init();
    ASSERT_NE(nullptr, wire);
    sproto_set_version(wire, 29);
    sthread_simple_run(
        [](void *conn, void *thread) {
          static_cast<supla_connection *>(conn)->execute(thread);
        }, connection, false, &worker);
    ASSERT_NE(nullptr, worker);
  }

  void TearDown() override {
    if (worker) {
      sthread_terminate(worker, false);
      connection->raise_event();
      sthread_wait(worker);
      sthread_free(worker);
    }
    delete connection;
    if (client >= 0) close(client);
    if (wire) sproto_free(wire);
    if (listener) ssocket_free(listener);
    supla_connection::cleanup();
  }

  void send_packet(unsigned int call, const void *data, size_t size) {
    TSuplaDataPacket packet = {};
    sproto_sdp_init(wire, &packet);
    auto bytes_in = const_cast<char *>(static_cast<const char *>(data));
    ASSERT_TRUE(sproto_set_data(&packet, bytes_in, size, call));
    ASSERT_TRUE(sproto_out_buffer_append(wire, &packet));
    char bytes[4096];
    while (auto count = sproto_pop_out_data(wire, bytes, sizeof(bytes))) {
      size_t sent = 0;
      while (sent < count) {
        int n = send(client, bytes + sent, count - sent, MSG_NOSIGNAL);
        ASSERT_GT(n, 0);
        sent += n;
      }
    }
  }

  bool receive_packet(TSuplaDataPacket *packet, int timeout_ms = 2000) {
    auto deadline = std::chrono::steady_clock::now() +
                    std::chrono::milliseconds(timeout_ms);
    while (sproto_pop_in_sdp(wire, packet) != SUPLA_RESULT_TRUE) {
      auto left = std::chrono::duration_cast<std::chrono::milliseconds>(
          deadline - std::chrono::steady_clock::now()).count();
      if (left <= 0) return false;
      pollfd fd = {client, POLLIN, 0};
      if (poll(&fd, 1, left) <= 0) return false;
      char bytes[4096];
      int n = recv(client, bytes, sizeof(bytes), MSG_DONTWAIT);
      if (n <= 0) return false;
      if (sproto_in_buffer_append(wire, bytes, n) != SUPLA_RESULT_TRUE)
        return false;
    }
    return true;
  }

  void acknowledge(const TSDS_SetChannelConfig &config) {
    TSDS_SetChannelConfigResult reply = {};
    reply.ChannelNumber = config.ChannelNumber;
    reply.ConfigType = config.ConfigType;
    reply.Result = SUPLA_CONFIG_RESULT_TRUE;
    send_packet(SUPLA_DS_CALL_SET_CHANNEL_CONFIG_RESULT, &reply, sizeof(reply));
  }

  void notify(unsigned long long scope) {
    testing::IpcSocketAdapterMock socket(-1);
    EXPECT_CALL(socket, send_data(std::string("OK:99126\n"))).Times(1);
    supla_on_channel_config_changed_command command(&socket);
    std::string input = "USER-ON-CHANNEL-CONFIG-CHANGED:99126," +
        std::to_string(destination) + "," + std::to_string(hvac) + "," +
        std::to_string(SUPLA_CHANNELTYPE_HVAC) + "," +
        std::to_string(SUPLA_CHANNELFNC_HVAC_THERMOSTAT) + "," +
        std::to_string(scope) + "\n";
    std::vector<char> buffer(input.begin(), input.end());
    buffer.push_back(0);
    ASSERT_TRUE(command.process_command(buffer.data(), buffer.size(),
                                        input.size()));
  }
};

TEST_F(SupLanHvacWireIntegrationTest, MissingWeeklyAfterIpcKeepsDeviceUsable) {
  TDS_SuplaRegisterDevice_G registration = {};
  snprintf(registration.Email, sizeof(registration.Email), "%s",
           "hvac-wire@test.invalid");
  memset(registration.GUID, 0xab, sizeof(registration.GUID));
  registration.AuthKey[0] = 1;
  registration.Flags = SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED |
                       SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED;
  registration.channel_count = 1;
  auto &channel = registration.channels[0];
  channel.Number = 240;
  channel.Type = SUPLA_CHANNELTYPE_HVAC;
  channel.Default = SUPLA_CHANNELFNC_HVAC_THERMOSTAT;
  channel.FuncList = SUPLA_BIT_FUNC_HVAC_THERMOSTAT;
  channel.Flags = SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE |
                  SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE;
  send_packet(SUPLA_DS_CALL_REGISTER_DEVICE_G, &registration,
              sizeof(TDS_SuplaRegisterDeviceHeader) + sizeof(channel));
  TSuplaDataPacket packet = {};
  bool registered = false, configured = false, synced = false;
  for (int i = 0; i < 8 && !synced; ++i) {
    ASSERT_TRUE(receive_packet(&packet));
    switch (packet.call_id) {
      case SUPLA_SD_CALL_REGISTER_DEVICE_RESULT:
      case SUPLA_SD_CALL_REGISTER_DEVICE_RESULT_B: {
        TSD_SuplaRegisterDeviceResult result = {};
        ASSERT_GE(packet.data_size, sizeof(result));
        memcpy(&result, packet.data, sizeof(result));
        ASSERT_EQ(SUPLA_RESULTCODE_TRUE, result.result_code);
        registered = true;
        break;
      }
      case SUPLA_SD_CALL_SUPLAN_DEVICE_IDENTITIES: {
        TDS_SuplaDeviceIdentitiesResult reply = {SUPLA_SUPLAN_RESULT_OK, 789u};
        send_packet(SUPLA_DS_CALL_SUPLAN_DEVICE_IDENTITIES_RESULT,
                    &reply, sizeof(reply));
        break;
      }
      case SUPLA_SD_CALL_SET_CHANNEL_CONFIG: {
        TSDS_SetChannelConfig config = {};
        memcpy(&config, packet.data, packet.data_size);
        EXPECT_EQ(SUPLA_CONFIG_TYPE_DEFAULT, config.ConfigType);
        acknowledge(config);
        configured = true;
        break;
      }
      case SUPLA_SD_CALL_CHANNEL_CONFIG_FINISHED:
        break;
      case SUPLA_SD_CALL_DEVICE_SYNC_DONE:
        synced = true;
        break;
      default:
        FAIL() << "Unexpected registration packet: " << packet.call_id;
    }
  }
  ASSERT_TRUE(registered && configured && synced);
  for (auto scope : {CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE,
                     CONFIG_CHANGE_SCOPE_JSON_ALT_WEEKLY_SCHEDULE}) {
    notify(scope);
    // Absence must produce no config and must not close the TCP connection.
    EXPECT_FALSE(receive_packet(&packet, 200));
    TDCS_SuplaPingServer ping = {};
    send_packet(SUPLA_DCS_CALL_PING_SERVER, &ping, sizeof(ping));
    ASSERT_TRUE(receive_packet(&packet));
    ASSERT_EQ(SUPLA_SDC_CALL_PING_SERVER_RESULT, packet.call_id);
  }
  EXPECT_FALSE(receive_packet(&packet, 200));
  query("UPDATE supla_dev_channel SET user_config='{\"minOnTimeS\":17,"
        "\"altWeeklySchedule\":{\"programSettings\":{\"1\":{"
        "\"mode\":\"HEAT\",\"setpointTemperatureHeat\":2300}},"
        "\"quarters\":[1]}}' WHERE id=" + std::to_string(hvac));
  notify(CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE |
         CONFIG_CHANGE_SCOPE_JSON_ALT_WEEKLY_SCHEDULE);
  // No heartbeat or further IPC may drive delivery of the remaining intent.
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packet.call_id);
  TSDS_SetChannelConfig schedule = {};
  memcpy(&schedule, packet.data, packet.data_size);
  ASSERT_EQ(SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE, schedule.ConfigType);
  ASSERT_EQ(sizeof(TChannelConfig_WeeklySchedule), schedule.ConfigSize);
  TChannelConfig_WeeklySchedule weekly = {};
  memcpy(&weekly, schedule.Config, sizeof(weekly));
  EXPECT_EQ(SUPLA_HVAC_MODE_HEAT, weekly.Program[0].Mode);
  EXPECT_EQ(2300, weekly.Program[0].SetpointTemperatureHeat);
  EXPECT_EQ(1, weekly.Quarters[0]);
  acknowledge(schedule);
  query("UPDATE supla_dev_channel SET user_config=JSON_SET(user_config,"
        "'$.minOnTimeS',23) WHERE id=" + std::to_string(hvac));
  notify(CONFIG_CHANGE_SCOPE_JSON_DEFAULT);
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packet.call_id);
  TSDS_SetChannelConfig config = {};
  memcpy(&config, packet.data, packet.data_size);
  ASSERT_EQ(SUPLA_CONFIG_TYPE_DEFAULT, config.ConfigType);
  ASSERT_EQ(sizeof(TChannelConfig_HVAC), config.ConfigSize);
  TChannelConfig_HVAC hvac_config = {};
  memcpy(&hvac_config, config.Config, sizeof(hvac_config));
  EXPECT_EQ(23, hvac_config.MinOnTimeS);
  acknowledge(config);
}
}  // namespace
