// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <poll.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "amazon/alexa_discover_request_search_condition.h"
#include "asynctask/asynctask_queue.h"
#include "client/client.h"
#include "conn/connection.h"
#include "db/database.h"
#include "device/device_dao.h"
#include "device/devicechannels.h"
#include "doubles/SrpcAdapterMock.h"
#include "google/google_home_sync_search_condition.h"
#include "ipc/on_channel_config_changed_command.h"
#include "ipc/on_channel_deleted_command.h"
#include "ipc/on_device_deleted_command.h"
#include "jsonconfig/channel/container_config.h"
#include "jsonconfig/channel/hvac_config.h"
#include "jsonconfig/channel/valve_config.h"
#include "jsonconfig/channel/weekly_schedule_config.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"
#include "srpc/srpc_adapter.h"
#include "sthread.h"
#include "supla-socket.h"
#include "suplan/peer_dao.h"
#include "suplan/server_peer_transport.h"
#include "test/doubles/device/DeviceStub.h"
#include "test/doubles/ipc/IpcSocketAdapterMock.h"
#include "test/doubles/suplan/PeerProvisionerTestAccess.h"
#include "test/integration/IntegrationTest.h"
#include "user.h"
using namespace supla_suplan;  // NOLINT
// Inject only at the transport-adapter boundary. Serialization, queueing,
// framing, registration and the connection thread remain production code.
class PublicationPauseAdapter : public supla_srpc_adapter {
 public:
  std::atomic<bool> pause_get{false};
  std::promise<void> prepared, release;
  std::shared_future<void> resume = release.get_future().share();
  explicit PublicationPauseAdapter(void *srpc) : supla_srpc_adapter(srpc) {}
  _supla_int_t sd_async_get_channel_config_result(
      TSD_ChannelConfig *config) override {
    if (pause_get.exchange(false)) {
      prepared.set_value();
      resume.wait_for(std::chrono::seconds(3));
    }
    return supla_srpc_adapter::sd_async_get_channel_config_result(config);
  }
};
class supla_connection_test_access {
 public:
  static PublicationPauseAdapter *install(supla_connection *connection) {
    auto adapter = new PublicationPauseAdapter(connection->_srpc);
    delete connection->srpc_adapter;
    connection->srpc_adapter = adapter;
    return adapter;
  }
};
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
TEST_F(SupLanHvacIntegrationTest, M4EnsureRejectsMalformedBindingIds) {
  auto pump = add_channel(source, 238, SUPLA_CHANNELTYPE_RELAY,
                          SUPLA_CHANNELFNC_PUMPSWITCH);
  TDS_SuplaEnsureResourceShare share = {};
  share.SourceResource = {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, hvac};
  share.DestinationResource = {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, pump};
  share.Permissions = SUPLA_SUPLAN_PERMISSION_READ;
  for (const auto &value : {"\"" + std::to_string(pump) + "\"",
                            std::to_string(pump) + ".5", std::string("-1"),
                            std::string("4294967296"), std::string("null")}) {
    query("UPDATE supla_dev_channel SET user_config="
          "'{\"pumpSwitchChannelId\":" + value + "}' WHERE id=" +
          std::to_string(hvac));
    EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
              service.ensure_share(destination, share).Result);
    std::vector<uint64_t> ids;
    ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN,
                                     channel_config_origin(hvac, 5), &ids));
    EXPECT_TRUE(ids.empty());
  }
}

TEST_F(SupLanHvacIntegrationTest, M4AllHvacOriginsAndSourceFirstShare) {
  auto binary = add_channel(source, 238, SUPLA_CHANNELTYPE_BINARYSENSOR,
                            SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW);
  auto master = add_channel(source, 237, SUPLA_CHANNELTYPE_HVAC,
                            SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  auto pump = add_channel(source, 236, SUPLA_CHANNELTYPE_RELAY,
                          SUPLA_CHANNELFNC_PUMPSWITCH);
  auto heat = add_channel(source, 235, SUPLA_CHANNELTYPE_RELAY,
                          SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH);
  query(
      "UPDATE supla_dev_channel SET "
      "user_config='{\"mainThermometerChannelId\":" +
      std::to_string(resource) +
      ",\"auxThermometerChannelId\":" + std::to_string(resource) +
      ",\"binarySensorChannelId\":" + std::to_string(binary) +
      ",\"masterThermostatChannelId\":" + std::to_string(master) +
      ",\"pumpSwitchChannelId\":" + std::to_string(pump) +
      ",\"heatOrColdSourceSwitchChannelId\":" + std::to_string(heat) +
      "}' WHERE id=" + std::to_string(hvac));
  ASSERT_TRUE(reconcile());
  auto repo = factory();
  for (uint8_t field = 1; field <= 6; ++field) {
    std::vector<uint64_t> ids;
    ASSERT_TRUE(repo->for_origin(CHANNEL_CONFIG_ORIGIN,
                                 channel_config_origin(hvac, field), &ids));
    ASSERT_EQ(1u, ids.size());
    Association a;
    ASSERT_TRUE(repo->load(ids[0], &a));
    EXPECT_EQ(field <= 4 ? source : destination, a.source);
    EXPECT_EQ(field <= 4 ? destination : source, a.destination);
    std::vector<Grant> grants;
    ASSERT_TRUE(repo->grants(a.id, &grants));
    unsigned found = 0;
    for (const auto &g : grants) {
      if (g.origin_id == channel_config_origin(hvac, field)) {
        ++found;
        EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_READ, g.permissions);
        EXPECT_EQ(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, g.resource_type);
        EXPECT_EQ(field <= 2   ? resource
                  : field == 3 ? binary
                  : field == 4 ? master
                               : hvac,
                  g.resource_id);
      }
    }
    EXPECT_EQ(1u, found);
  }
  TDS_SuplaEnsureResourceShare share = {};
  share.SourceResource = {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, hvac};
  share.DestinationResource = {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, pump};
  share.Permissions = SUPLA_SUPLAN_PERMISSION_READ;
  auto result = service.ensure_share(destination, share);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_OK, result.Result);
  EXPECT_EQ(source, result.DestinationDeviceId);
  share.Permissions = SUPLA_SUPLAN_PERMISSION_CONTROL;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
            service.ensure_share(destination, share).Result);
  Association before;
  ASSERT_TRUE(repo->find(source, destination, &before));
  ASSERT_TRUE(reconcile());
  Association after;
  ASSERT_TRUE(repo->load(before.id, &after));
  EXPECT_TRUE(same_state(before, after));
  // Delayed Cloud cleanup: incompatible metadata removes only that origin.
  query("UPDATE supla_dev_channel SET func=0 WHERE id=" +
        std::to_string(binary));
  ASSERT_TRUE(supla_suplan_server_peers::reconcile_dependencies(99124, binary));
  std::vector<uint64_t> ids;
  ASSERT_TRUE(repo->for_origin(CHANNEL_CONFIG_ORIGIN,
                               channel_config_origin(hvac, 3), &ids));
  EXPECT_TRUE(ids.empty());
  ASSERT_EQ(1u, origin_grants().size());
}

TEST_F(SupLanHvacIntegrationTest, DeprecatedRelaysCannotAuthorizeHvacBindings) {
  const int types[] = {SUPLA_CHANNELTYPE_RELAYHFD4,
                       SUPLA_CHANNELTYPE_RELAYG5LA1A,
                       SUPLA_CHANNELTYPE_2XRELAYG5LA1A};
  for (uint8_t field : {5, 6}) {
    const int function = field == 5 ? SUPLA_CHANNELFNC_PUMPSWITCH
                                   : SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH;
    const char *key = hvac_config::reference_keys[field - 1];
    query("UPDATE supla_dev_channel SET user_config='{\"" +
          std::string(key) + "\":" + std::to_string(resource) +
          "}' WHERE id=" + std::to_string(hvac));
    ASSERT_TRUE(service.upsert(destination, source,
                              {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, hvac,
                               99, 5000, SUPLA_SUPLAN_PERMISSION_READ}));
    for (int type : types) {
      SCOPED_TRACE(std::to_string(field) + ":" + std::to_string(type));
      query("UPDATE supla_dev_channel SET type=" +
            std::to_string(SUPLA_CHANNELTYPE_RELAY) + ",func=" +
            std::to_string(function) + " WHERE id=" + std::to_string(resource));
      ASSERT_TRUE(reconcile());
      std::vector<uint64_t> ids;
      ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN,
                                         channel_config_origin(hvac, field),
                                         &ids));
      ASSERT_EQ(1u, ids.size());
      Association before;
      ASSERT_TRUE(factory()->load(ids[0], &before));
      query("UPDATE supla_dev_channel SET type=" + std::to_string(type) +
            " WHERE id=" + std::to_string(resource));
      ASSERT_TRUE(supla_suplan_server_peers::reconcile_dependencies(99124,
                                                                   resource));
      ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN,
                                         channel_config_origin(hvac, field),
                                         &ids));
      EXPECT_TRUE(ids.empty());
      Association after;
      ASSERT_TRUE(factory()->load(before.id, &after));
      EXPECT_TRUE(same_state(before, after));  // Independent READ still exists.
      ASSERT_TRUE(reconcile());  // Invalid relation must not recreate a Grant.
      ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN,
                                         channel_config_origin(hvac, field),
                                         &ids));
      EXPECT_TRUE(ids.empty());
      TDS_SuplaEnsureResourceShare share = {};
      share.SourceResource = {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, hvac};
      share.DestinationResource = {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL,
                                   resource};
      share.Permissions = SUPLA_SUPLAN_PERMISSION_READ;
      EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
                service.ensure_share(destination, share).Result);
    }
  }
}

TEST_F(SupLanHvacIntegrationTest,
       DeviceUploadRejectsDeprecatedLocalHvacRelays) {
  for (int function : {SUPLA_CHANNELFNC_PUMPSWITCH,
                       SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH}) {
    for (int type : {SUPLA_CHANNELTYPE_RELAYHFD4,
                     SUPLA_CHANNELTYPE_RELAYG5LA1A,
                     SUPLA_CHANNELTYPE_2XRELAYG5LA1A,
                     SUPLA_CHANNELTYPE_RELAY}) {
      query("UPDATE supla_dev_channel SET type=" + std::to_string(type) +
            ",func=" + std::to_string(function) + " WHERE id=" +
            std::to_string(local));
      for (int flags : {0, SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED}) {
        query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
              std::to_string(hvac));
        with_destination([&](supla_device_channel *channel) {
          TChannelConfig_HVAC incoming = {};
          if (function == SUPLA_CHANNELFNC_PUMPSWITCH) {
            if (flags) {
              incoming.PumpSwitchChannelId = local;
            } else {
              incoming.PumpSwitchIsSet = 1;
              incoming.PumpSwitchChannelNo = 241;
            }
          } else if (flags) {
            incoming.HeatOrColdSourceSwitchChannelId = local;
          } else {
            incoming.HeatOrColdSourceSwitchIsSet = 1;
            incoming.HeatOrColdSourceSwitchChannelNo = 241;
          }
          ASSERT_EQ(SUPLA_CONFIG_RESULT_TRUE,
                    channel->set_user_config(SUPLA_CONFIG_TYPE_DEFAULT,
                                             sizeof(incoming),
                                             reinterpret_cast<char *>(
                                                 &incoming)));
          std::unique_ptr<supla_json_config> current(
              channel->get_json_config());
          EXPECT_EQ(type == SUPLA_CHANNELTYPE_RELAY ? local : 0u,
                    hvac_config(current.get()).reference(
                        function == SUPLA_CHANNELFNC_PUMPSWITCH ? 4 : 5));
        }, flags);
      }
    }
  }
}

