// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include <arpa/inet.h>
#include <dlfcn.h>
#include <gtest/gtest.h>
#include <poll.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <future>
#include <iostream>
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
#include "user/user_dao.h"
// Test-only driver boundary probe: always execute real MariaDB operations.
// No SQL text/payload logging, production hook, schema change or provider mock.
namespace {
struct Sql004Probe;
thread_local Sql004Probe *sql004 = nullptr;
struct Sql004Probe {
  unsigned connections = 0, prepares = 0, executions = 0;
  unsigned selects = 0, dml = 0, config_reads = 0, config_cas = 0;
  MYSQL_STMT *last = nullptr, *config_statement = nullptr;
  std::string statement;
  std::function<bool(unsigned)> after_config;
  std::chrono::steady_clock::time_point started =
      std::chrono::steady_clock::now();
  Sql004Probe() { sql004 = this; }
  ~Sql004Probe() { sql004 = nullptr; }
  void count(const std::string &sql) {
    ++executions;
    if (sql.compare(0, 6, "SELECT") == 0) ++selects;
    if (sql.compare(0, 6, "INSERT") == 0 || sql.compare(0, 6, "UPDATE") == 0 ||
        sql.compare(0, 6, "DELETE") == 0) ++dml;
  }
  void finish(const char *label) {
    sql004 = nullptr;
    std::cout << "SQL004 " << label << " config_reads=" << config_reads
              << " selects=" << selects << " dml=" << dml
              << " prepares=" << prepares << " executions=" << executions
              << " connect_attempts=" << connections << " us="
              << std::chrono::duration_cast<std::chrono::microseconds>(
                     std::chrono::steady_clock::now() - started).count()
              << std::endl;
  }
};
template <typename T> T sql004_native(const char *name) {
  auto function = reinterpret_cast<T>(dlsym(RTLD_NEXT, name));
  if (!function) std::abort();
  return function;
}
}  // namespace
extern "C" MYSQL *STDCALL mysql_real_connect(
    MYSQL *mysql, const char *host, const char *user, const char *password,
    const char *database, unsigned int port, const char *socket,
    unsigned long flags) {
  static auto native = sql004_native<decltype(&mysql_real_connect)>(
      "mysql_real_connect");
  if (sql004) ++sql004->connections;
  return native(mysql, host, user, password, database, port, socket, flags);
}
extern "C" int STDCALL mysql_query(MYSQL *mysql, const char *query) {
  static auto native = sql004_native<decltype(&mysql_query)>("mysql_query");
  if (sql004) sql004->count(query);
  return native(mysql, query);
}
extern "C" int STDCALL mysql_stmt_prepare(MYSQL_STMT *stmt, const char *query,
                                           unsigned long length) {
  static auto native = sql004_native<decltype(&mysql_stmt_prepare)>(
      "mysql_stmt_prepare");
  if (sql004) {
    ++sql004->prepares;
    sql004->last = stmt;
    sql004->statement.assign(query, length);
    if (sql004->statement.find("SELECT user_config, properties, MD5(") == 0)
      sql004->config_statement = stmt;
  }
  return native(stmt, query, length);
}
extern "C" int STDCALL mysql_stmt_execute(MYSQL_STMT *stmt) {
  static auto native = sql004_native<decltype(&mysql_stmt_execute)>(
      "mysql_stmt_execute");
  if (sql004) {
    sql004->count(stmt == sql004->last ? sql004->statement : "");
    if (stmt == sql004->config_statement) ++sql004->config_reads;
    if (stmt == sql004->last && sql004->statement ==
        "CALL `supla_set_channel_json_config`(?,?,?,?,?,?)")
      ++sql004->config_cas;
  }
  return native(stmt);
}
extern "C" int STDCALL mysql_stmt_fetch(MYSQL_STMT *stmt) {
  static auto native = sql004_native<decltype(&mysql_stmt_fetch)>(
      "mysql_stmt_fetch");
  int result = native(stmt);
  if (!result && sql004 && stmt == sql004->config_statement) {
    auto probe = sql004;
    probe->config_statement = nullptr;
    sql004 = nullptr;  // A simulated external writer is outside measured work.
    bool fail = probe->after_config && probe->after_config(probe->config_reads);
    sql004 = probe;
    if (fail) return 1;
  }
  return result;
}
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
  static std::weak_ptr<TEventHandler> event(supla_connection *connection) {
    return connection->eh;
  }
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
  void manual_schema() {
    runSqlScript("SupLanM5TestSchema.sql");
    std::string out;
    sqlQuery(("UPDATE supla_iodevice SET flags=flags | " +
              std::to_string(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) +
              ",protocol_version=29 WHERE user_id=2").c_str(), &out);
    sqlQuery(("UPDATE supla_dev_channel SET type=" +
              std::to_string(SUPLA_CHANNELTYPE_RELAY) + ",func=" +
              std::to_string(SUPLA_CHANNELFNC_POWERSWITCH) +
              " WHERE id=" + std::to_string(resource)).c_str(), &out);
  }
  void stream_schema() {
    manual_schema();
    runSqlScript("SupLanM7TestSchema.sql");
    supla_mariadb_access_provider db;
    ASSERT_TRUE(db.connect());
    void *stmt = nullptr;
    int channel = 0;
    ASSERT_TRUE(db.stmt_get_int(&stmt, &channel, nullptr, nullptr, nullptr,
        ("SELECT MIN(id) FROM supla_dev_channel WHERE iodevice_id=" +
         std::to_string(destination)).c_str(), nullptr, 0, true));
    ASSERT_TRUE(factory()->accept_identity(destination,
                                           {456, {uint32_t(channel)}}));
  }
  TSD_SuplaEnsureResourceAccessResult stream_ensure(bool approval = false) {
    return service.ensure_stream(destination, resource, approval, nullptr,
                                 true);
  }
  std::vector<ApprovalDecision> decisions() {
    auto repo = factory();
    std::vector<ApprovalPair> pairs;
    EXPECT_TRUE(repo->approval_pairs(resource, destination, &pairs));
    std::vector<ApprovalDecision> rows;
    if (!pairs.empty()) {
      EXPECT_TRUE(repo->approval_lock(&pairs.front(), false));
      EXPECT_TRUE(repo->approval_decisions(pairs.front().id, &rows));
    }
    return rows;
  }
  TSD_SuplaEnsureResourceAccessResult manual_ensure(uint8_t permissions,
                                                   uint8_t flags = 1) {
    TDS_SuplaEnsureResourceAccess q = {
        {1, resource}, permissions, SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER, flags};
    return service.ensure_access(destination, q);
  }
  int decide(const ApprovalDecision &snapshot, const char *status) {
    // Cloud business transaction boundary, not a Grant/ACL fixture. Guard the
    // displayed snapshot and take the same pair lock used by the real service.
    std::string out;
    std::string id = std::to_string(snapshot.id);
    std::string query =
        "START TRANSACTION; SELECT pair_id INTO @pair FROM "
        "supla_resource_access_decision WHERE id=" + id +
        "; SELECT id INTO @locked FROM supla_resource_access_pair "
        "WHERE id=@pair FOR UPDATE; UPDATE supla_resource_access_decision "
        "SET status='" + status + "',open_pair_id=" +
        (std::string(status) == "REJECTED" ? "pair_id" : "NULL") +
        " WHERE id=" + id + " AND revision=" +
        std::to_string(snapshot.revision) + " AND permissions=" +
        std::to_string(snapshot.permissions) +
        " AND status<>'REMOVED'; SELECT ROW_COUNT() AS changed; COMMIT";
    sqlQuery(query.c_str(), &out);
    return out == "changed\n1\n" ? 1 : 0;
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
TEST_F(SupLanPeerDaoIntegrationTest, OwnershipVerdictSeparatesQueryFailure) {
  auto dao = factory();
  Acl acl{{SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, resource,
           SUPLA_SUPLAN_PERMISSION_READ}};
  bool owned = false;
  ASSERT_TRUE(dao->check_acl_ownership(source, destination, acl, &owned));
  ASSERT_TRUE(owned);
  PeerDao other_account(1);
  owned = true;
  EXPECT_TRUE(other_account.check_acl_ownership(source, destination, acl,
                                               &owned));
  EXPECT_FALSE(owned);
  // Fault only the disposable Core test DB, restore before any assertion
  // can abort the test and before the next fixture starts.
  supla_mariadb_access_provider db;
  ASSERT_TRUE(db.connect());
  int renamed =
      db.query("RENAME TABLE supla_dev_channel TO m6_owner_lookup", true);
  ASSERT_EQ(0, renamed);
  owned = true;
  bool queried = dao->check_acl_ownership(source, destination, acl, &owned);
  int restored =
      db.query("RENAME TABLE m6_owner_lookup TO supla_dev_channel", true);
  ASSERT_EQ(0, restored);
  EXPECT_FALSE(queried);
  EXPECT_FALSE(owned);
  EXPECT_TRUE(dao->check_acl_ownership(source, destination, acl, &owned));
  EXPECT_TRUE(owned);
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
  uint64_t session(int d) override { return d; }
  int recovery_device() override { return 0; }
  void wake(int, uint64_t) override {}
  bool reconnect_session(int, uint64_t) override { return false; }
  bool ready(int) override { return true; }
  bool owner_ready(int, uint32_t) override { return true; }
  SendResult send(int d, TSDS_SuplaSetSuplanSourceAssociation *q) override {
    sources.push_back(*q);
    return {SendCode::Queued, session(d)};
  }
  SendResult send(int d,
                  TSDS_SuplaSetSuplanDestinationAssociation *q) override {
    destinations.push_back(*q);
    return {SendCode::Queued, session(d)};
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
       DeprecatedThermometerCannotAuthorizeRemoteOrNewLocalHvacReferences) {
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
      unsigned int unset = flags ? 0 : channel->get_channel_number();
      EXPECT_EQ(unset, received.MainThermometerChannelId);
      EXPECT_EQ(unset, received.AuxThermometerChannelId);
    }, flags);
    EXPECT_EQ(0, scalar("SELECT JSON_VALUE(user_config,"
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
       SharedDatabaseVersionCheckRequiresM7Migration) {
  EXPECT_STREQ("20261013215600", DB_VERSION);
  query("DELETE FROM migration_versions");
  query("INSERT INTO migration_versions(version) "
        "VALUES('Version20261013215600')");
  database db;
  EXPECT_TRUE(db.check_db_version(DB_VERSION, 0));
  query("UPDATE migration_versions SET version='Version20261011120000'");
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

  // Reuse the existing wire fixture for a second real D2S connection.
  class OtherConnection {
    SupLanHvacWireIntegrationTest *fixture;
    void *listener, *worker, *wire;
    int client;
    supla_connection *connection;
    PublicationPauseAdapter *adapter;
    bool other_selected = true;
    bool closed = false;

   public:
    explicit OtherConnection(SupLanHvacWireIntegrationTest *fixture)
        : fixture(fixture),
          listener(fixture->listener),
          worker(fixture->worker),
          wire(fixture->wire),
          client(fixture->client),
          connection(fixture->connection),
          adapter(fixture->publication_adapter) {
      fixture->listener = fixture->worker = fixture->wire = nullptr;
      fixture->client = -1;
      fixture->connection = nullptr;
      fixture->publication_adapter = nullptr;
      fixture->start_connection();
    }
    void select(bool other) {
      if (other == other_selected) return;
      std::swap(listener, fixture->listener);
      std::swap(worker, fixture->worker);
      std::swap(wire, fixture->wire);
      std::swap(client, fixture->client);
      std::swap(connection, fixture->connection);
      std::swap(adapter, fixture->publication_adapter);
      other_selected = other;
    }
    void close() {
      if (closed) return;
      select(true);
      fixture->close_connection();
      select(false);
      closed = true;
    }
    ~OtherConnection() { close(); }
  };

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
                                int selected_function, bool weekly = false,
                                bool suplan = true,
                                std::function<void(const TSuplaDataPacket &)>
                                    observe = {}) {
    TDS_SuplaRegisterDevice_G registration = {};
    snprintf(registration.Email, sizeof(registration.Email), "%s",
             "hvac-wire@test.invalid");
    memset(registration.GUID, 0xab, sizeof(registration.GUID));
    registration.AuthKey[0] = 1;
    registration.Flags = (suplan ? SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED : 0) |
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
      if (observe) observe(packet);
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

  void expect_access(uint32_t channel, uint8_t result, uint8_t status,
                     uint8_t flags = 0, uint8_t permissions = 1,
                     uint8_t delivery = SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER) {
    TDS_SuplaEnsureResourceAccess request = {
        {SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, channel},
        permissions, delivery, flags};
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

TEST_F(SupLanHvacWireIntegrationTest,
       M6StreamWakeHandleRacesConnectionDestruction) {
  auto signal = connection->event_notifier();
  auto original = supla_connection_test_access::event(connection);
  std::promise<void> started;
  std::thread source([&] {
    started.set_value();
    for (unsigned i = 0; i < 2000; ++i) signal();
  });
  started.get_future().wait();
  close_connection();
  source.join();
  EXPECT_TRUE(original.expired());
  start_connection();
  auto replacement = supla_connection_test_access::event(connection);
  EXPECT_FALSE(replacement.expired());
  EXPECT_TRUE(original.owner_before(replacement) ||
              replacement.owner_before(original));
  for (unsigned i = 0; i < 10; ++i) signal();
  // Retaining the old callback neither retains nor signals the new connection.
  EXPECT_TRUE(original.expired());
}

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

TEST_F(SupLanHvacWireIntegrationTest,
       AssociationBatchUsesRealSrpcWithoutAdmissionWindow) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  PeerDao repo(99126);
  PeerService previous([] { return std::make_unique<PeerDao>(99126); },
                       nullptr);
  uint32_t first_channel = 0;
  for (unsigned i = 0; i < 16; ++i) {
    // Historical registered, currently offline Sources in the isolated Core DB.
    query(
        "INSERT INTO supla_iodevice "
        "(location_id,user_id,guid,enabled,reg_date,protocol_version,flags) "
        "SELECT location_id,99126,UNHEX(MD5('m6-batch-" +
        std::to_string(i) + "')),1,reg_date,29," +
        std::to_string(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) +
        " FROM supla_iodevice WHERE id=" + std::to_string(source));
    int owner =
        scalar("SELECT MAX(id) FROM supla_iodevice WHERE user_id=99126");
    auto channel = add_channel(owner, 1, SUPLA_CHANNELTYPE_THERMOMETER,
                               SUPLA_CHANNELFNC_THERMOMETER);
    query("UPDATE supla_dev_channel SET user_id=99126 WHERE id=" +
          std::to_string(channel));
    if (!i) first_channel = channel;
    ASSERT_TRUE(repo.accept_identity(owner, {123 + i, {channel}}));
    ASSERT_TRUE(
        previous.upsert(owner, destination, {1, channel, 99, 7000 + i, 1}));
    Association a;
    ASSERT_TRUE(repo.find(owner, destination, &a));
    ASSERT_TRUE(previous.provisioned(a, 123 + i));
  }
  auto peers = supla_user::find(99126, false)->get_suplan_peers()->peers();
  peers->reconnect(destination);
  std::vector<TSDS_SuplaSetSuplanDestinationAssociation> queued;
  auto receive_association = [&] {
    TSuplaDataPacket packet{};
    ASSERT_TRUE(receive_packet(&packet));
    ASSERT_EQ(SUPLA_SD_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION, packet.call_id);
    ASSERT_LE(packet.data_size,
              sizeof(TSDS_SuplaSetSuplanDestinationAssociation));
    TSDS_SuplaSetSuplanDestinationAssociation q{};
    memcpy(&q, packet.data, packet.data_size);
    EXPECT_EQ(0, q.PeerKeySize);
    queued.push_back(q);
  };
  for (unsigned i = 0; i < 16; ++i) receive_association();
  ASSERT_FALSE(HasFatalFailure());
  TSuplaDataPacket extra{};
  EXPECT_FALSE(receive_packet(&extra, 100));  // One request per association.
  for (unsigned i = 0; i < 16; ++i) {
    ASSERT_GT(queued.size(), i);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto q = queued[i];
    TDS_SuplaSetSuplanDestinationAssociationResult ok = {
        q.PeerContext, q.AclRevision, SUPLA_SUPLAN_RESULT_OK};
    send_packet(SUPLA_DS_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION_RESULT, &ok,
                sizeof(ok));
  }
  auto until = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (PeerProvisionerTestAccess::count(*peers) &&
         std::chrono::steady_clock::now() < until)
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(*peers));
  auto target = supla_user::find(99126, false)->get_devices()->get(destination);
  ASSERT_NE(nullptr, target);
  EXPECT_EQ(120000000u, target->wait_time_usec());
  EXPECT_GT(first_channel, 0u);
  EXPECT_FALSE(receive_packet(&extra, 100));
}

TEST_F(SupLanHvacWireIntegrationTest,
       QuietConnectionWatchdogReconnectsAndReplaysDraining) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  PeerDao repo(99126);
  ASSERT_TRUE(repo.accept_identity(source, {123, {resource}}));
  // Previously accepted Source context, reconstructed from authoritative DB.
  // Source is now offline. The real Destination socket stays connected.
  PeerService previous([] { return std::make_unique<PeerDao>(99126); },
                       nullptr);
  ASSERT_TRUE(previous.upsert(source, destination, {1, resource, 99, 6001, 1}));
  Association a;
  ASSERT_TRUE(repo.find(source, destination, &a));
  ASSERT_TRUE(previous.provisioned(a, 123));
  auto user = supla_user::find(99126, false);
  ASSERT_NE(nullptr, user);
  auto peers = user->get_suplan_peers()->peers();
  peers->reconcile(a.id);
  std::vector<TSDS_SuplaSetSuplanDestinationAssociation> registration_replays;
  auto observe = [&](const TSuplaDataPacket &packet) {
    if (packet.call_id != SUPLA_SD_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION)
      return;
    ASSERT_LE(packet.data_size,
              sizeof(TSDS_SuplaSetSuplanDestinationAssociation));
    TSDS_SuplaSetSuplanDestinationAssociation replay{};
    memcpy(&replay, packet.data, packet.data_size);
    registration_replays.push_back(replay);
  };
  auto receive_association = [&] {
    if (!registration_replays.empty()) {
      auto replay = registration_replays.front();
      registration_replays.erase(registration_replays.begin());
      return replay;
    }
    TSuplaDataPacket packet = {};
    TSDS_SuplaSetSuplanDestinationAssociation q = {};
    bool received = receive_packet(&packet, 25000);
    EXPECT_TRUE(received);
    if (received) {
      EXPECT_EQ(SUPLA_SD_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION,
                packet.call_id);
      EXPECT_LE(packet.data_size, sizeof(q));
      if (packet.data_size <= sizeof(q))
        memcpy(&q, packet.data, packet.data_size);
    }
    return q;
  };
  auto first = receive_association();
  ASSERT_FALSE(HasFailure());
  ASSERT_EQ(1, first.ResourceCount);
  ASSERT_EQ(resource, first.Resources[0].ResourceId);
  ASSERT_EQ(0, first.PeerKeySize);
  auto wait_for_watchdog = [&] {
    auto started = std::chrono::steady_clock::now();
    TSuplaDataPacket packet{};
    EXPECT_FALSE(
        receive_packet(&packet, 19000));  // EOF, never an identical retry.
    char byte;
    EXPECT_EQ(0, recv(client, &byte, 1, MSG_PEEK | MSG_DONTWAIT));
    auto elapsed = std::chrono::steady_clock::now() - started;
    EXPECT_GE(elapsed, std::chrono::seconds(13));
    EXPECT_LT(elapsed, std::chrono::seconds(19));
  };
  // No packet/ping drives the deadline; the own connection wait does.
  wait_for_watchdog();
  close_connection();
  start_connection();
  register_existing_family(SUPLA_CHANNELTYPE_HVAC,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT, false, true,
                           observe);
  auto replay = receive_association();
  ASSERT_FALSE(HasFailure());
  EXPECT_EQ(0, memcmp(&first, &replay, sizeof(first)));
  TDS_SuplaSetSuplanDestinationAssociationResult ok = {
      replay.PeerContext, replay.AclRevision, SUPLA_SUPLAN_RESULT_OK};
  // A matched result during a real DAO read outage must cancel the Device
  // watchdog, retain unverified work and recover on its own 5-second deadline.
  query("RENAME TABLE supla_suplan_peer_association TO m6_unavailable_peer");
  send_packet(SUPLA_DS_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION_RESULT,
              &ok, sizeof(ok));
  bool db_retry = false;
  auto until = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (std::chrono::steady_clock::now() < until) {
    auto left = peers->time_left_usec(destination);
    if (left && left <= 5000000u) {
      db_retry = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  // Restore even when the assertion below fails; this is the isolated test DB.
  query("RENAME TABLE m6_unavailable_peer TO supla_suplan_peer_association");
  ASSERT_TRUE(db_retry);
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(*peers));
  replay = receive_association();  // No new ENSURE, ping or reconnect.
  ASSERT_FALSE(HasFailure());
  EXPECT_EQ(0, memcmp(&first, &replay, sizeof(first)));
  send_packet(SUPLA_DS_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION_RESULT,
              &ok, sizeof(ok));
  expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                SUPLA_SUPLAN_ACCESS_STATUS_GRANTED);
  ASSERT_TRUE(peers->grants()->delete_origin(99, 6001));
  first = receive_association();
  ASSERT_FALSE(HasFailure());
  EXPECT_EQ(0, first.ResourceCount);
  wait_for_watchdog();
  close_connection();
  start_connection();
  register_existing_family(SUPLA_CHANNELTYPE_HVAC,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT, false, true,
                           observe);
  replay = receive_association();
  ASSERT_FALSE(HasFailure());
  EXPECT_EQ(0, replay.ResourceCount);
  ok = {replay.PeerContext, replay.AclRevision, SUPLA_SUPLAN_RESULT_OK};
  send_packet(SUPLA_DS_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION_RESULT,
              &ok, sizeof(ok));
  expect_access(resource, SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
                SUPLA_SUPLAN_ACCESS_STATUS_INVALID);
  ASSERT_TRUE(repo.load(a.id, &a));
  EXPECT_EQ(Lifecycle::Draining, a.lifecycle);
  EXPECT_TRUE(a.destination_acked);
  EXPECT_FALSE(a.source_acked);
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
       EnsureConfigOriginFailureDoesNotStarveLaterRevocation) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  PeerDao repo(99126);
  PeerService grants([] { return std::make_unique<PeerDao>(99126); }, nullptr);
  const auto main_origin = channel_config_origin(hvac, 1);
  const auto aux_origin = channel_config_origin(hvac, 2);
  query("UPDATE supla_dev_channel SET user_config='{"
        "\"auxThermometerChannelId\":" + std::to_string(resource) +
        "}' WHERE id=" + std::to_string(hvac));
  ASSERT_TRUE(grants.reconcile_reference(hvac, 2, [&](uint32_t *id) {
    return repo.reference(hvac, 2, id);
  }));
  const auto independent = add_channel(source, 240,
      SUPLA_CHANNELTYPE_THERMOMETER, SUPLA_CHANNELFNC_THERMOMETER);
  query("UPDATE supla_dev_channel SET user_id=99126 WHERE id=" +
        std::to_string(independent));
  ASSERT_TRUE(repo.accept_identity(source, {123, {resource, independent}}));
  ASSERT_TRUE(grants.upsert(source, destination,
                            {1, independent, 99, 9001, 1}));
  ASSERT_TRUE(grants.upsert(source, other_source, {1, resource, 99, 9002, 1}));
  Association before, unrelated;
  ASSERT_TRUE(repo.find(source, destination, &before));
  ASSERT_TRUE(repo.find(source, other_source, &unrelated));
  // Current config wants Main and no longer wants Aux. Fail only the first
  // origin's actual SQL write; later Aux removal must still commit.
  query("UPDATE supla_dev_channel SET user_config='{"
        "\"mainThermometerChannelId\":" + std::to_string(resource) +
        "}' WHERE id=" + std::to_string(hvac));
  query("CREATE TRIGGER suplan_fail_ensure_origin BEFORE INSERT ON "
        "supla_suplan_grant FOR EACH ROW BEGIN IF NEW.origin_type=" +
        std::to_string(CHANNEL_CONFIG_ORIGIN) + " AND NEW.origin_id=" +
        std::to_string(main_origin) +
        " THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Injected origin "
        "write failure'; END IF; END");
  for (int attempt = 0; attempt < 2; ++attempt) {
    expect_access(resource, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR,
                  SUPLA_SUPLAN_ACCESS_STATUS_INVALID);
    ASSERT_FALSE(HasFatalFailure());
    std::vector<uint64_t> ids;
    ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN, aux_origin, &ids));
    EXPECT_TRUE(ids.empty());
    ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN, main_origin, &ids));
    EXPECT_TRUE(ids.empty());
    Association after;
    ASSERT_TRUE(repo.find(source, destination, &after));
    ASSERT_EQ(1u, after.acl.size());
    EXPECT_EQ(independent, after.acl.front().ResourceId);
    EXPECT_EQ(before.revision + 1, after.revision);  // Revoke stale READ once.
    EXPECT_EQ(before.generation, after.generation);
    ASSERT_TRUE(repo.find(source, other_source, &after));
    EXPECT_TRUE(same_state(unrelated, after));
  }
  query("DROP TRIGGER suplan_fail_ensure_origin");
  for (int attempt = 0; attempt < 2; ++attempt) {
    expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                  SUPLA_SUPLAN_ACCESS_STATUS_GRANTED);
    ASSERT_FALSE(HasFatalFailure());
  }
  std::vector<uint64_t> ids;
  ASSERT_TRUE(repo.for_origin(CHANNEL_CONFIG_ORIGIN, main_origin, &ids));
  ASSERT_EQ(1u, ids.size());
  Association after;
  ASSERT_TRUE(repo.find(source, destination, &after));
  EXPECT_EQ(2u, after.acl.size());
  // Repair once; retry is a no-op.
  EXPECT_EQ(before.revision + 2, after.revision);
  EXPECT_EQ(before.generation, after.generation);
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

