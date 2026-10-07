// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "suplan/peer_dao.h"
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
}  // namespace