TEST_F(SupLanHvacIntegrationTest,
       M4SourceFirstTargetDeletionAndIndependentOrigin) {
  auto pump = add_channel(source, 238, SUPLA_CHANNELTYPE_RELAY,
                          SUPLA_CHANNELFNC_PUMPSWITCH);
  query("UPDATE supla_dev_channel SET user_config='{\"pumpSwitchChannelId\":" +
        std::to_string(pump) + "}' WHERE id=" + std::to_string(hvac));
  ASSERT_TRUE(reconcile());
  Grant manual{SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, hvac, 2, 999,
               SUPLA_SUPLAN_PERMISSION_READ};
  ASSERT_TRUE(service.upsert(destination, source, manual));
  ASSERT_TRUE(supla_suplan_server_peers::channel_deleted(99124, pump));
  auto repo = factory();
  std::vector<uint64_t> ids;
  ASSERT_TRUE(repo->for_origin(CHANNEL_CONFIG_ORIGIN,
                               channel_config_origin(hvac, 5), &ids));
  EXPECT_TRUE(ids.empty());
  Association a;
  ASSERT_TRUE(repo->find(destination, source, &a));
  std::vector<Grant> grants;
  ASSERT_TRUE(repo->grants(a.id, &grants));
  ASSERT_EQ(1u, grants.size());
  EXPECT_EQ(2, grants[0].origin_type);
}

TEST_F(SupLanHvacIntegrationTest,
       M4BindingDeletionAfterCloudAlreadyClearedReference) {
  auto pump = add_channel(source, 238, SUPLA_CHANNELTYPE_RELAY,
                          SUPLA_CHANNELFNC_PUMPSWITCH);
  query("UPDATE supla_dev_channel SET user_config='{\"pumpSwitchChannelId\":" +
        std::to_string(pump) + "}' WHERE id=" + std::to_string(hvac));
  ASSERT_TRUE(reconcile());
  query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
        std::to_string(hvac));
  query("DELETE FROM supla_dev_channel WHERE id=" + std::to_string(pump));
  ASSERT_TRUE(supla_suplan_server_peers::channel_deleted(99124, pump));
  std::vector<uint64_t> ids;
  ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN,
                                    channel_config_origin(hvac, 5), &ids));
  EXPECT_TRUE(ids.empty());
}

TEST_F(SupLanHvacIntegrationTest,
       M4ArrayOriginsCanonicalWireAndLegacyBoundary) {
  for (bool container : {true, false}) {
    query("UPDATE supla_dev_channel SET type=" +
          std::to_string(container ? SUPLA_CHANNELTYPE_CONTAINER
                                   : SUPLA_CHANNELTYPE_VALVE_OPENCLOSE) +
          ",func=" +
          std::to_string(container ? SUPLA_CHANNELFNC_CONTAINER
                                   : SUPLA_CHANNELFNC_VALVE_OPENCLOSE) +
          " WHERE id=" + std::to_string(hvac));
    query("UPDATE supla_dev_channel SET type=" +
          std::to_string(SUPLA_CHANNELTYPE_BINARYSENSOR) + ",func=" +
          std::to_string(container ? SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR
                                   : SUPLA_CHANNELFNC_FLOOD_SENSOR) +
          " WHERE id IN (" + std::to_string(resource) + "," +
          std::to_string(local) + ")");
    std::unique_ptr<supla_json_config> json(
        container ? static_cast<supla_json_config *>(new container_config())
                  : static_cast<supla_json_config *>(new valve_config()));
    if (container) {
      TChannelConfig_Container cfg = {};
      for (size_t slot = 0; slot < 10; ++slot) {
        cfg.SensorInfo[slot].ChannelId = slot == 0 ? local : resource;
        cfg.SensorInfo[slot].FillLevel = slot * 10;
      }
      container_config writer(json.get());
      writer.set_config(&cfg);
    } else {
      TChannelConfig_Valve cfg = {};
      for (auto &entry : cfg.SensorInfo) {
        entry.ChannelId = resource;
      }
      cfg.SensorInfo[0].ChannelId = local;
      valve_config writer(json.get());
      writer.set_config(&cfg);
    }
    supla_mariadb_access_provider db;
    supla_device_dao dao(&db);
    ASSERT_TRUE(dao.set_channel_config(99124, hvac, json.get()));
    ASSERT_TRUE(reconcile());
    auto repo = factory();
    Association before;
    ASSERT_TRUE(repo->find(source, destination, &before));
    ASSERT_EQ(1u, before.acl.size());
    ASSERT_TRUE(reconcile());
    Association after;
    ASSERT_TRUE(repo->load(before.id, &after));
    EXPECT_TRUE(same_state(before, after));
    for (auto flags : {SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED, 0}) {
      with_destination(
          [&](supla_device_channel *channel) {
            TSD_ChannelConfig wire = {};
            ASSERT_TRUE(
                channel->get_config(&wire, SUPLA_CONFIG_TYPE_DEFAULT, 0));
            if (container) {
              auto cfg =
                  reinterpret_cast<TChannelConfig_Container *>(wire.Config);
              for (unsigned slot = 0; slot < 10; ++slot) {
                EXPECT_EQ(slot * 10, cfg->SensorInfo[slot].FillLevel);
                if (flags) {
                  EXPECT_EQ(slot == 0 ? local : resource,
                            cfg->SensorInfo[slot].ChannelId);
                } else if (slot == 0) {
                  EXPECT_EQ(1, cfg->SensorInfo[slot].IsSet);
                  EXPECT_EQ(241, cfg->SensorInfo[slot].ChannelNo);
                } else {
                  EXPECT_EQ(0, cfg->SensorInfo[slot].ChannelId);
                }
              }
            } else {
              auto cfg = reinterpret_cast<TChannelConfig_Valve *>(wire.Config);
              for (unsigned slot = 0; slot < 20; ++slot) {
                if (flags) {
                  EXPECT_EQ(slot == 0 ? local : resource,
                            cfg->SensorInfo[slot].ChannelId);
                } else if (slot == 0) {
                  EXPECT_EQ(1, cfg->SensorInfo[slot].IsSet);
                  EXPECT_EQ(241, cfg->SensorInfo[slot].ChannelNo);
                } else {
                  EXPECT_EQ(0, cfg->SensorInfo[slot].ChannelId);
                }
              }
            }
          },
          flags);
    }
    // Remove this family's origins before reusing this fixture Channel type.
    query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
          std::to_string(hvac));
    ASSERT_TRUE(reconcile());
  }
}

TEST_F(SupLanHvacIntegrationTest,
       M4AllFieldsReplacementRemovalAndCapabilityRevocation) {
  const char *keys[] = {
      "mainThermometerChannelId", "auxThermometerChannelId",
      "binarySensorChannelId",    "masterThermostatChannelId",
      "pumpSwitchChannelId",      "heatOrColdSourceSwitchChannelId"};
  for (uint8_t field = 2; field <= 6; ++field) {
    SCOPED_TRACE(field);
    int type = field == 2   ? SUPLA_CHANNELTYPE_THERMOMETER
               : field == 3 ? SUPLA_CHANNELTYPE_BINARYSENSOR
               : field == 4 ? SUPLA_CHANNELTYPE_HVAC
                            : SUPLA_CHANNELTYPE_RELAY;
    int func = field == 2   ? SUPLA_CHANNELFNC_THERMOMETER
               : field == 3 ? SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW
               : field == 4 ? SUPLA_CHANNELFNC_HVAC_THERMOSTAT
               : field == 5 ? SUPLA_CHANNELFNC_PUMPSWITCH
                            : SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH;
    query("UPDATE supla_dev_channel SET type=" + std::to_string(type) +
          ",func=" + std::to_string(func) + ",user_config='{}' WHERE id IN (" +
          std::to_string(resource) + "," + std::to_string(other_resource) +
          "," + std::to_string(local) + ")");
    auto select = [&](uint32_t id) {
      query("UPDATE supla_dev_channel SET user_config='{\"" +
            std::string(keys[field - 1]) + "\":" + std::to_string(id) +
            "}' WHERE id=" + std::to_string(hvac));
      ASSERT_TRUE(reconcile());
    };
    auto origins = [&]() {
      std::vector<uint64_t> ids;
      EXPECT_TRUE(factory()->for_origin(
          CHANNEL_CONFIG_ORIGIN, channel_config_origin(hvac, field), &ids));
      return ids;
    };
    select(local);
    EXPECT_TRUE(origins().empty());
    select(resource);
    auto a = origins();
    ASSERT_EQ(1u, a.size());
    Association before;
    ASSERT_TRUE(factory()->load(a[0], &before));
    ASSERT_TRUE(reconcile());
    Association after;
    ASSERT_TRUE(factory()->load(a[0], &after));
    EXPECT_TRUE(same_state(before, after));
    select(other_resource);
    auto b = origins();
    ASSERT_EQ(1u, b.size());
    EXPECT_NE(a[0], b[0]);
    select(0);
    EXPECT_TRUE(origins().empty());
    select(resource);
    for (int endpoint : {source, destination}) {
      query("UPDATE supla_iodevice SET flags=flags & ~" +
            std::to_string(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) +
            " WHERE id=" + std::to_string(endpoint));
      ASSERT_TRUE(reconcile());
      EXPECT_TRUE(origins().empty());
      query("UPDATE supla_iodevice SET flags=flags | " +
            std::to_string(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) +
            " WHERE id=" + std::to_string(endpoint));
      ASSERT_TRUE(reconcile());
      ASSERT_EQ(1u, origins().size());
    }
    query("UPDATE supla_dev_channel SET func=0 WHERE id=" +
          std::to_string(resource));
    ASSERT_TRUE(
        supla_suplan_server_peers::reconcile_dependencies(99124, resource));
    EXPECT_TRUE(origins().empty());
    query("UPDATE supla_dev_channel SET func=" + std::to_string(func) +
          " WHERE id=" + std::to_string(resource));
    ASSERT_TRUE(
        supla_suplan_server_peers::reconcile_dependencies(99124, resource));
    EXPECT_EQ(1u, origins().size());
    select(local);
    EXPECT_TRUE(origins().empty());
    select(0);
  }
}

TEST_F(SupLanHvacIntegrationTest, M4MasterChainsAndConsumerTypeChangeRevoke) {
  auto master = add_channel(source, 238, SUPLA_CHANNELTYPE_HVAC,
                            SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  query(
      "UPDATE supla_dev_channel SET "
      "user_config='{\"masterThermostatChannelId\":" +
      std::to_string(master) + "}' WHERE id=" + std::to_string(hvac));
  ASSERT_TRUE(reconcile());
  std::vector<uint64_t> ids;
  ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN,
                                    channel_config_origin(hvac, 4), &ids));
  ASSERT_EQ(1u, ids.size());
  query(
      "UPDATE supla_dev_channel SET "
      "user_config='{\"masterThermostatChannelId\":" +
      std::to_string(hvac) + "}' WHERE id=" + std::to_string(master));
  ASSERT_TRUE(reconcile());
  ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN,
                                    channel_config_origin(hvac, 4), &ids));
  EXPECT_TRUE(ids.empty());
  query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
        std::to_string(master));
  ASSERT_TRUE(reconcile());
  ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN,
                                    channel_config_origin(hvac, 4), &ids));
  ASSERT_EQ(1u, ids.size());
  query("UPDATE supla_dev_channel SET type=" +
        std::to_string(SUPLA_CHANNELTYPE_RELAY) +
        ",func=" + std::to_string(SUPLA_CHANNELFNC_POWERSWITCH) +
        " WHERE id=" + std::to_string(hvac));
  ASSERT_TRUE(reconcile());
  ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN,
                                    channel_config_origin(hvac, 4), &ids));
  EXPECT_TRUE(ids.empty());
}