TEST_F(SupLanPeerDaoIntegrationTest,
       M5EnsureUsesCommittedOwnershipCapabilityAndIdentityWithoutApprovalRows) {
  std::string out;
  sqlQuery(("UPDATE supla_iodevice SET flags=flags | " +
            std::to_string(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) +
            ",protocol_version=29 WHERE user_id=2").c_str(), &out);
  sqlQuery(("UPDATE supla_dev_channel SET type=" +
            std::to_string(SUPLA_CHANNELTYPE_RELAY) + ",func=" +
            std::to_string(SUPLA_CHANNELFNC_POWERSWITCH) +
            " WHERE id=" + std::to_string(resource)).c_str(), &out);
  const Grant read{1, resource, CHANNEL_CONFIG_ORIGIN, 12345, 1};
  ASSERT_TRUE(service.upsert(source, destination, read));
  ASSERT_TRUE(service.provisioned(state(), 123));
  const auto before = state();
  TDS_SuplaEnsureResourceAccess q = {
      {1, resource}, 1, SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER, 1};
  for (int i = 0; i < 10; ++i)
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
              service.ensure_access(destination, q).AccessStatus);
  q.Permissions = 2;
  for (int i = 0; i < 10; ++i) {
    auto result = service.ensure_access(destination, q);
    EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, result.Result);
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_INVALID, result.AccessStatus);
  }
  EXPECT_TRUE(same_state(before, state()));
  std::vector<Grant> grants;
  ASSERT_TRUE(factory()->grants(before.id, &grants));
  ASSERT_EQ(1u, grants.size());
  EXPECT_EQ(read.origin_id, grants.front().origin_id);
  EXPECT_EQ(read.permissions, grants.front().permissions);
  q.Permissions = 4;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED,
            service.ensure_access(destination, q).Result);
  q.Permissions = 1;
  sqlQuery(("UPDATE supla_iodevice SET protocol_version=28 WHERE id=" +
            std::to_string(destination)).c_str(), &out);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED,
            service.ensure_access(destination, q).Result);
  sqlQuery(("UPDATE supla_iodevice SET protocol_version=29 WHERE id=" +
            std::to_string(destination)).c_str(), &out);
  PeerService foreign{[] { return std::make_unique<PeerDao>(1); }, nullptr};
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
            foreign.ensure_access(destination, q).Result);
  ASSERT_TRUE(factory()->accept_identity(source, {123, {}}));
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
            service.ensure_access(destination, q).Result);
  EXPECT_TRUE(same_state(before, state()));
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M5PendingRejectWidenReconsiderAndRemovalUseDurableBusinessState) {
  manual_schema();
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, manual_ensure(1, 0).Result);
  EXPECT_TRUE(decisions().empty());
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  auto rows = decisions();
  ASSERT_EQ(1u, rows.size());
  auto read = rows.front();
  for (int i = 0; i < 10; ++i)
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
              manual_ensure(1).AccessStatus);
  EXPECT_EQ(read.revision, decisions().front().revision);
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(2).AccessStatus);
  auto control = decisions().front();
  EXPECT_EQ(2, control.permissions);
  EXPECT_EQ(read.revision + 1, control.revision);
  EXPECT_EQ(0, decide(read, "ACCEPTED"));  // stale displayed mask/revision
  ASSERT_EQ(1, decide(control, "REJECTED"));
  for (int i = 0; i < 10; ++i)
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_REJECTED,
              manual_ensure(3).AccessStatus);
  auto rejected = decisions().front();
  EXPECT_EQ(2, rejected.permissions);
  EXPECT_EQ(control.revision + 1, rejected.revision);
  EXPECT_EQ(0u, state().id);
  // Direct reconsideration, no new ENSURE/ConsumeIntent needed.
  ASSERT_EQ(1, decide(rejected, "ACCEPTED"));
  ASSERT_TRUE(supla_suplan_server_peers::reconcile_manual(2, resource));
  auto accepted = state();
  ASSERT_EQ(1u, accepted.acl.size());
  EXPECT_EQ(2, accepted.acl.front().Permissions);
  std::vector<Grant> grants;
  ASSERT_TRUE(factory()->grants(accepted.id, &grants));
  ASSERT_EQ(1u, grants.size());
  EXPECT_EQ(USER_APPROVAL_ORIGIN, grants.front().origin_type);
  EXPECT_EQ(rejected.id, grants.front().origin_id);
  ASSERT_EQ(1, decide(decisions().front(), "REMOVED"));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  EXPECT_TRUE(state().acl.empty());
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  rows = decisions();
  ASSERT_EQ(2u, rows.size());
  EXPECT_NE(read.id, rows.back().id);
  ASSERT_EQ(1, decide(rows.back(), "REMOVED"));  // remove Pending
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  rows = decisions();
  ASSERT_EQ(1, decide(rows.back(), "REJECTED"));
  ASSERT_EQ(1, decide(decisions().back(), "REMOVED"));  // clear sticky deny
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M5AcceptedReadSurvivesRejectedControlAndIndependentOriginRemoval) {
  manual_schema();
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  ASSERT_EQ(1, decide(decisions().front(), "ACCEPTED"));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  auto read = state();
  ASSERT_TRUE(service.provisioned(read, 123));
  Grant independent{1, resource, CHANNEL_CONFIG_ORIGIN, 88888, 1};
  ASSERT_TRUE(service.upsert(source, destination, independent));
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(2).AccessStatus);
  auto rows = decisions();
  ASSERT_EQ(2u, rows.size());
  EXPECT_EQ(2, rows.back().permissions);
  EXPECT_EQ(1, state().acl.front().Permissions);
  auto pending_state = state();
  ASSERT_EQ(1, decide(rows.back(), "REJECTED"));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_REJECTED,
            manual_ensure(2).AccessStatus);
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            manual_ensure(1).AccessStatus);
  ASSERT_EQ(1, decide(decisions().front(), "REMOVED"));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  EXPECT_TRUE(same_state(pending_state, state()));
  ASSERT_EQ(1, decide(decisions().back(), "ACCEPTED"));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  EXPECT_EQ(2, state().acl.front().Permissions);
  EXPECT_EQ(read.revision + 1, state().revision);
  ASSERT_EQ(1, decide(decisions().back(), "REMOVED"));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  EXPECT_EQ(1, state().acl.front().Permissions);
  EXPECT_EQ(read.generation, state().generation);
  EXPECT_EQ(read.revision + 2, state().revision);
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M5ConcurrentEnsureDeduplicatesAndStaleFunctionCannotGrant) {
  manual_schema();
  std::atomic<int> pending{0};
  std::vector<std::thread> threads;
  for (int i = 0; i < 6; ++i) threads.emplace_back([&] {
    mysql_thread_init();
    if (manual_ensure(1).AccessStatus ==
        SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL) ++pending;
    mysql_thread_end();
  });
  for (auto &thread : threads) thread.join();
  EXPECT_EQ(6, pending);
  auto rows = decisions();
  ASSERT_EQ(1u, rows.size());
  EXPECT_EQ(1u, rows.front().revision);
  ASSERT_EQ(1, decide(rows.front(), "ACCEPTED"));
  std::string out;
  sqlQuery(("UPDATE supla_dev_channel SET func=0 WHERE id=" +
            std::to_string(resource)).c_str(), &out);
  ASSERT_TRUE(service.reconcile_approvals(resource));
  EXPECT_TRUE(state().acl.empty());
  EXPECT_EQ(ApprovalStatus::Accepted, decisions().front().status);
  ASSERT_EQ(1, decide(decisions().front(), "REMOVED"));
  sqlQuery(("UPDATE supla_dev_channel SET func=" +
            std::to_string(SUPLA_CHANNELFNC_POWERSWITCH) + " WHERE id=" +
            std::to_string(resource)).c_str(), &out);
  ASSERT_TRUE(service.reconcile_approvals(resource));
  EXPECT_TRUE(state().acl.empty());  // explicit Remove cannot be resurrected
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M5ConcurrentDisplayedApprovalAndExtensionCannotEscalate) {
  manual_schema();
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  const auto displayed = decisions().front();
  std::atomic<int> accepted{0};
  std::thread cloud([&] {
    mysql_thread_init();
    accepted = decide(displayed, "ACCEPTED");
    mysql_thread_end();
  });
  std::thread device([&] {
    mysql_thread_init();
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
              manual_ensure(2).AccessStatus);
    mysql_thread_end();
  });
  cloud.join();
  device.join();
  ASSERT_TRUE(service.reconcile_approvals(resource));
  const auto rows = decisions();
  ASSERT_FALSE(rows.empty());
  EXPECT_EQ(ApprovalStatus::Pending, rows.back().status);
  EXPECT_EQ(2, rows.back().permissions);
  if (accepted) {
    ASSERT_EQ(2u, rows.size());
    ASSERT_EQ(1u, state().acl.size());
    EXPECT_EQ(1, state().acl.front().Permissions);
  } else {
    ASSERT_EQ(1u, rows.size());
    EXPECT_TRUE(state().acl.empty());
    EXPECT_EQ(0, decide(displayed, "ACCEPTED"));
  }
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M5AtActionApprovalIsIndependentAndRejectsStateOrControl) {
  manual_schema();
  std::string out;
  sqlQuery(("UPDATE supla_dev_channel SET type=" +
            std::to_string(SUPLA_CHANNELTYPE_ACTIONTRIGGER) + ",func=" +
            std::to_string(SUPLA_CHANNELFNC_ACTIONTRIGGER) +
            ",properties=JSON_OBJECT('actionTriggerCapabilities',"
            "JSON_ARRAY('TURN_ON')) "
            "WHERE id=" + std::to_string(resource)).c_str(), &out);
  for (uint8_t mask : {1, 2, 3, 5, 6, 7})
    EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED, manual_ensure(mask).Result);
  EXPECT_TRUE(decisions().empty());
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(4).AccessStatus);
  auto action = decisions().front();
  EXPECT_EQ(4, action.permissions);
  ASSERT_EQ(1, decide(action, "ACCEPTED"));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  auto before = state();
  ASSERT_EQ(1u, before.acl.size());
  EXPECT_EQ(4, before.acl.front().Permissions);
  for (int i = 0; i < 10; ++i)
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
              manual_ensure(4).AccessStatus);
  EXPECT_TRUE(same_state(before, state()));
  EXPECT_EQ(action.revision + 1, decisions().front().revision);
  sqlQuery(("UPDATE supla_dev_channel SET properties='{}' WHERE id=" +
            std::to_string(resource)).c_str(), &out);
  ASSERT_TRUE(service.reconcile_approvals(resource));
  EXPECT_TRUE(state().acl.empty());
  EXPECT_EQ(ApprovalStatus::Accepted, decisions().front().status);
}


