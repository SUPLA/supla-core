// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "ipc/on_channel_deleted_command.h"
#include "ipc/on_device_deleted_command.h"
#include "suplan/peer_dao.h"
#include "suplan/server_peer_transport.h"
#include "test/doubles/ipc/IpcSocketAdapterMock.h"
#include "test/doubles/suplan/PeerProvisionerTestAccess.h"
#include "user.h"
#include "test/integration/IntegrationTest.h"
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