TEST_F(SupLanHvacIntegrationTest,
       M4ArrayDeviceUploadProtectedMergeAndLegacyNumberZero) {
  for (bool container : {true, false}) {
    SCOPED_TRACE(container ? "Container" : "Valve");
    query("UPDATE supla_dev_channel SET type=" +
          std::to_string(container ? SUPLA_CHANNELTYPE_CONTAINER
                                   : SUPLA_CHANNELTYPE_VALVE_PERCENTAGE) +
          ",func=" +
          std::to_string(container ? SUPLA_CHANNELFNC_CONTAINER
                                   : SUPLA_CHANNELFNC_VALVE_PERCENTAGE) +
          " WHERE id=" + std::to_string(hvac));
    int function = container ? SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR
                             : SUPLA_CHANNELFNC_FLOOD_SENSOR;
    query("UPDATE supla_dev_channel SET type=" +
          std::to_string(SUPLA_CHANNELTYPE_BINARYSENSOR) +
          ",func=" + std::to_string(function) + " WHERE id IN (" +
          std::to_string(resource) + "," + std::to_string(other_resource) +
          "," + std::to_string(local) + ")");
    query("DELETE FROM supla_dev_channel WHERE iodevice_id=" +
          std::to_string(destination) + " AND channel_number=0 AND id<>" +
          std::to_string(local));
    query("UPDATE supla_dev_channel SET channel_number=0 WHERE id=" +
          std::to_string(local));
    for (uint32_t current : {0u, local, resource}) {
      std::unique_ptr<supla_json_config> root(
          container ? static_cast<supla_json_config *>(new container_config())
                    : static_cast<supla_json_config *>(new valve_config()));
      if (container) {
        TChannelConfig_Container raw = {};
        raw.SensorInfo[0].ChannelId = current;
        container_config(root.get()).set_config(&raw);
      } else {
        TChannelConfig_Valve raw = {};
        raw.SensorInfo[0].ChannelId = current;
        valve_config(root.get()).set_config(&raw);
      }
      supla_mariadb_access_provider db;
      supla_device_dao dao(&db);
      ASSERT_TRUE(dao.set_channel_config(99124, hvac, root.get()));
      ASSERT_TRUE(reconcile());
      for (uint32_t incoming : {current, 0u, local, other_resource}) {
        with_destination([&](supla_device_channel *channel) {
          TChannelConfig_Container tank = {};
          TChannelConfig_Valve valve = {};
          tank.SensorInfo[0].ChannelId = incoming;
          tank.SensorInfo[1].ChannelId = local;
          tank.SensorInfo[0].FillLevel = 60;
          tank.WarningAboveLevel = 31;
          valve.SensorInfo[0].ChannelId = incoming;
          valve.SensorInfo[1].ChannelId = local;
          valve.CloseValveOnFloodType =
              SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ON_CHANGE;
          ASSERT_EQ(SUPLA_CONFIG_RESULT_TRUE,
                    channel->set_user_config(
                        SUPLA_CONFIG_TYPE_DEFAULT,
                        container ? sizeof(tank) : sizeof(valve),
                        container ? reinterpret_cast<char *>(&tank)
                                  : reinterpret_cast<char *>(&valve)));
          std::unique_ptr<supla_json_config> merged(channel->get_json_config());
          if (container) {
            container_config(merged.get()).get_config(&tank);
            EXPECT_EQ(current == resource          ? resource
                      : incoming == other_resource ? current
                                                   : incoming,
                      tank.SensorInfo[0].ChannelId);
            EXPECT_EQ(local, tank.SensorInfo[1].ChannelId);
            EXPECT_EQ(60, tank.SensorInfo[0].FillLevel);
            EXPECT_EQ(31, tank.WarningAboveLevel);
          } else {
            valve_config(merged.get()).get_config(&valve);
            EXPECT_EQ(current == resource          ? resource
                      : incoming == other_resource ? current
                                                   : incoming,
                      valve.SensorInfo[0].ChannelId);
            EXPECT_EQ(local, valve.SensorInfo[1].ChannelId);
            EXPECT_EQ(SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ON_CHANGE,
                      valve.CloseValveOnFloodType);
          }
        });
        // Restore authority between independent matrix rows.
        ASSERT_TRUE(dao.set_channel_config(99124, hvac, root.get()));
      }
    }
    query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
          std::to_string(hvac));
    ASSERT_TRUE(reconcile());
    with_destination(
        [&](supla_device_channel *channel) {
          TChannelConfig_Container tank = {};
          TChannelConfig_Valve valve = {};
          tank.SensorInfo[0].IsSet = 1;
          tank.SensorInfo[0].ChannelNo = 0;
          tank.SensorInfo[1].IsSet = 1;
          tank.SensorInfo[1].ChannelNo = 238;
          valve.SensorInfo[0].IsSet = 1;
          valve.SensorInfo[0].ChannelNo = 0;
          valve.SensorInfo[1].IsSet = 1;
          valve.SensorInfo[1].ChannelNo = 238;
          ASSERT_EQ(SUPLA_CONFIG_RESULT_TRUE,
                    channel->set_user_config(
                        SUPLA_CONFIG_TYPE_DEFAULT,
                        container ? sizeof(tank) : sizeof(valve),
                        container ? reinterpret_cast<char *>(&tank)
                                  : reinterpret_cast<char *>(&valve)));
          std::unique_ptr<supla_json_config> canonical(
              channel->get_json_config());
          if (container) {
            container_config(canonical.get()).get_config(&tank);
            EXPECT_EQ(local, tank.SensorInfo[0].ChannelId);
            EXPECT_EQ(0, tank.SensorInfo[1].ChannelId);
          } else {
            valve_config(canonical.get()).get_config(&valve);
            EXPECT_EQ(local, valve.SensorInfo[0].ChannelId);
            EXPECT_EQ(0, valve.SensorInfo[1].ChannelId);
          }
        },
        0);
    std::vector<uint64_t> origins;
    ASSERT_TRUE(PeerDao(99124).config_origin_fields(hvac, &origins));
    EXPECT_TRUE(origins.empty());
    query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
          std::to_string(hvac));
    ASSERT_TRUE(reconcile());
  }
}

TEST_F(SupLanHvacIntegrationTest,
       CloudBinaryFunctionsHaveExactReadGrantsAcrossAllConsumerFamilies) {
  const int functions[] = {
      SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY,
      SUPLA_CHANNELFNC_OPENINGSENSOR_GATE,
      SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR,
      SUPLA_CHANNELFNC_NOLIQUIDSENSOR,
      SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR,
      SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER,
      SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW,
      SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW, SUPLA_CHANNELFNC_HOTELCARDSENSOR,
      SUPLA_CHANNELFNC_ALARMARMAMENTSENSOR, SUPLA_CHANNELFNC_MAILSENSOR,
      SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR, SUPLA_CHANNELFNC_FLOOD_SENSOR,
      SUPLA_CHANNELFNC_MOTION_SENSOR, SUPLA_CHANNELFNC_BINARY_SENSOR};
  const int types[] = {SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELTYPE_CONTAINER,
                       SUPLA_CHANNELTYPE_VALVE_OPENCLOSE};
  const int consumer_functions[] = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                                   SUPLA_CHANNELFNC_CONTAINER,
                                   SUPLA_CHANNELFNC_VALVE_OPENCLOSE};
  const uint8_t fields[] = {3, 32, 64};
  const std::string configs[] = {
      "{\"binarySensorChannelId\":" + std::to_string(resource) + "}",
      "{\"sensors\":[{\"channelId\":" + std::to_string(resource) + "}]}",
      "{\"floodSensorChannelIds\":[" + std::to_string(resource) + "]}"};
  for (unsigned family = 0; family < 3; ++family) {
    SCOPED_TRACE(family);
    query("UPDATE supla_dev_channel SET type=" + std::to_string(types[family]) +
          ",func=" + std::to_string(consumer_functions[family]) +
          ",user_config='" + configs[family] + "' WHERE id=" +
          std::to_string(hvac));
    const uint64_t origin = channel_config_origin(hvac, fields[family]);
    for (int function : functions) {
      SCOPED_TRACE(function);
      query("UPDATE supla_dev_channel SET type=" +
            std::to_string(SUPLA_CHANNELTYPE_BINARYSENSOR) + ",func=" +
            std::to_string(function) + " WHERE id=" + std::to_string(resource));
      ASSERT_TRUE(reconcile());
      std::vector<uint64_t> ids;
      ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN, origin, &ids));
      ASSERT_EQ(1u, ids.size());
      Association before;
      ASSERT_TRUE(factory()->load(ids[0], &before));
      EXPECT_EQ(source, before.source);
      EXPECT_EQ(destination, before.destination);
      std::vector<Grant> grants;
      ASSERT_TRUE(factory()->grants(ids[0], &grants));
      ASSERT_EQ(1u, grants.size());
      EXPECT_EQ(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, grants[0].resource_type);
      EXPECT_EQ(resource, grants[0].resource_id);
      EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_READ, grants[0].permissions);
      EXPECT_EQ(CHANNEL_CONFIG_ORIGIN, grants[0].origin_type);
      EXPECT_EQ(origin, grants[0].origin_id);
      ASSERT_TRUE(reconcile());
      EXPECT_TRUE(same_state(before, state()));
    }
    // A separate authorization must survive config eligibility revocation.
    ASSERT_TRUE(service.upsert(source, destination,
                              {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, resource,
                               99, 5000, SUPLA_SUPLAN_PERMISSION_READ}));
    Association manual = state();
    for (int invalid : {0, SUPLA_CHANNELFNC_THERMOMETER, 123456}) {
      query("UPDATE supla_dev_channel SET func=" + std::to_string(invalid) +
            " WHERE id=" + std::to_string(resource));
      ASSERT_TRUE(supla_suplan_server_peers::reconcile_dependencies(99124,
                                                                  resource));
      std::vector<uint64_t> ids;
      ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN, origin, &ids));
      EXPECT_TRUE(ids.empty());
      ASSERT_TRUE(factory()->for_origin(99, 5000, &ids));
      EXPECT_EQ(1u, ids.size());
      EXPECT_TRUE(same_state(manual, state()));
    }
    query("UPDATE supla_dev_channel SET func=" +
          std::to_string(SUPLA_CHANNELFNC_BINARY_SENSOR) + " WHERE id=" +
          std::to_string(resource));
    ASSERT_TRUE(supla_suplan_server_peers::reconcile_dependencies(99124,
                                                                resource));
    std::vector<uint64_t> ids;
    ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN, origin, &ids));
    EXPECT_EQ(1u, ids.size());
    query("UPDATE supla_dev_channel SET type=" +
          std::to_string(SUPLA_CHANNELTYPE_THERMOMETER) + " WHERE id=" +
          std::to_string(resource));
    ASSERT_TRUE(reconcile());
    ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN, origin, &ids));
    EXPECT_TRUE(ids.empty());
    query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
          std::to_string(hvac));
    query("UPDATE supla_dev_channel SET type=" +
          std::to_string(SUPLA_CHANNELTYPE_BINARYSENSOR) + " WHERE id=" +
          std::to_string(resource));
    ASSERT_TRUE(supla_suplan_server_peers::reconcile_dependencies(99124,
                                                                resource));
    ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN, origin, &ids));
    EXPECT_TRUE(ids.empty());
    ASSERT_TRUE(service.delete_origin(99, 5000));
  }
}