class ManualPairFailureTransport : public DeleteTransport {
 public:
  int errors = 0;
  void diagnostic(uint64_t, int, uint8_t result) override {
    EXPECT_EQ(SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR, result);
    ++errors;
  }
};
TEST_F(SupLanPeerDaoIntegrationTest,
       M5CapacityFailureDoesNotBlockOtherDestinationRevocationOrEnsure) {
  manual_schema();
  const int healthy = destination;
  supla_mariadb_access_provider db;
  ASSERT_TRUE(db.connect());
  void *stmt = nullptr;
  int full = 0;
  ASSERT_TRUE(db.stmt_get_int(&stmt, &full, nullptr, nullptr, nullptr,
      ("SELECT MIN(id) FROM supla_iodevice WHERE user_id=2 AND id<>" +
       std::to_string(source) + " AND id<>" + std::to_string(healthy)).c_str(),
      nullptr, 0, true));
  ASSERT_GT(full, 0);
  // Create the failing pair first, matching authoritative fan-out ordering.
  destination = full;
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  const auto full_pending = decisions().front();
  destination = healthy;
  ASSERT_TRUE(service.upsert(source, destination, {1, resource, 1, 777, 1}));
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(2).AccessStatus);
  ASSERT_EQ(1, decide(decisions().front(), "ACCEPTED"));
  ASSERT_TRUE(service.reconcile_approvals(resource, destination));
  ASSERT_EQ(2, state().acl.front().Permissions);

  Identity identity{123, {resource}};
  for (int i = 0; i < SUPLA_SUPLAN_MAX_ACL_ENTRIES; ++i) {
    const std::string number = std::to_string(100 + i);
    const std::string query = "INSERT INTO supla_dev_channel "
        "(iodevice_id,user_id,channel_number,type,func,param1,param2,param3) "
        "VALUES (" + std::to_string(source) + ",2," + number + "," +
        std::to_string(SUPLA_CHANNELTYPE_RELAY) + "," +
        std::to_string(SUPLA_CHANNELFNC_POWERSWITCH) + ",0,0,0)";
    ASSERT_EQ(0, db.query(query.c_str(), true));
    int channel = 0;
    ASSERT_TRUE(db.stmt_get_int(&stmt, &channel, nullptr, nullptr, nullptr,
        ("SELECT id FROM supla_dev_channel WHERE iodevice_id=" +
         std::to_string(source) + " AND channel_number=" + number).c_str(),
        nullptr, 0, true));
    identity.channels.push_back(channel);
    ASSERT_TRUE(service.upsert(source, full,
        {1, static_cast<uint32_t>(channel), 1,
         static_cast<uint64_t>(1000 + i), 1}));
  }
  ASSERT_TRUE(factory()->accept_identity(source, identity));
  ASSERT_EQ(1, decide(full_pending, "ACCEPTED"));
  ASSERT_EQ(1, decide(decisions().front(), "REMOVED"));
  const auto before = state();
  // Same Source/resource, two real DAO transactions. The first cannot project
  // a 90th ACL entry; the second must still retract CONTROL, retaining READ.
  std::vector<ApprovalPair> failed;
  EXPECT_FALSE(service.reconcile_approvals(resource, 0, &failed));
  ASSERT_EQ(1u, failed.size());
  EXPECT_EQ(full, failed.front().destination);
  ASSERT_EQ(1u, state().acl.size());
  EXPECT_EQ(1, state().acl.front().Permissions);
  EXPECT_EQ(before.revision + 1, state().revision);
  const auto read = state();
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, manual_ensure(1).AccessStatus);
  EXPECT_TRUE(same_state(read, state()));

  // A new accepted extension on the healthy pair still works. Lose its remove
  // notice, then reconnect the shared Source: publish only the healthy pair's
  // current READ ACL, never stale authorization for the failed pair.
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(2).AccessStatus);
  ASSERT_EQ(1, decide(decisions().back(), "ACCEPTED"));
  ASSERT_TRUE(service.reconcile_approvals(resource, destination));
  ASSERT_EQ(2, state().acl.front().Permissions);
  ASSERT_EQ(1, decide(decisions().back(), "REMOVED"));
  ManualPairFailureTransport transport;
  PeerProvisioner peers(factory, &transport);
  peers.reconnect(source);
  EXPECT_EQ(1, transport.errors);
  ASSERT_EQ(1u, transport.sources.size());
  EXPECT_EQ(healthy, transport.sources.front().PeerContext.DestinationNodeId);
  EXPECT_EQ(1, transport.sources.front().Acl[0].Permissions);
  EXPECT_TRUE(transport.destinations.empty());
  Association unchanged;
  ASSERT_TRUE(factory()->find(source, full, &unchanged));
  EXPECT_EQ(SUPLA_SUPLAN_MAX_ACL_ENTRIES, unchanged.acl.size());
}