TEST_F(SupLanHvacIntegrationTest,
       Ds18b20NeverGetsRemoteReadButLocalWireCompatibilitySurvives) {
  for (const auto &field : {std::make_pair("mainThermometerChannelId", 1),
                            std::make_pair("auxThermometerChannelId", 2)}) {
    SCOPED_TRACE(field.first);
    query("UPDATE supla_dev_channel SET user_config='{\"" +
          std::string(field.first) + "\":" + std::to_string(resource) +
          "}' WHERE id=" + std::to_string(hvac));
    for (int type : {SUPLA_CHANNELTYPE_THERMOMETERDS18B20,
                     SUPLA_CHANNELTYPE_THERMOMETER,
                     SUPLA_CHANNELTYPE_THERMOMETERDS18B20,
                     SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR}) {
      int function = type == SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR
                         ? SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE
                         : SUPLA_CHANNELFNC_THERMOMETER;
      query("UPDATE supla_dev_channel SET type=" + std::to_string(type) +
            ",func=" + std::to_string(function) +
            " WHERE id=" + std::to_string(resource));
      ASSERT_TRUE(reconcile());
      std::vector<uint64_t> ids;
      ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN,
                                         channel_config_origin(
                                             hvac, field.second),
                                         &ids));
      EXPECT_EQ(type == SUPLA_CHANNELTYPE_THERMOMETERDS18B20 ? 0u : 1u,
                ids.size());
    }
    query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
          std::to_string(hvac));
    ASSERT_TRUE(reconcile());
  }
  query("UPDATE supla_dev_channel SET type=" +
        std::to_string(SUPLA_CHANNELTYPE_THERMOMETERDS18B20) + " WHERE id=" +
        std::to_string(local));
  for (int flags : {0, SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED}) {
    with_destination([&](supla_device_channel *channel) {
      TChannelConfig_HVAC uploaded = {};
      uploaded.MainThermometerChannelId = flags ? local : 241;
      uploaded.AuxThermometerChannelId = flags ? local : 241;
      EXPECT_EQ(SUPLA_CONFIG_RESULT_TRUE,
                channel->set_user_config(SUPLA_CONFIG_TYPE_DEFAULT,
                                         sizeof(uploaded),
                                         reinterpret_cast<char *>(&uploaded)));
      TSD_ChannelConfig published = {};
      ASSERT_TRUE(
          channel->get_config(&published, SUPLA_CONFIG_TYPE_DEFAULT, 0));
      TChannelConfig_HVAC received = {};
      ASSERT_EQ(sizeof(received), published.ConfigSize);
      memcpy(&received, published.Config, sizeof(received));
      EXPECT_EQ(flags ? local : 241u, received.MainThermometerChannelId);
      EXPECT_EQ(flags ? local : 241u, received.AuxThermometerChannelId);
    }, flags);
    EXPECT_EQ(local, scalar("SELECT JSON_VALUE(user_config,"
                            "'$.mainThermometerChannelId') "
                            "FROM supla_dev_channel WHERE id=" +
                            std::to_string(hvac)));
    std::vector<uint64_t> origins;
    ASSERT_TRUE(PeerDao(99124).config_origin_fields(hvac, &origins));
    EXPECT_TRUE(origins.empty());
  }
}


TEST_F(SupLanHvacIntegrationTest,
       BinaryNoneRevokesOnlyItsConfigOriginAndPreservesOtherConfigAndManual) {
  query("UPDATE supla_dev_channel SET type=" +
        std::to_string(SUPLA_CHANNELTYPE_BINARYSENSOR) + ",func=" +
        std::to_string(SUPLA_CHANNELFNC_BINARY_SENSOR) + " WHERE id=" +
        std::to_string(resource));
  query("UPDATE supla_dev_channel SET user_config='{"
        "\"mainThermometerChannelId\":" + std::to_string(other_resource) +
        ",\"binarySensorChannelId\":" + std::to_string(resource) +
        "}' WHERE id=" + std::to_string(hvac));
  ASSERT_TRUE(reconcile());
  ASSERT_TRUE(service.upsert(source, destination,
                            {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, resource,
                             99, 5000, SUPLA_SUPLAN_PERMISSION_READ}));
  Association manual = state(), other;
  ASSERT_TRUE(factory()->find(other_source, destination, &other));
  query("UPDATE supla_dev_channel SET func=0 WHERE id=" +
        std::to_string(resource));
  ASSERT_TRUE(
      supla_suplan_server_peers::reconcile_dependencies(99124, resource));
  std::vector<uint64_t> ids;
  ASSERT_TRUE(factory()->for_origin(CHANNEL_CONFIG_ORIGIN,
                                     channel_config_origin(hvac, 3), &ids));
  EXPECT_TRUE(ids.empty());
  auto main = origin_grants();
  ASSERT_EQ(1u, main.size());
  EXPECT_EQ(other_resource, main[0].resource_id);
  EXPECT_TRUE(same_state(manual, state()));
  Association after;
  ASSERT_TRUE(factory()->find(other_source, destination, &after));
  EXPECT_TRUE(same_state(other, after));
}

TEST_F(SupLanHvacIntegrationTest,
       LocalBinaryUploadCompatibilityRemainsTypeBasedForAllConsumers) {
  query("UPDATE supla_dev_channel SET type=" +
        std::to_string(SUPLA_CHANNELTYPE_BINARYSENSOR) + " WHERE id=" +
        std::to_string(local));
  for (int function : {0, SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW}) {
    query("UPDATE supla_dev_channel SET func=" + std::to_string(function) +
          " WHERE id=" + std::to_string(local));
    for (int family = 0; family < 3; ++family) {
      const int types[] = {SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELTYPE_CONTAINER,
                           SUPLA_CHANNELTYPE_VALVE_OPENCLOSE};
      const int functions[] = {SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                               SUPLA_CHANNELFNC_CONTAINER,
                               SUPLA_CHANNELFNC_VALVE_OPENCLOSE};
      query("UPDATE supla_dev_channel SET type=" +
            std::to_string(types[family]) +
            ",func=" + std::to_string(functions[family]) +
            ",user_config='{}' WHERE id=" + std::to_string(hvac));
      for (int flags : {0, SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED}) {
        with_destination([&](supla_device_channel *channel) {
          TChannelConfig_HVAC hvac_cfg = {};
          TChannelConfig_Container tank = {};
          TChannelConfig_Valve valve = {};
          unsigned int id = flags ? local : 241;
          hvac_cfg.BinarySensorChannelId = id;
          if (flags) {
            tank.SensorInfo[0].ChannelId = local;
            valve.SensorInfo[0].ChannelId = local;
          } else {
            tank.SensorInfo[0].IsSet = 1;
            tank.SensorInfo[0].ChannelNo = 241;
            valve.SensorInfo[0].IsSet = 1;
            valve.SensorInfo[0].ChannelNo = 241;
          }
          char *payload = family == 0 ? reinterpret_cast<char *>(&hvac_cfg)
                          : family == 1 ? reinterpret_cast<char *>(&tank)
                                        : reinterpret_cast<char *>(&valve);
          size_t size = family == 0 ? sizeof(hvac_cfg)
                        : family == 1 ? sizeof(tank) : sizeof(valve);
          ASSERT_EQ(SUPLA_CONFIG_RESULT_TRUE,
                    channel->set_user_config(SUPLA_CONFIG_TYPE_DEFAULT,
                                             size, payload));
          std::unique_ptr<supla_json_config> current(
              channel->get_json_config());
          if (family == 0) {
            EXPECT_EQ(local, hvac_config(current.get()).reference(2));
          } else if (family == 1) {
            ASSERT_TRUE(container_config(current.get()).get_config(&tank));
            EXPECT_EQ(local, tank.SensorInfo[0].ChannelId);
          } else {
            ASSERT_TRUE(valve_config(current.get()).get_config(&valve));
            EXPECT_EQ(local, valve.SensorInfo[0].ChannelId);
          }
        }, flags);
      }
    }
  }
}

TEST_F(SupLanHvacIntegrationTest,
       SharedDatabaseVersionCheckRequiresM4Migration) {
  EXPECT_STREQ("20261008160000", DB_VERSION);
  query("DELETE FROM migration_versions");
  query("INSERT INTO migration_versions(version) "
        "VALUES('Version20261008160000')");
  database db;
  EXPECT_TRUE(db.check_db_version(DB_VERSION, 0));
  query("UPDATE migration_versions SET version='Version20261008120000'");
  EXPECT_FALSE(db.check_db_version(DB_VERSION, 0));
}

}  // namespace

// A Client access-list fixture; dispatch, protected DB writes and the Device
// socket publisher remain production code. No Client registration claim.
class M4ConfigClient : public supla_client {
 public:
  explicit M4ConfigClient(supla_connection *connection, supla_user *user)
      : supla_client(connection) {
    set_user(user);
    set_id(91234);
    set_registered(true);
  }
};

namespace {
// Only the remote Device and IPC socket are test peers. Registration, DB reads,
// reconciliation, dispatch, publication and SRPC run in production code.
class SupLanHvacWireIntegrationTest : public SupLanHvacIntegrationTest {
 protected:
  void *listener = nullptr, *worker = nullptr, *wire = nullptr;
  int client = -1;
  supla_connection *connection = nullptr;
  PublicationPauseAdapter *publication_adapter = nullptr;
  int publication_type = SUPLA_CHANNELTYPE_HVAC;
  int publication_function = SUPLA_CHANNELFNC_HVAC_THERMOSTAT;

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
    start_connection();
  }

  void start_connection() {
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
    publication_adapter = supla_connection_test_access::install(connection);
    wire = sproto_init();
    ASSERT_NE(nullptr, wire);
    sproto_set_version(wire, 29);
    sthread_simple_run(
        [](void *conn, void *thread) {
          static_cast<supla_connection *>(conn)->execute(thread);
        }, connection, false, &worker);
    ASSERT_NE(nullptr, worker);
  }

  void close_connection() {
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
    worker = wire = listener = nullptr;
    connection = nullptr;
    publication_adapter = nullptr;
    client = -1;
  }

  void TearDown() override {
    close_connection();
    // The real IPC handler also schedules delayed voice-assistant work. Its
    // credentials belong to this runtime user, which cleanup destroys below.
    // Remove only this fixture's requests; keep the global pool usable by
    // subsequent suites.
    supla_alexa_discover_request_search_condition alexa(99126);
    supla_google_home_sync_search_condition google(99126);
    auto queue = supla_asynctask_queue::global_instance();
    queue->cancel_tasks(&alexa);
    queue->cancel_tasks(&google);
    EXPECT_EQ(0u, queue->get_task_count(&alexa));
    EXPECT_EQ(0u, queue->get_task_count(&google));
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
    std::string input =
        "USER-ON-CHANNEL-CONFIG-CHANGED:99126," + std::to_string(destination) +
        "," + std::to_string(hvac) + "," + std::to_string(publication_type) +
        "," + std::to_string(publication_function) + "," +
        std::to_string(scope) + "\n";
    std::vector<char> buffer(input.begin(), input.end());
    buffer.push_back(0);
    ASSERT_TRUE(command.process_command(buffer.data(), buffer.size(),
                                        input.size()));
  }
  void register_family(int type, int function, bool weekly = false) {
    publication_type = type;
    publication_function = function;
    query("UPDATE supla_dev_channel SET type=" + std::to_string(type) +
          ",func=" + std::to_string(function) +
          ",flist=0,user_config='{}' WHERE id=" + std::to_string(hvac));
    register_existing_family(type, function, function, weekly);
  }