class ManualOfflineTransport : public DeleteTransport {
 public:
  int source = 0;
  bool online = false;
  bool ready(int device) override { return device != source || online; }
};
TEST_F(SupLanPeerDaoIntegrationTest,
       M5OfflineApprovalRevocationAndLostNoticeConvergeOnReconnect) {
  manual_schema();
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  ASSERT_EQ(1, decide(decisions().front(), "ACCEPTED"));
  ManualOfflineTransport transport;
  transport.source = source;
  PeerProvisioner peers(factory, &transport);
  // No Cloud notice: Destination reconnect reloads committed business state.
  peers.reconnect(destination);
  ASSERT_EQ(1u, state().acl.size());
  EXPECT_TRUE(transport.sources.empty());
  EXPECT_TRUE(transport.destinations.empty());
  EXPECT_EQ(0u, state().root);
  transport.online = true;
  peers.reconnect(source);
  ASSERT_EQ(1u, transport.sources.size());
  EXPECT_TRUE(transport.destinations.empty());  // Source first, no fake key.
  auto q = transport.sources.back();
  TDS_SuplaSetSuplanSourceAssociationResult reply = {};
  reply.PeerContext = q.PeerContext;
  reply.AclRevision = q.AclRevision;
  reply.Result = SUPLA_SUPLAN_RESULT_OK;
  reply.PeerKeySize = 32;
  memset(reply.PeerKey, 0xA5, sizeof(reply.PeerKey));
  peers.on_source(source, &reply);
  ASSERT_EQ(1u, transport.destinations.size());
  const auto destination_q = transport.destinations.back();
  TDS_SuplaSetSuplanDestinationAssociationResult accepted = {};
  accepted.PeerContext = destination_q.PeerContext;
  accepted.AclRevision = destination_q.AclRevision;
  accepted.Result = SUPLA_SUPLAN_RESULT_OK;
  peers.on_destination(destination, &accepted);
  auto provisioned = state();
  EXPECT_EQ(123u, provisioned.root);
  transport.online = false;
  ASSERT_EQ(1, decide(decisions().front(), "REMOVED"));
  peers.reconnect(destination);  // recover a second lost notice
  EXPECT_TRUE(state().acl.empty());
  EXPECT_EQ(provisioned.generation, state().generation);
  EXPECT_EQ(provisioned.root, state().root);
  EXPECT_EQ(provisioned.revision + 1, state().revision);
  EXPECT_EQ(1u, transport.sources.size());  // offline Source cannot apply yet
  transport.online = true;
  peers.reconnect(source);
  ASSERT_EQ(2u, transport.sources.size());
  EXPECT_EQ(0, transport.sources.back().AclEntryCount);
  EXPECT_EQ(ApprovalStatus::Removed, decisions().front().status);
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M5DeletedResourceInvalidatesAcceptedWithoutResurrectingOrOtherOrigins) {
  manual_schema();
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  ASSERT_EQ(1, decide(decisions().front(), "ACCEPTED"));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  Grant independent{SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE,
                    static_cast<uint32_t>(source), 99, 9876, 1};
  ASSERT_TRUE(service.upsert(source, destination, independent));
  std::string out;
  sqlQuery(("DELETE FROM supla_dev_channel WHERE id=" +
            std::to_string(resource)).c_str(), &out);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_FOUND, manual_ensure(1).Result);
  EXPECT_EQ(ApprovalStatus::Accepted, decisions().front().status);
  std::vector<Grant> grants;
  ASSERT_TRUE(factory()->grants(state().id, &grants));
  ASSERT_EQ(1u, grants.size());
  EXPECT_EQ(99, grants.front().origin_type);
  EXPECT_EQ(9876u, grants.front().origin_id);
  EXPECT_EQ(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE,
            state().acl.front().ResourceType);
}