  void register_existing_family(int type, int advertised_function,
                                int selected_function, bool weekly = false) {
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
    channel.Type = type;
    channel.Default = advertised_function;
    channel.Flags = SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE |
                    (weekly ? SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE : 0);
    send_packet(SUPLA_DS_CALL_REGISTER_DEVICE_G, &registration,
                sizeof(TDS_SuplaRegisterDeviceHeader) + sizeof(channel));
    bool registered = false, synced = false;
    TSuplaDataPacket packet = {};
    for (unsigned i = 0; i < 10 && !synced; ++i) {
      ASSERT_TRUE(receive_packet(&packet));
      if (packet.call_id == SUPLA_SD_CALL_REGISTER_DEVICE_RESULT ||
          packet.call_id == SUPLA_SD_CALL_REGISTER_DEVICE_RESULT_B) {
        TSD_SuplaRegisterDeviceResult reply = {};
        memcpy(&reply, packet.data, sizeof(reply));
        ASSERT_EQ(SUPLA_RESULTCODE_TRUE, reply.result_code);
        registered = true;
      } else if (packet.call_id == SUPLA_SD_CALL_SUPLAN_DEVICE_IDENTITIES) {
        TDS_SuplaDeviceIdentitiesResult reply = {SUPLA_SUPLAN_RESULT_OK, 789u};
        send_packet(SUPLA_DS_CALL_SUPLAN_DEVICE_IDENTITIES_RESULT, &reply,
                    sizeof(reply));
      } else if (packet.call_id == SUPLA_SD_CALL_SET_CHANNEL_CONFIG) {
        TSDS_SetChannelConfig config = {};
        memcpy(&config, packet.data, packet.data_size);
        EXPECT_EQ(selected_function, config.Func);
        acknowledge(config);
      } else if (packet.call_id == SUPLA_SD_CALL_DEVICE_SYNC_DONE) {
        synced = true;
      }
    }
    ASSERT_TRUE(registered && synced);
  }

  void expect_access(uint32_t channel, uint8_t result, uint8_t status) {
    TDS_SuplaEnsureResourceAccess request = {
        {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, channel},
        SUPLA_SUPLAN_PERMISSION_READ, SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER, 0};
    send_packet(SUPLA_DS_CALL_ENSURE_SUPLAN_RESOURCE_ACCESS, &request,
                sizeof(request));
    TSuplaDataPacket packet = {};
    for (unsigned n = 0; n < 10; ++n) {
      ASSERT_TRUE(receive_packet(&packet));
      if (packet.call_id != SUPLA_SD_CALL_ENSURE_SUPLAN_RESOURCE_ACCESS_RESULT)
        continue;
      ASSERT_EQ(sizeof(TSD_SuplaEnsureResourceAccessResult), packet.data_size);
      TSD_SuplaEnsureResourceAccessResult reply = {};
      memcpy(&reply, packet.data, sizeof(reply));
      EXPECT_EQ(result, reply.Result);
      EXPECT_EQ(status, reply.AccessStatus);
      return;
    }
    FAIL() << "No ENSURE_ACCESS response";
  }

  void access_moved_origin_with_pressure(bool binding) {
    register_family(binding ? SUPLA_CHANNELTYPE_RELAY : SUPLA_CHANNELTYPE_HVAC,
                    binding ? SUPLA_CHANNELFNC_PUMPSWITCH
                            : SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
    ASSERT_FALSE(HasFatalFailure());
    auto add = [&](int device, int number, int type, int function) {
      auto id = add_channel(device, number, type, function);
      query("UPDATE supla_dev_channel SET user_id=99126 WHERE id=" +
            std::to_string(id));
      return id;
    };
    uint32_t consumer = binding ? resource : hvac;
    uint8_t field = binding ? 5 : 1;
    uint32_t next = other_resource;
    if (binding) {
      query("UPDATE supla_dev_channel SET type=" +
            std::to_string(SUPLA_CHANNELTYPE_HVAC) + ",func=" +
            std::to_string(SUPLA_CHANNELFNC_HVAC_THERMOSTAT) +
            " WHERE id=" + std::to_string(resource));
      next = add(other_source, 236, SUPLA_CHANNELTYPE_RELAY,
                 SUPLA_CHANNELFNC_PUMPSWITCH);
    }
    auto select = [&](uint32_t id) {
      std::string key = binding ? "pumpSwitchChannelId"
                                : "mainThermometerChannelId";
      query("UPDATE supla_dev_channel SET user_config='{\"" + key + "\":" +
            std::to_string(id) + "}' WHERE id=" + std::to_string(consumer));
    };
    select(binding ? hvac : resource);
    PeerDao repo(99126);
    PeerService grants([] { return std::make_unique<PeerDao>(99126); },
                       nullptr);
    ASSERT_TRUE(grants.reconcile_reference(consumer, field, [&](uint32_t *id) {
      return repo.reference(consumer, field, id);
    }));
    ASSERT_TRUE(grants.upsert(
        source, destination,
        {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, resource, 99, 6000,
         SUPLA_SUPLAN_PERMISSION_READ}));
    int next_source = binding ? source : other_source;
    int next_destination = binding ? other_source : destination;
    for (unsigned i = 0; i < SUPLA_SUPLAN_MAX_ACL_ENTRIES; ++i) {
      auto channel = add(next_source, 100 + i, SUPLA_CHANNELTYPE_THERMOMETER,
                         SUPLA_CHANNELFNC_THERMOMETER);
      ASSERT_TRUE(grants.upsert(
          next_source, next_destination,
          {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, channel, 99, 7000 + i,
           SUPLA_SUPLAN_PERMISSION_READ}));
    }
    Association current, full;
    ASSERT_TRUE(repo.find(source, destination, &current));
    ASSERT_TRUE(repo.find(next_source, next_destination, &full));
    ASSERT_EQ(SUPLA_SUPLAN_MAX_ACL_ENTRIES, full.acl.size());
    // Cloud committed the replacement before its IPC/config delivery. A stale
    // Device request for the old resource is valid through a manual Grant.
    select(next);
    expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                  SUPLA_SUPLAN_ACCESS_STATUS_GRANTED);
    std::vector<uint64_t> ids;
    ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN,
                                channel_config_origin(consumer, field), &ids));
    EXPECT_TRUE(ids.empty());
    ASSERT_TRUE(repo.for_origin(99, 6000, &ids));
    ASSERT_EQ(1u, ids.size());
    EXPECT_EQ(current.id, ids[0]);
    Association after;
    ASSERT_TRUE(repo.find(source, destination, &after));
    EXPECT_TRUE(same_state(current, after));
    ASSERT_TRUE(repo.find(next_source, next_destination, &after));
    EXPECT_TRUE(same_state(full, after));
  }

  void binding_pressure(uint32_t *pump, uint32_t *heat) {
    auto add = [&](int device, int number, int type, int function) {
      auto id = add_channel(device, number, type, function);
      query("UPDATE supla_dev_channel SET user_id=99126 WHERE id=" +
            std::to_string(id));
      return id;
    };
    *pump = add(source, 237, SUPLA_CHANNELTYPE_RELAY,
                SUPLA_CHANNELFNC_PUMPSWITCH);
    *heat = add(other_source, 237, SUPLA_CHANNELTYPE_RELAY,
                SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH);
    PeerDao repo(99126);
    PeerService grants([] { return std::make_unique<PeerDao>(99126); },
                       nullptr);
    for (unsigned i = 0; i < SUPLA_SUPLAN_MAX_ACL_ENTRIES; ++i) {
      auto channel = add(destination, i, SUPLA_CHANNELTYPE_THERMOMETER,
                         SUPLA_CHANNELFNC_THERMOMETER);
      ASSERT_TRUE(grants.upsert(
          destination, other_source,
          {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, channel, 99, 4000 + i,
           SUPLA_SUPLAN_PERMISSION_READ}));
    }
    query("UPDATE supla_dev_channel SET user_config='{"
          "\"pumpSwitchChannelId\":" + std::to_string(*pump) +
          ",\"heatOrColdSourceSwitchChannelId\":" + std::to_string(*heat) +
          "}' WHERE id=" + std::to_string(hvac));
    ASSERT_TRUE(grants.reconcile_reference(hvac, 5, [&](uint32_t *id) {
      return repo.reference(hvac, 5, id);
    }));
    // This separate relation really cannot fit its desired ACL.
    ASSERT_FALSE(grants.reconcile_reference(hvac, 6, [&](uint32_t *id) {
      return repo.reference(hvac, 6, id);
    }));
  }

  void delete_binding_with_pressure(bool cloud_cleared) {
    register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
    ASSERT_FALSE(HasFatalFailure());
    uint32_t pump = 0, heat = 0;
    binding_pressure(&pump, &heat);
    ASSERT_FALSE(HasFatalFailure());
    PeerDao repo(99126);
    PeerService grants([] { return std::make_unique<PeerDao>(99126); },
                       nullptr);
    // An independent overlapping Grant survives binding revocation.
    ASSERT_TRUE(grants.upsert(
        destination, source,
        {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, hvac, 99, 3000,
         SUPLA_SUPLAN_PERMISSION_READ}));
    ASSERT_TRUE(grants.upsert(
        source, destination,
        {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, pump, 99, 3001,
         SUPLA_SUPLAN_PERMISSION_READ}));
    Association unrelated;
    ASSERT_TRUE(repo.find(destination, other_source, &unrelated));
    if (cloud_cleared) {
      query("UPDATE supla_dev_channel SET user_config=JSON_SET(user_config,"
            "'$.pumpSwitchChannelId',NULL) WHERE id=" + std::to_string(hvac));
      query("DELETE FROM supla_dev_channel WHERE id=" + std::to_string(pump));
    }
    testing::IpcSocketAdapterMock socket(-1);
    EXPECT_CALL(socket, send_data(std::string("OK:99126\n"))).Times(1);
    supla_on_channel_deleted_command command(&socket);
    std::string input = "USER-ON-CHANNEL-DELETED:99126," +
                        std::to_string(source) + "," + std::to_string(pump) +
                        "\n";
    std::vector<char> buffer(input.begin(), input.end());
    buffer.push_back(0);
    ASSERT_TRUE(command.process_command(buffer.data(), buffer.size(),
                                        input.size()));
    std::vector<uint64_t> ids;
    ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN,
                                channel_config_origin(hvac, 5), &ids));
    EXPECT_TRUE(ids.empty());
    ASSERT_TRUE(repo.for_resource(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, pump,
                                  &ids));
    EXPECT_TRUE(ids.empty());
    ASSERT_TRUE(repo.for_origin(99, 3000, &ids));
    ASSERT_EQ(1u, ids.size());
    ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN,
                                channel_config_origin(hvac, 6), &ids));
    EXPECT_TRUE(ids.empty());
    Association after;
    ASSERT_TRUE(repo.find(destination, other_source, &after));
    EXPECT_TRUE(same_state(unrelated, after));
    EXPECT_EQ(0, scalar("SELECT IFNULL(JSON_VALUE(user_config,"
                        "'$.pumpSwitchChannelId'),0) FROM supla_dev_channel "
                        "WHERE id=" + std::to_string(hvac)));
  }

  void coherent_projection(bool function_only);

  void paused_get_then_newer_ipc(bool container) {
    register_family(container ? SUPLA_CHANNELTYPE_CONTAINER
                              : SUPLA_CHANNELTYPE_VALVE_OPENCLOSE,
                    container ? SUPLA_CHANNELFNC_CONTAINER
                              : SUPLA_CHANNELFNC_VALVE_OPENCLOSE);
    ASSERT_FALSE(HasFatalFailure());
    auto save = [&](bool newer) {
      std::string json =
          container ? (newer ? "{\"warningAboveLevel\":22}"
                             : "{\"warningAboveLevel\":16}")
                    : (newer ? "{\"closeValveOnFloodType\":\"ON_CHANGE\"}"
                             : "{\"closeValveOnFloodType\":\"ALWAYS\"}");
      query("UPDATE supla_dev_channel SET user_config='" + json +
            "' WHERE id=" + std::to_string(hvac));
    };
    save(false);
    auto paused = publication_adapter->prepared.get_future();
    publication_adapter->pause_get = true;
    TDS_GetChannelConfigRequest request = {};
    request.ChannelNumber = 240;
    request.ConfigType = SUPLA_CONFIG_TYPE_DEFAULT;
    send_packet(SUPLA_DS_CALL_GET_CHANNEL_CONFIG, &request, sizeof(request));
    auto status = paused.wait_for(std::chrono::seconds(2));
    if (status != std::future_status::ready) {
      publication_adapter->release.set_value();
      FAIL() << "GET did not reach the real SRPC enqueue boundary";
    }
    save(true);
    // This is the real IPC producer while the Device thread is paused.
    // It must only queue an intent and must not publish/replace Device cache.
    notify(CONFIG_CHANGE_SCOPE_JSON_DEFAULT);
    publication_adapter->release.set_value();
    TSuplaDataPacket packet = {};
    ASSERT_TRUE(receive_packet(&packet));
    EXPECT_EQ(SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT, packet.call_id);
    ASSERT_TRUE(receive_packet(&packet));
    ASSERT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packet.call_id);
    TSDS_SetChannelConfig latest = {};
    memcpy(&latest, packet.data, packet.data_size);
    ASSERT_EQ(SUPLA_CONFIG_TYPE_DEFAULT, latest.ConfigType);
    if (container) {
      TChannelConfig_Container cfg = {};
      ASSERT_EQ(sizeof(cfg), latest.ConfigSize);
      memcpy(&cfg, latest.Config, sizeof(cfg));
      EXPECT_EQ(23, cfg.WarningAboveLevel);
    } else {
      TChannelConfig_Valve cfg = {};
      ASSERT_EQ(sizeof(cfg), latest.ConfigSize);
      memcpy(&cfg, latest.Config, sizeof(cfg));
      EXPECT_EQ(SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ON_CHANGE,
                cfg.CloseValveOnFloodType);
    }
    acknowledge(latest);
    EXPECT_FALSE(receive_packet(&packet, 200));
    // A later GET proves the owner's publication cache did not regress.
    send_packet(SUPLA_DS_CALL_GET_CHANNEL_CONFIG, &request, sizeof(request));
    ASSERT_TRUE(receive_packet(&packet));
    ASSERT_EQ(SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT, packet.call_id);
    TSD_ChannelConfig current = {};
    memcpy(&current, packet.data, packet.data_size);
    if (container) {
      EXPECT_EQ(23, reinterpret_cast<TChannelConfig_Container *>(current.Config)
                        ->WarningAboveLevel);
    } else {
      EXPECT_EQ(SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ON_CHANGE,
                reinterpret_cast<TChannelConfig_Valve *>(current.Config)
                    ->CloseValveOnFloodType);
    }
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