TEST_F(SupLanHvacWireIntegrationTest,
       M5RegisteredDeviceEnsureAndCommittedIpcDecisionUseProductionDispatch) {
  runSqlScript("SupLanM5TestSchema.sql");
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  query("UPDATE supla_iodevice SET protocol_version=29 WHERE user_id=99126");
  PeerDao repo(99126);
  ASSERT_TRUE(repo.accept_identity(source, {123, {resource}}));
  expect_access(resource, SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
                SUPLA_SUPLAN_ACCESS_STATUS_INVALID);
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_decision"));
  for (int i = 0; i < 3; ++i)
    expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                  SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL, 1);
  ASSERT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_decision"));
  EXPECT_EQ(1, scalar("SELECT revision FROM supla_resource_access_decision"));
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_suplan_grant"));
  // Committed Cloud boundary; the same existing IPC command carries no grant.
  query("UPDATE supla_resource_access_decision SET status='ACCEPTED',"
        "open_pair_id=NULL WHERE status='PENDING' AND revision=1");
  testing::IpcSocketAdapterMock socket(-1);
  EXPECT_CALL(socket, send_data(std::string("OK:99126\n"))).Times(1);
  supla_on_channel_config_changed_command command(&socket);
  std::string input = "USER-ON-CHANNEL-CONFIG-CHANGED:99126," +
      std::to_string(source) + "," + std::to_string(resource) + "," +
      std::to_string(SUPLA_CHANNELTYPE_THERMOMETER) + "," +
      std::to_string(SUPLA_CHANNELFNC_THERMOMETER) + "," +
      std::to_string(CONFIG_CHANGE_SCOPE_RELATIONS) + "\n";
  std::vector<char> buffer(input.begin(), input.end());
  buffer.push_back(0);
  ASSERT_TRUE(command.process_command(buffer.data(), buffer.size(),
                                      input.size()));
  expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, 1);
  EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_suplan_grant WHERE "
                     "origin_type=2 AND permissions=1"));
  EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_decision"));
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M5RevisionExhaustionRejectsMutationAndKeepsIndependentRead) {
  manual_schema();
  Grant independent{1, resource, CHANNEL_CONFIG_ORIGIN, 778899, 1};
  ASSERT_TRUE(service.upsert(source, destination, independent));
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(2).AccessStatus);
  // A terminal revision cannot accept a new displayed mask/status. Use real
  // trigger exhaustion rather than a fake DAO failure or guessed method order.
  std::string out;
  sqlQuery("DROP TRIGGER resource_access_decision_revision", &out);
  sqlQuery("UPDATE supla_resource_access_decision SET revision=2147483647",
           &out);
  runSqlScript("SupLanM5TestSchema.sql");
  const auto before = state();
  const auto displayed = decisions().front();
  supla_mariadb_access_provider db;
  ASSERT_TRUE(db.connect());
  EXPECT_NE(0, db.query(("UPDATE supla_resource_access_decision SET "
                         "status='ACCEPTED',open_pair_id=NULL WHERE id=" +
                         std::to_string(displayed.id)).c_str(), true));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  EXPECT_EQ(ApprovalStatus::Pending, decisions().front().status);
  EXPECT_TRUE(same_state(before, state()));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            manual_ensure(1).AccessStatus);
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M5StateReadAndIndependentAtActionDoNotShareDecisionsOrRevocation) {
  manual_schema();
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  ASSERT_EQ(1, decide(decisions().front(), "ACCEPTED"));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  const auto state_resource = resource;
  supla_mariadb_access_provider db;
  ASSERT_TRUE(db.connect());
  const std::string insert = "INSERT INTO supla_dev_channel "
      "(iodevice_id,user_id,channel_number,type,func,param1,param2,param3,"
      "properties) VALUES (" + std::to_string(source) + ",2,254," +
      std::to_string(SUPLA_CHANNELTYPE_ACTIONTRIGGER) + "," +
      std::to_string(SUPLA_CHANNELFNC_ACTIONTRIGGER) +
      ",0,0,0,JSON_OBJECT('actionTriggerCapabilities',JSON_ARRAY('TURN_ON')))";
  ASSERT_EQ(0, db.query(insert.c_str(), true));
  int at = 0;
  void *stmt = nullptr;
  ASSERT_TRUE(db.stmt_get_int(&stmt, &at, nullptr, nullptr, nullptr,
      ("SELECT id FROM supla_dev_channel WHERE iodevice_id=" +
       std::to_string(source) + " AND channel_number=254").c_str(),
      nullptr, 0, true));
  ASSERT_GT(at, static_cast<int>(state_resource));
  resource = at;
  ASSERT_TRUE(factory()->accept_identity(source, {123, {state_resource,
                                                       resource}}));
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(4).AccessStatus);
  ASSERT_EQ(1u, decisions().size());
  ASSERT_EQ(1, decide(decisions().front(), "REJECTED"));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_REJECTED,
            manual_ensure(4).AccessStatus);
  resource = state_resource;
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            manual_ensure(1).AccessStatus);
  EXPECT_EQ(ApprovalStatus::Accepted, decisions().front().status);
  resource = at;
  ASSERT_EQ(1, decide(decisions().front(), "ACCEPTED"));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  ASSERT_EQ(2u, state().acl.size());
  EXPECT_EQ(1, state().acl.front().Permissions);
  EXPECT_EQ(4, state().acl.back().Permissions);
  ASSERT_EQ(1, decide(decisions().front(), "REMOVED"));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  ASSERT_EQ(1u, state().acl.size());
  EXPECT_EQ(state_resource, state().acl.front().ResourceId);
  EXPECT_EQ(1, state().acl.front().Permissions);
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M5AccountDeletionCascadesBusinessAndProjectionWithoutOtherAccounts) {
  manual_schema();
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  ASSERT_EQ(1, decide(decisions().front(), "ACCEPTED"));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  const auto independent = state();
  const auto independent_factory = factory;
  supla_mariadb_access_provider db;
  ASSERT_TRUE(db.connect());
  auto query = [&](const std::string &sql) {
    EXPECT_EQ(0, db.query(sql.c_str(), true)) << sql;
  };
  query("INSERT INTO supla_user (id,short_unique_id,long_unique_id,salt,"
        "email,enabled,reg_date,timezone,home_latitude,home_longitude) "
        "SELECT 99131,REPEAT('e',32),'m5-account-test',salt,"
        "'m5-account@test.invalid',enabled,reg_date,timezone,"
        "home_latitude,home_longitude FROM supla_user WHERE id=2");
  for (const char *guid : {"ee", "ff"}) {
    query("INSERT INTO supla_iodevice "
          "(location_id,user_id,guid,enabled,reg_date,protocol_version,flags) "
          "SELECT location_id,99131,UNHEX(REPEAT('" + std::string(guid) +
          "',16)),1,reg_date,29," +
          std::to_string(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) +
          " FROM supla_iodevice WHERE id=" + std::to_string(source));
  }
  void *stmt = nullptr;
  ASSERT_TRUE(db.stmt_get_int(&stmt, &source, &destination, nullptr, nullptr,
      "SELECT MIN(id),MAX(id) FROM supla_iodevice WHERE user_id=99131",
      nullptr, 0, true));
  query("INSERT INTO supla_dev_channel "
        "(iodevice_id,user_id,channel_number,type,func,param1,param2,param3) "
        "VALUES(" + std::to_string(source) + ",99131,0," +
        std::to_string(SUPLA_CHANNELTYPE_RELAY) + "," +
        std::to_string(SUPLA_CHANNELFNC_POWERSWITCH) + ",0,0,0)");
  int channel = 0;
  ASSERT_TRUE(db.stmt_get_int(&stmt, &channel, nullptr, nullptr, nullptr,
      ("SELECT id FROM supla_dev_channel WHERE iodevice_id=" +
       std::to_string(source)).c_str(), nullptr, 0, true));
  resource = channel;
  factory = [] { return std::make_unique<PeerDao>(99131); };
  service = PeerService(factory, nullptr);
  ASSERT_TRUE(factory()->accept_identity(source, {123, {resource}}));
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  ASSERT_EQ(1, decide(decisions().front(), "ACCEPTED"));
  ASSERT_TRUE(service.reconcile_approvals(resource));
  const auto removed = state();
  ASSERT_FALSE(removed.acl.empty());
  // Existing Cloud account removal deletes its Device children first; both
  // actual schema cascades then remove business/technical authorization.
  query("DELETE FROM supla_iodevice WHERE user_id=99131");
  query("DELETE FROM supla_user WHERE id=99131");
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, manual_ensure(1).Result);
  EXPECT_TRUE(decisions().empty());
  Association gone, survivor;
  ASSERT_TRUE(factory()->load(removed.id, &gone));
  EXPECT_EQ(0u, gone.id);
  ASSERT_TRUE(independent_factory()->load(independent.id, &survivor));
  EXPECT_TRUE(same_state(independent, survivor));
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M5CoveredPendingRemainsExplicitIndependentApproval) {
  manual_schema();
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  const auto pending = decisions().front();
  const Grant config_read{1, resource, CHANNEL_CONFIG_ORIGIN, 12345, 1};
  ASSERT_TRUE(service.upsert(source, destination, config_read));
  const auto covered = state();
  ASSERT_TRUE(service.reconcile_approvals(resource));
  for (int i = 0; i < 3; ++i)
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
              manual_ensure(1).AccessStatus);
  ASSERT_EQ(1u, decisions().size());
  EXPECT_EQ(ApprovalStatus::Pending, decisions().front().status);
  EXPECT_EQ(pending.revision, decisions().front().revision);
  ASSERT_EQ(1, decide(pending, "ACCEPTED"));  // Explicit displayed consent.
  ASSERT_TRUE(supla_suplan_server_peers::reconcile_manual(2, resource));
  EXPECT_TRUE(same_state(covered, state()));  // Same effective READ.
  ASSERT_TRUE(service.delete_origin(CHANNEL_CONFIG_ORIGIN, 12345));
  EXPECT_TRUE(same_state(covered, state()));  // Manual READ survives config.
  ASSERT_EQ(1u, decisions().size());
  EXPECT_EQ(ApprovalStatus::Accepted, decisions().front().status);
  std::vector<Grant> grants;
  ASSERT_TRUE(factory()->grants(state().id, &grants));
  ASSERT_EQ(1u, grants.size());
  EXPECT_EQ(USER_APPROVAL_ORIGIN, grants.front().origin_type);
  EXPECT_EQ(pending.id, grants.front().origin_id);
  EXPECT_EQ(1, grants.front().permissions);
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M7LegacyStreamApprovalIsDurableWithoutPeer) {
  stream_schema();
  std::string out;
  sqlQuery(("UPDATE supla_iodevice SET flags=0,protocol_version=28 WHERE id=" +
            std::to_string(source))
               .c_str(),
           &out);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, stream_ensure().Result);
  EXPECT_TRUE(decisions().empty());
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            stream_ensure(true).AccessStatus);
  auto pending = decisions();
  ASSERT_EQ(1u, pending.size());
  EXPECT_EQ(SUPLA_RESOURCE_DELIVERY_SERVER_STREAM, pending[0].delivery_mode);
  for (int i = 0; i < 5; ++i)
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
              stream_ensure(true).AccessStatus);
  EXPECT_EQ(pending[0].revision, decisions()[0].revision);
  ASSERT_EQ(1, decide(pending[0], "REJECTED"));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_REJECTED,
            stream_ensure(true).AccessStatus);
  auto rejected = decisions()[0];
  ASSERT_EQ(1, decide(rejected, "ACCEPTED"));
  ASSERT_TRUE(service.reconcile_approvals(resource, destination));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, stream_ensure().AccessStatus);
  EXPECT_EQ(0u, state().id);
  ASSERT_EQ(1, decide(decisions()[0], "REMOVED"));
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, stream_ensure().Result);
  EXPECT_EQ(0u, state().id);
}
TEST_F(SupLanPeerDaoIntegrationTest, M7VirtualKeepsRealOwnerNoIdentityOrPeer) {
  stream_schema();
  std::string out;
  sqlQuery(
      ("UPDATE supla_dev_channel SET is_virtual=1,type=" +
       std::to_string(SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT) +
       ",func=" + std::to_string(SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT) +
       " WHERE id=" + std::to_string(resource))
          .c_str(),
      &out);
  sqlQuery(("UPDATE supla_iodevice SET is_virtual=1,flags=0,protocol_version=0 "
            "WHERE id=" +
            std::to_string(source))
               .c_str(),
           &out);
  sqlQuery(("DELETE FROM supla_suplan_device_state WHERE device_id=" +
            std::to_string(source))
               .c_str(),
           &out);
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            stream_ensure(true).AccessStatus);
  ASSERT_EQ(1, decide(decisions()[0], "ACCEPTED"));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, stream_ensure().AccessStatus);
  ASSERT_TRUE(service.reconcile_approvals(resource, destination));
  EXPECT_EQ(0u, state().id);
  EXPECT_NE(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, manual_ensure(1).AccessStatus);
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, stream_ensure().AccessStatus);
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M7ReusedDecisionProvenanceIsRevisionedAndIdempotent) {
  stream_schema();
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  auto pending = decisions().front();
  EXPECT_EQ(1, pending.delivery_mode);
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            stream_ensure(true).AccessStatus);
  auto stream_pending = decisions().front();
  EXPECT_EQ(2, stream_pending.delivery_mode);
  EXPECT_EQ(pending.revision + 1, stream_pending.revision);
  EXPECT_EQ(0, decide(pending, "ACCEPTED"));  // Stale displayed snapshot.
  ASSERT_EQ(1, decide(stream_pending, "ACCEPTED"));
  auto accepted = decisions().front();
  for (int i = 0; i < 5; ++i)
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, stream_ensure().AccessStatus);
  ASSERT_EQ(1u, decisions().size());
  EXPECT_EQ(accepted.revision, decisions().front().revision);
  EXPECT_EQ(accepted.id, decisions().front().id);
}

TEST_F(SupLanPeerDaoIntegrationTest, M7CurrentAuthorityPublicationCost) {
  stream_schema();
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            stream_ensure(true).AccessStatus);
  ASSERT_EQ(1, decide(decisions().front(), "ACCEPTED"));
  auto began = std::chrono::steady_clock::now();
  for (int i = 0; i < 256; ++i) {
    auto access = service.ensure_stream(destination, resource, false, [&] {
      return supla_suplan_server_peers::reconcile_access_dependencies(
          2, resource, destination);
    });
    ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, access.AccessStatus);
  }
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::steady_clock::now() - began)
                     .count();
  std::cout << "STREAM SQL: 256 authoritative publication checks incl. "
               "config audit; elapsed_ms="
            << elapsed << std::endl;
  EXPECT_EQ(0u, state().id);
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M7AcceptedControlCoversStreamReadWithoutNewConsentOrAcl) {
  stream_schema();
  std::string out;
  sqlQuery(("UPDATE supla_dev_channel SET type=" +
            std::to_string(SUPLA_CHANNELTYPE_RELAY) +
            ",func=" + std::to_string(SUPLA_CHANNELFNC_POWERSWITCH) +
            " WHERE id=" + std::to_string(resource))
               .c_str(),
           &out);
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(2).AccessStatus);
  ASSERT_EQ(1, decide(decisions().front(), "ACCEPTED"));
  ASSERT_TRUE(service.reconcile_approvals(resource, destination));
  const auto before = state();
  const auto accepted = decisions().front();
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, stream_ensure().AccessStatus);
  ASSERT_EQ(1u, decisions().size());
  const auto reused = decisions().front();
  EXPECT_EQ(accepted.id, reused.id);
  EXPECT_EQ(2, reused.permissions);
  EXPECT_EQ(2, reused.delivery_mode);
  EXPECT_EQ(accepted.revision + 1, reused.revision);
  EXPECT_TRUE(same_state(before, state()));
  EXPECT_EQ(0, decide(accepted, "REMOVED"));
  ASSERT_TRUE(service.reconcile_approvals(resource, destination));
  EXPECT_TRUE(same_state(before, state()));
}

TEST_F(SupLanPeerDaoIntegrationTest,
       M7DisconnectedPhysicalCacheIsCanonicalWithoutDoubleCalibration) {
  stream_schema();
  supla_mariadb_access_provider db;
  ASSERT_TRUE(db.connect());
  ASSERT_EQ(
      0, db.query(("UPDATE supla_dev_channel SET is_virtual=0,"
                   "user_config='{\"temperatureAdjustment\":200}' WHERE id=" +
                   std::to_string(resource))
                      .c_str(),
                  true));
  ASSERT_EQ(
      0,
      db.query(("INSERT INTO supla_dev_channel_value "
                "(user_id,channel_id,value,valid_to,update_time) VALUES (2," +
                std::to_string(resource) +
                ",UNHEX('0000000000003C40'),"
                "UTC_TIMESTAMP()+INTERVAL 5 SECOND,UTC_TIMESTAMP()) "
                "ON DUPLICATE KEY UPDATE "
                "value=VALUES(value),valid_to=VALUES(valid_to),"
                "update_time=VALUES(update_time)")
                   .c_str(),
               true));
  supla_user_dao dao(&db);
  auto user = supla_user::find(2, true);
  auto current = dao.get_virtual_channels(user, 0, resource);
  ASSERT_EQ(1u, current.size());
  EXPECT_EQ(source, current.front().get_device_id());
  EXPECT_EQ(resource, current.front().get_channel_id());
  TDS_SuplaDeviceChannel_E snapshot{};
  int64_t remaining = 0;
  ASSERT_TRUE(current.front().get_stream_value(&snapshot, &remaining));
  double temperature = 0;
  memcpy(&temperature, snapshot.value, sizeof(temperature));
  EXPECT_DOUBLE_EQ(28, temperature);  // Already canonical; must not become 30.
  EXPECT_EQ(0, snapshot.Offline);
  EXPECT_GT(remaining, 0);
  EXPECT_LE(remaining, 5000000);
  for (auto &channel : dao.get_virtual_channels(user, 0))
    EXPECT_NE(resource, channel.get_channel_id());  // Legacy read unchanged.
  ChannelInfo info;
  ASSERT_TRUE(factory()->channel(resource, &info));
  EXPECT_FALSE(info.is_virtual);  // No Source-model or persistent mutation.
}
TEST_F(SupLanPeerDaoIntegrationTest,
       M7StreamDestinationGateAndExactPermissions) {
  stream_schema();
  TDS_SuplaEnsureResourceAccess request{{1, resource}, 1, 2, 1};
  for (unsigned char bits : {0, 2, 3, 4, 5, 7, 128}) {
    request.Permissions = bits;
    EXPECT_NE(SUPLA_SUPLAN_RESULT_OK,
              service.ensure_access(destination, request).Result);
  }
  EXPECT_TRUE(decisions().empty());
  request.Permissions = 1;
  std::string out;
  for (const auto &setting : {"flags=0", "protocol_version=28"}) {
    sqlQuery(("UPDATE supla_iodevice SET " + std::string(setting) +
              " WHERE id=" + std::to_string(destination))
                 .c_str(),
             &out);
    EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED,
              service.ensure_access(destination, request).Result);
  }
  EXPECT_TRUE(decisions().empty());
  PeerService foreign{[] { return std::make_unique<PeerDao>(1); }, nullptr};
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
            foreign.ensure_access(destination, request).Result);
}
TEST_F(SupLanPeerDaoIntegrationTest, M7OriginUnionNeverTrustsStaleAcl) {
  stream_schema();
  ASSERT_TRUE(service.upsert(source, destination, {1, resource, 99, 8001, 1}));
  ASSERT_TRUE(service.upsert(source, destination, {1, resource, 99, 8002, 1}));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            stream_ensure(true).AccessStatus);
  EXPECT_TRUE(decisions().empty());
  auto revision = state().revision;
  ASSERT_TRUE(service.delete_origin(99, 8001));
  EXPECT_EQ(revision, state().revision);
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, stream_ensure().AccessStatus);
  ASSERT_TRUE(service.delete_origin(99, 8002));
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, stream_ensure().Result);
}

TEST_F(SupLanHvacWireIntegrationTest,
       M7VirtualApprovalSnapshotExpiryAndTerminal) {
  runSqlScript("SupLanM5TestSchema.sql");
  runSqlScript("SupLanM7TestSchema.sql");
  query(
      "UPDATE supla_iodevice SET flags=0,protocol_version=0,is_virtual=1 "
      "WHERE id=" +
      std::to_string(source));
  query("UPDATE supla_dev_channel SET is_virtual=1 WHERE id=" +
        std::to_string(resource));
  query("DELETE FROM supla_suplan_device_state WHERE device_id=" +
        std::to_string(source));
  query(
      "INSERT INTO supla_dev_channel_value "
      "(user_id,channel_id,value,valid_to,update_time) "
      "VALUES (99126," +
      std::to_string(resource) +
      ",UNHEX('0000000000003440'),UTC_TIMESTAMP()+INTERVAL 4 SECOND,"
      "UTC_TIMESTAMP()) ON DUPLICATE KEY UPDATE value=VALUES(value),"
      "valid_to=VALUES(valid_to),update_time=VALUES(update_time)");
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  query(
      "UPDATE supla_dev_channel_value SET valid_to=UTC_TIMESTAMP()+INTERVAL 5 "
      "SECOND "
      "WHERE channel_id=" +
      std::to_string(resource));
  expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL, 1, 1, 2);
  TSuplaDataPacket packet{};
  EXPECT_FALSE(receive_packet(&packet, 100));
  query(
      "UPDATE supla_resource_access_decision SET status='ACCEPTED',"
      "open_pair_id=NULL WHERE status='PENDING'");
  auto user = supla_user::find(99126, false);
  ASSERT_NE(nullptr, user);
  testing::IpcSocketAdapterMock socket(-1);
  EXPECT_CALL(socket, send_data(std::string("OK:99126\n"))).Times(3);
  supla_on_channel_config_changed_command ipc(&socket);
  // Same committed Cloud RELATIONS hint as M5, no second Device ENSURE.
  auto notify = [&] {
    std::string input = "USER-ON-CHANNEL-CONFIG-CHANGED:99126," +
                        std::to_string(source) + "," +
                        std::to_string(resource) + "," +
                        std::to_string(SUPLA_CHANNELTYPE_THERMOMETER) + "," +
                        std::to_string(SUPLA_CHANNELFNC_THERMOMETER) + "," +
                        std::to_string(CONFIG_CHANGE_SCOPE_RELATIONS) + "\n";
    std::vector<char> buffer(input.begin(), input.end());
    buffer.push_back(0);
    EXPECT_TRUE(
        ipc.process_command(buffer.data(), buffer.size(), input.size()));
  };
  notify();
  ASSERT_TRUE(receive_packet(&packet, 5000));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  ASSERT_EQ(sizeof(TSD_SuplaRemoteChannelState), packet.data_size);
  TSD_SuplaRemoteChannelState snapshot{};
  memcpy(&snapshot, packet.data, sizeof(snapshot));
  EXPECT_EQ(resource, snapshot.ChannelId);
  EXPECT_EQ(255, snapshot.Channel.Number);
  EXPECT_EQ(SUPLA_CHANNELTYPE_THERMOMETER, int(snapshot.Channel.Type));
  EXPECT_EQ(SUPLA_CHANNELFNC_THERMOMETER, int(snapshot.Channel.Default));
  EXPECT_EQ(0, snapshot.Channel.Offline);
  double temperature = 0;
  memcpy(&temperature, snapshot.Channel.value, sizeof(temperature));
  EXPECT_DOUBLE_EQ(20, temperature);
  // The legacy virtual updater's second-resolution cursor must not hide a
  // STREAM change. Leave update_time unchanged and read the current row.
  query("UPDATE supla_dev_channel_value SET value=UNHEX('0000000000003540'),"
        "valid_to=UTC_TIMESTAMP()+INTERVAL 3 SECOND WHERE channel_id=" +
        std::to_string(resource));
  notify();
  ASSERT_TRUE(receive_packet(&packet, 5000));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  memcpy(&snapshot, packet.data, sizeof(snapshot));
  memcpy(&temperature, snapshot.Channel.value, sizeof(temperature));
  EXPECT_DOUBLE_EQ(21, temperature);
  ASSERT_TRUE(receive_packet(&packet, 7000));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  memcpy(&snapshot, packet.data, sizeof(snapshot));
  EXPECT_EQ(1, snapshot.Channel.Offline);
  close_connection();
  start_connection();
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, 0, 1, 2);
  ASSERT_TRUE(receive_packet(&packet, 5000));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  memcpy(&snapshot, packet.data, sizeof(snapshot));
  EXPECT_EQ(1, snapshot.Channel.Offline);  // No old connection cache revival.
  query(
      "UPDATE supla_resource_access_decision SET status='REMOVED',"
      "open_pair_id=NULL WHERE status='ACCEPTED'");
  notify();
  ASSERT_TRUE(receive_packet(&packet, 5000));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  memcpy(&snapshot, packet.data, sizeof(snapshot));
  EXPECT_EQ(1, snapshot.Channel.Offline);
  Association association;
  ASSERT_TRUE(PeerDao(99126).find(source, destination, &association));
  EXPECT_EQ(0u, association.id);
}

TEST_F(SupLanHvacWireIntegrationTest, M7UnflaggedV29CannotRequestStream) {
  runSqlScript("SupLanM5TestSchema.sql");
  runSqlScript("SupLanM7TestSchema.sql");
  register_existing_family(SUPLA_CHANNELTYPE_HVAC,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                           SUPLA_CHANNELFNC_HVAC_THERMOSTAT, false, false);
  ASSERT_FALSE(HasFatalFailure());
  expect_access(resource, SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
                SUPLA_SUPLAN_ACCESS_STATUS_INVALID, 1, 1, 2);
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_decision"));
  TSuplaDataPacket packet{};
  EXPECT_FALSE(receive_packet(&packet, 100));
  auto user = supla_user::find(99126, false);
  ASSERT_NE(nullptr, user);
  auto device = user->get_devices()->get(destination);
  ASSERT_NE(nullptr, device);
  EXPECT_EQ(nullptr, device->get_stream(false));
}