TEST_F(SupLanHvacWireIntegrationTest,
       ContainerPausedGetCannotPublishStaleLast) {
  paused_get_then_newer_ipc(true);
}
TEST_F(SupLanHvacWireIntegrationTest, ValvePausedGetCannotPublishStaleLast) {
  paused_get_then_newer_ipc(false);
}
TEST_F(SupLanHvacWireIntegrationTest,
       RelayPausedWeeklyGetThenClientAndIpcPublishLatest) {
  register_family(SUPLA_CHANNELTYPE_RELAY, SUPLA_CHANNELFNC_LIGHTSWITCH, true);
  ASSERT_FALSE(HasFatalFailure());
  TChannelConfig_WeeklySchedule old = {};
  old.Quarters[0] = 0x11;
  old.Program[0].Mode = SUPLA_RELAY_MODE_START_ON;
  weekly_schedule_config stored;
  stored.set_config(&old, SUPLA_CHANNELFNC_LIGHTSWITCH);
  supla_mariadb_access_provider db;
  supla_device_dao dao(&db);
  ASSERT_TRUE(dao.set_channel_config(99126, hvac, &stored));
  auto paused = publication_adapter->prepared.get_future();
  publication_adapter->pause_get = true;
  TDS_GetChannelConfigRequest get = {};
  get.ChannelNumber = 240;
  get.ConfigType = SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE;
  send_packet(SUPLA_DS_CALL_GET_CHANNEL_CONFIG, &get, sizeof(get));
  if (paused.wait_for(std::chrono::seconds(2)) != std::future_status::ready) {
    publication_adapter->release.set_value();
    FAIL() << "weekly GET did not pause";
  }
  auto api = std::make_shared<M4ConfigClient>(connection,
                                              supla_user::find(99126, false));
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  ASSERT_TRUE(api->get_channels()->add(new supla_client_channel(
      api->get_channels(), hvac, 240, destination, 0, SUPLA_CHANNELTYPE_RELAY,
      SUPLA_CHANNELFNC_LIGHTSWITCH, 0, 0, 0, 0, nullptr, nullptr, nullptr, "",
      0, 0, 0, 0, 29, SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE, value, 0, "{}", "{}",
      false)));
  TSCS_ChannelConfig request = {};
  request.ChannelId = hvac;
  request.ConfigType = SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE;
  auto newer = old;
  newer.Quarters[0] = 0x22;
  request.ConfigSize = sizeof(newer);
  memcpy(request.Config, &newer, sizeof(newer));
  TsrpcReceivedData rd = {};
  rd.data.scs_channel_config = &request;
  testing::SrpcAdapterMock reply;
  EXPECT_CALL(reply, sc_async_channel_config_update_or_result(testing::_))
      .WillOnce([](TSC_ChannelConfigUpdateOrResult *r) {
        EXPECT_EQ(SUPLA_CONFIG_RESULT_TRUE, r->Result);
        return 1;
      });
  EXPECT_TRUE(api->get_srpc_call_handler_collection()->handle_call(
      api, &reply, &rd, SUPLA_CS_CALL_SET_CHANNEL_CONFIG, 29));
  // A later IPC writer races the same paused Device connection owner.
  auto latest = old;
  latest.Quarters[0] = 0x33;
  stored.set_config(&latest, SUPLA_CHANNELFNC_LIGHTSWITCH);
  ASSERT_TRUE(dao.set_channel_config(99126, hvac, &stored));
  notify(CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE);
  publication_adapter->release.set_value();
  TSuplaDataPacket packet = {};
  ASSERT_TRUE(receive_packet(&packet));
  EXPECT_EQ(SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT, packet.call_id);
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packet.call_id);
  TSDS_SetChannelConfig config = {};
  memcpy(&config, packet.data, packet.data_size);
  ASSERT_EQ(SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE, config.ConfigType);
  ASSERT_EQ(sizeof(latest), config.ConfigSize);
  memcpy(&latest, config.Config, sizeof(latest));
  EXPECT_EQ(0x33, latest.Quarters[0]);
  acknowledge(config);
  EXPECT_FALSE(receive_packet(&packet, 200));
}

TEST_F(SupLanHvacWireIntegrationTest,
       AccessIgnoresUnrelatedEndpointProjectionFailure) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  PeerDao repo(99126);
  PeerService grants([] { return std::make_unique<PeerDao>(99126); }, nullptr);
  // Fill the opposite-direction ACL through the real Grant service. Its
  // Container origin overlaps a manual Grant; replacing the sensor then
  // needs one entry beyond the protocol limit and must roll back.
  auto add = [&](int device, int number, int type, int function) {
    auto id = add_channel(device, number, type, function);
    query("UPDATE supla_dev_channel SET user_id=99126 WHERE id=" +
          std::to_string(id));
    return id;
  };
  auto consumer = add(source, 237, SUPLA_CHANNELTYPE_CONTAINER,
                      SUPLA_CHANNELFNC_CONTAINER);
  auto old_sensor = add(destination, 238, SUPLA_CHANNELTYPE_BINARYSENSOR,
                        SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR);
  auto new_sensor = add(destination, 239, SUPLA_CHANNELTYPE_BINARYSENSOR,
                        SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR);
  auto select = [&](uint32_t sensor) {
    query("UPDATE supla_dev_channel SET user_config='{\"sensors\":[{"
          "\"channelId\":" + std::to_string(sensor) +
          "}]}' WHERE id=" + std::to_string(consumer));
  };
  select(old_sensor);
  ASSERT_TRUE(grants.reconcile_reference(consumer, 32, [&](uint32_t *id) {
    return repo.reference(consumer, 32, id);
  }));
  for (unsigned i = 0; i < SUPLA_SUPLAN_MAX_ACL_ENTRIES; ++i) {
    auto channel = i ? add(destination, i, SUPLA_CHANNELTYPE_BINARYSENSOR,
                           SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR)
                     : old_sensor;
    ASSERT_TRUE(grants.upsert(
        destination, source,
        {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, channel, 99, 1000 + i,
         SUPLA_SUPLAN_PERMISSION_READ}));
  }
  Association before;
  ASSERT_TRUE(repo.find(destination, source, &before));
  ASSERT_EQ(SUPLA_SUPLAN_MAX_ACL_ENTRIES, before.acl.size());
  select(new_sensor);
  ASSERT_FALSE(grants.reconcile_reference(consumer, 32, [&](uint32_t *id) {
    return repo.reference(consumer, 32, id);
  }));
  ASSERT_TRUE(grants.upsert(
      source, destination,
      {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, resource, 99, 2000,
       SUPLA_SUPLAN_PERMISSION_READ}));
  expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                SUPLA_SUPLAN_ACCESS_STATUS_GRANTED);
  Association after;
  ASSERT_TRUE(repo.find(destination, source, &after));
  EXPECT_TRUE(same_state(before, after));
  std::vector<Grant> desired;
  ASSERT_TRUE(repo.grants(after.id, &desired));
  bool found = false;
  for (const auto &grant : desired) {
    if (grant.origin_type == CHANNEL_CONFIG_ORIGIN) {
      EXPECT_EQ(old_sensor, grant.resource_id);
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(SupLanHvacWireIntegrationTest,
       AccessRevokesMovedConsumerOriginWithoutExpandingAnotherResource) {
  access_moved_origin_with_pressure(false);
}
TEST_F(SupLanHvacWireIntegrationTest,
       AccessRevokesMovedBindingOriginWithoutExpandingAnotherRecipient) {
  access_moved_origin_with_pressure(true);
}

TEST_F(SupLanHvacWireIntegrationTest,
       AccessDerivesCurrentOriginAndRevokesStaleRelatedGrant) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  // Simulate the authoritative Cloud commit before its IPC notification.
  config(resource);
  expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                SUPLA_SUPLAN_ACCESS_STATUS_GRANTED);
  PeerDao repo(99126);
  std::vector<uint64_t> ids;
  ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN,
                              main_thermometer_origin(hvac), &ids));
  ASSERT_EQ(1u, ids.size());
  Association before;
  ASSERT_TRUE(repo.load(ids[0], &before));
  expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                SUPLA_SUPLAN_ACCESS_STATUS_GRANTED);
  Association after;
  ASSERT_TRUE(repo.load(ids[0], &after));
  EXPECT_TRUE(same_state(before, after));
  query("UPDATE supla_dev_channel SET func=0 WHERE id=" +
        std::to_string(resource));
  expect_access(resource, SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
                SUPLA_SUPLAN_ACCESS_STATUS_INVALID);
  ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN,
                              main_thermometer_origin(hvac), &ids));
  EXPECT_TRUE(ids.empty());
}

TEST_F(SupLanHvacWireIntegrationTest, AccessRevokesDeletedResourceBeforeReply) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  config(resource);
  expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                SUPLA_SUPLAN_ACCESS_STATUS_GRANTED);
  // Deletion precedes the normal business-reference/IPC cleanup.
  query("DELETE FROM supla_dev_channel WHERE id=" + std::to_string(resource));
  expect_access(resource, SUPLA_SUPLAN_RESULT_NOT_FOUND,
                SUPLA_SUPLAN_ACCESS_STATUS_INVALID);
  std::vector<uint64_t> ids;
  ASSERT_TRUE(PeerDao(99126).for_origin(CHANNEL_CONFIG_ORIGIN,
                                       main_thermometer_origin(hvac), &ids));
  EXPECT_TRUE(ids.empty());
}

TEST_F(SupLanHvacWireIntegrationTest,
       AccessDerivesSourceFirstOriginAndRevokesAfterUnbind) {
  register_family(SUPLA_CHANNELTYPE_RELAY, SUPLA_CHANNELFNC_PUMPSWITCH);
  ASSERT_FALSE(HasFatalFailure());
  query("UPDATE supla_dev_channel SET type=" +
        std::to_string(SUPLA_CHANNELTYPE_HVAC) + ",func=" +
        std::to_string(SUPLA_CHANNELFNC_HVAC_THERMOSTAT) +
        ",user_config='{\"pumpSwitchChannelId\":" + std::to_string(hvac) +
        "}' WHERE id=" + std::to_string(resource));
  expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                SUPLA_SUPLAN_ACCESS_STATUS_GRANTED);
  std::vector<uint64_t> ids;
  PeerDao repo(99126);
  ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN,
                              channel_config_origin(resource, 5), &ids));
  ASSERT_EQ(1u, ids.size());
  query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
        std::to_string(resource));
  expect_access(resource, SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
                SUPLA_SUPLAN_ACCESS_STATUS_INVALID);
  ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN,
                              channel_config_origin(resource, 5), &ids));
  EXPECT_TRUE(ids.empty());
}

TEST_F(SupLanHvacWireIntegrationTest,
       DeleteBindingIgnoresUnrelatedProjectionFailureBeforeCloudCleanup) {
  delete_binding_with_pressure(false);
}
TEST_F(SupLanHvacWireIntegrationTest,
       DeleteBindingIgnoresUnrelatedProjectionFailureAfterCloudCleanup) {
  delete_binding_with_pressure(true);
}
TEST_F(SupLanHvacWireIntegrationTest,
       ShareIgnoresOtherRelayProjectionFailure) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  uint32_t pump = 0, heat = 0;
  binding_pressure(&pump, &heat);
  ASSERT_FALSE(HasFatalFailure());
  PeerDao repo(99126);
  Association unrelated;
  ASSERT_TRUE(repo.find(destination, other_source, &unrelated));
  TDS_SuplaEnsureResourceShare request = {};
  request.SourceResource = {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, hvac};
  request.DestinationResource = {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, pump};
  request.Permissions = SUPLA_SUPLAN_PERMISSION_READ;
  send_packet(SUPLA_DS_CALL_ENSURE_SUPLAN_RESOURCE_SHARE, &request,
              sizeof(request));
  bool got = false;
  TSuplaDataPacket packet = {};
  for (unsigned n = 0; n < 10 && !got; ++n) {
    ASSERT_TRUE(receive_packet(&packet));
    if (packet.call_id != SUPLA_SD_CALL_ENSURE_SUPLAN_RESOURCE_SHARE_RESULT)
      continue;
    TSD_SuplaEnsureResourceShareResult result = {};
    ASSERT_EQ(sizeof(result), packet.data_size);
    memcpy(&result, packet.data, sizeof(result));
    EXPECT_EQ(SUPLA_SUPLAN_RESULT_OK, result.Result);
    EXPECT_EQ(source, result.DestinationDeviceId);
    got = true;
  }
  ASSERT_TRUE(got);
  Association after;
  ASSERT_TRUE(repo.find(destination, other_source, &after));
  EXPECT_TRUE(same_state(unrelated, after));
  std::vector<uint64_t> ids;
  ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN,
                              channel_config_origin(hvac, 6), &ids));
  EXPECT_TRUE(ids.empty());
}

TEST_F(SupLanHvacWireIntegrationTest,
       ShareRevokesMovedOriginWithoutExpandingUnrequestedPair) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  uint32_t pump = 0, heat = 0;
  binding_pressure(&pump, &heat);
  ASSERT_FALSE(HasFatalFailure());
  auto old_heat = add_channel(source, 236, SUPLA_CHANNELTYPE_RELAY,
                              SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH);
  query("UPDATE supla_dev_channel SET user_id=99126 WHERE id=" +
        std::to_string(old_heat));
  auto select_heat = [&](uint32_t id) {
    query("UPDATE supla_dev_channel SET user_config=JSON_SET(user_config,"
          "'$.heatOrColdSourceSwitchChannelId'," + std::to_string(id) +
          ") WHERE id=" + std::to_string(hvac));
  };
  PeerDao repo(99126);
  PeerService grants([] { return std::make_unique<PeerDao>(99126); }, nullptr);
  select_heat(old_heat);
  ASSERT_TRUE(grants.reconcile_reference(hvac, 6, [&](uint32_t *id) {
    return repo.reference(hvac, 6, id);
  }));
  Association before, unrelated;
  ASSERT_TRUE(repo.find(destination, source, &before));
  ASSERT_TRUE(repo.find(destination, other_source, &unrelated));
  // Cloud committed HOCS A -> B, but B has no capacity for the new resource.
  select_heat(heat);
  TDS_SuplaEnsureResourceShare request = {};
  request.SourceResource = {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, hvac};
  request.DestinationResource = {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, pump};
  request.Permissions = SUPLA_SUPLAN_PERMISSION_READ;
  send_packet(SUPLA_DS_CALL_ENSURE_SUPLAN_RESOURCE_SHARE, &request,
              sizeof(request));
  bool got = false;
  TSuplaDataPacket packet = {};
  for (unsigned n = 0; n < 10 && !got; ++n) {
    ASSERT_TRUE(receive_packet(&packet));
    if (packet.call_id != SUPLA_SD_CALL_ENSURE_SUPLAN_RESOURCE_SHARE_RESULT)
      continue;
    TSD_SuplaEnsureResourceShareResult result = {};
    ASSERT_EQ(sizeof(result), packet.data_size);
    memcpy(&result, packet.data, sizeof(result));
    EXPECT_EQ(SUPLA_SUPLAN_RESULT_OK, result.Result);
    EXPECT_EQ(source, result.DestinationDeviceId);
    got = true;
  }
  ASSERT_TRUE(got);
  std::vector<uint64_t> ids;
  ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN,
                              channel_config_origin(hvac, 5), &ids));
  EXPECT_EQ(1u, ids.size());
  ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN,
                              channel_config_origin(hvac, 6), &ids));
  EXPECT_TRUE(ids.empty());
  Association after;
  ASSERT_TRUE(repo.find(destination, source, &after));
  EXPECT_TRUE(same_state(before, after));
  ASSERT_TRUE(repo.find(destination, other_source, &after));
  EXPECT_TRUE(same_state(unrelated, after));
}

TEST_F(SupLanHvacWireIntegrationTest,
       FunctionChangeRevokesAllConsumersDespiteEarlierAuxCapacityFailure) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  auto add = [&](int device, int number, int type, int function) {
    auto id = add_channel(device, number, type, function);
    query("UPDATE supla_dev_channel SET user_id=99126 WHERE id=" +
          std::to_string(id));
    return id;
  };
  auto binary = add(source, 237, SUPLA_CHANNELTYPE_BINARYSENSOR,
                    SUPLA_CHANNELFNC_FLOOD_SENSOR);
  auto new_aux = add(source, 236, SUPLA_CHANNELTYPE_THERMOMETER,
                     SUPLA_CHANNELFNC_THERMOMETER);
  auto second = add(destination, 238, SUPLA_CHANNELTYPE_HVAC,
                    SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  auto select = [&](uint32_t consumer, uint32_t aux) {
    query("UPDATE supla_dev_channel SET user_config='{"
          "\"auxThermometerChannelId\":" + std::to_string(aux) +
          ",\"binarySensorChannelId\":" + std::to_string(binary) +
          "}' WHERE id=" + std::to_string(consumer));
  };
  select(hvac, resource);
  select(second, 0);
  PeerDao repo(99126);
  PeerService grants([] { return std::make_unique<PeerDao>(99126); }, nullptr);
  for (auto origin : {std::make_pair(hvac, 2), std::make_pair(hvac, 3),
                      std::make_pair(second, 3)}) {
    ASSERT_TRUE(grants.reconcile_reference(origin.first, origin.second,
                                           [&](uint32_t *id) {
      return repo.reference(origin.first, origin.second, id);
    }));
  }
  // Independent origins keep the full effective ACL after config revocation.
  for (unsigned i = 0; i < SUPLA_SUPLAN_MAX_ACL_ENTRIES; ++i) {
    uint32_t channel = i == 0 ? resource : binary;
    if (i > 1) {
      channel = add(source, 100 + i, SUPLA_CHANNELTYPE_THERMOMETER,
                    SUPLA_CHANNELFNC_THERMOMETER);
    }
    ASSERT_TRUE(grants.upsert(
        source, destination,
        {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, channel, 99, 5000 + i,
         SUPLA_SUPLAN_PERMISSION_READ}));
  }
  Association before;
  ASSERT_TRUE(repo.find(source, destination, &before));
  ASSERT_EQ(SUPLA_SUPLAN_MAX_ACL_ENTRIES, before.acl.size());
  select(hvac, new_aux);
  // The function change reaches Core before Cloud cleans up consumer refs.
  query("UPDATE supla_dev_channel SET func=0 WHERE id=" +
        std::to_string(binary));
  testing::IpcSocketAdapterMock socket(-1);
  EXPECT_CALL(socket, send_data(std::string("OK:99126\n"))).Times(1);
  supla_on_channel_config_changed_command command(&socket);
  std::string input = "USER-ON-CHANNEL-CONFIG-CHANGED:99126," +
      std::to_string(source) + "," + std::to_string(binary) + "," +
      std::to_string(SUPLA_CHANNELTYPE_BINARYSENSOR) + ",0," +
      std::to_string(CONFIG_CHANGE_SCOPE_FUNCTION) + "\n";
  std::vector<char> buffer(input.begin(), input.end());
  buffer.push_back(0);
  ASSERT_TRUE(command.process_command(buffer.data(), buffer.size(),
                                      input.size()));
  for (auto consumer : {hvac, second}) {
    std::vector<uint64_t> ids;
    ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN,
                                channel_config_origin(consumer, 3), &ids));
    EXPECT_TRUE(ids.empty()) << "consumer " << consumer;
  }
  std::vector<uint64_t> ids;
  ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN,
                              channel_config_origin(hvac, 2), &ids));
  EXPECT_TRUE(ids.empty());
  Association after;
  ASSERT_TRUE(repo.find(source, destination, &after));
  EXPECT_TRUE(same_state(before, after));
  std::vector<Grant> remaining;
  ASSERT_TRUE(repo.grants(after.id, &remaining));
  ASSERT_EQ(SUPLA_SUPLAN_MAX_ACL_ENTRIES, remaining.size());
  for (const auto &grant : remaining) EXPECT_EQ(99, grant.origin_type);
}