TEST_F(SupLanHvacWireIntegrationTest,
       M7PhysicalLegacySourcePublishesValueAndDisconnectWithoutPeer) {
  runSqlScript("SupLanM5TestSchema.sql");
  runSqlScript("SupLanM7TestSchema.sql");
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  {
    auto target =
        supla_user::find(99126, false)->get_devices()->get(destination);
    ASSERT_NE(nullptr, target);
    EXPECT_EQ(120000000u, target->wait_time_usec());
  }
  query(
      "UPDATE supla_iodevice SET guid=UNHEX(REPEAT('cd',16)),auth_key=NULL,"
      "enabled=1,flags=0 WHERE id=" +
      std::to_string(source));
  OtherConnection source_connection(this);
  ASSERT_FALSE(HasFatalFailure());
  TDS_SuplaRegisterDevice_G registration{};
  snprintf(registration.Email, sizeof(registration.Email), "%s",
           "hvac-wire@test.invalid");
  memset(registration.GUID, 0xcd, sizeof(registration.GUID));
  registration.AuthKey[0] = 1;
  registration.Flags = SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED;
  registration.channel_count = 1;
  auto &channel = registration.channels[0];
  channel.Number = scalar(
      "SELECT channel_number FROM supla_dev_channel "
      "WHERE id=" +
      std::to_string(resource));
  channel.Type = SUPLA_CHANNELTYPE_THERMOMETER;
  channel.Default = SUPLA_CHANNELFNC_THERMOMETER;
  double temperature = 20;
  memcpy(channel.value, &temperature, sizeof(temperature));
  send_packet(SUPLA_DS_CALL_REGISTER_DEVICE_G, &registration,
              sizeof(TDS_SuplaRegisterDeviceHeader) + sizeof(channel));
  bool registered = false, synced = false;
  TSuplaDataPacket packet{};
  for (int i = 0; i < 10 && !synced; ++i) {
    ASSERT_TRUE(receive_packet(&packet));
    if (packet.call_id == SUPLA_SD_CALL_REGISTER_DEVICE_RESULT ||
        packet.call_id == SUPLA_SD_CALL_REGISTER_DEVICE_RESULT_B) {
      TSD_SuplaRegisterDeviceResult reply{};
      memcpy(&reply, packet.data, sizeof(reply));
      ASSERT_EQ(SUPLA_RESULTCODE_TRUE, reply.result_code);
      registered = true;
    } else if (packet.call_id == SUPLA_SD_CALL_DEVICE_SYNC_DONE) {
      synced = true;
    } else if (packet.call_id == SUPLA_SD_CALL_SET_CHANNEL_CONFIG) {
      TSDS_SetChannelConfig config{};
      memcpy(&config, packet.data, packet.data_size);
      acknowledge(config);
    } else {
      EXPECT_NE(SUPLA_SD_CALL_SUPLAN_DEVICE_IDENTITIES, packet.call_id);
    }
  }
  ASSERT_TRUE(registered && synced);
  auto live_source = supla_user::find(99126, false)->get_devices()->get(source);
  ASSERT_NE(nullptr, live_source);
  ASSERT_TRUE(live_source->get_channels()->channel_exists(resource));
  live_source.reset();  // The connection lifecycle waits for borrowed owners.
  source_connection.select(false);
  expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL, 1, 1, 2);
  query(
      "UPDATE supla_resource_access_decision SET status='ACCEPTED',"
      "open_pair_id=NULL WHERE status='PENDING'");
  testing::IpcSocketAdapterMock socket(-1);
  EXPECT_CALL(socket, send_data(std::string("OK:99126\n"))).Times(1);
  supla_on_channel_config_changed_command ipc(&socket);
  std::string input = "USER-ON-CHANNEL-CONFIG-CHANGED:99126," +
                      std::to_string(source) + "," + std::to_string(resource) +
                      "," + std::to_string(SUPLA_CHANNELTYPE_THERMOMETER) +
                      "," + std::to_string(SUPLA_CHANNELFNC_THERMOMETER) + "," +
                      std::to_string(CONFIG_CHANGE_SCOPE_RELATIONS) + "\n";
  std::vector<char> buffer(input.begin(), input.end());
  buffer.push_back(0);
  ASSERT_TRUE(ipc.process_command(buffer.data(), buffer.size(), input.size()));
  ASSERT_TRUE(receive_packet(&packet, 5000));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  TSD_SuplaRemoteChannelState snapshot{};
  memcpy(&snapshot, packet.data, sizeof(snapshot));
  EXPECT_EQ(resource, snapshot.ChannelId);
  EXPECT_EQ(0, snapshot.Channel.Offline);
  memcpy(&temperature, snapshot.Channel.value, sizeof(temperature));
  EXPECT_DOUBLE_EQ(20, temperature);
  {
    auto target =
        supla_user::find(99126, false)->get_devices()->get(destination);
    auto timeout = target->wait_time_usec();
    EXPECT_GT(timeout, 1000000u);
    EXPECT_LE(timeout, 30000000u);
  }
  source_connection.select(true);
  TDS_SuplaDeviceChannelValue_B update{};
  update.ChannelNumber = channel.Number;
  temperature = 25;
  memcpy(update.value, &temperature, sizeof(temperature));
  send_packet(SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_B, &update,
              sizeof(update));
  source_connection.select(false);
  ASSERT_TRUE(receive_packet(&packet, 5000));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  memcpy(&snapshot, packet.data, sizeof(snapshot));
  memcpy(&temperature, snapshot.Channel.value, sizeof(temperature));
  EXPECT_DOUBLE_EQ(25, temperature);
  std::shared_ptr<StreamConnection> previous_stream;
  {
    auto target =
        supla_user::find(99126, false)->get_devices()->get(destination);
    previous_stream = target->get_stream(false);
  }
  close_connection();
  previous_stream->dirty(resource);
  previous_stream->iterate();
  EXPECT_EQ(0u, previous_stream->size());
  start_connection();
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  {
    auto target =
        supla_user::find(99126, false)->get_devices()->get(destination);
  }
  expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, 0, 1, 2);
  ASSERT_TRUE(receive_packet(&packet, 5000));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  memcpy(&snapshot, packet.data, sizeof(snapshot));
  memcpy(&temperature, snapshot.Channel.value, sizeof(temperature));
  EXPECT_DOUBLE_EQ(25, temperature);
  previous_stream->dirty(resource);  // An old demand cannot wake/revive itself.
  EXPECT_FALSE(receive_packet(&packet, 100));
  source_connection.select(true);
  TDS_SuplaDeviceChannelValue_C cached{};
  cached.ChannelNumber = channel.Number;
  cached.Offline = 0;
  cached.ValidityTimeSec = 5;
  temperature = 26;
  memcpy(cached.value, &temperature, sizeof(temperature));
  send_packet(SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_C, &cached,
              sizeof(cached));
  source_connection.select(false);
  ASSERT_TRUE(receive_packet(&packet, 5000));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  memcpy(&snapshot, packet.data, sizeof(snapshot));
  EXPECT_EQ(0, snapshot.Channel.Offline);
  memcpy(&temperature, snapshot.Channel.value, sizeof(temperature));
  EXPECT_DOUBLE_EQ(26, temperature);
  source_connection.close();
  EXPECT_FALSE(
      receive_packet(&packet, 100));  // Valid cache survives Source loss.
  ASSERT_TRUE(receive_packet(&packet, 7000));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  memcpy(&snapshot, packet.data, sizeof(snapshot));
  EXPECT_EQ(1, snapshot.Channel.Offline);
  memcpy(&temperature, snapshot.Channel.value, sizeof(temperature));
  EXPECT_DOUBLE_EQ(26, temperature);
  Association association;
  ASSERT_TRUE(PeerDao(99126).find(source, destination, &association));
  EXPECT_EQ(0u, association.id);
  query("DELETE FROM supla_dev_channel WHERE id=" + std::to_string(resource));
  supla_user::on_channel_deleted(99126, source, resource, supla_caller());
  ASSERT_TRUE(receive_packet(&packet, 5000));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  memcpy(&snapshot, packet.data, sizeof(snapshot));
  EXPECT_EQ(1, snapshot.Channel.Offline);
  auto current_destination =
      supla_user::find(99126, false)->get_devices()->get(destination);
  ASSERT_NE(nullptr, current_destination);
  EXPECT_EQ(0u, current_destination->get_stream(false)->size());
}

namespace {
TEST_F(SupLanHvacIntegrationTest, Sql004ReadsAndRealDatabaseMetrics) {
  auto binary = add_channel(source, 235, SUPLA_CHANNELTYPE_BINARYSENSOR,
                             SUPLA_CHANNELFNC_FLOOD_SENSOR);
  auto master = add_channel(source, 236, SUPLA_CHANNELTYPE_HVAC,
                             SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  auto pump = add_channel(source, 237, SUPLA_CHANNELTYPE_RELAY,
                           SUPLA_CHANNELFNC_PUMPSWITCH);
  auto hocs = add_channel(source, 238, SUPLA_CHANNELTYPE_RELAY,
                           SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH);
  ASSERT_TRUE(factory()->accept_identity(source,
                                         {123, {resource, binary, master,
                                                pump, hocs}}));
  ASSERT_TRUE(factory()->accept_identity(destination, {789, {hvac, local}}));
  auto measure = [&](const char *name) {
    std::unique_ptr<supla_json_config> current;
    int function = -1;
    Sql004Probe probe;
    bool ok = supla_suplan_server_peers::reconcile_hvac(99124, hvac,
                                                        &current, &function);
    probe.finish(name);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, current);
    EXPECT_GT(probe.config_reads, 0u);
    EXPECT_LE(probe.config_reads, 2u);
  };
  query("UPDATE supla_dev_channel SET type=" +
        std::to_string(SUPLA_CHANNELTYPE_RELAY) + ",func=" +
        std::to_string(SUPLA_CHANNELFNC_POWERSWITCH) +
        ",user_config='{}' WHERE id=" + std::to_string(hvac));
  query("UPDATE supla_iodevice SET flags=0 WHERE id=" +
        std::to_string(destination));
  measure("ordinary-legacy");
  query("UPDATE supla_dev_channel SET type=" +
        std::to_string(SUPLA_CHANNELTYPE_HVAC) + ",func=" +
        std::to_string(SUPLA_CHANNELFNC_HVAC_THERMOSTAT) +
        " WHERE id=" + std::to_string(hvac));
  measure("hvac-empty-legacy");
  query("UPDATE supla_iodevice SET flags=" +
        std::to_string(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) + " WHERE id=" +
        std::to_string(destination));
  measure("hvac-empty");
  config(resource);
  measure("hvac-one");
  measure("hvac-one-repeat");
  query("UPDATE supla_dev_channel SET user_config='{"
        "\"mainThermometerChannelId\":" + std::to_string(resource) +
        ",\"auxThermometerChannelId\":" + std::to_string(resource) +
        ",\"binarySensorChannelId\":" + std::to_string(binary) +
        ",\"masterThermostatChannelId\":" + std::to_string(master) +
        ",\"pumpSwitchChannelId\":" + std::to_string(pump) +
        ",\"heatOrColdSourceSwitchChannelId\":" + std::to_string(hocs) +
        "}' WHERE id=" + std::to_string(hvac));
  measure("hvac-six");
  EXPECT_EQ(6, scalar("SELECT COUNT(*) FROM supla_suplan_grant "
                     "WHERE origin_type=1"));
  auto before = state();
  query("UPDATE supla_dev_channel SET user_config=JSON_OBJECT("
        "'heatOrColdSourceSwitchChannelId'," + std::to_string(hocs) +
        ",'pumpSwitchChannelId'," + std::to_string(pump) +
        ",'masterThermostatChannelId'," + std::to_string(master) +
        ",'binarySensorChannelId'," + std::to_string(binary) +
        ",'auxThermometerChannelId'," + std::to_string(resource) +
        ",'mainThermometerChannelId'," + std::to_string(resource) +
        ") WHERE id=" + std::to_string(hvac));
  measure("hvac-six-repeat");
  EXPECT_TRUE(same_state(before, state()));
  config(0);
  measure("removed-six");
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_suplan_grant"));
  for (bool container : {true, false}) {
    query("UPDATE supla_dev_channel SET type=" +
          std::to_string(container ? SUPLA_CHANNELTYPE_CONTAINER
                                   : SUPLA_CHANNELTYPE_VALVE_OPENCLOSE) +
          ",func=" + std::to_string(container ? SUPLA_CHANNELFNC_CONTAINER
                        : SUPLA_CHANNELFNC_VALVE_OPENCLOSE) +
          " WHERE id=" + std::to_string(hvac));
    for (unsigned count : {1u, container ? 10u : 20u}) {
      std::unique_ptr<supla_json_config> json(
          container ? static_cast<supla_json_config *>(new container_config())
                    : static_cast<supla_json_config *>(new valve_config()));
      if (container) {
        TChannelConfig_Container raw{};
        for (unsigned i = 0; i < count; ++i)
          raw.SensorInfo[i].ChannelId = binary;
        container_config(json.get()).set_config(&raw);
      } else {
        TChannelConfig_Valve raw{};
        for (unsigned i = 0; i < count; ++i)
          raw.SensorInfo[i].ChannelId = binary;
        valve_config(json.get()).set_config(&raw);
      }
      supla_mariadb_access_provider db;
      supla_device_dao dao(&db);
      ASSERT_TRUE(dao.set_channel_config(99124, hvac, json.get()));
      measure((std::string(container ? "container-" : "valve-") +
               std::to_string(count)).c_str());
    }
  }
  query("DELETE FROM supla_dev_channel WHERE id=" + std::to_string(binary));
  measure("deleted-source");
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_suplan_grant"));
  query("UPDATE supla_dev_channel SET func=0 WHERE id=" + std::to_string(hvac));
  measure("function-none");
  query("UPDATE supla_dev_channel SET type=" +
        std::to_string(SUPLA_CHANNELTYPE_RELAY) + " WHERE id=" +
        std::to_string(hvac));
  measure("type-relay");
}
TEST_F(SupLanHvacIntegrationTest, Sql004ConcurrentRootAndChurnCleanup) {
  config(resource);
  Grant independent{1, resource, 99, 444, SUPLA_SUPLAN_PERMISSION_READ};
  ASSERT_TRUE(service.upsert(source, destination, independent));
  for (bool churn : {false, true}) {
    config(resource);
    Sql004Probe probe;
    probe.after_config = [&](unsigned read) {
      if (churn || read == 1) {
        config(churn && read % 2 == 0 ? local : other_resource);
        // Mutate other root members too; references alone are not the fence.
        query("UPDATE supla_dev_channel SET properties='{\"epoch\":" +
              std::to_string(read) + "}' WHERE id=" + std::to_string(hvac));
      }
      return false;
    };
    std::unique_ptr<supla_json_config> current;
    bool ok = supla_suplan_server_peers::reconcile_hvac(99124, hvac, &current);
    probe.finish(churn ? "persistent-churn" : "one-concurrent-change");
    EXPECT_EQ(!churn, ok);
    if (churn) {
      EXPECT_EQ(6u, probe.config_reads);  // Three snapshot/check attempts.
      EXPECT_EQ(nullptr, current);
      EXPECT_TRUE(origin_grants().empty());  // Latest is local.
    } else {
      ASSERT_NE(nullptr, current);
      EXPECT_EQ(other_resource, hvac_config(current.get()).reference(0));
    }
    EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_suplan_grant "
                        "WHERE origin_type=99"));
  }
}
TEST_F(SupLanHvacIntegrationTest, Sql004FinalReadFailureCannotLeaveNewGrant) {
  config(resource);
  Sql004Probe probe;
  probe.after_config = [&](unsigned read) {
    if (read == 1) config(local);
    return read >= 2;
  };
  std::unique_ptr<supla_json_config> current;
  EXPECT_FALSE(
      supla_suplan_server_peers::reconcile_hvac(99124, hvac, &current));
  probe.finish("final-read-failure");
  EXPECT_EQ(nullptr, current);
  EXPECT_TRUE(origin_grants().empty());
}