TEST_F(SupLanHvacWireIntegrationTest,
       SourceFirstShareDispatchUsesExactCurrentConfig) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  query("UPDATE supla_dev_channel SET type=" +
        std::to_string(SUPLA_CHANNELTYPE_RELAY) +
        ",func=" + std::to_string(SUPLA_CHANNELFNC_PUMPSWITCH) +
        " WHERE id=" + std::to_string(resource));
  query("UPDATE supla_dev_channel SET user_config='{\"pumpSwitchChannelId\":" +
        std::to_string(resource) + "}' WHERE id=" + std::to_string(hvac));
  TDS_SuplaEnsureResourceShare request = {};
  request.SourceResource = {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, hvac};
  request.DestinationResource = {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, resource};
  request.Permissions = SUPLA_SUPLAN_PERMISSION_READ;
  send_packet(SUPLA_DS_CALL_ENSURE_SUPLAN_RESOURCE_SHARE, &request,
              sizeof(request));
  bool got = false;
  TSuplaDataPacket packet = {};
  for (int n = 0; n < 8 && !got; ++n) {
    ASSERT_TRUE(receive_packet(&packet));
    if (packet.call_id == SUPLA_SD_CALL_ENSURE_SUPLAN_RESOURCE_SHARE_RESULT) {
      TSD_SuplaEnsureResourceShareResult result = {};
      memcpy(&result, packet.data, sizeof(result));
      EXPECT_EQ(SUPLA_SUPLAN_RESULT_OK, result.Result);
      EXPECT_EQ(source, result.DestinationDeviceId);
      got = true;
    }
  }
  ASSERT_TRUE(got);
  std::vector<uint64_t> origins;
  ASSERT_TRUE(PeerDao(99126).for_origin(
      CHANNEL_CONFIG_ORIGIN, channel_config_origin(hvac, 5), &origins));
  ASSERT_EQ(1u, origins.size());
  Association a;
  ASSERT_TRUE(PeerDao(99126).load(origins[0], &a));
  EXPECT_EQ(destination, a.source);
  EXPECT_EQ(source, a.destination);
  ASSERT_EQ(1u, a.acl.size());
  EXPECT_EQ(hvac, a.acl[0].ResourceId);
  EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_READ, a.acl[0].Permissions);
  // Authoritative empty config revokes without trusting a replayed SHARE.
  query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
        std::to_string(hvac));
  send_packet(SUPLA_DS_CALL_ENSURE_SUPLAN_RESOURCE_SHARE, &request,
              sizeof(request));
  got = false;
  for (int n = 0; n < 8 && !got; ++n) {
    ASSERT_TRUE(receive_packet(&packet));
    if (packet.call_id == SUPLA_SD_CALL_ENSURE_SUPLAN_RESOURCE_SHARE_RESULT) {
      TSD_SuplaEnsureResourceShareResult result = {};
      memcpy(&result, packet.data, sizeof(result));
      EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, result.Result);
      got = true;
    }
  }
  ASSERT_TRUE(got);
  ASSERT_TRUE(PeerDao(99126).for_origin(
      CHANNEL_CONFIG_ORIGIN, channel_config_origin(hvac, 5), &origins));
  EXPECT_TRUE(origins.empty());
}

void SupLanHvacWireIntegrationTest::coherent_projection(bool function_only) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  query(
      "UPDATE supla_dev_channel SET "
      "user_config='{\"mainThermometerChannelId\":" +
      std::to_string(resource) + "}' WHERE id=" + std::to_string(hvac));
  std::promise<void> locked, release;
  auto released = release.get_future().share();
  auto holder = std::async(std::launch::async, [&] {
    PeerService gate([] { return std::make_unique<PeerDao>(99126); }, nullptr);
    return gate.reconcile_reference(hvac, 6, [&](uint32_t *id) {
      locked.set_value();
      if (released.wait_for(std::chrono::seconds(5)) !=
          std::future_status::ready) {
        return false;
      }
      *id = 0;
      return true;
    });
  });
  auto ready = locked.get_future().wait_for(std::chrono::seconds(2));
  if (ready != std::future_status::ready) {
    release.set_value();
    holder.get();
    FAIL() << "origin gate not entered";
  }
  TDS_GetChannelConfigRequest get = {};
  get.ChannelNumber = 240;
  get.ConfigType = SUPLA_CONFIG_TYPE_DEFAULT;
  send_packet(SUPLA_DS_CALL_GET_CHANNEL_CONFIG, &get, sizeof(get));
  // Observe a real earlier field commit while the owner is blocked at field6.
  bool projected = false;
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (!projected && std::chrono::steady_clock::now() < deadline) {
    std::vector<uint64_t> ids;
    PeerDao(99126).for_origin(CHANNEL_CONFIG_ORIGIN,
                              channel_config_origin(hvac, 1), &ids);
    projected = !ids.empty();
    if (!projected) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }
  if (function_only) {
    query("UPDATE supla_dev_channel SET func=0 WHERE id=" +
          std::to_string(hvac));
  } else {
    query(
        "UPDATE supla_dev_channel SET "
        "user_config='{\"mainThermometerChannelId\":" +
        std::to_string(other_resource) + "}' WHERE id=" + std::to_string(hvac));
  }
  release.set_value();
  EXPECT_TRUE(holder.get());
  ASSERT_TRUE(projected);
  TSuplaDataPacket packet = {};
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT, packet.call_id);
  TSD_ChannelConfig config = {};
  memcpy(&config, packet.data, packet.data_size);
  TChannelConfig_HVAC payload = {};
  ASSERT_EQ(sizeof(payload), config.ConfigSize);
  memcpy(&payload, config.Config, sizeof(payload));
  EXPECT_EQ(function_only ? resource : other_resource,
            payload.MainThermometerChannelId);
  EXPECT_EQ(function_only ? 0 : SUPLA_CHANNELFNC_HVAC_THERMOSTAT, config.Func);
  std::vector<uint64_t> ids;
  ASSERT_TRUE(PeerDao(99126).for_origin(CHANNEL_CONFIG_ORIGIN,
                                        channel_config_origin(hvac, 1), &ids));
  if (function_only) {
    EXPECT_TRUE(ids.empty());
    return;
  }
  ASSERT_EQ(1u, ids.size());
  std::vector<Grant> grants;
  ASSERT_TRUE(PeerDao(99126).grants(ids[0], &grants));
  ASSERT_EQ(1u, grants.size());
  EXPECT_EQ(other_resource, grants[0].resource_id);
}

TEST_F(SupLanHvacWireIntegrationTest,
       MultiFieldProjectionCannotDeliverMixedAuthority) {
  coherent_projection(false);
}
TEST_F(SupLanHvacWireIntegrationTest,
       FunctionOnlyChangeDuringProjectionRevokesAlreadyCommittedOrigins) {
  coherent_projection(true);
}

TEST_F(SupLanHvacWireIntegrationTest,
       RetainedDeviceDefaultCannotOverwriteSelectedNoneOnRegistration) {
  query("UPDATE supla_dev_channel SET func=0 WHERE id=" + std::to_string(hvac));
  register_existing_family(SUPLA_CHANNELTYPE_HVAC,
                            SUPLA_CHANNELFNC_HVAC_THERMOSTAT, 0);
  ASSERT_FALSE(HasFatalFailure());
  EXPECT_EQ(0, scalar("SELECT func FROM supla_dev_channel WHERE id=" +
                      std::to_string(hvac)));
  TDS_GetChannelConfigRequest request = {};
  request.ChannelNumber = 240;
  request.ConfigType = SUPLA_CONFIG_TYPE_DEFAULT;
  send_packet(SUPLA_DS_CALL_GET_CHANNEL_CONFIG, &request, sizeof(request));
  TSuplaDataPacket packet = {};
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT, packet.call_id);
  TSD_ChannelConfig current = {};
  memcpy(&current, packet.data, packet.data_size);
  EXPECT_EQ(0, current.Func);
}

TEST_F(SupLanHvacWireIntegrationTest,
       PausedOldGetCannotWinAfterAuthoritativeFunctionChangesToNone) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  auto paused = publication_adapter->prepared.get_future();
  publication_adapter->pause_get = true;
  TDS_GetChannelConfigRequest request = {};
  request.ChannelNumber = 240;
  request.ConfigType = SUPLA_CONFIG_TYPE_DEFAULT;
  send_packet(SUPLA_DS_CALL_GET_CHANNEL_CONFIG, &request, sizeof(request));
  if (paused.wait_for(std::chrono::seconds(2)) != std::future_status::ready) {
    publication_adapter->release.set_value();
    FAIL() << "GET did not reach the SRPC enqueue boundary";
  }
  query("UPDATE supla_dev_channel SET func=0 WHERE id=" + std::to_string(hvac));
  notify(CONFIG_CHANGE_SCOPE_JSON_DEFAULT);
  publication_adapter->release.set_value();
  TSuplaDataPacket packet = {};
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT, packet.call_id);
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packet.call_id);
  TSDS_SetChannelConfig latest = {};
  memcpy(&latest, packet.data, packet.data_size);
  EXPECT_EQ(0, latest.Func);
  acknowledge(latest);
  // A Device upload advertises its retained function, but cannot change it.
  latest.Func = SUPLA_CHANNELFNC_HVAC_THERMOSTAT;
  send_packet(SUPLA_DS_CALL_SET_CHANNEL_CONFIG, &latest,
              sizeof(latest) - sizeof(latest.Config) + latest.ConfigSize);
  bool authoritative = false;
  for (unsigned i = 0; i < 5 && !authoritative; ++i) {
    ASSERT_TRUE(receive_packet(&packet));
    if (packet.call_id == SUPLA_SD_CALL_SET_CHANNEL_CONFIG) {
      memcpy(&latest, packet.data, packet.data_size);
      EXPECT_EQ(0, latest.Func);
      acknowledge(latest);
      authoritative = true;
    }
  }
  ASSERT_TRUE(authoritative);
  EXPECT_EQ(0, scalar("SELECT func FROM supla_dev_channel WHERE id=" +
                      std::to_string(hvac)));
  send_packet(SUPLA_DS_CALL_GET_CHANNEL_CONFIG, &request, sizeof(request));
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT, packet.call_id);
  TSD_ChannelConfig current = {};
  memcpy(&current, packet.data, packet.data_size);
  EXPECT_EQ(0, current.Func);
}

TEST_F(SupLanHvacWireIntegrationTest,
       FunctionIpcReconnectReplaysLatestSelectedFunctionIncludingNone) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  for (int selected : {0, SUPLA_CHANNELFNC_HVAC_THERMOSTAT}) {
    query("UPDATE supla_dev_channel SET func=" + std::to_string(selected) +
          " WHERE id=" + std::to_string(hvac));
    publication_function = selected;
    notify(CONFIG_CHANGE_SCOPE_FUNCTION);
    // Preserve the established targeted reconnect path for function changes.
    TSuplaDataPacket packet = {};
    EXPECT_FALSE(receive_packet(&packet));
    close_connection();
    start_connection();
    ASSERT_FALSE(HasFatalFailure());
    register_existing_family(SUPLA_CHANNELTYPE_HVAC,
                              SUPLA_CHANNELFNC_HVAC_THERMOSTAT, selected);
    ASSERT_FALSE(HasFatalFailure());
    EXPECT_EQ(selected, scalar("SELECT func FROM supla_dev_channel WHERE id=" +
                               std::to_string(hvac)));
  }
}

}  // namespace