TEST_F(SupLanHvacIntegrationTest,
       Sql004DeviceMergeRetriesCurrentCloudAuthority) {
  uint32_t injected = add_channel(source, 239, SUPLA_CHANNELTYPE_THERMOMETER,
                                 SUPLA_CHANNELFNC_THERMOMETER);
  ASSERT_TRUE(factory()->accept_identity(source, {123, {resource, injected}}));
  ASSERT_TRUE(factory()->accept_identity(destination, {789, {hvac, local}}));
  // An unrelated origin/resource must survive replacement and revoke.
  ASSERT_TRUE(service.upsert(destination, source,
      {1, local, 99, 444, SUPLA_SUPLAN_PERMISSION_READ}));
  for (uint8_t field : {1, 5, 41, 83}) {
    query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
          std::to_string(hvac));
    ASSERT_TRUE(reconcile());  // Retire the previous family's config origin.
    bool binding = field == 5, container = field == 41, valve = field == 83;
    int type = container ? SUPLA_CHANNELTYPE_CONTAINER
               : valve ? SUPLA_CHANNELTYPE_VALVE_OPENCLOSE
                       : SUPLA_CHANNELTYPE_HVAC;
    int function = container ? SUPLA_CHANNELFNC_CONTAINER
                   : valve ? SUPLA_CHANNELFNC_VALVE_OPENCLOSE
                           : SUPLA_CHANNELFNC_HVAC_THERMOSTAT;
    int target_type = binding ? SUPLA_CHANNELTYPE_RELAY
                      : field == 1 ? SUPLA_CHANNELTYPE_THERMOMETER
                                   : SUPLA_CHANNELTYPE_BINARYSENSOR;
    int target_function = binding ? SUPLA_CHANNELFNC_PUMPSWITCH
                          : field == 1 ? SUPLA_CHANNELFNC_THERMOMETER
                                       : SUPLA_CHANNELFNC_FLOOD_SENSOR;
    query("UPDATE supla_dev_channel SET type=" + std::to_string(type) +
          ",func=" + std::to_string(function) + " WHERE id=" +
          std::to_string(hvac));
    query("UPDATE supla_dev_channel SET type=" + std::to_string(target_type) +
          ",func=" + std::to_string(target_function) + " WHERE id IN (" +
          std::to_string(resource) + "," + std::to_string(other_resource) +
          "," + std::to_string(injected) + ")");
    auto select = [&](uint32_t id) {
      std::string root;
      if (container || valve) {
        root = container ? "{\"sensors\":[" : "{\"floodSensorChannelIds\":[";
        for (unsigned i = 0; i < (container ? 9u : 19u); ++i) root += "null,";
        root += container ? "{\"channelId\":" + std::to_string(id) +
                                ",\"fillLevel\":60}],\"warningAboveLevel\":7}"
                          : std::to_string(id) +
                                "],\"closeValveOnFloodType\":\"ALWAYS\"}";
      } else {
        root = std::string("{\"") + (binding ? "pumpSwitchChannelId"
                                             : "mainThermometerChannelId") +
               "\":" + std::to_string(id) + ",\"minOnTimeS\":7}";
      }
      // Independent trusted writer; no Cloud IPC/Grant mutation is simulated.
      query("UPDATE supla_dev_channel SET user_config='" + root +
            "' WHERE id=" + std::to_string(hvac));
    };
    auto verify_grants = [&](uint32_t reference) {
      auto repo = factory();
      std::vector<uint64_t> ids;
      ASSERT_TRUE(repo->for_origin(CHANNEL_CONFIG_ORIGIN,
                                   channel_config_origin(hvac, field), &ids));
      ASSERT_EQ(reference ? 1u : 0u, ids.size());
      if (reference) {
        Association a;
        ASSERT_TRUE(repo->load(ids[0], &a));
        EXPECT_EQ(binding ? destination : reference == resource
                            ? source : other_source, a.source);
        EXPECT_EQ(binding ? reference == resource ? source : other_source
                          : destination, a.destination);
        std::vector<Grant> grants;
        ASSERT_TRUE(repo->grants(a.id, &grants));
        unsigned matched = 0;
        for (const auto &g : grants) {
          if (g.origin_type != CHANNEL_CONFIG_ORIGIN) continue;
          ++matched;
          EXPECT_EQ(channel_config_origin(hvac, field), g.origin_id);
          EXPECT_EQ(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, g.resource_type);
          EXPECT_EQ(binding ? hvac : reference, g.resource_id);
          EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_READ, g.permissions);
        }
        EXPECT_EQ(1u, matched);
      }
      for (int endpoint : {source, other_source}) {
        Association a;
        ASSERT_TRUE(repo->find(binding ? destination : endpoint,
                               binding ? endpoint : destination, &a));
        uint32_t selected = endpoint == source ? resource : other_resource;
        unsigned expected = reference == selected ? 1 : 0;
        if (binding && endpoint == source) ++expected;  // Independent origin.
        EXPECT_EQ(expected, a.acl.size());
        for (const auto &entry : a.acl) {
          EXPECT_EQ(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, entry.ResourceType);
          EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_READ, entry.Permissions);
          EXPECT_TRUE((binding && endpoint == source &&
                       entry.ResourceId == local)
                      || (reference == selected &&
                          entry.ResourceId == (binding ? hvac : selected)));
        }
      }
      EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_suplan_grant "
                          "WHERE origin_type=99 AND origin_id=444 AND "
                          "resource_id=" + std::to_string(local) +
                          " AND permissions=1"));
    };
    // One no-conflict control, four single-conflict cases, one bounded failure.
    for (unsigned scenario = 0; scenario < 6; ++scenario) {
      uint32_t uploaded = scenario % 2 ? injected : resource;
      uint32_t cloud = scenario >= 3 && scenario < 5 ? other_resource : 0;
      if (!scenario) cloud = resource;
      SCOPED_TRACE(::testing::Message() << "field=" << unsigned(field)
                   << " scenario=" << scenario << " incoming=" << uploaded);
      select(resource);
      ASSERT_TRUE(reconcile());
      verify_grants(resource);
      ASSERT_FALSE(HasFatalFailure());
      unsigned edits = 0;
      with_destination([&](supla_device_channel *channel) {
        TChannelConfig_HVAC h = {};
        TChannelConfig_Container c = {};
        TChannelConfig_Valve v = {};
        if (binding) h.PumpSwitchChannelId = uploaded;
        else h.MainThermometerChannelId = uploaded;
        h.MinOnTimeS = 19;
        c.SensorInfo[9].ChannelId = uploaded;
        c.WarningAboveLevel = 20;  // Existing wire encoding: JSON level + 1.
        v.SensorInfo[19].ChannelId = uploaded;
        v.CloseValveOnFloodType = SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ON_CHANGE;
        Sql004Probe probe;
        probe.after_config = [&](unsigned read) {
          if (scenario && (read == 1 || scenario == 5)) {
            cloud = scenario == 5 && read % 2 == 0 ? other_resource : cloud;
            if (scenario == 5 && read % 2) cloud = 0;
            select(cloud);
            ++edits;
          }
          return false;
        };
        int result = channel->set_user_config(
            SUPLA_CONFIG_TYPE_DEFAULT, container ? sizeof(c)
                                      : valve ? sizeof(v) : sizeof(h),
            container ? reinterpret_cast<char *>(&c)
            : valve ? reinterpret_cast<char *>(&v)
                    : reinterpret_cast<char *>(&h));
        probe.finish("device-cloud-cas");
        EXPECT_EQ(scenario == 5 ? SUPLA_CONFIG_RESULT_FALSE
                               : SUPLA_CONFIG_RESULT_TRUE, result);
        EXPECT_EQ(scenario == 0 ? 1u : scenario == 5 ? 3u : 2u,
                  probe.config_cas);
        EXPECT_EQ(scenario == 0 ? 2u : 3u, probe.config_reads);
        std::cout << "SQL004CAS field=" << unsigned(field) << " scenario="
                  << scenario << " R1=" << resource << " R2=" << other_resource
                  << " R3=" << injected << " cloud=" << cloud << " edits="
                  << edits << " cas=" << probe.config_cas << std::endl;
      });
      ASSERT_FALSE(HasFatalFailure());
      EXPECT_EQ(scenario == 0 ? 0u : scenario == 5 ? 3u : 1u, edits);
      uint32_t stored = 0;
      ASSERT_TRUE(factory()->reference(hvac, field, &stored));
      EXPECT_EQ(cloud, stored);
      std::string scalar_key = container ? "warningAboveLevel"
                               : valve ? "closeValveOnFloodType" : "minOnTimeS";
      std::string expected = valve ? scenario == 5 ? "ALWAYS" : "ON_CHANGE"
                                   : scenario == 5 ? "7" : "19";
      EXPECT_EQ(1, scalar("SELECT JSON_VALUE(user_config,'$." + scalar_key +
                          "')='" + expected +
                          "' FROM supla_dev_channel WHERE id=" +
                          std::to_string(hvac)));
      verify_grants(resource);  // The trusted SQL edit delivered no IPC.
      ASSERT_TRUE(reconcile());
      verify_grants(cloud);
      ASSERT_FALSE(HasFatalFailure());
      with_destination([&](supla_device_channel *channel) {
        TSD_ChannelConfig wire = {};
        ASSERT_TRUE(channel->get_config(&wire, SUPLA_CONFIG_TYPE_DEFAULT, 0));
        if (container) {
          ASSERT_EQ(sizeof(TChannelConfig_Container), wire.ConfigSize);
          EXPECT_EQ(cloud, reinterpret_cast<TChannelConfig_Container *>(
                               wire.Config)->SensorInfo[9].ChannelId);
          EXPECT_EQ(scenario == 5 ? 8 : 20,
                    reinterpret_cast<TChannelConfig_Container *>(
                        wire.Config)->WarningAboveLevel);
        } else if (valve) {
          ASSERT_EQ(sizeof(TChannelConfig_Valve), wire.ConfigSize);
          EXPECT_EQ(cloud, reinterpret_cast<TChannelConfig_Valve *>(
                               wire.Config)->SensorInfo[19].ChannelId);
          EXPECT_EQ(scenario == 5 ? SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ALWAYS
                                 : SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ON_CHANGE,
                    reinterpret_cast<TChannelConfig_Valve *>(
                        wire.Config)->CloseValveOnFloodType);
        } else {
          ASSERT_EQ(sizeof(TChannelConfig_HVAC), wire.ConfigSize);
          auto config = reinterpret_cast<TChannelConfig_HVAC *>(wire.Config);
          EXPECT_EQ(cloud, binding ? config->PumpSwitchChannelId
                                   : config->MainThermometerChannelId);
          EXPECT_EQ(scenario == 5 ? 7 : 19, config->MinOnTimeS);
        }
      });
    }
  }
}
}  // namespace
