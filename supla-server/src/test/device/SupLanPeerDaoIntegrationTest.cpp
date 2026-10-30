// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include <arpa/inet.h>
#include <dlfcn.h>
#include <gtest/gtest.h>
#include <netinet/tcp.h>
#include <openssl/ssl.h>
#include <poll.h>
#include <sys/resource.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "amazon/alexa_discover_request_search_condition.h"
#include "asynctask/asynctask_queue.h"
#include "cfg.h"
#include "client/client.h"
#include "client/clientchannel.h"
#include "conn/connection.h"
#include "db/database.h"
#include "device/call_handler/register_device.h"
#include "device/call_handler/set_channel_config.h"
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
#include "svrcfg.h"
#include "test/doubles/device/DeviceStub.h"
#include "test/doubles/ipc/IpcSocketAdapterMock.h"
#include "test/doubles/suplan/PeerProvisionerTestAccess.h"
#include "test/integration/IntegrationTest.h"
#include "test/integration/MySqlShell.h"
#include "user.h"
#include "user/channel_relation_index.h"
#include "user/user_dao.h"
// Optional all-thread driver counters for actual TLS registration workers.
std::atomic<bool> hvac_perf_tracking{false};
std::atomic<uint64_t> hvac_perf_sql[7]{};
std::atomic<uint64_t> hvac_perf_relation_reads{0};
thread_local MYSQL_STMT *hvac_perf_last = nullptr;
thread_local std::string hvac_perf_statement;
void hvac_perf_sql_count(const std::string &sql) {
  ++hvac_perf_sql[0];
  if (sql.compare(0, 6, "SELECT") == 0) ++hvac_perf_sql[1];
  if (sql.compare(0, 5, "CALL ") == 0) ++hvac_perf_sql[2];
}
std::atomic<uint64_t> hvac_perf_counts[5]{};
void hvac_perf_count(unsigned kind) { ++hvac_perf_counts[kind]; }
void hvac_perf_reset() {
  for (auto &value : hvac_perf_counts) value = 0;
}
void hvac_perf_dump(const char *label) {
  std::cout << "PERF_COUNTS label=" << label;
  for (unsigned i = 0; i < 5; ++i)
    std::cout << " c" << i << "=" << hvac_perf_counts[i].load();
  std::cout << std::endl;
}
// Test-only driver boundary probe: always execute real MariaDB operations.
// No SQL text/payload logging, production hook, schema change or provider mock.
namespace {
struct Sql004Probe;
thread_local Sql004Probe *sql004 = nullptr;
struct Sql004Probe {
  unsigned connections = 0, prepares = 0, executions = 0;
  unsigned selects = 0, dml = 0, calls = 0, config_reads = 0, config_cas = 0;
  bool restricted = false;
  MYSQL_STMT *last = nullptr, *config_statement = nullptr;
  MYSQL_STMT *permissions_statement = nullptr;
  std::string statement;
  std::function<bool(unsigned)> after_config;
  std::function<void()> after_permissions, after_relation_read;
  MYSQL_STMT *relation_statement = nullptr;
  std::function<bool(const std::string &)> fail_execute;
  unsigned stream_metadata_reads = 0;
  uint64_t sql_usec = 0, connect_usec = 0;
  std::map<std::string, std::pair<unsigned, uint64_t>> timings;
  static uint64_t clock_usec() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
  }
  void elapsed(const std::string &query, uint64_t start) {
    auto us = clock_usec() - start;
    sql_usec += us;
    // Parameter values and credentials are never emitted.
    std::string label = "other";
    if (query.find("SELECT user_config, properties, MD5(") == 0)
      label = "config_select";
    else if (query.find("supla_set_channel_json_config") != std::string::npos)
      label = "config_call";
    else if (query.find("SELECT c.iodevice_id,c.type") != std::string::npos)
      label = "channel_validation";
    else if (query.find("FOR UPDATE") != std::string::npos)
      label = "row_lock";
    auto &value = timings[label];
    ++value.first;
    value.second += us;
  }
  std::chrono::steady_clock::time_point started =
      std::chrono::steady_clock::now();
  Sql004Probe() { sql004 = this; }
  ~Sql004Probe() { sql004 = nullptr; }
  void count(const std::string &sql) {
    ++executions;
    if (sql.compare(0, 5, "CALL ") == 0) ++calls;
    if (sql.compare(0, 6, "SELECT") == 0) ++selects;
    if (sql.find("SELECT type,func,COALESCE(flist,0)") == 0)
      ++stream_metadata_reads;
    if (sql.compare(0, 6, "INSERT") == 0 || sql.compare(0, 6, "UPDATE") == 0 ||
        sql.compare(0, 6, "DELETE") == 0) ++dml;
  }
  void finish(const char *label) {
    sql004 = nullptr;
    std::cout << "SQL004 " << label << " config_reads=" << config_reads
              << " selects=" << selects << " dml=" << dml << " calls=" << calls
              << " prepares=" << prepares << " executions=" << executions
              << " connect_attempts=" << connections << " us="
              << std::chrono::duration_cast<std::chrono::microseconds>(
                     std::chrono::steady_clock::now() - started)
                     .count()
              << " sql_us=" << sql_usec << " connect_us=" << connect_usec
              << std::endl;
    for (const auto &entry : timings)
      std::cout << "PERF_SQL " << label << " " << entry.first
                << " count=" << entry.second.first
                << " us=" << entry.second.second << std::endl;
  }
};
template <typename T> T sql004_native(const char *name) {
  auto function = reinterpret_cast<T>(dlsym(RTLD_NEXT, name));
  if (!function) std::abort();
  return function;
}
}  // namespace
extern "C" MYSQL *STDCALL
mysql_real_connect(MYSQL *mysql, const char *host, const char *user,
                   const char *password, const char *database,
                   unsigned int port, const char *socket, unsigned long flags) {
  static auto native =
      sql004_native<decltype(&mysql_real_connect)>("mysql_real_connect");
  bool perf = hvac_perf_tracking;
  if (perf) ++hvac_perf_sql[3];
  if (sql004) ++sql004->connections;
  if (sql004 && sql004->restricted) {
    user = "suplan_core_test";
    password = "";
  }
  auto start = Sql004Probe::clock_usec();
  auto result =
      native(mysql, host, user, password, database, port, socket, flags);
  if (sql004) sql004->connect_usec += Sql004Probe::clock_usec() - start;
  if (perf) hvac_perf_sql[4] += Sql004Probe::clock_usec() - start;
  return result;
}
extern "C" int STDCALL mysql_query(MYSQL *mysql, const char *query) {
  static auto native = sql004_native<decltype(&mysql_query)>("mysql_query");
  bool perf = hvac_perf_tracking;
  if (perf) hvac_perf_sql_count(query);
  if (sql004) sql004->count(query);
  auto start = Sql004Probe::clock_usec();
  auto result = native(mysql, query);
  if (sql004) sql004->elapsed(query, start);
  if (perf) hvac_perf_sql[5] += Sql004Probe::clock_usec() - start;
  return result;
}
extern "C" int STDCALL mysql_stmt_prepare(MYSQL_STMT *stmt, const char *query,
                                           unsigned long length) {
  static auto native = sql004_native<decltype(&mysql_stmt_prepare)>(
      "mysql_stmt_prepare");
  if (hvac_perf_tracking) {
    ++hvac_perf_sql[6];
    hvac_perf_last = stmt;
    hvac_perf_statement.assign(query, length);
    if (hvac_perf_statement.find("SELECT c.id,c.iodevice_id") == 0)
      ++hvac_perf_relation_reads;
  }
  if (sql004) {
    ++sql004->prepares;
    sql004->last = stmt;
    sql004->statement.assign(query, length);
    if (sql004->statement.find("SELECT c.id,c.iodevice_id") == 0)
      sql004->relation_statement = stmt;
    if (sql004->statement.find("SELECT user_config, properties, MD5(") == 0)
      sql004->config_statement = stmt;
    if (sql004->statement.find("SELECT COALESCE(BIT_OR(permissions),0) FROM "
                              "supla_resource_access_grant_valid") == 0)
      sql004->permissions_statement = stmt;
  }
  return native(stmt, query, length);
}
extern "C" int STDCALL mysql_stmt_execute(MYSQL_STMT *stmt) {
  static auto native = sql004_native<decltype(&mysql_stmt_execute)>(
      "mysql_stmt_execute");
  if (sql004) {
    sql004->count(stmt == sql004->last ? sql004->statement : "");
    if (stmt == sql004->last && sql004->fail_execute &&
        sql004->fail_execute(sql004->statement)) return 1;
    if (stmt == sql004->config_statement) ++sql004->config_reads;
    if (stmt == sql004->last && sql004->statement ==
        "CALL `supla_set_channel_json_config`(?,?,?,?,?,?)")
      ++sql004->config_cas;
  }
  bool perf = hvac_perf_tracking;
  if (perf)
    hvac_perf_sql_count(stmt == hvac_perf_last ? hvac_perf_statement : "");
  auto start = Sql004Probe::clock_usec();
  auto result = native(stmt);
  if (sql004)
    sql004->elapsed(stmt == sql004->last ? sql004->statement : "", start);
  if (perf) hvac_perf_sql[5] += Sql004Probe::clock_usec() - start;
  return result;
}
extern "C" int STDCALL mysql_stmt_fetch(MYSQL_STMT *stmt) {
  static auto native =
      sql004_native<decltype(&mysql_stmt_fetch)>("mysql_stmt_fetch");
  int result = native(stmt);
  if (!result && sql004 && stmt == sql004->config_statement) {
    auto probe = sql004;
    probe->config_statement = nullptr;
    sql004 = nullptr;  // A simulated external writer is outside measured work.
    bool fail = probe->after_config && probe->after_config(probe->config_reads);
    sql004 = probe;
    if (fail) return 1;
  }
  if (!result && sql004 && stmt == sql004->relation_statement) {
    auto probe = sql004;
    probe->relation_statement = nullptr;
    sql004 = nullptr;
    auto callback = std::move(probe->after_relation_read);
    if (callback) callback();
    sql004 = probe;
  }
  if (!result && sql004 && stmt == sql004->permissions_statement) {
    auto probe = sql004;
    probe->permissions_statement = nullptr;
    sql004 = nullptr;
    auto callback = std::move(probe->after_permissions);
    if (callback) callback();
    sql004 = probe;
  }
  return result;
}
extern "C" cJSON *cJSON_Parse(const char *text) {
  static auto native = sql004_native<decltype(&cJSON_Parse)>("cJSON_Parse");
  if (getenv("SUPLA_HVAC_PERF")) hvac_perf_count(3);
  return native(text);
}
extern "C" cJSON *cJSON_Duplicate(const cJSON *item, cJSON_bool recurse) {
  static auto native =
      sql004_native<decltype(&cJSON_Duplicate)>("cJSON_Duplicate");
  if (getenv("SUPLA_HVAC_PERF")) hvac_perf_count(2);
  return native(item, recurse);
}
using namespace supla_suplan;  // NOLINT
// Inject only at the transport-adapter boundary. Serialization, queueing,
// framing, registration and the connection thread remain production code.
class PublicationPauseAdapter : public supla_srpc_adapter {
 public:
  std::function<void()> before_registration_reply;
  std::atomic<bool> registration_probe_ready{false};
  _supla_int_t sd_async_registerdevice_result(
      TSD_SuplaRegisterDeviceResult *result) override {
    if (registration_probe_ready) before_registration_reply();
    return supla_srpc_adapter::sd_async_registerdevice_result(result);
  }
  _supla_int_t sd_async_registerdevice_result_b(
      TSD_SuplaRegisterDeviceResult_B *result) override {
    if (registration_probe_ready) before_registration_reply();
    return supla_srpc_adapter::sd_async_registerdevice_result_b(result);
  }

  std::atomic<bool> pause_get{false};
  std::atomic<bool> fail_device_config{false};
  _supla_int_t sd_async_set_device_config_request(
      TSDS_SetDeviceConfig *config) override {
    if (fail_device_config.exchange(false)) return 0;
    return supla_srpc_adapter::sd_async_set_device_config_request(config);
  }
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
  void *listener = nullptr;
  int peer = -1;
  std::unique_ptr<supla_connection> connection;

 public:
  // DAO/serialization fixtures still need a negotiated protocol version.
  // Use a real transport object rather than treating a null connection as v29.
  supla_connection_test_access() { supla_connection::init(); }
  ~supla_connection_test_access() {
    connection.reset();
    if (peer >= 0) close(peer);
    if (listener) ssocket_free(listener);
    supla_connection::cleanup();
  }
  bool open() {
    listener = ssocket_server_init(nullptr, nullptr, 0, 0);
    if (!listener || !ssocket_openlistener(listener)) return false;
    sockaddr_in address = {};
    socklen_t size = sizeof(address);
    if (getsockname(ssocket_get_fd(listener),
                    reinterpret_cast<sockaddr *>(&address), &size)) {
      return false;
    }
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    peer = socket(AF_INET, SOCK_STREAM, 0);
    if (peer < 0 || connect(peer, reinterpret_cast<sockaddr *>(&address), size))
      return false;
    void *accepted = nullptr;
    unsigned int ip = 0;
    if (!ssocket_accept(listener, &ip, &accepted)) return false;
    connection.reset(new supla_connection(listener, accepted, ip));
    connection->get_srpc_adapter()->set_proto_version(29);
    return true;
  }
  supla_connection *get() { return connection.get(); }
  static std::weak_ptr<TEventHandler> event(supla_connection *connection) {
    return connection->eh;
  }
  static supla_abstract_srpc_adapter *swap_adapter(
      supla_connection *connection, supla_abstract_srpc_adapter *adapter) {
    auto old = connection->srpc_adapter;
    connection->srpc_adapter = adapter;
    return old;
  }
  static PublicationPauseAdapter *install(supla_connection *connection) {
    auto adapter = new PublicationPauseAdapter(connection->_srpc);
    delete connection->srpc_adapter;
    connection->srpc_adapter = adapter;
    return adapter;
  }
};
namespace {
class DeleteTransport : public peer_transport {
 public:
  std::vector<TSDS_SuplaSetSuplanSourceAssociation> sources;
  std::vector<TSDS_SuplaSetSuplanDestinationAssociation> destinations;
  uint64_t session(int d) override { return d; }
  int recovery_device() override { return 0; }
  void wake(int, uint64_t) override {}
  bool reconnect_session(int, uint64_t) override { return false; }
  bool ready(int) override { return true; }
  bool owner_ready(int, uint32_t) override { return true; }
  send_result_t send(int d, TSDS_SuplaSetSuplanSourceAssociation *q) override {
    sources.push_back(*q);
    return {send_code::queued, session(d)};
  }
  send_result_t send(int d,
                     TSDS_SuplaSetSuplanDestinationAssociation *q) override {
    destinations.push_back(*q);
    return {send_code::queued, session(d)};
  }
  void rebootstrap(int) override { ADD_FAILURE(); }
  void diagnostic(uint64_t, int, uint8_t) override { ADD_FAILURE(); }
};

// Trusted test actor supplies explicit business state through the current Cloud
// procedure. No JSON reference parsing, origin discovery or grant projection in
// C++; production Core is exercised separately after this setup.
void cloud_grant(int user, uint64_t origin, int source, int destination,
                 uint32_t resource, unsigned permissions,
                 uint32_t binding = 0) {
  supla_mariadb_access_provider db;
  ASSERT_TRUE(db.connect());
  auto execute = [&](const std::string &sql) {
    MYSQL_STMT *statement = nullptr;
    if (!db.stmt_execute(reinterpret_cast<void **>(&statement), sql.c_str(),
                         nullptr, 0, true)) return false;
    int result;
    do {
      mysql_stmt_free_result(statement);
      result = mysql_stmt_next_result(statement);
    } while (!result);
    mysql_stmt_close(statement);
    return result == -1;
  };
  std::string grants = "[]";
  if (permissions) {
    channel_info_t channel;
    ASSERT_TRUE(peer_dao(user).channel(resource, &channel));
    grants = "[{\"source\":" + std::to_string(source) +
        ",\"destination\":" + std::to_string(destination) +
        ",\"resource\":" + std::to_string(resource) +
        ",\"permissions\":" + std::to_string(permissions) +
        ",\"type\":" + std::to_string(channel.type) +
        ",\"function\":" + std::to_string(channel.function) +
        ",\"peerEligible\":1,\"requiresPeer\":1,\"bindingTarget\":" +
        (binding ? std::to_string(binding) : "null") + "}]";
  }
  ASSERT_TRUE(execute("START TRANSACTION"));
  bool ok = execute("CALL supla_resource_access_replace(" +
      std::to_string(user) + ",1," + std::to_string(origin) + ",'" + grants +
      "')");
  EXPECT_TRUE(ok);
  EXPECT_TRUE(execute(ok ? "COMMIT" : "ROLLBACK"));
}

class SupLanPeerDaoIntegrationTest : public testing::Test,
                                     public testing::IntegrationTest {
 protected:
  peer_service::factory_t factory = [] {
    return std::make_unique<peer_dao>(2);
  };
  peer_service service{factory, nullptr};
  int source = 0, destination = 0;
  uint32_t resource = 0;
  void SetUp() override {
    initTestDatabase();
    runSqlScript("SupLanM2TestSchema.sql");
    runSqlScript("SupLanM5TestSchema.sql");
    runSqlScript("SupLanM7TestSchema.sql");
    runSqlScript("SupLanCloudGrantContract.sql");
    std::string prepared;
    sqlQuery("UPDATE supla_iodevice SET protocol_version=29,flags=flags|524288 "
             "WHERE user_id=2", &prepared);
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
    sqlQuery(("UPDATE supla_dev_channel SET type=" +
              std::to_string(SUPLA_CHANNELTYPE_THERMOMETER) + ",func=" +
              std::to_string(SUPLA_CHANNELFNC_THERMOMETER) + " WHERE id=" +
              std::to_string(resource)).c_str(), &prepared);
    ASSERT_GT(source, 0);
    ASSERT_GT(destination, source);
    ASSERT_GT(resource, 0u);
    auto dao = factory();
    ASSERT_TRUE(dao->accept_identity(source, {123, {resource}}));
    int destination_channel = 0;
    ASSERT_TRUE(db.stmt_get_int(
        &stmt, &destination_channel, nullptr, nullptr, nullptr,
        ("SELECT MIN(id) FROM supla_dev_channel WHERE iodevice_id=" +
         std::to_string(destination)).c_str(), nullptr, 0, true));
    ASSERT_TRUE(dao->accept_identity(destination,
                                    {456, {uint32_t(destination_channel)}}));
  }
  void manual_schema() {
    runSqlScript("SupLanM5TestSchema.sql");
    runSqlScript("SupLanM7TestSchema.sql");
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
  std::vector<approval_decision_t> decisions() {
    auto repo = factory();
    std::vector<approval_pair_t> pairs;
    EXPECT_TRUE(repo->approval_pairs(resource, destination, &pairs));
    std::vector<approval_decision_t> rows;
    if (!pairs.empty()) {
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
  int commit_cloud_decision(const approval_decision_t &snapshot,
                            const char *status) {
    // Trusted fixture-side Cloud transaction. Core never authors this Grant.
    // Tests exercise the real migrated procedures, not Cloud PHP validation/UI.
    auto repo = factory();
    channel_info_t channel;
    device_info_t from, to;
    if (!repo->channel(resource, &channel) || !repo->device(source, &from) ||
        !repo->device(destination, &to)) return 0;
    bool peer = !channel.is_virtual && !from.is_virtual &&
        from.protocol >= 29 && to.protocol >= 29 &&
        (from.flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) &&
        (to.flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED);
    std::string grants = "[]";
    if (std::string(status) == "ACCEPTED") {
      grants = "[{\"source\":" + std::to_string(source) +
          ",\"destination\":" + std::to_string(destination) +
          ",\"resource\":" + std::to_string(resource) +
          ",\"permissions\":" + std::to_string(snapshot.permissions) +
          ",\"type\":" + std::to_string(channel.type) +
          ",\"function\":" + std::to_string(channel.function) +
          ",\"peerEligible\":" + std::to_string(peer) +
          ",\"requiresPeer\":0}]";
    }
    supla_mariadb_access_provider db;
    if (!db.connect()) return 0;
    auto execute = [&](const std::string &sql, bool *changed = nullptr) {
      MYSQL_STMT *statement = nullptr;
      if (!db.stmt_execute(reinterpret_cast<void **>(&statement), sql.c_str(),
                            nullptr, 0, true)) return false;
      if (changed) *changed = mysql_stmt_affected_rows(statement) == 1;
      int result;
      do {
        mysql_stmt_free_result(statement);
        result = mysql_stmt_next_result(statement);
      } while (result == 0);
      mysql_stmt_close(statement);
      return result == -1;
    };
    int user = 0;
    void *statement = nullptr;
    EXPECT_TRUE(db.stmt_get_int(&statement, &user, nullptr, nullptr, nullptr,
        ("SELECT user_id FROM supla_iodevice WHERE id=" +
         std::to_string(destination)).c_str(), nullptr, 0, true));
    bool changed = false;
    if (!execute("START TRANSACTION") ||
        !execute("SELECT id INTO @fixture_account FROM supla_user WHERE id=" +
                 std::to_string(user) + " FOR UPDATE") || !execute(
        "UPDATE supla_resource_access_decision SET status='" +
        std::string(status) + "',open_pair_id=" +
        (std::string(status) == "REJECTED" ? "pair_id" : "NULL") +
        " WHERE id=" + std::to_string(snapshot.id) + " AND revision=" +
        std::to_string(snapshot.revision) + " AND permissions=" +
        std::to_string(snapshot.permissions) + " AND status<>'REMOVED'",
        &changed) || !changed) {
      execute("ROLLBACK");
      return 0;
    }
    bool ok = execute("CALL supla_resource_access_replace(" +
        std::to_string(user) + ",2," + std::to_string(snapshot.id) +
        ",'" + grants + "')");
    // A recorded PEER demand projects the already committed business rights.
    // STREAM-only approvals deliberately leave no physical association.
    if (ok && peer && snapshot.delivery_mode == 1 && grants != "[]") {
      ok = execute("CALL supla_suplan_project(" + std::to_string(user) + "," +
          std::to_string(source) + "," + std::to_string(destination) + ",1)");
    }
    if (!ok || !execute("COMMIT")) {
      execute("ROLLBACK");
      return 0;
    }
    return 1;
  }
  association_t state() {
    association_t a;
    auto dao = factory();
    EXPECT_TRUE(dao->find(source, destination, &a));
    return a;
  }
};

TEST_F(SupLanPeerDaoIntegrationTest, OwnershipVerdictSeparatesQueryFailure) {
  auto dao = factory();
  acl_t acl{{SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, resource,
             SUPLA_SUPLAN_PERMISSION_READ}};
  bool owned = false;
  ASSERT_TRUE(dao->check_acl_ownership(source, destination, acl, &owned));
  ASSERT_TRUE(owned);
  peer_dao other_account(1);
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

// Exercise production IPC dispatch, including the static user lookup.
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
    query("UPDATE supla_suplan_device_state SET user_id=99124 WHERE user_id=2");
    factory = [] { return std::make_unique<peer_dao>(99124); };
    service = peer_service(factory, nullptr);
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
    ASSERT_TRUE(factory()->accept_identity(destination, {789, {hvac, local}}));
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

  void with_destination(
      const std::function<void(supla_device_channel *)> &visit,
      int flags = SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) {
    supla_connection_test_access protocol;
    ASSERT_TRUE(protocol.open());
    testing::DeviceStub device(protocol.get());
    device.set_id(destination);
    device.set_flags(flags);
    device.set_user(supla_user::find(99124, true));
    // Simulate the normal registration boundary after this fixture's trusted
    // SQL edits (the fixture intentionally resets DB while Users stay alive).
    device.get_user()->get_relation_index()->invalidate();
    device.get_user()->get_relation_index()->refresh();
    supla_mariadb_access_provider db;
    ASSERT_TRUE(db.connect());
    supla_device_dao dao(&db);
    device.set_channels(
        new supla_device_channels(&dao, &device, nullptr, nullptr, 0));
    device.get_channels()->access_channel(hvac, visit);
  }
};

TEST_F(SupLanHvacIntegrationTest,
       RemoteThermometerDeviceRelationsFromBothEnds) {
  // DeviceId and ChannelId are separate namespaces: deliberately equal here.
  query("INSERT INTO supla_iodevice "
        "(id,location_id,user_id,guid,enabled,reg_date,protocol_version,flags) "
        "SELECT " + std::to_string(hvac) + ",location_id,user_id,"
        "UNHEX(MD5('hvac-relation-source')),enabled,reg_date,protocol_version,"
        "flags FROM supla_iodevice WHERE id=" + std::to_string(source));
  source = hvac;
  query("UPDATE supla_dev_channel SET iodevice_id=" + std::to_string(source) +
        " WHERE id=" + std::to_string(resource));
  auto aux = add_channel(source, 242, SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR,
                         SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE);
  query("UPDATE supla_dev_channel SET user_config=JSON_OBJECT("
        "'mainThermometerChannelId'," + std::to_string(resource) +
        ",'auxThermometerChannelId'," + std::to_string(aux) +
        ",'auxThermometerType','FLOOR') WHERE id=" + std::to_string(hvac));
  supla_connection_test_access protocol;
  ASSERT_TRUE(protocol.open());
  auto user = supla_user::find(99124, true);
  supla_mariadb_access_provider db;
  ASSERT_TRUE(db.connect());
  supla_device_dao dao(&db);
  auto make_device = [&](int id, supla_connection *connection) {
    auto device = std::make_shared<testing::DeviceStub>(connection);
    device->set_id(id);
    device->set_user(user);
    char guid[SUPLA_GUID_SIZE] = {};
    memcpy(guid, &id, sizeof(id));
    device->set_guid(guid);
    device->set_channels(new supla_device_channels(
        &dao, device.get(), nullptr, nullptr, 0));
    EXPECT_TRUE(user->get_devices()->add(device, nullptr));
    return device;
  };
  auto thermostat = make_device(destination, protocol.get());
  auto thermometer = make_device(source, protocol.get());
  user->get_relation_index()->invalidate();
  user->get_relation_index()->refresh();
  auto children = thermostat->get_channels()->get_channel_relations(
      hvac, relation_with_sub_channel);
  ASSERT_EQ(2u, children.size());
  EXPECT_EQ(resource, children[0].get_id());
  EXPECT_EQ(CHANNEL_RELATION_TYPE_MAIN_TERMOMETER,
            children[0].get_relation_type());
  EXPECT_EQ(aux, children[1].get_id());
  EXPECT_EQ(CHANNEL_RELATION_TYPE_AUX_THERMOMETER_FLOOR,
            children[1].get_relation_type());
  for (auto id : {resource, aux}) {
    auto parents = thermometer->get_channels()->get_channel_relations(
        id, relation_with_parent_channel);
    ASSERT_EQ(1u, parents.size());
    EXPECT_EQ(id, parents[0].get_id());
    EXPECT_EQ(hvac, parents[0].get_parent_id());
  }
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
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
}

TEST_F(SupLanHvacIntegrationTest,
       DeviceUploadProtectedMergeAndSpoofCannotAuthorize) {
  for (auto current : {0u, local, resource}) {
    config(current);
    cloud_grant(99124, channel_config_origin(hvac, 1), source, destination,
                resource, current == resource ? 1 : 0);
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
      EXPECT_EQ(current == resource ? 1 : 0,
                scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
      // Restore the trusted business state for each independent matrix cell.
      config(current);
      }
  }
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
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
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
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
  config(resource);

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
    EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
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
    EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
    query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
          std::to_string(hvac));
  }
}

TEST_F(SupLanHvacIntegrationTest,
       Adr016PlainRegistrationHasNoDatabaseSideEffects) {
  query("UPDATE supla_iodevice SET guid=UNHEX(REPEAT('ab',16)) WHERE id=" +
        std::to_string(destination));
  for (unsigned char guid_byte : {0xab, 0xcd}) {
    supla_connection_test_access protocol;
    ASSERT_TRUE(protocol.open());
    ASSERT_FALSE(protocol.get()->is_secure());
    auto device = std::make_shared<testing::DeviceStub>(protocol.get());
    testing::SrpcAdapterMock adapter;
    EXPECT_CALL(adapter, sd_async_registerdevice_result(testing::_))
        .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
          EXPECT_EQ(SUPLA_RESULTCODE_TLS_REQUIRED, result->result_code);
          EXPECT_EQ(20, result->activity_timeout);
          EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
          EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
          return 1;
        });
    TDS_SuplaRegisterDevice_G registration{};
    memset(registration.GUID, guid_byte, sizeof(registration.GUID));
    registration.AuthKey[0] = 1;
    registration.Flags = SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED;
    snprintf(registration.Email, sizeof(registration.Email), "%s",
             "suplan-hvac@test.invalid");
    // Existing and unknown GUIDs must be rejected before even opening SQL.
    Sql004Probe probe;
    supla_register_device registrar;
    registrar.register_device(device, nullptr, &registration, &adapter,
                               0, 0, 20);
    EXPECT_EQ(0u, probe.connections);
    EXPECT_EQ(0u, probe.executions);
    EXPECT_FALSE(device->is_registered());
    EXPECT_EQ(nullptr, device->get_user());
    EXPECT_EQ(0u, device->get_suplan_root_epoch());
    EXPECT_EQ(nullptr, device->get_stream(false));
    probe.finish("adr016-plain-registration");
  }
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
       SharedDatabaseVersionCheckRequiresCloudGrantMigration) {
  EXPECT_STREQ("20261016120000", DB_VERSION);
  query("DELETE FROM migration_versions");
  query("INSERT INTO migration_versions(version) "
        "VALUES('Version20261016120000')");
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
// dispatch, publication and SRPC run in production code.
class SupLanHvacWireIntegrationTest : public SupLanHvacIntegrationTest {
 protected:
  void *listener = nullptr, *worker = nullptr, *wire = nullptr;
  int client = -1;
  SSL *client_tls = nullptr;
  bool secure = true;
  int extra_device_flags = 0;
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
    query(
        "UPDATE supla_suplan_device_state SET user_id=99126 WHERE "
        "user_id=99124");
    factory = [] { return std::make_unique<peer_dao>(99126); };
    service = peer_service(factory, nullptr);
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

  void config(uint32_t selected) {
    SupLanHvacIntegrationTest::config(selected);
  }

  void start_connection() {
    listener = ssocket_server_init(
        secure ? "../../test/cert/cert.crt" : nullptr,
        secure ? "../../test/cert/private.key" : nullptr, 0, secure ? 1 : 0);
    ASSERT_NE(nullptr, listener);
    ASSERT_TRUE(ssocket_openlistener(listener));
    sockaddr_in address = {};
    socklen_t size = sizeof(address);
    ASSERT_EQ(0, getsockname(ssocket_get_fd(listener),
                            reinterpret_cast<sockaddr *>(&address), &size));
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    client = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(client, 0);
    if (getenv("SUPLA_HVAC_PERF")) {
      int enabled = 1;
      ASSERT_EQ(0, setsockopt(client, IPPROTO_TCP, TCP_NODELAY,
                              &enabled, sizeof(enabled)));
    }
    ASSERT_EQ(0, connect(client, reinterpret_cast<sockaddr *>(&address), size));
    timeval timeout = {2, 0};
    ASSERT_EQ(0, setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                            sizeof(timeout)));
    ASSERT_EQ(0, setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout,
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
    if (secure) {
      auto context = SSL_CTX_new(TLS_client_method());
      ASSERT_NE(nullptr, context);
      // Dedicated repository test certificate; not a product trust override.
      SSL_CTX_set_verify(context, SSL_VERIFY_NONE, nullptr);
      client_tls = SSL_new(context);
      SSL_CTX_free(context);
      ASSERT_NE(nullptr, client_tls);
      SSL_set_fd(client_tls, client);
      ASSERT_EQ(1, SSL_connect(client_tls));
      ASSERT_TRUE(connection->is_secure());
    }
  }

  void close_connection() {
    if (worker) {
      sthread_terminate(worker, false);
      connection->raise_event();
      sthread_wait(worker);
      sthread_free(worker);
    }
    delete connection;
    if (client_tls) SSL_free(client_tls);
    client_tls = nullptr;
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
    SSL *client_tls;
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
          client_tls(fixture->client_tls),
          adapter(fixture->publication_adapter) {
      fixture->listener = fixture->worker = fixture->wire = nullptr;
      fixture->client = -1;
      fixture->client_tls = nullptr;
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
      std::swap(client_tls, fixture->client_tls);
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
        int n = client_tls ? SSL_write(client_tls, bytes + sent, count - sent)
                           : send(client, bytes + sent, count - sent,
                                  MSG_NOSIGNAL);
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
      if ((!client_tls || !SSL_pending(client_tls)) &&
          poll(&fd, 1, left) <= 0) return false;
      char bytes[4096];
      int n = client_tls ? SSL_read(client_tls, bytes, sizeof(bytes))
                         : recv(client, bytes, sizeof(bytes), MSG_DONTWAIT);
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
  void config_delivery_with_unrelated_peer_failure(bool weekly) {
    register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                    weekly);
    ASSERT_FALSE(HasFatalFailure());
    auto user = supla_user::find(99126, false);
    ASSERT_NE(nullptr, user);
    ASSERT_TRUE(user->get_suplan_peers()->drain_changes());
    // Another consumer's committed grant has a dirty peer whose Source map
    // no longer contains its resource. Config reads/writes remain healthy.
    auto consumer = add_channel(destination, 239, SUPLA_CHANNELTYPE_HVAC,
                                SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
    query("UPDATE supla_dev_channel SET user_id=99126 WHERE id=" +
          std::to_string(consumer));
    cloud_grant(99126, channel_config_origin(consumer, 1), source, destination,
                resource, 1);
    ASSERT_TRUE(factory()->accept_identity(source, {123, {}}));
    peer_dao repo(99126);
    std::vector<std::pair<uint64_t, uint64_t>> before, after;
    ASSERT_TRUE(repo.changed_pairs(0, 64, &before));
    ASSERT_EQ(1u, before.size());
    if (weekly) {
      TChannelConfig_WeeklySchedule schedule{};
      schedule.Quarters[0] = 0x22;
      schedule.Program[0].Mode = SUPLA_HVAC_MODE_HEAT;
      weekly_schedule_config config;
      config.set_config(&schedule, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
      supla_mariadb_access_provider db;
      supla_device_dao dao(&db);
      ASSERT_TRUE(dao.set_channel_config(99126, hvac, &config));
    } else {
      query("UPDATE supla_dev_channel SET user_config='{\"minOnTimeS\":23}' "
            "WHERE id=" + std::to_string(hvac));
    }
    unsigned long long scope = weekly ? CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE
                                      : CONFIG_CHANGE_SCOPE_JSON_DEFAULT;
    testing::IpcSocketAdapterMock socket(-1);
    EXPECT_CALL(socket, send_data(std::string("ERROR:99126\n"))).Times(1);
    supla_on_channel_config_changed_command command(&socket);
    auto input = "USER-ON-CHANNEL-CONFIG-CHANGED:99126," +
        std::to_string(destination) + "," + std::to_string(hvac) + "," +
        std::to_string(publication_type) + "," +
        std::to_string(publication_function) + "," + std::to_string(scope) +
        "\n";
    std::vector<char> buffer(input.begin(), input.end());
    buffer.push_back(0);
    ASSERT_TRUE(command.process_command(buffer.data(), buffer.size(),
                                        input.size()));
    ASSERT_TRUE(repo.changed_pairs(0, 64, &after));
    EXPECT_EQ(before, after);  // Failed handoff must not consume the marker.
    TSuplaDataPacket packet{};
    ASSERT_TRUE(receive_packet(&packet));
    ASSERT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packet.call_id);
    TSDS_SetChannelConfig config{};
    memcpy(&config, packet.data, packet.data_size);
    if (weekly) {
      ASSERT_EQ(SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE, config.ConfigType);
      ASSERT_EQ(sizeof(TChannelConfig_WeeklySchedule), config.ConfigSize);
      EXPECT_EQ(0x22, reinterpret_cast<TChannelConfig_WeeklySchedule *>(
                          config.Config)->Quarters[0]);
    } else {
      ASSERT_EQ(SUPLA_CONFIG_TYPE_DEFAULT, config.ConfigType);
      ASSERT_EQ(sizeof(TChannelConfig_HVAC), config.ConfigSize);
      EXPECT_EQ(23, reinterpret_cast<TChannelConfig_HVAC *>(
                        config.Config)->MinOnTimeS);
    }
    EXPECT_EQ(240, config.ChannelNumber);
    acknowledge(config);
    EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
    // A repeated handoff still fails: recovery must retry this marker,
    // rather than skip it and return success after an empty pagination tail.
    EXPECT_FALSE(user->get_suplan_peers()->drain_changes());
    ASSERT_TRUE(repo.changed_pairs(0, 64, &after));
    EXPECT_EQ(before, after);
  }

  void register_family(int type, int function, bool weekly = false) {
    publication_type = type;
    publication_function = function;
    query("UPDATE supla_dev_channel SET type=" + std::to_string(type) +
          ",func=" + std::to_string(function) +
          ",flist=0,user_config='{}' WHERE id=" + std::to_string(hvac));
    register_existing_family(type, function, function, weekly);
  }

  void begin_registration(int type, int advertised_function,
                          bool weekly = false, bool suplan = true) {
    TDS_SuplaRegisterDevice_G registration = {};
    snprintf(registration.Email, sizeof(registration.Email), "%s",
             "hvac-wire@test.invalid");
    memset(registration.GUID, 0xab, sizeof(registration.GUID));
    registration.AuthKey[0] = 1;
    registration.Flags = (suplan ? SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED : 0) |
                         SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED |
                         extra_device_flags;
    registration.channel_count = 1;
    auto &channel = registration.channels[0];
    channel.Number = 240;
    channel.Type = type;
    channel.Default = advertised_function;
    channel.Flags = SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE |
                    (weekly ? SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE : 0);
    send_packet(SUPLA_DS_CALL_REGISTER_DEVICE_G, &registration,
                sizeof(TDS_SuplaRegisterDeviceHeader) + sizeof(channel));
  }

  void register_existing_family(int type, int advertised_function,
                                int selected_function, bool weekly = false,
                                bool suplan = true,
                                std::function<void(const TSuplaDataPacket &)>
                                    observe = {}) {
    begin_registration(type, advertised_function, weekly, suplan);
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

  void weekly_ipc_publication(int type, int function, unsigned char mode) {
    register_family(type, function, true);
    ASSERT_FALSE(HasFatalFailure());
    TChannelConfig_WeeklySchedule weekly = {};
    weekly.Quarters[0] = 0x11;
    weekly.Program[0].Mode = mode;
    weekly_schedule_config stored;
    stored.set_config(&weekly, function);
    supla_mariadb_access_provider db;
    supla_device_dao dao(&db);
    ASSERT_TRUE(dao.set_channel_config(99126, hvac, &stored));
    {
      Sql004Probe probe;
      notify(CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE);
      // One mutation read; publication performs no second config read.
      EXPECT_EQ(1u, probe.config_reads);
    }
    TSuplaDataPacket packet = {};
    ASSERT_TRUE(receive_packet(&packet));
    ASSERT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packet.call_id);
    TSDS_SetChannelConfig published = {};
    memcpy(&published, packet.data, packet.data_size);
    EXPECT_EQ(SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE, published.ConfigType);
    ASSERT_EQ(sizeof(weekly), published.ConfigSize);
    EXPECT_EQ(0, memcmp(&weekly, published.Config, sizeof(weekly)));
    acknowledge(published);
    auto target =
        supla_user::find(99126, false)->get_devices()->get(destination);
    ASSERT_NE(nullptr, target);
    EXPECT_EQ(789u, target->get_suplan_root_epoch());
    target->get_channels()->access_channel(hvac, [&](supla_device_channel *c) {
      Sql004Probe probe;
      for (unsigned n = 0; n < 3; ++n) {
        TSD_ChannelConfig config = {};
        ASSERT_TRUE(
            c->get_config(&config, SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE, 0));
        ASSERT_EQ(sizeof(weekly), config.ConfigSize);
        EXPECT_EQ(0, memcmp(&weekly, config.Config, sizeof(weekly)));
      }
      EXPECT_EQ(0u, probe.config_reads);
      EXPECT_EQ(0u, probe.selects);
    });
  }

  void config_writer_then_cloud(bool from_device, unsigned char cloud_type) {
    register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                    true);
    ASSERT_FALSE(HasFatalFailure());
    auto user = supla_user::find(99126, false);
    auto target = user->get_devices()->get(destination);
    ASSERT_NE(nullptr, target);
    supla_device_channel *channel = nullptr;
    target->get_channels()->access_channel(
        hvac, [&](supla_device_channel *c) { channel = c; });
    ASSERT_NE(nullptr, channel);
    auto api = std::make_shared<M4ConfigClient>(connection, user);
    char value[SUPLA_CHANNELVALUE_SIZE] = {};
    ASSERT_TRUE(api->get_channels()->add(new supla_client_channel(
        api->get_channels(), hvac, 240, destination, 0, SUPLA_CHANNELTYPE_HVAC,
        SUPLA_CHANNELFNC_HVAC_THERMOSTAT, 0, 0, 0, 0, nullptr, nullptr, nullptr,
        "", 0, 0, 0, 0, 29, SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE, value, 0, "{}",
        "{}", false)));
    TChannelConfig_WeeklySchedule weekly = {};
    weekly.Quarters[0] = 0x22;
    TSCS_ChannelConfig client_request = {};
    client_request.ChannelId = hvac;
    client_request.ConfigType = SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE;
    client_request.ConfigSize = sizeof(weekly);
    memcpy(client_request.Config, &weekly, sizeof(weekly));
    TSDS_SetChannelConfig device_request = {};
    device_request.ChannelNumber = 240;
    device_request.ConfigType = client_request.ConfigType;
    device_request.ConfigSize = sizeof(weekly);
    memcpy(device_request.Config, &weekly, sizeof(weekly));
    std::promise<void> selected, release;
    auto selected_future = selected.get_future();
    auto released = release.get_future();
    auto writer = std::async(std::launch::async, [&] {
      Sql004Probe probe;
      probe.after_config = [&](unsigned reads) {
        // First SELECT precedes CAS; second reads the committed merged root.
        if (reads == 2) {
          selected.set_value();
          released.wait();
        }
        return false;
      };
      testing::SrpcAdapterMock reply;
      TsrpcReceivedData rd = {};
      if (from_device) {
        rd.data.sds_set_channel_config_request = &device_request;
        EXPECT_CALL(reply, sd_async_set_channel_config_result(testing::_))
            .WillOnce([](TSDS_SetChannelConfigResult *r) {
              EXPECT_EQ(SUPLA_CONFIG_RESULT_TRUE, r->Result);
              return 1;
            });
        EXPECT_TRUE(target->get_srpc_call_handler_collection()->handle_call(
            target, &reply, &rd, SUPLA_DS_CALL_SET_CHANNEL_CONFIG, 29));
      } else {
        rd.data.scs_channel_config = &client_request;
        EXPECT_CALL(reply, sc_async_channel_config_update_or_result(testing::_))
            .WillOnce([](TSC_ChannelConfigUpdateOrResult *r) {
              EXPECT_EQ(SUPLA_CONFIG_RESULT_TRUE, r->Result);
              return 1;
            });
        EXPECT_TRUE(api->get_srpc_call_handler_collection()->handle_call(
            api, &reply, &rd, SUPLA_CS_CALL_SET_CHANNEL_CONFIG, 29));
      }
      EXPECT_EQ(2u, probe.config_reads);
    });
    if (selected_future.wait_for(std::chrono::seconds(2)) !=
        std::future_status::ready) {
      release.set_value();
      writer.get();
      FAIL() << "Writer did not reach merged-root SELECT";
    }
    // Deterministic ordering check, from another thread than the writer.
    bool unlocked = channel->get_config_mutex().try_lock();
    if (unlocked) channel->get_config_mutex().unlock();
    EXPECT_FALSE(unlocked) << "DB snapshot must remain fenced until cached";
    // Ordinary cached reads do not take the producer lock or issue SQL.
    {
      Sql004Probe probe;
      TSD_ChannelConfig cached = {};
      EXPECT_TRUE(channel->get_config(&cached, SUPLA_CONFIG_TYPE_DEFAULT, 0));
      EXPECT_EQ(0u, probe.selects);
    }
    // A separate trusted Cloud-like transaction is not blocked by Core's lock.
    unsigned long long scope = CONFIG_CHANGE_SCOPE_JSON_DEFAULT;
    if (cloud_type == SUPLA_CONFIG_TYPE_DEFAULT) {
      query(
          "UPDATE supla_dev_channel SET user_config="
          "JSON_SET(user_config,'$.minOnTimeS',23) WHERE id=" +
          std::to_string(hvac));
    } else {
      const char *key = cloud_type == SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE
                            ? "weeklySchedule"
                            : "altWeeklySchedule";
      query(
          "UPDATE supla_dev_channel SET user_config="
          "JSON_SET(user_config,'$." +
          std::string(key) +
          "',JSON_OBJECT('quarters',JSON_ARRAY(3,3))) WHERE id=" +
          std::to_string(hvac));
      scope = cloud_type == SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE
                  ? CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE
                  : CONFIG_CHANGE_SCOPE_JSON_ALT_WEEKLY_SCHEDULE;
    }
    auto ipc = std::async(std::launch::async, [&] {
      Sql004Probe probe;
      probe.after_config = [&](unsigned) {
        TSD_ChannelConfig cached = {};
        EXPECT_TRUE(
            channel->get_config(&cached, SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE, 0));
        EXPECT_EQ(sizeof(weekly), cached.ConfigSize);
        EXPECT_EQ(0, memcmp(&weekly, cached.Config, sizeof(weekly)))
            << "IPC must read only after the preceding writer installs cache";
        return false;
      };
      notify(scope);
      EXPECT_EQ(1u, probe.config_reads);
    });
    release.set_value();
    writer.get();
    ipc.get();
    // A delayed hint for the earlier weekly edit carries no old JSON: it must
    // reload the latest root, including Cloud's newer default/weekly/alt edit.
    {
      Sql004Probe probe;
      notify(CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE);
      EXPECT_EQ(1u, probe.config_reads);
    }
    unlocked = channel->get_config_mutex().try_lock();
    EXPECT_TRUE(unlocked);
    if (unlocked) channel->get_config_mutex().unlock();
    supla_mariadb_access_provider db;
    supla_device_dao dao(&db);
    std::unique_ptr<supla_json_config> root(
        dao.get_channel_config(hvac, nullptr, nullptr));
    ASSERT_NE(nullptr, root);
    std::unique_ptr<supla_json_config> cached_root(channel->get_json_config());
    char *authoritative_json = root->get_user_config();
    char *cached_json = cached_root->get_user_config();
    EXPECT_STREQ(authoritative_json, cached_json);
    free(authoritative_json);
    free(cached_json);
    for (auto type :
         {SUPLA_CONFIG_TYPE_DEFAULT, SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE,
          SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE}) {
      // Compare fresh DB serialization with cached and real SRPC GET bytes.
      TSD_ChannelConfig cached = {};
      {
        Sql004Probe probe;
        ASSERT_TRUE(channel->get_config(&cached, type, 0));
        EXPECT_EQ(0u, probe.selects);
      }
      if (type == cloud_type && type == SUPLA_CONFIG_TYPE_DEFAULT) {
        EXPECT_EQ(
            23,
            reinterpret_cast<TChannelConfig_HVAC *>(cached.Config)->MinOnTimeS);
      } else if (type == cloud_type) {
        EXPECT_EQ(0x33, reinterpret_cast<TChannelConfig_WeeklySchedule *>(
                            cached.Config)
                            ->Quarters[0]);
      }
      TDS_GetChannelConfigRequest get = {};
      get.ChannelNumber = 240;
      get.ConfigType = type;
      send_packet(SUPLA_DS_CALL_GET_CHANNEL_CONFIG, &get, sizeof(get));
      TSuplaDataPacket packet = {};
      bool received = false;
      for (unsigned n = 0; n < 10 && !received; ++n) {
        ASSERT_TRUE(receive_packet(&packet));
        if (packet.call_id == SUPLA_SD_CALL_SET_CHANNEL_CONFIG) {
          TSDS_SetChannelConfig pushed = {};
          memcpy(&pushed, packet.data, packet.data_size);
          acknowledge(pushed);
        } else if (packet.call_id == SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT) {
          TSD_ChannelConfig actual = {};
          memcpy(&actual, packet.data, packet.data_size);
          EXPECT_EQ(cached.Func, actual.Func);
          EXPECT_EQ(cached.ConfigSize, actual.ConfigSize);
          EXPECT_EQ(0, memcmp(cached.Config, actual.Config, cached.ConfigSize));
          received = true;
        }
      }
      EXPECT_TRUE(received);
    }
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
    notify(CONFIG_CHANGE_SCOPE_JSON_DEFAULT);
    TSuplaDataPacket initial = {};
    ASSERT_TRUE(receive_packet(&initial));
    ASSERT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, initial.call_id);
    TSDS_SetChannelConfig established = {};
    memcpy(&established, initial.data, initial.data_size);
    acknowledge(established);
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
    // It updates the cache before queuing an owner-thread publication.
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

TEST_F(SupLanHvacWireIntegrationTest, ClientWeeklyThenCloudDefaultKeepsBoth) {
  config_writer_then_cloud(false, SUPLA_CONFIG_TYPE_DEFAULT);
}
TEST_F(SupLanHvacWireIntegrationTest, ClientWeeklyThenCloudWeeklyKeepsLatest) {
  config_writer_then_cloud(false, SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE);
}
TEST_F(SupLanHvacWireIntegrationTest, ClientWeeklyThenCloudAltKeepsBoth) {
  config_writer_then_cloud(false, SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE);
}
TEST_F(SupLanHvacWireIntegrationTest, DeviceWeeklyThenCloudDefaultKeepsBoth) {
  config_writer_then_cloud(true, SUPLA_CONFIG_TYPE_DEFAULT);
}

TEST_F(SupLanHvacWireIntegrationTest,
       QuietIdentityBootstrapDisconnectsAtDeadline) {
  begin_registration(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  bool registered = false, identities = false;
  TSuplaDataPacket packet = {};
  for (unsigned n = 0; n < 10 && !identities; ++n) {
    ASSERT_TRUE(receive_packet(&packet));
    if (packet.call_id == SUPLA_SD_CALL_REGISTER_DEVICE_RESULT ||
        packet.call_id == SUPLA_SD_CALL_REGISTER_DEVICE_RESULT_B) {
      TSD_SuplaRegisterDeviceResult result = {};
      memcpy(&result, packet.data, sizeof(result));
      ASSERT_EQ(SUPLA_RESULTCODE_TRUE, result.result_code);
      registered = true;
    } else if (packet.call_id == SUPLA_SD_CALL_SUPLAN_DEVICE_IDENTITIES) {
      identities = true;
    } else if (packet.call_id == SUPLA_SD_CALL_SET_CHANNEL_CONFIG ||
               packet.call_id == SUPLA_SD_CALL_DEVICE_SYNC_DONE) {
      FAIL() << "Configuration must wait for identity confirmation";
    }
  }
  ASSERT_TRUE(registered && identities);
  auto target = supla_user::find(99126, false)->get_devices()->get(destination);
  ASSERT_NE(nullptr, target);
  EXPECT_LE(target->wait_time_usec(), 15000001u);
  // No Result, ping, manual iterate or injected timer: the real connection
  // loop must close the otherwise quiet TLS session at the bootstrap deadline.
  EXPECT_FALSE(receive_packet(&packet, 17000));
  EXPECT_TRUE(sthread_isterminated(worker));
  EXPECT_EQ(0u, target->get_suplan_root_epoch());
}

TEST_F(SupLanHvacWireIntegrationTest, DeviceConfigEnqueueFailureIsNotSuccess) {
  extra_device_flags = SUPLA_DEVICE_FLAG_DEVICE_CONFIG_SUPPORTED;
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  auto target = supla_user::find(99126, false)->get_devices()->get(destination);
  ASSERT_NE(nullptr, target);
  publication_adapter->fail_device_config = true;
  EXPECT_FALSE(target->send_config_to_device());
  EXPECT_FALSE(publication_adapter->fail_device_config.load());
}

TEST_F(SupLanHvacWireIntegrationTest, PeerCredentialsNeverUsePlainConnection) {
  close_connection();
  secure = false;
  start_connection();
  ASSERT_FALSE(HasFatalFailure());
  begin_registration(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  TSuplaDataPacket packet{};
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_REGISTER_DEVICE_RESULT, packet.call_id);
  ASSERT_EQ(sizeof(TSD_SuplaRegisterDeviceResult), packet.data_size);
  TSD_SuplaRegisterDeviceResult result{};
  memcpy(&result, packet.data, sizeof(result));
  EXPECT_EQ(SUPLA_RESULTCODE_TLS_REQUIRED, result.result_code);
  auto rejected =
      std::dynamic_pointer_cast<supla_device>(connection->get_object());
  ASSERT_NE(nullptr, rejected);
  EXPECT_FALSE(rejected->is_registered());
  EXPECT_FALSE(rejected->is_server_stream_ready());
  EXPECT_EQ(0u, rejected->get_suplan_root_epoch());
  EXPECT_EQ(nullptr, rejected->get_stream(false));
  ASSERT_FALSE(connection->is_secure());
  auto peers = supla_user::find(99126, true)->get_suplan_peers();
  EXPECT_FALSE(peers->ready(destination));
  TSDS_SuplaSetSuplanSourceAssociation source_request = {};
  source_request.Flags = SUPLA_SUPLAN_SOURCE_FLAG_RETURN_PEER_KEY;
  EXPECT_EQ(peer_transport::send_code::not_ready,
            peers->send(destination, &source_request).code);
  TSDS_SuplaSetSuplanDestinationAssociation destination_request = {};
  destination_request.PeerKeySize = SUPLA_SUPLAN_PEER_KEY_SIZE;
  memset(destination_request.PeerKey, 0xab,
         sizeof(destination_request.PeerKey));
  EXPECT_EQ(peer_transport::send_code::not_ready,
            peers->send(destination, &destination_request).code);
}

TEST_F(SupLanHvacWireIntegrationTest,
       Adr016AllFlaggedRegistrationVariantsRequireTls) {
  close_connection();
  secure = false;
  for (unsigned int call : {SUPLA_DS_CALL_REGISTER_DEVICE_E,
                            SUPLA_DS_CALL_REGISTER_DEVICE_F,
                            SUPLA_DS_CALL_REGISTER_DEVICE_G}) {
    start_connection();
    ASSERT_FALSE(HasFatalFailure());
    TDS_SuplaRegisterDeviceHeader registration{};
    memset(registration.GUID, 0xab, sizeof(registration.GUID));
    registration.AuthKey[0] = 1;
    registration.Flags = SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED;
    snprintf(registration.Email, sizeof(registration.Email), "%s",
             "hvac-wire@test.invalid");
    // E/F/G use this same header. Channel count zero deliberately requires
    // the transport rejection to precede normal registration validation.
    send_packet(call, &registration, sizeof(registration));
    TSuplaDataPacket packet{};
    ASSERT_TRUE(receive_packet(&packet));
    ASSERT_EQ(SUPLA_SD_CALL_REGISTER_DEVICE_RESULT, packet.call_id);
    ASSERT_EQ(sizeof(TSD_SuplaRegisterDeviceResult), packet.data_size);
    TSD_SuplaRegisterDeviceResult result{};
    memcpy(&result, packet.data, sizeof(result));
    EXPECT_EQ(SUPLA_RESULTCODE_TLS_REQUIRED, result.result_code);
    EXPECT_EQ(SUPLA_PROTO_VERSION, result.version);
    EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result.version_min);
    auto rejected = connection->get_object();
    ASSERT_NE(nullptr, rejected);
    EXPECT_FALSE(rejected->is_registered());
    rejected.reset();
    close_connection();
  }
}

TEST_F(SupLanHvacWireIntegrationTest,
       Adr016PlainLegacyRegistrationCannotUpgradeToSupLan) {
  close_connection();
  secure = false;
  start_connection();
  ASSERT_FALSE(HasFatalFailure());
  register_existing_family(SUPLA_CHANNELTYPE_HVAC,
                            SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                            SUPLA_CHANNELFNC_HVAC_THERMOSTAT, false, false);
  ASSERT_FALSE(HasFatalFailure());
  auto target = supla_user::find(99126, false)->get_devices()->get(destination);
  ASSERT_NE(nullptr, target);
  EXPECT_TRUE(target->is_registered());
  EXPECT_FALSE(target->get_flags() & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED);
  auto flags = scalar("SELECT flags FROM supla_iodevice WHERE id=" +
                      std::to_string(destination));
  begin_registration(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  TSuplaDataPacket packet{};
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_REGISTER_DEVICE_RESULT, packet.call_id);
  TSD_SuplaRegisterDeviceResult result{};
  memcpy(&result, packet.data, sizeof(result));
  EXPECT_EQ(SUPLA_RESULTCODE_TLS_REQUIRED, result.result_code);
  EXPECT_FALSE(target->is_suplan_peer_ready());
  EXPECT_FALSE(target->is_server_stream_ready());
  EXPECT_EQ(flags, scalar("SELECT flags FROM supla_iodevice WHERE id=" +
                         std::to_string(destination)));
}

TEST_F(SupLanHvacWireIntegrationTest,
       Adr016FlagClearOnReregistrationClosesExistingStream) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  cloud_grant(99126, channel_config_origin(hvac, 1), source, destination,
              resource, 1);
  expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, 0, 1, 2);
  TSuplaDataPacket packet{};
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  auto target = supla_user::find(99126, false)->get_devices()->get(destination);
  auto stream = target->get_stream(false);
  ASSERT_NE(nullptr, stream);
  ASSERT_EQ(1u, stream->size());
  target.reset();
  begin_registration(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
                      false, false);
  EXPECT_FALSE(receive_packet(&packet, 5000));
  EXPECT_EQ(0u, stream->size());
  stream->dirty(resource);
  stream->iterate();
  EXPECT_EQ(0u, stream->size());
}

TEST_F(SupLanHvacWireIntegrationTest,
       PeerCredentialsUseRegisteredTlsConnection) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  ASSERT_TRUE(connection->is_secure());
  auto peers = supla_user::find(99126, false)->get_suplan_peers();
  ASSERT_TRUE(peers->ready(destination));
  association_t pair;
  pair.source = destination;
  pair.destination = source;
  TSDS_SuplaSetSuplanSourceAssociation source_request = {};
  source_request.PeerContext = context(pair, 789);
  source_request.AclRevision = 1;
  source_request.Flags = SUPLA_SUPLAN_SOURCE_FLAG_RETURN_PEER_KEY;
  ASSERT_EQ(peer_transport::send_code::queued,
            peers->send(destination, &source_request).code);
  TSuplaDataPacket packet = {};
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_SET_SUPLAN_SOURCE_ASSOCIATION, packet.call_id);
  TSDS_SuplaSetSuplanSourceAssociation source_wire = {};
  memcpy(&source_wire, packet.data, packet.data_size);
  EXPECT_EQ(source_request.Flags, source_wire.Flags);
  EXPECT_EQ(789u, source_wire.PeerContext.RootEpoch);
  pair.source = source;
  pair.destination = destination;
  TSDS_SuplaSetSuplanDestinationAssociation request = {};
  request.PeerContext = context(pair, 123);
  request.AclRevision = 1;
  request.PeerKeySize = sizeof(request.PeerKey);
  memset(request.PeerKey, 0xab, sizeof(request.PeerKey));
  ASSERT_EQ(peer_transport::send_code::queued,
            peers->send(destination, &request).code);
  ASSERT_TRUE(receive_packet(&packet));
  ASSERT_EQ(SUPLA_SD_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION, packet.call_id);
  TSDS_SuplaSetSuplanDestinationAssociation actual = {};
  memcpy(&actual, packet.data, packet.data_size);
  EXPECT_EQ(request.PeerKeySize, actual.PeerKeySize);
  EXPECT_EQ(0,
            memcmp(request.PeerKey, actual.PeerKey, sizeof(request.PeerKey)));
}

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
       UnrelatedPeerFailureStillPublishesCurrentHvacConfig) {
  config_delivery_with_unrelated_peer_failure(false);
}
TEST_F(SupLanHvacWireIntegrationTest,
       UnrelatedPeerFailureStillPublishesCurrentWeeklySchedule) {
  config_delivery_with_unrelated_peer_failure(true);
}

TEST_F(SupLanHvacWireIntegrationTest,
       ContainerPausedGetCannotPublishStaleLast) {
  paused_get_then_newer_ipc(true);
}
TEST_F(SupLanHvacWireIntegrationTest, ValvePausedGetCannotPublishStaleLast) {
  paused_get_then_newer_ipc(false);
}
TEST_F(SupLanHvacWireIntegrationTest,
       RelayWeeklyIpcUpdatesCacheWithoutReconnect) {
  weekly_ipc_publication(SUPLA_CHANNELTYPE_RELAY, SUPLA_CHANNELFNC_LIGHTSWITCH,
                         SUPLA_RELAY_MODE_START_ON);
}
TEST_F(SupLanHvacWireIntegrationTest,
       ActionTriggerWeeklyIpcUpdatesCacheWithoutReconnect) {
  weekly_ipc_publication(SUPLA_CHANNELTYPE_ACTIONTRIGGER,
                         SUPLA_CHANNELFNC_ACTIONTRIGGER,
                         SUPLA_BUTTON_MODE_LOCKED);
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
  notify(CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE);
  TSuplaDataPacket initial = {};
  ASSERT_TRUE(receive_packet(&initial));
  ASSERT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, initial.call_id);
  TSDS_SetChannelConfig established = {};
  memcpy(&established, initial.data, initial.data_size);
  acknowledge(established);
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
       AssociationBatchUsesRealSrpcWithoutAdmissionWindow) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  peer_dao repo(99126);
  peer_service previous([] { return std::make_unique<peer_dao>(99126); },
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
    auto consumer = add_channel(destination, 100 + i, SUPLA_CHANNELTYPE_HVAC,
                                SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
    query("UPDATE supla_dev_channel SET user_id=99126 WHERE id=" +
          std::to_string(consumer));
    cloud_grant(99126, channel_config_origin(consumer, 1), owner, destination,
                channel, 1);
    association_t a;
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
       SingleRelationsIpcDrainsMoreThanOneMarkerBatch) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  peer_dao repo(99126);
  peer_service previous([] { return std::make_unique<peer_dao>(99126); },
                        nullptr);
  uint32_t first_channel = 0;
  for (unsigned i = 0; i < 65; ++i) {
    // Historical registered, currently offline Sources in the isolated Core DB.
    query(
        "INSERT INTO supla_iodevice "
        "(location_id,user_id,guid,enabled,reg_date,protocol_version,flags) "
        "SELECT location_id,99126,UNHEX(MD5('ipc-batch-" +
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
    auto consumer = add_channel(destination, 100 + i, SUPLA_CHANNELTYPE_HVAC,
                                SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
    query("UPDATE supla_dev_channel SET user_id=99126 WHERE id=" +
          std::to_string(consumer));
    cloud_grant(99126, channel_config_origin(consumer, 1), owner, destination,
                channel, 1);
    association_t a;
    ASSERT_TRUE(repo.find(owner, destination, &a));
    ASSERT_TRUE(previous.provisioned(a, 123 + i));
  }
  auto peers = supla_user::find(99126, false)->get_suplan_peers()->peers();
  {
    unsigned pages = 0, bounds = 0;
    Sql004Probe probe;
    probe.fail_execute = [&](const std::string &sql) {
      if (sql.find("SELECT association_id,revision FROM ") == 0) ++pages;
      if (sql.find("SELECT COALESCE(MAX(association_id),0)") == 0) {
        ++bounds;
        // A different thread must fail explicitly while the gate is held.
        auto other = std::async(std::launch::async, [] {
          return supla_user::find(99126, false)->get_suplan_peers()
              ->drain_changes();
        });
        EXPECT_FALSE(other.get());
      }
      return false;
    };
    notify(CONFIG_CHANGE_SCOPE_RELATIONS);
    EXPECT_EQ(2u, pages);
    EXPECT_EQ(1u, bounds);
    EXPECT_EQ(0u, probe.dml);
    probe.finish("ipc-only-65-markers");
  }
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
  for (unsigned i = 0; i < 65; ++i) receive_association();
  ASSERT_FALSE(HasFatalFailure());
  TSuplaDataPacket extra{};
  EXPECT_FALSE(receive_packet(&extra, 100));  // One request per association.
  for (unsigned i = 0; i < 65; ++i) {
    ASSERT_GT(queued.size(), i);
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
  peer_dao repo(99126);
  ASSERT_TRUE(repo.accept_identity(source, {123, {resource}}));
  // Previously accepted Source context, reconstructed from authoritative DB.
  // Source is now offline. The real Destination socket stays connected.
  peer_service previous([] { return std::make_unique<peer_dao>(99126); },
                        nullptr);
  cloud_grant(99126, channel_config_origin(hvac, 1), source, destination,
              resource, 1);
  association_t a;
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
  cloud_grant(99126, channel_config_origin(hvac, 1), source, destination,
              resource, 0);
  ASSERT_TRUE(peers->reconcile(a.id));
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
  EXPECT_EQ(lifecycle_t::draining, a.lifecycle);
  EXPECT_TRUE(a.destination_acked);
  EXPECT_FALSE(a.source_acked);
}

TEST_F(SupLanHvacWireIntegrationTest,
       SourceFirstShareDispatchUsesExactCurrentCloudBinding) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  query("UPDATE supla_dev_channel SET type=" +
        std::to_string(SUPLA_CHANNELTYPE_RELAY) +
        ",func=" + std::to_string(SUPLA_CHANNELFNC_PUMPSWITCH) +
        " WHERE id=" + std::to_string(resource));
  query("UPDATE supla_dev_channel SET user_config='{\"pumpSwitchChannelId\":" +
        std::to_string(resource) + "}' WHERE id=" + std::to_string(hvac));
  cloud_grant(99126, channel_config_origin(hvac, 5), destination, source,
              hvac, 1, resource);
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
  EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_grant "
      "WHERE user_id=99126 AND origin_type=1 AND origin_id=" +
      std::to_string(channel_config_origin(hvac, 5))));
  association_t a;
  ASSERT_TRUE(peer_dao(99126).find(destination, source, &a));
  EXPECT_EQ(destination, a.source);
  EXPECT_EQ(source, a.destination);
  ASSERT_EQ(1u, a.acl.size());
  EXPECT_EQ(hvac, a.acl[0].ResourceId);
  EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_READ, a.acl[0].Permissions);
  // Authoritative empty config revokes without trusting a replayed SHARE.
  query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
        std::to_string(hvac));
  cloud_grant(99126, channel_config_origin(hvac, 5), destination, source,
              hvac, 0);
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
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
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
  // stale displayed mask/revision
  EXPECT_EQ(0, commit_cloud_decision(read, "ACCEPTED"));
  ASSERT_EQ(1, commit_cloud_decision(control, "REJECTED"));
  for (int i = 0; i < 10; ++i)
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_REJECTED,
              manual_ensure(3).AccessStatus);
  auto rejected = decisions().front();
  EXPECT_EQ(2, rejected.permissions);
  EXPECT_EQ(control.revision + 1, rejected.revision);
  EXPECT_EQ(0u, state().id);
  // Direct reconsideration, no new ENSURE/ConsumeIntent needed.
  ASSERT_EQ(1, commit_cloud_decision(rejected, "ACCEPTED"));
  auto accepted = state();
  ASSERT_EQ(1u, accepted.acl.size());
  EXPECT_EQ(2, accepted.acl.front().Permissions);
  supla_mariadb_access_provider db;
  void *statement = nullptr;
  int grant_count = 0;
  ASSERT_TRUE(db.connect());
  ASSERT_TRUE(db.stmt_get_int(
      &statement, &grant_count, nullptr, nullptr, nullptr,
      ("SELECT COUNT(*) FROM supla_resource_access_grant WHERE "
       "origin_type=2 AND origin_id=" + std::to_string(rejected.id) +
       " AND permissions=3").c_str(), nullptr, 0, true));
  EXPECT_EQ(1, grant_count);
  ASSERT_EQ(1, commit_cloud_decision(decisions().front(), "REMOVED"));
  EXPECT_TRUE(state().acl.empty());
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  rows = decisions();
  ASSERT_EQ(2u, rows.size());
  EXPECT_NE(read.id, rows.back().id);
  // remove Pending
  ASSERT_EQ(1, commit_cloud_decision(rows.back(), "REMOVED"));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
  rows = decisions();
  ASSERT_EQ(1, commit_cloud_decision(rows.back(), "REJECTED"));
  // clear sticky deny
  ASSERT_EQ(1, commit_cloud_decision(decisions().back(), "REMOVED"));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            manual_ensure(1).AccessStatus);
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
    accepted = commit_cloud_decision(displayed, "ACCEPTED");
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
  const auto rows = decisions();
  ASSERT_FALSE(rows.empty());
  EXPECT_EQ(approval_status::pending, rows.back().status);
  EXPECT_EQ(2, rows.back().permissions);
  if (accepted) {
    ASSERT_EQ(2u, rows.size());
    ASSERT_EQ(1u, state().acl.size());
    EXPECT_EQ(1, state().acl.front().Permissions);
  } else {
    ASSERT_EQ(1u, rows.size());
    EXPECT_TRUE(state().acl.empty());
    EXPECT_EQ(0, commit_cloud_decision(displayed, "ACCEPTED"));
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
  ASSERT_EQ(1, commit_cloud_decision(action, "ACCEPTED"));
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
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED, manual_ensure(4).Result);
  // Reads never rewrite business ACL.
  EXPECT_TRUE(same_state(before, state()));
  EXPECT_EQ(approval_status::accepted, decisions().front().status);
}

TEST_F(SupLanHvacWireIntegrationTest,
       M5RegisteredDeviceEnsureAndCommittedIpcDecisionUseProductionDispatch) {
  runSqlScript("SupLanM5TestSchema.sql");
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  query("UPDATE supla_iodevice SET protocol_version=29 WHERE user_id=99126");
  peer_dao repo(99126);
  ASSERT_TRUE(repo.accept_identity(source, {123, {resource}}));
  expect_access(resource, SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
                SUPLA_SUPLAN_ACCESS_STATUS_INVALID);
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_decision"));
  for (int i = 0; i < 3; ++i)
    expect_access(resource, SUPLA_SUPLAN_RESULT_OK,
                  SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL, 1);
  ASSERT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_decision"));
  EXPECT_EQ(1, scalar("SELECT revision FROM supla_resource_access_decision"));
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
  // Committed Cloud boundary; the same existing IPC command carries no grant.
  ASSERT_EQ(1, commit_cloud_decision(decisions().front(), "ACCEPTED"));
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
  EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_grant WHERE "
                     "origin_type=2 AND permissions=1"));
  EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_decision"));
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
  ASSERT_EQ(1, commit_cloud_decision(pending[0], "REJECTED"));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_REJECTED,
            stream_ensure(true).AccessStatus);
  auto rejected = decisions()[0];
  ASSERT_EQ(1, commit_cloud_decision(rejected, "ACCEPTED"));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, stream_ensure().AccessStatus);
  EXPECT_EQ(0u, state().id);
  ASSERT_EQ(1, commit_cloud_decision(decisions()[0], "REMOVED"));
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
  ASSERT_EQ(1, commit_cloud_decision(decisions()[0], "ACCEPTED"));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, stream_ensure().AccessStatus);
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
  // Stale displayed snapshot.
  EXPECT_EQ(0, commit_cloud_decision(pending, "ACCEPTED"));
  ASSERT_EQ(1, commit_cloud_decision(stream_pending, "ACCEPTED"));
  auto accepted = decisions().front();
  for (int i = 0; i < 5; ++i)
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, stream_ensure().AccessStatus);
  ASSERT_EQ(1u, decisions().size());
  EXPECT_EQ(accepted.revision, decisions().front().revision);
  EXPECT_EQ(accepted.id, decisions().front().id);
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
  ASSERT_EQ(1, commit_cloud_decision(decisions().front(), "ACCEPTED"));
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
  EXPECT_EQ(0, commit_cloud_decision(accepted, "REMOVED"));
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
  channel_info_t info;
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
  peer_service foreign{[] { return std::make_unique<peer_dao>(1); }, nullptr};
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
            foreign.ensure_access(destination, request).Result);
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
  ASSERT_EQ(1, commit_cloud_decision(decisions().front(), "ACCEPTED"));
  auto user = supla_user::find(99126, false);
  ASSERT_NE(nullptr, user);
  testing::IpcSocketAdapterMock socket(-1);
  EXPECT_CALL(socket, send_data(std::string("OK:99126\n"))).Times(3);
  supla_on_channel_config_changed_command ipc(&socket);
  // Same committed Cloud RELATIONS hint as M5, no second Device ENSURE.
  auto notify = [&](bool consumer_changed = false) {
    int device = consumer_changed ? destination : source;
    int channel = consumer_changed ? hvac : resource;
    int type = consumer_changed ? SUPLA_CHANNELTYPE_HVAC
                                : SUPLA_CHANNELTYPE_THERMOMETER;
    int function = consumer_changed ? SUPLA_CHANNELFNC_HVAC_THERMOSTAT
                                    : SUPLA_CHANNELFNC_THERMOMETER;
    std::string input = "USER-ON-CHANNEL-CONFIG-CHANGED:99126," +
                        std::to_string(device) + "," +
                        std::to_string(channel) + "," + std::to_string(type) +
                        "," + std::to_string(function) + "," +
                        std::to_string(CONFIG_CHANGE_SCOPE_RELATIONS) + "\n";
    std::vector<char> buffer(input.begin(), input.end());
    buffer.push_back(0);
    EXPECT_TRUE(
        ipc.process_command(buffer.data(), buffer.size(), input.size()));
  };
  // A consumer IPC must also revalidate interests in its Source Channels.
  notify(true);
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
  ASSERT_EQ(1, commit_cloud_decision(decisions().front(), "REMOVED"));
  notify();
  ASSERT_TRUE(receive_packet(&packet, 5000));
  ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
  memcpy(&snapshot, packet.data, sizeof(snapshot));
  EXPECT_EQ(1, snapshot.Channel.Offline);
  association_t association;
  ASSERT_TRUE(peer_dao(99126).find(source, destination, &association));
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
  // Existing channel metadata is owned by the product, not overwritten by
  // Device re-registration. Include nonzero ancillary fields in the wire check.
  query("UPDATE supla_dev_channel SET flist=" +
        std::to_string(SUPLA_BIT_FUNC_THERMOMETER) +
        ",alt_icon=1,sub_device_id=17 WHERE id=" + std::to_string(resource));
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
  channel.FuncList = SUPLA_BIT_FUNC_THERMOMETER;
  channel.DefaultIcon = 1;
  channel.SubDeviceId = 17;
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
  ASSERT_EQ(1, commit_cloud_decision(decisions().front(), "ACCEPTED"));
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
  EXPECT_EQ(0xFF, snapshot.Channel.Number);
  EXPECT_EQ(SUPLA_CHANNELTYPE_THERMOMETER, snapshot.Channel.Type);
  EXPECT_EQ(SUPLA_CHANNELFNC_THERMOMETER, snapshot.Channel.Default);
  EXPECT_EQ(SUPLA_BIT_FUNC_THERMOMETER, snapshot.Channel.FuncList);
  EXPECT_EQ(1, snapshot.Channel.DefaultIcon);
  EXPECT_EQ(17, snapshot.Channel.SubDeviceId);
  memcpy(&temperature, snapshot.Channel.value, sizeof(temperature));
  EXPECT_DOUBLE_EQ(20, temperature);
  {
    auto target =
        supla_user::find(99126, false)->get_devices()->get(destination);
    auto timeout = target->wait_time_usec();
    EXPECT_GT(timeout, 1000000u);
    EXPECT_EQ(0u, target->get_stream(false)->time_left_usec());
  }
  {
    auto user = supla_user::find(99126, false);
    auto target = user->get_devices()->get(destination);
    auto stream = user->get_suplan_peers()->new_stream(target.get());
    TDS_SuplaEnsureResourceAccess request{{1, static_cast<uint32_t>(resource)},
                                         1, 2, 0};
    stream->ensure(request, [](const TSD_SuplaEnsureResourceAccessResult &r) {
      EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, r.AccessStatus);
      return true;
    });
    ASSERT_TRUE(receive_packet(&packet, 5000));
    ASSERT_EQ(SUPLA_SD_CALL_REMOTE_CHANNEL_STATE, packet.call_id);
    // Run the production snapshot with cached authorization on this thread;
    // ordinary physical value updates must not open SQL connections.
    Sql004Probe probe;
    for (unsigned i = 0; i < 3; ++i) {
      stream->dirty(resource);
      stream->iterate();
    }
    EXPECT_EQ(0u, probe.selects);
    EXPECT_EQ(0u, probe.connections);
    EXPECT_EQ(0u, probe.stream_metadata_reads);
    EXPECT_EQ(0u, probe.calls);
    EXPECT_EQ(0u, probe.dml);
    probe.finish("stream-live-full-snapshot-three");
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
  std::shared_ptr<stream_connection> previous_stream;
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
  association_t association;
  ASSERT_TRUE(peer_dao(99126).find(source, destination, &association));
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
TEST_F(SupLanHvacIntegrationTest,
       Sql004DeviceMergeRetriesCurrentCloudAuthority) {
  uint32_t injected = add_channel(source, 239, SUPLA_CHANNELTYPE_THERMOMETER,
                                 SUPLA_CHANNELFNC_THERMOMETER);
  ASSERT_TRUE(factory()->accept_identity(source, {123, {resource, injected}}));
  ASSERT_TRUE(factory()->accept_identity(destination, {789, {hvac, local}}));
  // This independent committed right must survive every untrusted CAS merge.
  auto independent_consumer = add_channel(source, 238, SUPLA_CHANNELTYPE_HVAC,
                                         SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  const auto independent_origin =
      channel_config_origin(independent_consumer, 1);
  cloud_grant(99124, independent_origin, destination, source, local, 1);
  for (uint8_t field : {1, 5, 41, 83}) {
    query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
          std::to_string(hvac));
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
      // Explicit trusted product write plus its logical grant; no Core IPC.
      query("UPDATE supla_dev_channel SET user_config='" + root +
            "' WHERE id=" + std::to_string(hvac));
      int owner = id == other_resource ? other_source : source;
      cloud_grant(99124, channel_config_origin(hvac, field),
                  binding ? destination : owner, binding ? owner : destination,
                  binding ? hvac : id, id ? 1 : 0, binding ? id : 0);
    };
    auto verify_grants = [&](uint32_t selected) {
      EXPECT_EQ(selected ? 2 : 1,
                scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
      EXPECT_EQ(selected ? 1 : 0, scalar(
          "SELECT COUNT(*) FROM supla_resource_access_grant WHERE "
          "origin_type=1 AND origin_id=" +
          std::to_string(channel_config_origin(hvac, field)) +
          " AND resource_id=" + std::to_string(binding ? hvac : selected)));

      EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_grant "
                          "WHERE origin_type=1 AND origin_id=" +
                          std::to_string(independent_origin) +
                          " AND resource_id=" + std::to_string(local) +
                          " AND permissions=1"));
      association_t independent;
      ASSERT_TRUE(factory()->find(destination, source, &independent));
      EXPECT_TRUE(std::any_of(independent.acl.begin(), independent.acl.end(),
          [&](const TSuplaSuplanAclEntry &entry) {
            return entry.ResourceId == local && entry.Permissions == 1;
          }));
    };
    // One no-conflict control, four single-conflict cases, one bounded failure.
    for (unsigned scenario = 0; scenario < 6; ++scenario) {
      uint32_t uploaded = scenario % 2 ? injected : resource;
      uint32_t cloud = scenario >= 3 && scenario < 5 ? other_resource : 0;
      if (!scenario) cloud = resource;
      SCOPED_TRACE(::testing::Message() << "field=" << unsigned(field)
                   << " scenario=" << scenario << " incoming=" << uploaded);
      select(resource);
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
      verify_grants(cloud);  // Device merge cannot alter committed grants.
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
    cloud_grant(99124, channel_config_origin(hvac, field), source, destination,
                resource, 0);
  }
}
}  // namespace

namespace {
class SupLanCloudGrantIntegrationTest : public testing::Test,
                                       public testing::IntegrationTest {
 protected:
  supla_mariadb_access_provider admin;
  peer_service::factory_t factory = [] {
    return std::make_unique<peer_dao>(2);
  };
  peer_service service{factory, nullptr};
  int source = 0, destination = 0;
  uint32_t resource = 0, consumer = 0;
  void execute(const std::string &sql) {
    MYSQL_STMT *statement = nullptr;
    ASSERT_TRUE(admin.stmt_execute(reinterpret_cast<void **>(&statement),
                                  sql.c_str(), nullptr, 0, true)) << sql;
    int result;
    do {
      mysql_stmt_free_result(statement);
      result = mysql_stmt_next_result(statement);
    } while (result == 0);
    mysql_stmt_close(statement);
    ASSERT_EQ(-1, result);
  }
  void root_execute(const std::string &sql) {
    std::string result;
    testing::MySqlShell::sqlQuery(cfg_sql_dir, scfg_string(CFG_MYSQL_HOST),
                                  "root", scfg_string(CFG_MYSQL_DB),
                                  sql.c_str(), &result);
  }
  int scalar(const std::string &sql) {
    void *statement = nullptr;
    int value = -1;
    EXPECT_TRUE(admin.stmt_get_int(&statement, &value, nullptr, nullptr,
                                   nullptr, sql.c_str(), nullptr, 0, true));
    return value;
  }
  void SetUp() override {
    initTestDatabase();
    runSqlScript("SupLanM2TestSchema.sql");
    runSqlScript("SupLanM5TestSchema.sql");
    runSqlScript("SupLanM7TestSchema.sql");
    // Pinned test-only SQL snapshot; production migrations belong to Cloud.
    // The standard Core runner needs no sibling checkout or generator.
    runSqlScript("SupLanCloudGrantContract.sql");
    ASSERT_TRUE(admin.connect());
    source = scalar("SELECT MIN(id) FROM supla_iodevice WHERE user_id=2");
    destination = scalar("SELECT MAX(id) FROM supla_iodevice WHERE user_id=2");
    resource =
        scalar("SELECT MIN(id) FROM supla_dev_channel WHERE iodevice_id=" +
               std::to_string(source));
    consumer =
        scalar("SELECT MIN(id) FROM supla_dev_channel WHERE iodevice_id=" +
               std::to_string(destination));
    ASSERT_GT(destination, source);
    ASSERT_GT(resource, 0u);
    ASSERT_GT(consumer, 0u);
    execute("UPDATE supla_iodevice SET protocol_version=29,flags=flags|" +
            std::to_string(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) +
            " WHERE user_id=2");
    execute("UPDATE supla_dev_channel SET type=" +
            std::to_string(SUPLA_CHANNELTYPE_THERMOMETER) + ",func=" +
            std::to_string(SUPLA_CHANNELFNC_THERMOMETER) +
            " WHERE id=" + std::to_string(resource));
    ASSERT_TRUE(factory()->accept_identity(source, {123, {resource}}));
    ASSERT_TRUE(factory()->accept_identity(destination, {456, {consumer}}));
    root_execute("CREATE USER IF NOT EXISTS 'suplan_core_test'@'%'");
    root_execute(
        "REVOKE ALL PRIVILEGES, GRANT OPTION FROM 'suplan_core_test'@'%'");
    root_execute("GRANT SELECT ON supla_test.* TO 'suplan_core_test'@'%'");
    for (const char *name :
         {"supla_suplan_project", "supla_suplan_peer_prepare",
          "supla_suplan_peer_ack", "supla_suplan_accept_identity",
          "supla_suplan_peer_demand", "supla_suplan_share_demand",
          "supla_resource_access_pending", "supla_suplan_changed_ack",
          "supla_suplan_cleanup", "supla_suplan_observe_root",
          "supla_suplan_resource_cleanup"}) {
      root_execute("GRANT EXECUTE ON PROCEDURE supla_test." +
                   std::string(name) + " TO 'suplan_core_test'@'%'");
    }
  }
  void replace(uint16_t type, uint64_t origin, unsigned permissions,
               bool requires_peer = true, uint32_t binding = 0) {
    std::string grants = "[]";
    if (permissions) {
      int source_type = scalar("SELECT type FROM supla_dev_channel WHERE id=" +
                               std::to_string(resource));
      int function = scalar("SELECT func FROM supla_dev_channel WHERE id=" +
                            std::to_string(resource));
      bool peer = scalar("SELECT COUNT(*) FROM supla_iodevice WHERE id=" +
          std::to_string(source) + " AND is_virtual=0 AND protocol_version>=29 "
          "AND (flags & 524288)<>0") == 1;
      grants = "[{\"source\":" + std::to_string(source) +
          ",\"destination\":" + std::to_string(destination) +
          ",\"resource\":" + std::to_string(resource) +
          ",\"permissions\":" + std::to_string(permissions) +
          ",\"type\":" + std::to_string(source_type) +
          ",\"function\":" + std::to_string(function) +
          ",\"peerEligible\":" + std::to_string(peer) + ",\"requiresPeer\":" +
          std::to_string(requires_peer) + ",\"bindingTarget\":" +
          (binding ? std::to_string(binding) : "null") + "}]";
    }
    // Trusted Cloud-like business transaction, outside the restricted Core
    // probe. This tests Core consumption, not Cloud's PHP grant producer.
    execute("START TRANSACTION");
    if (type == USER_APPROVAL_ORIGIN && permissions) {
      execute(
          "INSERT INTO supla_resource_access_pair(user_id,"
          "destination_device_id,source_channel_id,source_device_id) "
          "VALUES(2," +
          std::to_string(destination) + "," + std::to_string(resource) + "," +
          std::to_string(source) + ") ON DUPLICATE KEY UPDATE id=id");
      execute(
          "INSERT INTO supla_resource_access_decision(id,pair_id,status,"
          "permissions,requested_delivery_mode) SELECT " +
          std::to_string(origin) + ",id,'ACCEPTED'," +
          std::to_string(permissions) +
          ",2 FROM "
          "supla_resource_access_pair WHERE user_id=2 AND "
          "destination_device_id=" +
          std::to_string(destination) +
          " AND source_channel_id=" + std::to_string(resource) +
          " AND NOT EXISTS(SELECT 1 FROM "
          "supla_resource_access_decision WHERE id=" +
          std::to_string(origin) + ")");
    }
    execute("CALL supla_resource_access_replace(2," + std::to_string(type) +
            "," + std::to_string(origin) + ",'" + grants + "')");
    execute("COMMIT");
  }
  association_t state() {
    association_t out;
    EXPECT_TRUE(factory()->find(source, destination, &out));
    return out;
  }
};
TEST_F(SupLanCloudGrantIntegrationTest,
       RestrictedAccountProjectsCommittedRightsAndCommitsExactSourceContext) {
  const auto origin = channel_config_origin(consumer, 1);
  replace(1, origin, 1);
  auto initial = state();
  ASSERT_EQ(1u, initial.acl.size());
  ASSERT_EQ(resource, initial.acl[0].ResourceId);
  ASSERT_EQ(1, initial.acl[0].Permissions);
  ASSERT_EQ(0u, initial.root);
  {
    Sql004Probe probe;
    probe.restricted = true;
    // Source OK persists root without changing the acknowledged ACL counters.
    EXPECT_TRUE(service.provisioned(initial, 123));
    auto committed = state();
    EXPECT_EQ(123u, committed.root);
    EXPECT_EQ(initial.generation, committed.provisioned_generation);
    EXPECT_EQ(initial.generation, committed.generation);
    EXPECT_EQ(initial.revision, committed.revision);
    EXPECT_EQ(0u, probe.dml);
    EXPECT_EQ(1u, probe.calls);
    probe.finish("adr013-source-ok");
  }
  replace(2, 90001, 1,
          false);  // Independent Accepted/transport-neutral origin.
  auto accepted = state();
  replace(1, origin, 0);
  EXPECT_TRUE(same_state(accepted, state()));
  replace(2, 90001, 0);
  auto draining = state();
  EXPECT_EQ(lifecycle_t::draining, draining.lifecycle);
  EXPECT_EQ(accepted.revision + 1, draining.revision);
  EXPECT_EQ(accepted.generation, draining.generation);
  EXPECT_TRUE(draining.acl.empty());
  {
    Sql004Probe probe;
    probe.restricted = true;
    EXPECT_TRUE(service.cleanup_ack(draining, true));
    auto partial = state();
    EXPECT_TRUE(partial.source_acked);
    EXPECT_FALSE(partial.destination_acked);
    EXPECT_EQ(lifecycle_t::draining, partial.lifecycle);
    EXPECT_TRUE(service.cleanup_ack(draining, false));
    EXPECT_EQ(lifecycle_t::dormant, state().lifecycle);
    EXPECT_EQ(0u, probe.dml);
    probe.finish("adr013-independent-empty-acks");
  }
}
TEST_F(SupLanCloudGrantIntegrationTest,
       CoveredPendingStaysExplicitAndControlExtensionKeepsIndependentRead) {
  execute("UPDATE supla_dev_channel SET type=" +
          std::to_string(SUPLA_CHANNELTYPE_RELAY) + ",func=" +
          std::to_string(SUPLA_CHANNELFNC_POWERSWITCH) + " WHERE id=" +
          std::to_string(resource));
  ASSERT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            service.ensure_stream(destination, resource, true).AccessStatus);
  const auto pending = scalar("SELECT id FROM supla_resource_access_decision");
  const auto revision =
      scalar("SELECT revision FROM supla_resource_access_decision");
  const auto origin = channel_config_origin(consumer, 1);
  replace(1, origin, 1);
  const auto read = state();
  for (int i = 0; i < 3; ++i)
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
              service.ensure_stream(destination, resource, true).AccessStatus);
  EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_decision"));
  EXPECT_EQ(revision,
            scalar("SELECT revision FROM supla_resource_access_decision"));
  EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_decision "
                     "WHERE status='PENDING'"));
  execute("UPDATE supla_resource_access_decision SET status='ACCEPTED',"
          "open_pair_id=NULL WHERE id=" + std::to_string(pending));
  replace(2, pending, 1, false);
  replace(1, origin, 0);
  EXPECT_TRUE(same_state(read, state()));
  TDS_SuplaEnsureResourceAccess request{{1, resource}, 2, 1, 1};
  {
    Sql004Probe probe;
    probe.restricted = true;
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
              service.ensure_access(destination, request).AccessStatus);
    EXPECT_EQ(0u, probe.dml);
  }
  EXPECT_TRUE(same_state(read, state()));
  EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_decision "
                     "WHERE status='PENDING' AND permissions=2"));
  execute("UPDATE supla_resource_access_decision SET status='REJECTED' "
          "WHERE status='PENDING'");
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_REJECTED,
            service.ensure_access(destination, request).AccessStatus);
  request.Permissions = 1;
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            service.ensure_access(destination, request).AccessStatus);
  EXPECT_TRUE(same_state(read, state()));
}
TEST_F(SupLanCloudGrantIntegrationTest,
       DirtyMarkerCompareAndClearDoesNotAcknowledgeDeviceOrNewerChange) {
  const auto origin = channel_config_origin(consumer, 1);
  replace(1, origin, 1);
  peer_dao dao(2);
  std::vector<std::pair<uint64_t, uint64_t>> markers;
  ASSERT_TRUE(dao.changed_pairs(0, 64, &markers));
  ASSERT_EQ(1u, markers.size());
  auto first = markers.front();
  replace(1, origin, 2);
  {
    Sql004Probe probe;
    probe.restricted = true;
    peer_dao restricted(2);
    ASSERT_TRUE(restricted.ack_changed(first.first, first.second));
    ASSERT_TRUE(restricted.changed_pairs(0, 64, &markers));
    ASSERT_EQ(1u, markers.size());
    EXPECT_GT(markers[0].second, first.second);
    EXPECT_FALSE(state().source_acked);
    EXPECT_FALSE(state().destination_acked);
    ASSERT_TRUE(restricted.ack_changed(markers[0].first, markers[0].second));
    ASSERT_TRUE(restricted.changed_pairs(0, 64, &markers));
    EXPECT_TRUE(markers.empty());
    EXPECT_EQ(0u, probe.dml);
  }
}
TEST_F(SupLanCloudGrantIntegrationTest,
       PendingRejectedAndTransportNeutralApprovalNeverAuthorCoreGrants) {
  auto access = service.ensure_stream(destination, resource, false);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, access.Result);
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_pair"));
  {
    Sql004Probe probe;
    probe.restricted = true;
    for (unsigned i = 0; i < 3; ++i) {
      auto pending = service.ensure_stream(destination, resource, true);
      EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
                pending.AccessStatus);
    }
    EXPECT_EQ(0u, probe.dml);
  }
  EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_decision"));
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
  EXPECT_EQ(0u, state().id);
  execute("UPDATE supla_resource_access_decision SET status='REJECTED'");
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_REJECTED,
            service.ensure_stream(destination, resource, true).AccessStatus);
  execute("UPDATE supla_resource_access_decision SET status='ACCEPTED',"
          "open_pair_id=NULL");
  replace(2, scalar("SELECT id FROM supla_resource_access_decision"), 1, false);
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            service.ensure_stream(destination, resource, false).AccessStatus);
  EXPECT_EQ(0u, state().id);  // No fabricated STREAM-only peer.
  TDS_SuplaEnsureResourceAccess peer{{1, resource}, 1, 1, 0};
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            service.ensure_access(destination, peer).AccessStatus);
  EXPECT_GT(state().id, 0u);  // Technical demand projects existing consent.
  EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_decision"));
}
TEST_F(SupLanCloudGrantIntegrationTest,
       StreamReadsGrantsOnlyOnEnsureAndInvalidation) {
  replace(2, 90002, 1, false);
  auto now = stream_connection::time_point_t() + std::chrono::seconds(1);
  unsigned value = 0;
  std::vector<TSD_SuplaRemoteChannelState> sent;
  stream_connection stream(
      32,
      [&](uint32_t id, bool approval, bool request,
          TDS_SuplaDeviceChannel_E *metadata) {
        return service.ensure_stream(destination, id, approval, nullptr,
                                     request, metadata);
      },
      [&](uint32_t, stream_connection::snapshot_t *out) {
        out->state.Channel.Type = SUPLA_CHANNELTYPE_THERMOMETER;
        out->state.Channel.Default = SUPLA_CHANNELFNC_THERMOMETER;
        out->state.Channel.value[0] = value;
        return true;
      },
      [&](const TSD_SuplaRemoteChannelState &state) {
        sent.push_back(state);
        return true;
      },
      [] { return true; }, [&] { return now; });
  TDS_SuplaEnsureResourceAccess request{{1, resource}, 1, 2, 0};
  stream.ensure(request, [](const TSD_SuplaEnsureResourceAccessResult &result) {
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, result.AccessStatus);
    return true;
  });
  ASSERT_EQ(1u, sent.size());
  {
    Sql004Probe probe;
    probe.restricted = true;
    for (unsigned i = 0; i < 100; ++i) {
      value = i + 1;
      stream.dirty(resource);
      stream.iterate();
    }
    now += std::chrono::seconds(30);
    stream.iterate();
    EXPECT_EQ(0u, probe.selects);
    EXPECT_EQ(0u, probe.connections);
    EXPECT_EQ(0u, probe.dml);
    EXPECT_EQ(0u, probe.calls);
    probe.finish("adr013-stream-100-plus-idle");
  }
  EXPECT_EQ(101u, sent.size());
  replace(2, 90002, 0);
  stream.invalidate_access();
  stream.iterate();
  ASSERT_EQ(102u, sent.size());
  EXPECT_EQ(1, sent.back().Channel.Offline);
  EXPECT_EQ(0xFF, sent.back().Channel.Number);
  EXPECT_EQ(resource, sent.back().ChannelId);
  stream.dirty(resource);
  stream.iterate();
  EXPECT_EQ(102u, sent.size());
  EXPECT_EQ(0u, stream.size());
}
TEST_F(SupLanCloudGrantIntegrationTest,
       ReadConfigDoesNotInterpretReferencesOrCreateLogicalAuthority) {
  execute("UPDATE supla_dev_channel SET type=" +
          std::to_string(SUPLA_CHANNELTYPE_HVAC) + ",func=" +
          std::to_string(SUPLA_CHANNELFNC_HVAC_THERMOSTAT) +
          ",user_config=JSON_OBJECT('mainThermometerChannelId'," +
          std::to_string(resource) + ") WHERE id=" + std::to_string(consumer));
  supla_connection_test_access protocol;
  ASSERT_TRUE(protocol.open());
  testing::DeviceStub device(protocol.get());
  device.set_id(destination);
  device.set_flags(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED);
  device.set_user(supla_user::find(2, true));
  supla_device_dao dao(&admin);
  device.set_channels(new supla_device_channels(&dao, &device, nullptr,
                                                nullptr, 0));
  device.get_channels()->access_channel(consumer,
      [&](supla_device_channel *channel) {
        Sql004Probe probe;
        probe.restricted = true;
        for (unsigned i = 0; i < 3; ++i) {
          TSD_ChannelConfig config{};
          EXPECT_TRUE(
              channel->get_config(&config, SUPLA_CONFIG_TYPE_DEFAULT, 0));
          EXPECT_EQ(SUPLA_CHANNELFNC_HVAC_THERMOSTAT, config.Func);
        }
        EXPECT_EQ(0u, probe.config_reads);
        EXPECT_EQ(0u, probe.selects);
        EXPECT_EQ(0u, probe.connections);
        EXPECT_EQ(0u, probe.dml);
        EXPECT_EQ(0u, probe.calls);
        probe.finish("adr013-get-config-three");
      });
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
  EXPECT_EQ(0u, state().id);
}
TEST_F(SupLanCloudGrantIntegrationTest,
       CoreSqlAccountCannotWriteTablesOrCallBusinessReplacement) {
  Sql004Probe probe;
  probe.restricted = true;
  supla_mariadb_access_provider restricted;
  ASSERT_TRUE(restricted.connect());
  EXPECT_NE(0, restricted.query(
      "UPDATE supla_dev_channel SET func=0 WHERE user_id=2", false));
  EXPECT_NE(0, restricted.query(
      "DELETE FROM supla_resource_access_grant WHERE user_id=2", false));
  EXPECT_NE(0, restricted.query(
      "CALL supla_resource_access_replace(2,1,100,'[]')", false));
}

TEST_F(SupLanCloudGrantIntegrationTest,
       SourceOkCommitsBeforeForwardingKeyAndRevokeKeepsIndependentSides) {
  replace(1, channel_config_origin(consumer, 1), 1);
  DeleteTransport transport;
  peer_provisioner peers(factory, &transport);
  const auto initial = state();
  {
    Sql004Probe probe;
    probe.restricted = true;
    ASSERT_TRUE(peers.reconcile(initial.id));
    ASSERT_EQ(1u, transport.sources.size());
    EXPECT_TRUE(transport.destinations.empty());
    TDS_SuplaSetSuplanSourceAssociationResult reply{};
    reply.PeerContext = transport.sources[0].PeerContext;
    reply.AclRevision = transport.sources[0].AclRevision;
    reply.Result = SUPLA_SUPLAN_RESULT_OK;
    reply.PeerKeySize = sizeof(reply.PeerKey);
    memset(reply.PeerKey, 0xA5, sizeof(reply.PeerKey));
    peers.on_source(source, &reply);
    EXPECT_EQ(123u, state().root);
    ASSERT_EQ(1u, transport.destinations.size());
    EXPECT_EQ(0xA5, transport.destinations[0].PeerKey[0]);
    TDS_SuplaSetSuplanDestinationAssociationResult accepted{};
    accepted.PeerContext = transport.destinations[0].PeerContext;
    accepted.AclRevision = transport.destinations[0].AclRevision;
    accepted.Result = SUPLA_SUPLAN_RESULT_OK;
    peers.on_destination(destination, &accepted);
    EXPECT_EQ(0u, probe.dml);
  }
  replace(1, channel_config_origin(consumer, 1), 0);
  auto draining = state();
  {
    Sql004Probe probe;
    probe.restricted = true;
    ASSERT_TRUE(peers.reconcile(draining.id));
    ASSERT_EQ(2u, transport.sources.size());
    ASSERT_EQ(2u, transport.destinations.size());
    EXPECT_EQ(0, transport.sources.back().AclEntryCount);
    EXPECT_EQ(0, transport.destinations.back().ResourceCount);
    TDS_SuplaSetSuplanDestinationAssociationResult reply{};
    reply.PeerContext = transport.destinations.back().PeerContext;
    reply.AclRevision = transport.destinations.back().AclRevision;
    reply.Result = SUPLA_SUPLAN_RESULT_OK;
    peers.on_destination(destination, &reply);
    EXPECT_TRUE(state().destination_acked);
    EXPECT_FALSE(state().source_acked);
    EXPECT_EQ(lifecycle_t::draining, state().lifecycle);
    TDS_SuplaSetSuplanSourceAssociationResult empty{};
    empty.PeerContext = transport.sources.back().PeerContext;
    empty.AclRevision = transport.sources.back().AclRevision;
    empty.Result = SUPLA_SUPLAN_RESULT_OK;
    peers.on_source(source, &empty);
    EXPECT_EQ(lifecycle_t::dormant, state().lifecycle);
    EXPECT_EQ(0u, probe.dml);
  }
}
TEST_F(SupLanCloudGrantIntegrationTest,
       SourceFirstShareRequiresExactCommittedBindingNotJsonOrSameDeviceTarget) {
  execute("UPDATE supla_dev_channel SET type=" +
          std::to_string(SUPLA_CHANNELTYPE_HVAC) + ",func=" +
          std::to_string(SUPLA_CHANNELFNC_HVAC_THERMOSTAT) +
          " WHERE id=" + std::to_string(resource));
  execute("UPDATE supla_dev_channel SET type=" +
          std::to_string(SUPLA_CHANNELTYPE_RELAY) + ",func=" +
          std::to_string(SUPLA_CHANNELFNC_PUMPSWITCH) +
          " WHERE id=" + std::to_string(consumer));
  replace(1, channel_config_origin(resource, 5), 1, true, consumer);
  TDS_SuplaEnsureResourceShare request{};
  request.SourceResource = {1, resource};
  request.DestinationResource = {1, consumer};
  request.Permissions = SUPLA_SUPLAN_PERMISSION_READ;
  {
    Sql004Probe probe;
    probe.restricted = true;
    EXPECT_EQ(SUPLA_SUPLAN_RESULT_OK,
              service.ensure_share(source, request).Result);
    EXPECT_EQ(0u, probe.dml);
    EXPECT_EQ(1u, probe.calls);
    probe.finish("adr013-source-first-share");
  }
  const auto unchanged = state();
  execute("INSERT INTO supla_dev_channel(iodevice_id,user_id,channel_number,"
          "type,func,param1,param2,param3) VALUES(" +
          std::to_string(destination) + ",2,254,2900,960,0,0,0)");
  int other = scalar("SELECT id FROM supla_dev_channel WHERE iodevice_id=" +
                     std::to_string(destination) + " AND channel_number=254");
  ASSERT_GT(other, 0);
  request.DestinationResource.ResourceId = other;
  {
    Sql004Probe probe;
    probe.restricted = true;
    EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
              service.ensure_share(source, request).Result);
    EXPECT_EQ(0u, probe.calls);
  }
  EXPECT_TRUE(same_state(unchanged, state()));
  request.DestinationResource.ResourceId = consumer;
  replace(1, channel_config_origin(resource, 5), 0);
  execute("UPDATE supla_dev_channel SET user_config=JSON_OBJECT("
          "'pumpSwitchChannelId'," + std::to_string(consumer) +
          ") WHERE id=" + std::to_string(resource));
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
            service.ensure_share(source, request).Result);
  EXPECT_TRUE(state().acl.empty());
}
TEST_F(SupLanCloudGrantIntegrationTest,
       StreamLegacyAndVirtualSourcesNeedNoPeerButRejectStaleSourceMetadata) {
  execute("UPDATE supla_iodevice SET flags=0,protocol_version=28 WHERE id=" +
          std::to_string(source));
  replace(2, 90003, 1, false);
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            service.ensure_stream(destination, resource, false).AccessStatus);
  EXPECT_EQ(0u, state().id);
  execute("UPDATE supla_iodevice SET is_virtual=1 WHERE id=" +
          std::to_string(source));
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            service.ensure_stream(destination, resource, false).AccessStatus);
  EXPECT_EQ(0u, state().id);
  execute("UPDATE supla_dev_channel SET func=0 WHERE id=" +
          std::to_string(resource));
  EXPECT_NE(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            service.ensure_stream(destination, resource, false).AccessStatus);
  EXPECT_EQ(0u, state().id);
}
TEST_F(SupLanCloudGrantIntegrationTest,
       ConfigSqlCostIsIndependentOfSixTenOrTwentyReferenceSlots) {
  struct Case {
    int type, function;
    const char *json, *label;
    bool suplan = true;
  };
  for (const auto &item : std::vector<Case>{
      {SUPLA_CHANNELTYPE_RELAY, SUPLA_CHANNELFNC_POWERSWITCH,
       "{}", "adr013-ordinary-legacy", false},
      {SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
       "{}", "adr013-hvac-empty-legacy", false},
      {SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
       "{}", "adr013-hvac-empty"},
      {SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
       "{\"mainThermometerChannelId\":1,\"auxThermometerChannelId\":2,"
       "\"binarySensorChannelId\":3,\"masterThermostatChannelId\":4,"
       "\"pumpSwitchChannelId\":5,\"heatOrColdSourceSwitchChannelId\":6}",
       "adr013-hvac-six"},
      {SUPLA_CHANNELTYPE_CONTAINER, SUPLA_CHANNELFNC_CONTAINER,
       "{\"sensors\":[{\"channelId\":1},{\"channelId\":2},{\"channelId\":3},"
       "{\"channelId\":4},{\"channelId\":5},{\"channelId\":6},"
       "{\"channelId\":7},{\"channelId\":8},{\"channelId\":9},"
       "{\"channelId\":10}]}", "adr013-container-ten"},
      {SUPLA_CHANNELTYPE_VALVE_OPENCLOSE, SUPLA_CHANNELFNC_VALVE_OPENCLOSE,
       "{\"floodSensorChannelIds\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,"
       "15,16,17,18,19,20]}", "adr013-valve-twenty"}}) {
    execute("UPDATE supla_dev_channel SET type=" + std::to_string(item.type) +
            ",func=" + std::to_string(item.function) + ",user_config='" +
            item.json + "' WHERE id=" + std::to_string(consumer));
    supla_connection_test_access protocol;
    ASSERT_TRUE(protocol.open());
    testing::DeviceStub device(protocol.get());
    device.set_id(destination);
    device.set_flags(item.suplan ? SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED : 0);
    device.set_user(supla_user::find(2, true));
    supla_device_dao dao(&admin);
    device.set_channels(new supla_device_channels(&dao, &device, nullptr,
                                                  nullptr, 0));
    device.get_channels()->access_channel(consumer,
        [&](supla_device_channel *channel) {
          Sql004Probe probe;
          probe.restricted = true;
          TSD_ChannelConfig config{};
          ASSERT_TRUE(
              channel->get_config(&config, SUPLA_CONFIG_TYPE_DEFAULT, 0));
          EXPECT_EQ(item.function, config.Func);
          EXPECT_EQ(0u, probe.config_reads);
          EXPECT_EQ(0u, probe.selects);
          EXPECT_EQ(0u, probe.connections);
          EXPECT_EQ(0u, probe.calls);
          EXPECT_EQ(0u, probe.dml);
          probe.finish(item.label);
        });
  }
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
}

TEST_F(SupLanCloudGrantIntegrationTest,
       LostIpcSourceReconnectKeepsIndependentReadThenSendsEmptyAcl) {
  const auto config_origin = channel_config_origin(consumer, 1);
  replace(CHANNEL_CONFIG_ORIGIN, config_origin, 1);
  replace(USER_APPROVAL_ORIGIN, 90005, 1);
  ASSERT_TRUE(service.provisioned(state(), 123));
  const auto initial = state();
  replace(CHANNEL_CONFIG_ORIGIN, config_origin, 0);
  EXPECT_EQ(initial.revision, state().revision);
  class SourceOnlyTransport : public DeleteTransport {
   public:
    int online = 0;
    bool ready(int device) override { return device == online; }
  } transport;
  transport.online = source;
  peer_provisioner peers(factory, &transport);
  for (unsigned count : {1u, 0u}) {
    if (!count) {
      peers.disconnected(source, transport.session(source));
      replace(USER_APPROVAL_ORIGIN, 90005, 0);
    }
    const auto desired = state();
    Sql004Probe probe;
    probe.restricted = true;
    // Natural registration's actual provisioner entry, no IPC or marker drain.
    peers.reconnect(source);
    ASSERT_EQ(2u - count, transport.sources.size());
    EXPECT_TRUE(transport.destinations.empty());
    const auto &request = transport.sources.back();
    EXPECT_EQ(count, request.AclEntryCount);
    EXPECT_EQ(0, request.Flags);
    EXPECT_EQ(123u, request.PeerContext.RootEpoch);
    EXPECT_EQ(initial.generation, request.PeerContext.PeerGeneration);
    EXPECT_EQ(desired.revision, request.AclRevision);
    if (count) {
      EXPECT_EQ(resource, request.Acl[0].ResourceId);
    }
    TDS_SuplaSetSuplanSourceAssociationResult reply{};
    reply.PeerContext = request.PeerContext;
    reply.AclRevision = request.AclRevision;
    reply.Result = SUPLA_SUPLAN_RESULT_OK;
    peers.on_source(source, &reply, transport.session(source));
    if (!count) {
      EXPECT_EQ(lifecycle_t::draining, state().lifecycle);
      EXPECT_TRUE(state().source_acked);
      EXPECT_FALSE(state().destination_acked);
    }
    EXPECT_EQ(0u, probe.dml);
    probe.finish("ipc-only-source-reconnect");
  }
}

TEST_F(SupLanCloudGrantIntegrationTest,
       FailedSourceCommitRollsBackAndRecoveryDerivesFreshKeyWithoutReconnect) {
  replace(1, channel_config_origin(consumer, 1), 1);
  class RecoveryTransport : public DeleteTransport {
    void diagnostic(uint64_t, int, uint8_t code) override {
      EXPECT_EQ(SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR, code);
    }
    bool reconnect_session(int, uint64_t) override {
      ADD_FAILURE() << "Core DB failure cannot disconnect Device";
      return false;
    }
  } transport;
  auto now = std::chrono::steady_clock::time_point() + std::chrono::seconds(1);
  peer_provisioner peers(factory, &transport, [&] { return now; });
  ASSERT_TRUE(peers.reconcile(state().id));
  ASSERT_EQ(1u, transport.sources.size());
  execute("CREATE TRIGGER suplan_fixture_fail_source BEFORE UPDATE ON "
          "supla_suplan_peer_association FOR EACH ROW SIGNAL SQLSTATE '45000' "
          "SET MESSAGE_TEXT='fixture source commit failure'");
  TDS_SuplaSetSuplanSourceAssociationResult reply{};
  reply.PeerContext = transport.sources.back().PeerContext;
  reply.AclRevision = transport.sources.back().AclRevision;
  reply.Result = SUPLA_SUPLAN_RESULT_OK;
  reply.PeerKeySize = sizeof(reply.PeerKey);
  memset(reply.PeerKey, 0xA5, sizeof(reply.PeerKey));
  {
    Sql004Probe probe;
    probe.restricted = true;
    peers.on_source(source, &reply);
    EXPECT_TRUE(transport.destinations.empty());
    EXPECT_EQ(0u, state().root);
    EXPECT_EQ(1u, transport.sources.size());
    EXPECT_EQ(0u, probe.dml);
  }
  execute("DROP TRIGGER suplan_fixture_fail_source");
  now += std::chrono::seconds(5);
  {
    Sql004Probe probe;
    probe.restricted = true;
    peers.iterate(source);
    ASSERT_EQ(2u, transport.sources.size());
    EXPECT_TRUE(transport.destinations.empty());
    reply.PeerContext = transport.sources.back().PeerContext;
    reply.AclRevision = transport.sources.back().AclRevision;
    memset(reply.PeerKey, 0xB6, sizeof(reply.PeerKey));
    peers.on_source(source, &reply);
    ASSERT_EQ(1u, transport.destinations.size());
    EXPECT_EQ(0xB6, transport.destinations.back().PeerKey[0]);
    EXPECT_EQ(123u, state().root);
    EXPECT_EQ(0u, probe.dml);
  }
}

TEST_F(SupLanCloudGrantIntegrationTest,
       DeletedConsumerCleanupPreservesIndependentLogicalOriginAndAclRevision) {
  replace(1, channel_config_origin(consumer, 1), 1);
  replace(2, 90004, 1, false);
  const auto before = state();
  execute("DELETE FROM supla_dev_channel WHERE id=" + std::to_string(consumer));
  {
    Sql004Probe probe;
    probe.restricted = true;
    EXPECT_TRUE(factory()->cleanup_resource(consumer));
    EXPECT_TRUE(factory()->cleanup_resource(consumer));
    EXPECT_EQ(0u, probe.dml);
    EXPECT_EQ(2u, probe.calls);
  }
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_grant "
                     "WHERE origin_type=1"));
  EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_grant "
                     "WHERE origin_type=2 AND origin_id=90004"));
  EXPECT_TRUE(same_state(before, state()));
  EXPECT_EQ(before.revision, state().revision);
}

TEST_F(SupLanCloudGrantIntegrationTest,
       RemovedEndpointNeedsSurvivorAckBeforeTerminalAssociationDelete) {
  replace(1, channel_config_origin(consumer, 1), 1);
  ASSERT_TRUE(service.provisioned(state(), 123));
  execute("DELETE FROM supla_iodevice WHERE id=" + std::to_string(source));
  {
    Sql004Probe probe;
    probe.restricted = true;
    ASSERT_TRUE(factory()->cleanup_device(source));
    auto draining = state();
    ASSERT_NE(0u, draining.id);
    EXPECT_EQ(lifecycle_t::draining, draining.lifecycle);
    EXPECT_TRUE(draining.source_removed);
    EXPECT_TRUE(draining.source_acked);
    EXPECT_FALSE(draining.destination_acked);
    EXPECT_TRUE(draining.acl.empty());
    ASSERT_TRUE(service.cleanup_ack(draining, false));
    EXPECT_EQ(0u, state().id);
    EXPECT_EQ(0u, probe.dml);
  }
}

TEST_F(SupLanCloudGrantIntegrationTest,
       RevokedLogicalReadCannotBecomeGrantedDuringTechnicalPeerDemand) {
  replace(2, 90006, 1, false);
  ASSERT_EQ(0u, state().id);
  unsigned revokes = 0;
  Sql004Probe probe;
  probe.restricted = true;
  probe.after_permissions = [&] {
    ++revokes;
    execute("START TRANSACTION");
    execute("UPDATE supla_resource_access_decision SET status='REMOVED',"
            "open_pair_id=NULL WHERE id=90006");
    execute("CALL supla_resource_access_replace(2,2,90006,'[]')");
    execute("COMMIT");
  };
  TDS_SuplaEnsureResourceAccess request{{1, resource}, 1, 1, 0};
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
            service.ensure_access(destination, request).Result);
  EXPECT_EQ(1u, revokes);
  EXPECT_EQ(0u, probe.dml);
  probe.finish("adr013-revoke-during-peer-demand");
  EXPECT_EQ(0u, state().id);
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));
  EXPECT_EQ(0, scalar("SELECT COUNT(*) FROM supla_resource_access_decision "
                     "WHERE status='PENDING'"));
}

TEST_F(SupLanHvacWireIntegrationTest,
       DirtyMarkerFailureCannotStarveUnrelatedRevocation) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  auto peers = supla_user::find(99126, false)->get_suplan_peers();
  peer_dao repo(99126);
  // A is a persistent, per-pair failure, not a database-wide outage.
  cloud_grant(99126, channel_config_origin(hvac, 1), source, destination,
              resource, 1);
  ASSERT_TRUE(repo.accept_identity(source, {123, {}}));
  association_t broken;
  ASSERT_TRUE(repo.find(source, destination, &broken));

  auto consumer = add_channel(destination, 239, SUPLA_CHANNELTYPE_HVAC,
                              SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  query("UPDATE supla_dev_channel SET user_id=99126 WHERE id=" +
        std::to_string(consumer));
  const auto origin = channel_config_origin(consumer, 1);
  cloud_grant(99126, origin, other_source, destination, other_resource, 1);
  association_t revoked;
  ASSERT_TRUE(repo.find(other_source, destination, &revoked));
  ASSERT_LT(broken.id, revoked.id);
  ASSERT_TRUE(service.provisioned(revoked, 456));
  cloud_grant(99126, origin, other_source, destination, other_resource, 0);
  ASSERT_TRUE(repo.load(revoked.id, &revoked));
  ASSERT_EQ(lifecycle_t::draining, revoked.lifecycle);
  ASSERT_TRUE(revoked.acl.empty());

  // Even while reporting A's failure, deliver B's current empty replacement.
  EXPECT_FALSE(peers->drain_changes());
  TSuplaDataPacket packet{};
  ASSERT_TRUE(receive_packet(&packet, 5000));
  ASSERT_EQ(SUPLA_SD_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION, packet.call_id);
  TSDS_SuplaSetSuplanDestinationAssociation replacement{};
  memcpy(&replacement, packet.data, packet.data_size);
  EXPECT_EQ(other_source, replacement.PeerContext.SourceNodeId);
  EXPECT_EQ(destination, replacement.PeerContext.DestinationNodeId);
  EXPECT_EQ(revoked.revision, replacement.AclRevision);
  EXPECT_EQ(0, replacement.ResourceCount);
  EXPECT_EQ(0, replacement.PeerKeySize);
  std::vector<std::pair<uint64_t, uint64_t>> markers;
  ASSERT_TRUE(repo.changed_pairs(0, 64, &markers));
  ASSERT_EQ(1u, markers.size());
  EXPECT_EQ(broken.id, markers.front().first);
  const auto retained = markers;
  EXPECT_FALSE(peers->drain_changes());
  ASSERT_TRUE(repo.changed_pairs(0, 64, &markers));
  EXPECT_EQ(retained, markers);
  ASSERT_TRUE(repo.load(revoked.id, &revoked));
  EXPECT_EQ(lifecycle_t::draining, revoked.lifecycle);
  EXPECT_FALSE(revoked.source_acked);
  EXPECT_FALSE(revoked.destination_acked);
  EXPECT_EQ(1, scalar("SELECT COUNT(*) FROM supla_resource_access_grant"));

  // Repairing A needs neither a reconnect nor a new business Grant.
  ASSERT_TRUE(repo.accept_identity(source, {123, {resource}}));
  ASSERT_TRUE(peers->drain_changes());
  ASSERT_TRUE(repo.changed_pairs(0, 64, &markers));
  EXPECT_TRUE(markers.empty());
}

TEST_F(SupLanHvacWireIntegrationTest,
       DirtyMarkerSqlFailureRetriesSameEntryOnNextPass) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  auto peers = supla_user::find(99126, false)->get_suplan_peers();
  peer_dao trusted(99126);
  const auto origin = channel_config_origin(hvac, 1);
  // Fail the range read, initial load, authoritative reload, then ACK.
  for (unsigned failure = 0; failure < 4; ++failure) {
    cloud_grant(99126, origin, source, destination, resource, 1);
    cloud_grant(99126, origin, source, destination, resource, 0);
    const auto other_origin = channel_config_origin(hvac, 2);
    cloud_grant(99126, other_origin, other_source, destination,
                other_resource, 1);
    cloud_grant(99126, other_origin, other_source, destination,
                other_resource, 0);
    std::vector<std::pair<uint64_t, uint64_t>> markers;
    ASSERT_TRUE(trusted.changed_pairs(0, 64, &markers));
    ASSERT_EQ(2u, markers.size());
    const auto failed_id = markers.front().first;
    unsigned loads = 0;
    bool injected = false;
    {
      Sql004Probe probe;
      probe.fail_execute = [&](const std::string &sql) {
        if (sql.find("canonical_acl FROM supla_suplan_peer_association") !=
            std::string::npos) ++loads;
        bool fail = !injected &&
            ((failure == 0 &&
              sql.find("SELECT COALESCE(MAX(association_id),0)") == 0) ||
             (failure > 0 && failure < 3 && loads == failure &&
              sql.find("canonical_acl FROM") != std::string::npos) ||
             (failure == 3 && sql.find("CALL supla_suplan_changed_ack(") == 0));
        if (fail) injected = true;
        return fail;
      };
      EXPECT_FALSE(peers->drain_changes());
    }
    ASSERT_TRUE(injected);
    ASSERT_TRUE(trusted.changed_pairs(0, 64, &markers));
    ASSERT_EQ(failure ? 1u : 2u, markers.size());
    EXPECT_EQ(failed_id, markers.front().first);
    // The unrelated marker was committed even after this row's SQL failure.
    // No empty cursor-wrap pass, reconnect, IPC or timer is needed to retry.
    ASSERT_TRUE(peers->drain_changes());
    ASSERT_TRUE(trusted.changed_pairs(0, 64, &markers));
    EXPECT_TRUE(markers.empty());
  }
}

TEST_F(SupLanHvacWireIntegrationTest,
       LostIpcDestinationRegistrationReplaysCurrentGrantAndRevocation) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  peer_dao trusted(99126);
  const auto origin = channel_config_origin(hvac, 1);
  for (unsigned permissions : {1u, 0u}) {
    // Trusted Cloud commit, deliberately without its IPC notification.
    cloud_grant(99126, origin, source, destination, resource, permissions);
    association_t desired;
    ASSERT_TRUE(trusted.find(source, destination, &desired));
    if (permissions) {
      ASSERT_TRUE(service.provisioned(desired, 123));
    }
    ASSERT_TRUE(trusted.load(desired.id, &desired));
    EXPECT_EQ(permissions ? lifecycle_t::active : lifecycle_t::draining,
              desired.lifecycle);
    TSuplaDataPacket packet{};
    EXPECT_FALSE(receive_packet(&packet, 100));
    close_connection();
    start_connection();
    unsigned replacements = 0;
    register_existing_family(SUPLA_CHANNELTYPE_HVAC,
        SUPLA_CHANNELFNC_HVAC_THERMOSTAT, SUPLA_CHANNELFNC_HVAC_THERMOSTAT,
        false, true, [&](const TSuplaDataPacket &incoming) {
          if (incoming.call_id !=
              SUPLA_SD_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION) return;
          TSDS_SuplaSetSuplanDestinationAssociation replacement{};
          ASSERT_LE(incoming.data_size, sizeof(replacement));
          memcpy(&replacement, incoming.data, incoming.data_size);
          ++replacements;
          EXPECT_EQ(source, replacement.PeerContext.SourceNodeId);
          EXPECT_EQ(destination, replacement.PeerContext.DestinationNodeId);
          EXPECT_EQ(123u, replacement.PeerContext.RootEpoch);
          EXPECT_EQ(desired.generation,
                    replacement.PeerContext.PeerGeneration);
          EXPECT_EQ(desired.revision, replacement.AclRevision);
          EXPECT_EQ(permissions, replacement.ResourceCount);
          EXPECT_EQ(0, replacement.PeerKeySize);
          if (permissions) {
            EXPECT_EQ(resource, replacement.Resources[0].ResourceId);
          }
          TDS_SuplaSetSuplanDestinationAssociationResult reply = {
              replacement.PeerContext, replacement.AclRevision,
              SUPLA_SUPLAN_RESULT_OK};
          send_packet(SUPLA_DS_CALL_SET_SUPLAN_DESTINATION_ASSOCIATION_RESULT,
                      &reply, sizeof(reply));
        });
    ASSERT_FALSE(HasFatalFailure());
    EXPECT_EQ(1u, replacements);
    // Source is offline; a Destination ACK cannot claim Source revocation.
    if (!permissions) {
      expect_access(resource, SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
                    SUPLA_SUPLAN_ACCESS_STATUS_INVALID);
      ASSERT_TRUE(trusted.load(desired.id, &desired));
      EXPECT_EQ(lifecycle_t::draining, desired.lifecycle);
      EXPECT_FALSE(desired.source_acked);
      EXPECT_TRUE(desired.destination_acked);
    }
  }
}

TEST_F(SupLanHvacWireIntegrationTest, IdleDeviceDoesNotPollGrantMarkers) {
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  ASSERT_FALSE(HasFatalFailure());
  auto target = supla_user::find(99126, false)->get_devices()->get(destination);
  ASSERT_NE(nullptr, target);
  Sql004Probe probe;
  for (unsigned i = 0; i < 100; ++i) {
    target->iterate();
    EXPECT_EQ(120000000u, target->wait_time_usec());
  }
  EXPECT_EQ(0u, probe.executions);
  EXPECT_EQ(0u, probe.connections);
  probe.finish("ipc-only-idle-device");
}

}  // namespace

// Opt-in native benchmark. No sleeps in DAO measurements; real MariaDB and
// existing production Device/Client paths. Setup is outside timed regions.
namespace {
class HvacPerfHandler : public supla_ch_set_channel_config {
 public:
  using supla_ch_set_channel_config::handle_call;
};
class HvacPerfClient : public supla_client {
 public:
  explicit HvacPerfClient(supla_connection *connection, supla_user *user,
                          int id = 1)
      : supla_client(connection) {
    set_id(id);
    char guid[SUPLA_GUID_SIZE]{};
    memcpy(guid, &id, sizeof(id));
    set_guid(guid);
    set_user(user);
  }
  void load() { load_config(); }
};
void perf_cloud_sql() {
  auto directory = getenv("SUPLA_HVAC_PERF_CLOUD_PROCEDURES");
  ASSERT_NE(nullptr, directory);
  supla_mariadb_access_provider db;
  ASSERT_TRUE(db.connect());
  for (const char *name : {"supla_set_channel_json_config",
                           "supla_update_iodevice", "supla_add_iodevice",
                           "supla_add_channel", "supla_update_channel_value"}) {
    std::ifstream input(std::string(directory) + "/" + name + ".sql");
    ASSERT_TRUE(input.good()) << name;
    std::string sql((std::istreambuf_iterator<char>(input)),
                    std::istreambuf_iterator<char>());
    auto start = sql.find("CREATE PROCEDURE");
    ASSERT_NE(std::string::npos, start);
    ASSERT_EQ(
        0, db.query((std::string("DROP PROCEDURE IF EXISTS ") + name).c_str(),
                    true));
    ASSERT_EQ(0, db.query(sql.substr(start).c_str(), true));
  }
  ASSERT_EQ(0, db.query("SET GLOBAL userstat=1", true));
}

uint64_t perf_cpu_usec() {
  rusage usage{};
  getrusage(RUSAGE_SELF, &usage);
  return usage.ru_utime.tv_sec * 1000000ULL + usage.ru_utime.tv_usec +
         usage.ru_stime.tv_sec * 1000000ULL + usage.ru_stime.tv_usec;
}
}  // namespace

TEST_F(SupLanHvacIntegrationTest, RegistrationPerformanceSqlAndClient) {
  if (!getenv("SUPLA_HVAC_PERF")) GTEST_SKIP() << "opt-in native benchmark";
  perf_cloud_sql();
  ASSERT_FALSE(HasFatalFailure());
  supla_connection_test_access protocol;
  ASSERT_TRUE(protocol.open());
  auto user = supla_user::find(99124, true);
  // Remove unrelated fixture channels from the visible Location, not schema.
  query("UPDATE supla_dev_channel SET hidden=1 WHERE user_id=99124");
  std::vector<int> devices;
  std::vector<std::vector<uint32_t>> channels;
  std::vector<uint32_t> thermometers;
  for (int d = 0; d < 12; ++d) {
    int id = 400000 + d;
    query(
        "INSERT INTO supla_iodevice "
        "(id,location_id,user_id,guid,enabled,reg_date,protocol_version,flags) "
        "SELECT " +
        std::to_string(id) + ",location_id,user_id,UNHEX(MD5('perf" +
        std::to_string(d) +
        "')),1,reg_date,29,flags FROM supla_iodevice "
        "WHERE id=" +
        std::to_string(destination));
    devices.push_back(id);
    channels.emplace_back();
    for (int c = 0; c < 15; ++c)
      channels.back().push_back(add_channel(id, c, SUPLA_CHANNELTYPE_HVAC,
                                            SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
    thermometers.push_back(add_channel(id, 15, SUPLA_CHANNELTYPE_THERMOMETER,
                                       SUPLA_CHANNELFNC_THERMOMETER));
  }
  query("UPDATE supla_client SET user_id=99124 WHERE id=1");
  query("UPDATE supla_dev_channel SET flags=" +
        std::to_string(SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE) +
        " WHERE user_id=99124");
  for (int count : {1, 3, 12}) {
    if (getenv("SUPLA_HVAC_PERF_DEVICES") &&
        atoi(getenv("SUPLA_HVAC_PERF_DEVICES")) > 0 &&
        atoi(getenv("SUPLA_HVAC_PERF_DEVICES")) != count)
      continue;
    // View visibility is Location based. Hide later devices in a fixture-only
    // disabled Function so the actual client SQL sees exactly N*16 channels.
    query("UPDATE supla_dev_channel SET hidden=1 WHERE user_id=99124");
    for (int d = 0; d < count; ++d)
      query("UPDATE supla_dev_channel SET hidden=0 WHERE iodevice_id=" +
            std::to_string(devices[d]));
    for (int mode : {0, 1, 2}) {
      if (getenv("SUPLA_HVAC_PERF_MODE") &&
          atoi(getenv("SUPLA_HVAC_PERF_MODE")) >= 0 &&
          atoi(getenv("SUPLA_HVAC_PERF_MODE")) != mode)
        continue;
      for (int d = 0; d < count; ++d) {
        auto ref =
            mode == 0
                ? 0
                : thermometers[mode == 1 ? d
                                         : (count == 1 ? 1 : (d + 1) % count)];
        query(
            "UPDATE supla_dev_channel SET user_config=JSON_OBJECT("
            "'mainThermometerChannelId'," +
            std::to_string(ref) + ",'minOnTimeS',17) WHERE iodevice_id=" +
            std::to_string(devices[d]) + " AND channel_number<15");
      }
      user->get_relation_index()->invalidate();
      for (int sample = -1; sample < 3; ++sample) {
        std::vector<std::future<std::string>> workers;
        auto start = Sql004Probe::clock_usec(), cpu = perf_cpu_usec();
        for (int d = 0; d < count; ++d) {
          workers.push_back(std::async(std::launch::async, [&, d] {
            Sql004Probe probe;
            auto worker_started = Sql004Probe::clock_usec();
            testing::DeviceStub device(protocol.get());
            device.set_id(devices[d]);
            device.set_flags(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED);
            device.set_user(user);
            supla_mariadb_access_provider db;
            EXPECT_TRUE(db.connect());
            supla_device_dao dao(&db);
            device.set_channels(
                new supla_device_channels(&dao, &device, nullptr, nullptr, 0));
            auto loaded = Sql004Probe::clock_usec();
            uint64_t upload_us = 0, prepare_us = 0;
            for (int c = 0; c < 15; ++c) {
              device.get_channels()->access_channel(
                  channels[d][c], [&](supla_device_channel *channel) {
                    TChannelConfig_HVAC incoming{};
                    // Remote business relationship is Cloud-owned. Device sends
                    // unset; protected merge preserves the current remote ref.
                    incoming.MainThermometerChannelId =
                        mode == 1 ? thermometers[d] : 0;
                    incoming.MinOnTimeS = 17;
                    auto upload_started = Sql004Probe::clock_usec();
                    EXPECT_EQ(SUPLA_CONFIG_RESULT_TRUE,
                              channel->set_user_config(
                                  SUPLA_CONFIG_TYPE_DEFAULT, sizeof(incoming),
                                  reinterpret_cast<char *>(&incoming)));
                    upload_us += Sql004Probe::clock_usec() - upload_started;
                    auto prepare_started = Sql004Probe::clock_usec();
                    TSDS_SetChannelConfig config{};
                    EXPECT_TRUE(channel->get_config(
                        &config, SUPLA_CONFIG_TYPE_DEFAULT, 0));
                    prepare_us += Sql004Probe::clock_usec() - prepare_started;
                  });
            }
            auto done = Sql004Probe::clock_usec();
            sql004 = nullptr;
            std::ostringstream out;
            out << "PERF_WORK devices=" << count << " mode=" << mode
                << " sample=" << sample << " d=" << d
                << " load_us=" << loaded - worker_started
                << " upload_us=" << upload_us << " prepare_us=" << prepare_us
                << " total_us=" << done - start << " selects=" << probe.selects
                << " calls=" << probe.calls
                << " config_reads=" << probe.config_reads
                << " connections=" << probe.connections
                << " sql_us=" << probe.sql_usec
                << " connect_us=" << probe.connect_usec;
            for (const auto &entry : probe.timings)
              out << " " << entry.first << "_n=" << entry.second.first << " "
                  << entry.first << "_us=" << entry.second.second;
            return out.str();
          }));
        }
        for (auto &worker : workers) std::cout << worker.get() << std::endl;
        std::cout << "PERF_BATCH devices=" << count << " mode=" << mode
                  << " sample=" << sample
                  << " wall_us=" << Sql004Probe::clock_usec() - start
                  << " cpu_us=" << perf_cpu_usec() - cpu << std::endl;
      }
      for (int clients : {1, 3}) {
        if (count != 12 && clients != 1) continue;
        for (int sample = -1; sample < 3; ++sample) {
          Sql004Probe probe;
          auto start = Sql004Probe::clock_usec(), cpu = perf_cpu_usec();
          uint64_t loads = 0, relations = 0, flags = 0, packs = 0, updates = 0;
          hvac_perf_reset();
          for (int i = 0; i < clients; ++i) {
            HvacPerfClient client(protocol.get(), user);
            auto t = Sql004Probe::clock_usec();
            client.load();
            loads += Sql004Probe::clock_usec() - t;
            auto collection = client.get_channels();
            t = Sql004Probe::clock_usec();
            supla_client_channel_reactions graph(nullptr);
            graph.load(collection);
            relations += Sql004Probe::clock_usec() - t;
            for (int d = 0; d < count; ++d) {
              auto ids = channels[d];
              ids.push_back(thermometers[d]);
              for (auto id : ids) {
                collection->channel_access(id, [&](supla_client_channel *ch) {
                  auto t = Sql004Probe::clock_usec();
                  ch->get_flags();
                  flags += Sql004Probe::clock_usec() - t;
                  TSC_SuplaChannel_E packet{};
                  t = Sql004Probe::clock_usec();
                  ch->proto_get(&packet, &client);
                  packs += Sql004Probe::clock_usec() - t;
                });
              }
            }
            hvac_config config;
            TChannelConfig_HVAC value{};
            value.MinOnTimeS = 18;
            config.set_config(&value);
            t = Sql004Probe::clock_usec();
            client.update_json_config(channels[0][0], SUPLA_CONFIG_TYPE_DEFAULT,
                                      &config);
            updates += Sql004Probe::clock_usec() - t;
          }
          std::cout << "PERF_CLIENT devices=" << count << " mode=" << mode
                    << " clients=" << clients << " sample=" << sample
                    << " load_us=" << loads << " relation_us=" << relations
                    << " flags_us=" << flags << " pack_us=" << packs
                    << " update_us=" << updates
                    << " wall_us=" << Sql004Probe::clock_usec() - start
                    << " cpu_us=" << perf_cpu_usec() - cpu << std::endl;
          hvac_perf_dump("client");
          probe.finish("client");
        }
      }

      if (count == 12) {
        testing::NiceMock<testing::SrpcAdapterMock> adapter;
        ON_CALL(adapter, get_proto_version())
            .WillByDefault(testing::Return(29));
        ON_CALL(adapter, sd_async_set_channel_config_result(testing::_))
            .WillByDefault(testing::Return(1));
        auto previous = supla_connection_test_access::swap_adapter(
            protocol.get(), &adapter);
        for (int clients : {0, 1, 3}) {
          std::vector<std::shared_ptr<HvacPerfClient>> active;
          for (int i = 0; i < clients; ++i) {
            int id = 500000 + i;
            query(
                "INSERT IGNORE INTO supla_client "
                "(id,access_id,guid,enabled,reg_date,software_version,"
                "protocol_version,user_id) SELECT " +
                std::to_string(id) + ",access_id,UNHEX(MD5('perf-client" +
                std::to_string(i) +
                "')),1,reg_date,'perf',29,user_id FROM supla_client WHERE "
                "id=1");
            auto client =
                std::make_shared<HvacPerfClient>(protocol.get(), user, id);
            client->load();
            ASSERT_TRUE(user->get_clients()->add(client));
            active.push_back(client);
          }
          auto device = std::make_shared<testing::DeviceStub>(protocol.get());
          device->set_id(devices[0]);
          device->set_flags(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED);
          device->set_user(user);
          supla_mariadb_access_provider db;
          ASSERT_TRUE(db.connect());
          supla_device_dao dao(&db);
          device->set_channels(new supla_device_channels(&dao, device.get(),
                                                         nullptr, nullptr, 0));
          for (int sample = -1; sample < 3; ++sample) {
            Sql004Probe probe;
            hvac_perf_reset();
            auto start = Sql004Probe::clock_usec(), cpu = perf_cpu_usec();
            for (int c = 0; c < 15; ++c) {
              TSDS_SetChannelConfig request{};
              request.ChannelNumber = c;
              request.ConfigType = SUPLA_CONFIG_TYPE_DEFAULT;
              request.ConfigSize = sizeof(TChannelConfig_HVAC);
              auto value =
                  reinterpret_cast<TChannelConfig_HVAC *>(request.Config);
              value->MinOnTimeS = 17;
              value->MainThermometerChannelId = mode == 1 ? thermometers[0] : 0;
              TsrpcReceivedData rd{};
              rd.data.sds_set_channel_config_request = &request;
              HvacPerfHandler handler;
              handler.handle_call(device, &adapter, &rd,
                                  SUPLA_DS_CALL_SET_CHANNEL_CONFIG, 29);
            }
            std::cout << "PERF_HANDLER devices=12 mode=" << mode
                      << " clients=" << clients << " sample=" << sample
                      << " uploads=15 wall_us="
                      << Sql004Probe::clock_usec() - start
                      << " cpu_us=" << perf_cpu_usec() - cpu << std::endl;
            hvac_perf_dump("handler");
            probe.finish("handler");
          }
          active.clear();
        }
        supla_connection_test_access::swap_adapter(protocol.get(), previous);
      }
    }
  }
}

TEST_F(SupLanHvacWireIntegrationTest, RegistrationPerformanceWire) {
  if (!getenv("SUPLA_HVAC_PERF")) GTEST_SKIP() << "opt-in native benchmark";
  perf_cloud_sql();
  ASSERT_FALSE(HasFatalFailure());
  // One real TLS/SRPC session, full production registration + identity + sync.
  query("UPDATE supla_dev_channel SET channel_number=0 WHERE id=" +
        std::to_string(hvac));
  for (int profile : {0, 20, -1, -3}) {
    for (int sample = -1; sample < 3; ++sample) {
      if (profile != 0 || sample != -1) {
        close_connection();
        start_connection();
      }
      TDS_SuplaRegisterDevice_G registration{};
      snprintf(registration.Email, sizeof(registration.Email), "%s",
               "hvac-wire@test.invalid");
      memset(registration.GUID, 0xab, sizeof(registration.GUID));
      registration.AuthKey[0] = 1;
      registration.Flags = SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED |
                           SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED;
      registration.channel_count = 16;
      for (int i = 0; i < 16; ++i) {
        auto &ch = registration.channels[i];
        ch.Number = i;
        ch.Type =
            i == 15 ? SUPLA_CHANNELTYPE_THERMOMETER : SUPLA_CHANNELTYPE_HVAC;
        ch.Default = i == 15 ? SUPLA_CHANNELFNC_THERMOMETER
                             : SUPLA_CHANNELFNC_HVAC_THERMOSTAT;
        ch.Flags = SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE;
      }
      uint64_t start = Sql004Probe::clock_usec(), accepted = 0, identity = 0;
      uint64_t cpu = perf_cpu_usec(), waited = 0;
      unsigned configs = 0, identities = 0;
      bool done = false;
      send_packet(SUPLA_DS_CALL_REGISTER_DEVICE_G, &registration,
                  sizeof(TDS_SuplaRegisterDeviceHeader) +
                      16 * sizeof(TDS_SuplaDeviceChannel_E));
      for (int n = 0; n < 100 && !done; ++n) {
        TSuplaDataPacket packet{};
        ASSERT_TRUE(receive_packet(&packet, 4000))
            << "profile=" << profile << " configs=" << configs;
        if (packet.call_id == SUPLA_SD_CALL_REGISTER_DEVICE_RESULT ||
            packet.call_id == SUPLA_SD_CALL_REGISTER_DEVICE_RESULT_B) {
          TSD_SuplaRegisterDeviceResult result{};
          memcpy(&result, packet.data, sizeof(result));
          ASSERT_EQ(SUPLA_RESULTCODE_TRUE, result.result_code);
          accepted = Sql004Probe::clock_usec();
        } else if (packet.call_id == SUPLA_SD_CALL_SUPLAN_DEVICE_IDENTITIES) {
          ++identities;
          TDS_SuplaDeviceIdentitiesResult result{SUPLA_SUPLAN_RESULT_OK, 789u};
          send_packet(SUPLA_DS_CALL_SUPLAN_DEVICE_IDENTITIES_RESULT, &result,
                      sizeof(result));
          identity = Sql004Probe::clock_usec();
        } else if (packet.call_id == SUPLA_SD_CALL_SET_CHANNEL_CONFIG) {
          TSDS_SetChannelConfig config{};
          memcpy(&config, packet.data, packet.data_size);
          ++configs;
          if (profile == -1 && configs == 1) continue;
          if (profile == -2) continue;
          if (profile > 0) {
            auto before = Sql004Probe::clock_usec();
            std::this_thread::sleep_for(std::chrono::milliseconds(profile));
            waited += Sql004Probe::clock_usec() - before;
          }
          TSDS_SetChannelConfigResult result{};
          result.ChannelNumber = config.ChannelNumber;
          result.ConfigType = config.ConfigType;
          result.Result = profile == -3 ? SUPLA_CONFIG_RESULT_FALSE
                                        : SUPLA_CONFIG_RESULT_TRUE;
          send_packet(SUPLA_DS_CALL_SET_CHANNEL_CONFIG_RESULT, &result,
                      sizeof(result));
        } else if (packet.call_id == SUPLA_SD_CALL_DEVICE_SYNC_DONE) {
          done = true;
        }
      }
      ASSERT_TRUE(done);
      if (profile == 0) {
        std::cout << "PERF_WIRE profile=" << profile << " sample=" << sample
                  << " registration_us=" << accepted - start
                  << " bootstrap_us=" << identity - accepted
                  << " sync_us=" << Sql004Probe::clock_usec() - identity
                  << " cpu_us=" << perf_cpu_usec() - cpu
                  << " injected_wait_us=" << waited << " configs=" << configs
                  << " identities=" << identities << " done=" << done
                  << std::endl;
        auto upload_start = Sql004Probe::clock_usec(),
             upload_cpu = perf_cpu_usec();
        unsigned results = 0, publications = 0;
        for (auto &value : hvac_perf_sql) value = 0;
        hvac_perf_relation_reads = 0;
        hvac_perf_tracking = true;
        for (int c = 0; c < 15; ++c) {
          TSDS_SetChannelConfig request{};
          request.ChannelNumber = c;
          request.ConfigType = SUPLA_CONFIG_TYPE_DEFAULT;
          request.ConfigSize = sizeof(TChannelConfig_HVAC);
          auto value = reinterpret_cast<TChannelConfig_HVAC *>(request.Config);
          value->MinOnTimeS = 17;
          send_packet(
              SUPLA_DS_CALL_SET_CHANNEL_CONFIG, &request,
              sizeof(request) - sizeof(request.Config) + request.ConfigSize);
          bool got_result = false, got_publication = false;
          while (!got_result || !got_publication) {
            TSuplaDataPacket packet{};
            ASSERT_TRUE(receive_packet(&packet));
            if (packet.call_id == SUPLA_SD_CALL_SET_CHANNEL_CONFIG_RESULT) {
              auto reply =
                  reinterpret_cast<TSDS_SetChannelConfigResult *>(packet.data);
              ASSERT_EQ(SUPLA_CONFIG_RESULT_TRUE, reply->Result);
              ASSERT_EQ(c, reply->ChannelNumber);
              ++results;
              got_result = true;
            } else if (packet.call_id == SUPLA_SD_CALL_SET_CHANNEL_CONFIG) {
              auto reply =
                  reinterpret_cast<TSDS_SetChannelConfig *>(packet.data);
              ASSERT_EQ(c, reply->ChannelNumber);
              ++publications;
              acknowledge(*reply);
              got_publication = true;
            }
          }
        }
        hvac_perf_tracking = false;
        std::cout << "PERF_UPLOAD_WIRE sample=" << sample << " uploads=15"
                  << " results=" << results << " publications=" << publications
                  << " wall_us=" << Sql004Probe::clock_usec() - upload_start
                  << " cpu_us=" << perf_cpu_usec() - upload_cpu
                  << " executions=" << hvac_perf_sql[0]
                  << " connections=" << hvac_perf_sql[3]
                  << " connect_us=" << hvac_perf_sql[4]
                  << " sql_us=" << hvac_perf_sql[5] << std::endl;
        // Preserve registration/sync timing before the upload measurement.
        continue;
      }
      std::cout << "PERF_WIRE profile=" << profile << " sample=" << sample
                << " registration_us=" << accepted - start
                << " bootstrap_us=" << identity - accepted
                << " sync_us=" << Sql004Probe::clock_usec() - identity
                << " cpu_us=" << perf_cpu_usec() - cpu
                << " injected_wait_us=" << waited << " configs=" << configs
                << " identities=" << identities << " done=" << done
                << std::endl;
    }
  }
}

TEST_F(SupLanHvacWireIntegrationTest, RegistrationPerformanceConcurrentWire) {
  if (!getenv("SUPLA_HVAC_PERF")) GTEST_SKIP() << "opt-in native benchmark";
  perf_cloud_sql();
  ASSERT_FALSE(HasFatalFailure());
  // Twelve distinct authenticated Device sessions; production connection
  // threads run in parallel. The test peers only register, accept identities
  // and ACK configuration, with no flash delay or real MCU stack claim.
  query("UPDATE supla_location SET user_id=99126 WHERE id=2");
  query("CALL supla_set_registration_enabled(99126,3600,3600)");
  query("UPDATE supla_user SET limit_iodev=200 WHERE id=99126");
  query(
      "UPDATE performance_schema.setup_instruments SET ENABLED='YES',"
      "TIMED='YES' WHERE NAME LIKE 'statement/%'");
  std::map<int, std::vector<std::vector<int>>> ids;
  for (int phase : {0, 1, 2, 3, 4, 5}) {
    for (int count : {1, 3, 12}) {
      if (phase >= 4 && count != 12) continue;
      for (int sample = -1; sample < 3; ++sample) {
        std::vector<std::unique_ptr<OtherConnection>> peers;
        for (int d = 0; d < count; ++d) {
          peers.emplace_back(new OtherConnection(this));
          peers.back()->select(false);
        }
        if (phase == 0) ids[count].emplace_back(count, 0);
        auto &group = ids[count][sample + 1];
        if (phase >= 2) {
          for (int d = 0; d < count; ++d) {
            int selected = group[phase == 2 ? d : (d + 1) % count];
            int thermometer =
                (phase >= 3 && count == 1)
                    ? resource
                    : scalar(
                          "SELECT id FROM supla_dev_channel WHERE "
                          "iodevice_id=" +
                          std::to_string(selected) + " AND channel_number=15");
            query(
                "UPDATE supla_dev_channel SET user_config=JSON_OBJECT("
                "'mainThermometerChannelId'," +
                std::to_string(thermometer) +
                ",'minOnTimeS',17) WHERE iodevice_id=" +
                std::to_string(group[d]) + " AND channel_number<15");
          }
        }
        std::vector<std::shared_ptr<HvacPerfClient>> active_clients;
        if (phase >= 4) {
          query("UPDATE supla_dev_channel SET hidden=1 WHERE user_id=99126");
          for (int id : group)
            query("UPDATE supla_dev_channel SET hidden=0 WHERE iodevice_id=" +
                  std::to_string(id));
          query("UPDATE supla_client SET user_id=99126 WHERE id=1");
          auto user = supla_user::find(99126, true);
          connection->get_srpc_adapter()->set_proto_version(29);
          for (int i = 0; i < (phase == 4 ? 1 : 3); ++i) {
            int id = 600000 + i;
            query(
                "INSERT IGNORE INTO supla_client "
                "(id,access_id,guid,enabled,reg_date,software_version,"
                "protocol_version,user_id) SELECT " +
                std::to_string(id) + ",access_id,UNHEX(MD5('perf-live-client" +
                std::to_string(i) +
                "')),1,reg_date,'perf',29,user_id FROM supla_client WHERE "
                "id=1");
            auto client =
                std::make_shared<HvacPerfClient>(connection, user, id);
            client->load();
            ASSERT_TRUE(user->get_clients()->add(client));
            active_clients.push_back(client);
          }
        }
        int sql_before = scalar(
            "SELECT CAST(COALESCE(SUM(COUNT_STAR),0) AS UNSIGNED) FROM "
            "performance_schema.events_statements_summary_by_digest "
            "WHERE SCHEMA_NAME='supla_test'");
        int time_before = scalar(
            "SELECT CAST(COALESCE(SUM(SUM_TIMER_WAIT)/1000000,0) AS UNSIGNED) "
            "FROM performance_schema.events_statements_summary_by_digest "
            "WHERE SCHEMA_NAME='supla_test'");
        int db_cpu_before = scalar(
            "SELECT CAST(COALESCE(SUM(CPU_TIME),0)*1000000 AS UNSIGNED) FROM "
            "information_schema.USER_STATISTICS WHERE USER='supla'");
        for (auto &value : hvac_perf_sql) value = 0;
        hvac_perf_relation_reads = 0;
        hvac_perf_tracking = true;
        auto start = Sql004Probe::clock_usec(), cpu = perf_cpu_usec();
        std::vector<uint64_t> accepted(count), identity(count), done(count);
        std::vector<unsigned> configs(count);
        for (int d = 0; d < count; ++d) {
          peers[d]->select(true);
          TDS_SuplaRegisterDevice_G registration{};
          snprintf(registration.Email, sizeof(registration.Email), "%s",
                   "hvac-wire@test.invalid");
          memset(registration.GUID, 0x73, sizeof(registration.GUID));
          registration.GUID[0] = count;
          registration.GUID[1] = sample + 1;
          registration.GUID[2] = d + 1;
          registration.AuthKey[0] = 1;
          registration.Flags = SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED |
                               SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED;
          registration.channel_count = 16;
          for (int c = 0; c < 16; ++c) {
            auto &ch = registration.channels[c];
            ch.Number = c;
            ch.Type = c == 15 ? SUPLA_CHANNELTYPE_THERMOMETER
                              : SUPLA_CHANNELTYPE_HVAC;
            ch.Default = c == 15 ? SUPLA_CHANNELFNC_THERMOMETER
                                 : SUPLA_CHANNELFNC_HVAC_THERMOSTAT;
            ch.Flags = SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE;
          }
          send_packet(SUPLA_DS_CALL_REGISTER_DEVICE_G, &registration,
                      sizeof(TDS_SuplaRegisterDeviceHeader) +
                          16 * sizeof(TDS_SuplaDeviceChannel_E));
          peers[d]->select(false);
        }
        int remaining = count;
        while (remaining) {
          for (int d = 0; d < count; ++d) {
            if (done[d]) continue;
            peers[d]->select(true);
            TSuplaDataPacket packet{};
            ASSERT_TRUE(receive_packet(&packet, 3000)) << "device=" << d;
            if (packet.call_id == SUPLA_SD_CALL_REGISTER_DEVICE_RESULT ||
                packet.call_id == SUPLA_SD_CALL_REGISTER_DEVICE_RESULT_B) {
              TSD_SuplaRegisterDeviceResult result{};
              memcpy(&result, packet.data, sizeof(result));
              ASSERT_EQ(SUPLA_RESULTCODE_TRUE, result.result_code);
              accepted[d] = Sql004Probe::clock_usec();
            } else if (packet.call_id ==
                       SUPLA_SD_CALL_SUPLAN_DEVICE_IDENTITIES) {
              if (phase == 0) {
                auto map =
                    reinterpret_cast<TSD_SuplaDeviceIdentities *>(packet.data);
                group[d] = map->DeviceId;
              }
              TDS_SuplaDeviceIdentitiesResult result{SUPLA_SUPLAN_RESULT_OK,
                                                     789u};
              send_packet(SUPLA_DS_CALL_SUPLAN_DEVICE_IDENTITIES_RESULT,
                          &result, sizeof(result));
              identity[d] = Sql004Probe::clock_usec();
            } else if (packet.call_id == SUPLA_SD_CALL_SET_CHANNEL_CONFIG) {
              TSDS_SetChannelConfig config{};
              memcpy(&config, packet.data, packet.data_size);
              ++configs[d];
              acknowledge(config);
            } else if (packet.call_id == SUPLA_SD_CALL_DEVICE_SYNC_DONE) {
              done[d] = Sql004Probe::clock_usec();
              --remaining;
            }
            peers[d]->select(false);
          }
        }
        for (int d = 0; d < count; ++d)
          std::cout << "PERF_MULTI phase=" << phase << " devices=" << count
                    << " sample=" << sample << " d=" << d
                    << " registration_us=" << accepted[d] - start
                    << " sync_us=" << done[d] - identity[d]
                    << " total_us=" << done[d] - start
                    << " configs=" << configs[d] << std::endl;
        hvac_perf_tracking = false;
        auto elapsed = Sql004Probe::clock_usec() - start;
        auto cpu_used = perf_cpu_usec() - cpu;
        int sql_after = scalar(
            "SELECT CAST(COALESCE(SUM(COUNT_STAR),0) AS UNSIGNED) FROM "
            "performance_schema.events_statements_summary_by_digest "
            "WHERE SCHEMA_NAME='supla_test'");
        int time_after = scalar(
            "SELECT CAST(COALESCE(SUM(SUM_TIMER_WAIT)/1000000,0) AS UNSIGNED) "
            "FROM performance_schema.events_statements_summary_by_digest "
            "WHERE SCHEMA_NAME='supla_test'");
        int db_cpu_after = scalar(
            "SELECT CAST(COALESCE(SUM(CPU_TIME),0)*1000000 AS UNSIGNED) FROM "
            "information_schema.USER_STATISTICS WHERE USER='supla'");
        std::cout << "PERF_MULTI_BATCH phase=" << phase << " devices=" << count
                  << " sample=" << sample << " wall_us=" << elapsed
                  << " cpu_us=" << cpu_used
                  << " ps_statements=" << sql_after - sql_before
                  << " ps_sql_us=" << time_after - time_before
                  << " db_cpu_us=" << db_cpu_after - db_cpu_before
                  << " executions=" << hvac_perf_sql[0]
                  << " selects=" << hvac_perf_sql[1]
                  << " calls=" << hvac_perf_sql[2]
                  << " relation_reads=" << hvac_perf_relation_reads.load()
                  << " connections=" << hvac_perf_sql[3]
                  << " connect_us=" << hvac_perf_sql[4]
                  << " sql_us=" << hvac_perf_sql[5] << std::endl;
        for (auto &peer : peers) peer->close();
      }
    }
  }
}

#include <malloc.h>

TEST_F(SupLanHvacIntegrationTest,
       RelationIndexCommittedMergeAndConcurrentWriters) {
  config(resource);
  auto user = supla_user::find(99124, true);
  auto cache = user->get_relation_index();
  cache->invalidate();
  ASSERT_TRUE(cache->refresh() || cache->snapshot());
  auto initial = cache->snapshot();
  ASSERT_TRUE(initial);
  EXPECT_EQ(1u, initial->get(hvac, relation_with_sub_channel, 29).size());
  auto stale = cache->begin(hvac);
  auto newer = cache->begin(hvac);
  query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
        std::to_string(hvac));
  supla_json_config unset;
  unset.set_user_config("{}");
  EXPECT_TRUE(
      cache->complete(hvac, newer, &unset, SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
  supla_json_config old;
  old.set_user_config(
      ("{\"mainThermometerChannelId\":" + std::to_string(resource) + "}")
          .c_str());
  EXPECT_FALSE(
      cache->complete(hvac, stale, &old, SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
  EXPECT_TRUE(
      cache->snapshot()->get(hvac, relation_with_sub_channel, 29).empty());
  EXPECT_EQ(1u, initial->get(hvac, relation_with_sub_channel, 29).size());

  auto second = add_channel(destination, 239, SUPLA_CHANNELTYPE_HVAC,
                            SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  cache->invalidate();
  cache->refresh();
  std::promise<void> start;
  auto ready = start.get_future().share();
  std::promise<void> a_started, b_started;
  auto write = [&](uint32_t id, std::promise<void> &begun) {
    auto ticket = cache->begin(id);
    query(
        "UPDATE supla_dev_channel SET "
        "user_config=JSON_OBJECT('mainThermometerChannelId'," +
        std::to_string(resource) + ") WHERE id=" + std::to_string(id));
    begun.set_value();
    ready.wait();
    return cache->complete(id, ticket, &old, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  };
  auto a = std::async(std::launch::async, write, hvac, std::ref(a_started));
  auto b = std::async(std::launch::async, write, second, std::ref(b_started));
  a_started.get_future().wait();
  b_started.get_future().wait();
  start.set_value();
  EXPECT_TRUE(a.get());
  EXPECT_TRUE(b.get());
  auto current = cache->snapshot();
  EXPECT_EQ(1u, current->get(hvac, relation_with_sub_channel, 29).size());
  EXPECT_EQ(1u, current->get(second, relation_with_sub_channel, 29).size());
  auto builds = cache->build_count();
  old.set_user_config(
      ("{\"mainThermometerChannelId\":" + std::to_string(resource) +
       ",\"minOnTimeS\":23,\"weeklySchedule\":{}}")
          .c_str());
  EXPECT_FALSE(cache->complete(hvac, cache->begin(hvac), &old,
                               SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
  EXPECT_EQ(builds, cache->build_count());
  {
    Sql004Probe probe;
    for (int i = 0; i < 100; ++i)
      cache->snapshot()->get(hvac, relation_any, 29);
    EXPECT_EQ(0u, probe.executions);
    EXPECT_EQ(0u, probe.connections);
  }
}
TEST_F(SupLanHvacIntegrationTest, RelationIndexSqlFailureKeepsSnapshot) {
  config(resource);
  auto cache = supla_user::find(99124, true)->get_relation_index();
  cache->invalidate();
  cache->refresh();
  auto initial = cache->snapshot();
  ASSERT_TRUE(initial);
  {
    Sql004Probe probe;
    probe.fail_execute = [](const std::string &sql) {
      return sql.find("SELECT c.id,c.iodevice_id") == 0;
    };
    cache->invalidate();
    EXPECT_FALSE(cache->refresh());
  }
  EXPECT_EQ(initial, cache->snapshot());
  EXPECT_GT(cache->time_left_usec(), 4000000u);
  cache->cancel(hvac, cache->begin(hvac));
  EXPECT_GT(cache->time_left_usec(), 4000000u);
  query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
        std::to_string(hvac));
  cache->invalidate();
  EXPECT_TRUE(cache->refresh());
  EXPECT_TRUE(
      cache->snapshot()->get(hvac, relation_with_sub_channel, 29).empty());
  EXPECT_EQ(0u, cache->time_left_usec());
}

TEST_F(SupLanHvacIntegrationTest, EmptyReferencesNeverOpenReferenceDao) {
  config(0);
  with_destination([&](supla_device_channel *channel) {
    TChannelConfig_HVAC value{};
    value.MinOnTimeS = 17;
    Sql004Probe probe;
    EXPECT_EQ(SUPLA_CONFIG_RESULT_TRUE,
              channel->set_user_config(SUPLA_CONFIG_TYPE_DEFAULT, sizeof(value),
                                       reinterpret_cast<char *>(&value)));
    EXPECT_EQ(2u, probe.config_reads);
    EXPECT_EQ(2u, probe.selects);
    EXPECT_EQ(2u, probe.connections);
    EXPECT_EQ(1u, probe.calls);
    EXPECT_EQ(0u, probe.timings["channel_validation"].first);
  });
}

TEST_F(SupLanHvacIntegrationTest, RelationIndexPerformanceLargest) {
  if (!getenv("SUPLA_HVAC_PERF")) GTEST_SKIP() << "opt-in native benchmark";
  perf_cloud_sql();
  ASSERT_FALSE(HasFatalFailure());
  query(
      "INSERT INTO supla_user "
      "(id,short_unique_id,long_unique_id,salt,email,enabled,"
      "reg_date,timezone,home_latitude,home_longitude) SELECT "
      "99234,REPEAT('7',32),"
      "'relation-perf',salt,'relation-perf@test.invalid',enabled,reg_date,"
      "timezone,"
      "home_latitude,home_longitude FROM supla_user WHERE id=99124");
  query(
      "INSERT INTO supla_location(id,user_id,password,enabled) "
      "VALUES(99234,99234,'test',1)");
  query(
      "INSERT INTO supla_accessid(id,user_id,password,enabled) "
      "VALUES(99234,99234,'test',1)");
  query(
      "INSERT INTO supla_rel_aidloc(access_id,location_id) "
      "VALUES(99234,99234)");
  std::vector<int> devices;
  std::vector<uint32_t> thermostats, thermometers;
  for (int d = 0; d < 12; ++d) {
    int id = 410000 + d;
    devices.push_back(id);
    query(
        "INSERT INTO "
        "supla_iodevice(id,location_id,user_id,guid,enabled,reg_date,protocol_"
        "version,flags) "
        "SELECT " +
        std::to_string(id) + ",99234,99234,UNHEX(MD5('relation-perf" +
        std::to_string(d) +
        "')),1,reg_date,29,flags FROM supla_iodevice WHERE id=" +
        std::to_string(destination));
    for (int c = 0; c < 15; ++c)
      thermostats.push_back(add_channel(id, c, SUPLA_CHANNELTYPE_HVAC,
                                        SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
    thermometers.push_back(add_channel(id, 15, SUPLA_CHANNELTYPE_THERMOMETER,
                                       SUPLA_CHANNELFNC_THERMOMETER));
  }
  query("UPDATE supla_dev_channel SET user_id=99234,flags=" +
        std::to_string(SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE) +
        " WHERE iodevice_id BETWEEN 410000 AND 410011");
  // Canonical, trusted Cloud-like reference writes, not Device remote upload.
  for (int d = 0; d < 12; ++d)
    query(
        "UPDATE supla_dev_channel SET "
        "user_config=JSON_OBJECT('mainThermometerChannelId'," +
        std::to_string(thermometers[(d + 1) % 12]) +
        ",'minOnTimeS',17) WHERE iodevice_id=" + std::to_string(devices[d]) +
        " AND channel_number<15");
  query("UPDATE supla_client SET access_id=99234,user_id=99234 WHERE id=1");
  supla_connection_test_access protocol;
  ASSERT_TRUE(protocol.open());
  auto user = supla_user::find(99234, true);
  auto cache = user->get_relation_index();
  for (int sample = -1; sample < 3; ++sample) {
    supla_channel_relation_index cold(99234);
    Sql004Probe probe;
    hvac_perf_reset();
    auto start = Sql004Probe::clock_usec();
    ASSERT_TRUE(cold.refresh() || cold.snapshot());
    auto elapsed = Sql004Probe::clock_usec() - start;
    auto snap = cold.snapshot();
    ASSERT_EQ(192u, snap->nodes.size());
    std::cout << "PERF_INDEX_FIRST sample=" << sample << " wall_us=" << elapsed
              << " selects=" << probe.selects
              << " connections=" << probe.connections
              << " payload_bytes=" << snap->memory_bytes()
              << " nodes=" << snap->nodes.size()
              << " edges=" << snap->edges.size() << std::endl;
  }
  cache->invalidate();
  cache->refresh();
  {
    auto before = mallinfo2().uordblks;
    auto snap = cache->snapshot();
    auto copy = std::make_shared<supla_relation_snapshot>();
    copy->nodes = snap->nodes;
    copy->build();
    auto allocated = mallinfo2().uordblks;
    std::cout << "PERF_INDEX_RAM node_size=" << sizeof(supla_relation_node)
              << " edge_size=" << sizeof(supla_relation_edge)
              << " owner_size=" << sizeof(supla_channel_relation_index)
              << " snapshot_live_delta=" << allocated - before
              << " payload_bytes=" << copy->memory_bytes() << std::endl;
  }
  for (int clients : {1, 3}) {
    std::vector<std::shared_ptr<HvacPerfClient>> active;
    for (int i = 0; i < clients; ++i) {
      int id = 610000 + i;
      query(
          "INSERT IGNORE INTO "
          "supla_client(id,access_id,user_id,guid,enabled,reg_date,"
          "software_version,protocol_version) SELECT " +
          std::to_string(id) + ",99234,99234,UNHEX(MD5('index-perf-client" +
          std::to_string(i) +
          "')),1,reg_date,'perf',29 FROM supla_client WHERE id=1");
      auto client = std::make_shared<HvacPerfClient>(protocol.get(), user, id);
      ASSERT_TRUE(user->get_clients()->add(client));
      active.push_back(client);
    }
    for (int sample = -1; sample < 3; ++sample) {
      Sql004Probe probe;
      auto start = Sql004Probe::clock_usec();
      for (auto &client : active) client->load();
      std::cout << "PERF_INDEX_CLIENT clients=" << clients
                << " sample=" << sample
                << " wall_us=" << Sql004Probe::clock_usec() - start
                << " selects=" << probe.selects << std::endl;
      hvac_perf_reset();
      auto builds = cache->build_count();
      {
        Sql004Probe reads;
        auto t = Sql004Probe::clock_usec();
        for (auto &client : active) {
          supla_client_channel_reactions projection(nullptr);
          projection.load(client->get_channels());
          client->get_channels()->for_each(
              [](supla_client_channel *c, bool *) { c->get_flags(); });
        }
        std::cout << "PERF_INDEX_READ clients=" << clients
                  << " sample=" << sample
                  << " wall_us=" << Sql004Probe::clock_usec() - t
                  << " selects=" << reads.selects
                  << " copies=" << hvac_perf_counts[2]
                  << " parses=" << hvac_perf_counts[3] << std::endl;
        EXPECT_EQ(0u, reads.executions);
        EXPECT_EQ(0u, hvac_perf_counts[2].load());
        EXPECT_EQ(0u, hvac_perf_counts[3].load());
      }
      // Use the production handler for 15 identical Device uploads.
      auto device = std::make_shared<testing::DeviceStub>(protocol.get());
      device->set_id(devices[0]);
      device->set_user(user);
      device->set_flags(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED);
      supla_mariadb_access_provider db;
      ASSERT_TRUE(db.connect());
      supla_device_dao dao(&db);
      device->set_channels(
          new supla_device_channels(&dao, device.get(), nullptr, nullptr, 0));
      testing::NiceMock<testing::SrpcAdapterMock> adapter;
      ON_CALL(adapter, sd_async_set_channel_config_result(testing::_))
          .WillByDefault(testing::Return(1));
      auto t = Sql004Probe::clock_usec();
      Sql004Probe writes;
      for (int c = 0; c < 15; ++c) {
        TSDS_SetChannelConfig request{};
        request.ChannelNumber = c;
        request.ConfigType = SUPLA_CONFIG_TYPE_DEFAULT;
        request.ConfigSize = sizeof(TChannelConfig_HVAC);
        reinterpret_cast<TChannelConfig_HVAC *>(request.Config)->MinOnTimeS =
            17;
        TsrpcReceivedData rd{};
        rd.data.sds_set_channel_config_request = &request;
        HvacPerfHandler handler;
        handler.handle_call(device, &adapter, &rd,
                            SUPLA_DS_CALL_SET_CHANNEL_CONFIG, 29);
      }
      std::cout << "PERF_INDEX_IDENTICAL clients=" << clients
                << " sample=" << sample
                << " wall_us=" << Sql004Probe::clock_usec() - t
                << " selects=" << writes.selects << " calls=" << writes.calls
                << " connections=" << writes.connections
                << " builds=" << cache->build_count() - builds << std::endl;
      EXPECT_EQ(builds, cache->build_count());
      sql004 = nullptr;
      // Trusted replace and remove, then only the existing Client projection.
      for (bool remove : {false, true}) {
        auto id = thermostats[0];
        auto ticket = cache->begin(id);
        int ref = remove ? 0 : thermometers[2];
        auto json = "{\"mainThermometerChannelId\":" + std::to_string(ref) +
                    ",\"minOnTimeS\":17}";
        query("UPDATE supla_dev_channel SET user_config='" + json +
              "' WHERE id=" + std::to_string(id));
        supla_json_config config;
        config.set_user_config(json.c_str());
        auto start = Sql004Probe::clock_usec();
        auto old_builds = cache->build_count();
        ASSERT_TRUE(cache->complete(id, ticket, &config,
                                    SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
        user->get_clients()->update_json_config(id, SUPLA_CONFIG_TYPE_DEFAULT,
                                                &config);
        std::cout << "PERF_INDEX_CHANGE clients=" << clients
                  << " sample=" << sample << " remove=" << remove
                  << " wall_us=" << Sql004Probe::clock_usec() - start
                  << " builds=" << cache->build_count() - old_builds
                  << std::endl;
      }
      // Restore the trusted remote reference for the next identical replay.
      auto json =
          "{\"mainThermometerChannelId\":" + std::to_string(thermometers[1]) +
          ",\"minOnTimeS\":17}";
      query("UPDATE supla_dev_channel SET user_config='" + json +
            "' WHERE id=" + std::to_string(thermostats[0]));
      supla_json_config restore;
      restore.set_user_config(json.c_str());
      cache->complete(thermostats[0], cache->begin(thermostats[0]), &restore,
                      SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
    }
  }
}

TEST_F(SupLanHvacIntegrationTest, FullRelationRefreshCannotLoseInFlightCommit) {
  config(resource);
  auto cache = supla_user::find(99124, true)->get_relation_index();
  cache->invalidate();
  cache->refresh();
  auto before = cache->snapshot();
  auto ticket = cache->begin(hvac);
  cache->invalidate();
  // DB SELECT may complete before an already-started writer commits.
  EXPECT_FALSE(cache->refresh());
  EXPECT_EQ(before, cache->snapshot());
  query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
        std::to_string(hvac));
  supla_json_config empty;
  empty.set_user_config("{}");
  EXPECT_TRUE(
      cache->complete(hvac, ticket, &empty, SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
  EXPECT_TRUE(
      cache->snapshot()->get(hvac, relation_with_sub_channel, 29).empty());
  EXPECT_EQ(1u, cache->time_left_usec());
  cache->refresh();
  EXPECT_TRUE(
      cache->snapshot()->get(hvac, relation_with_sub_channel, 29).empty());
  EXPECT_EQ(0u, cache->time_left_usec());
}

TEST_F(SupLanHvacIntegrationTest,
       IndexedClientProjectionPreservesVisibilityAndRemoval) {
  config(resource);
  query(
      "UPDATE supla_dev_channel SET hidden=1 WHERE user_id=99124 AND id NOT "
      "IN(" +
      std::to_string(hvac) + "," + std::to_string(resource) + ")");
  supla_connection_test_access protocol;
  ASSERT_TRUE(protocol.open());
  auto user = supla_user::find(99124, true);
  auto cache = user->get_relation_index();
  cache->invalidate();
  cache->refresh();
  HvacPerfClient client(protocol.get(), user);
  client.load();
  testing::NiceMock<testing::SrpcAdapterMock> adapter;
  ON_CALL(adapter, get_proto_version()).WillByDefault(testing::Return(29));
  TSC_SuplaChannelRelationPack captured{};
  EXPECT_CALL(adapter, sc_async_channel_relation_pack_update(testing::_))
      .WillRepeatedly([&](TSC_SuplaChannelRelationPack *pack) {
        captured = *pack;
        return 1;
      });
  supla_client_channel_relation_remote_updater updater(&adapter);
  supla_client_channel_reactions graph(&updater);
  ASSERT_TRUE(graph.load(client.get_channels()));
  ASSERT_TRUE(graph.update_remote());
  ASSERT_EQ(2, captured.count);
  EXPECT_EQ(resource, captured.items[0].Id);
  EXPECT_EQ(hvac, captured.items[0].ParentId);
  EXPECT_TRUE(captured.items[0].EOL & START_OF_LIST_BIT);
  auto revision = cache->build_count();
  supla_json_config scalar;
  scalar.set_user_config(("{\"mainThermometerChannelId\":" +
                          std::to_string(resource) + ",\"minOnTimeS\":22}")
                             .c_str());
  EXPECT_FALSE(cache->complete(hvac, cache->begin(hvac), &scalar,
                               SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
  EXPECT_EQ(revision, cache->build_count());
  EXPECT_FALSE(graph.load(client.get_channels(), false));
  query("UPDATE supla_dev_channel SET hidden=1 WHERE id=" +
        std::to_string(resource));
  client.load();
  ASSERT_TRUE(graph.load(client.get_channels()));
  ASSERT_TRUE(graph.update_remote());
  ASSERT_EQ(1, captured.count);
  EXPECT_EQ(0, captured.items[0].Id);
  EXPECT_TRUE(captured.items[0].EOL & START_OF_LIST_BIT);
  EXPECT_TRUE(captured.items[0].EOL & END_OF_LIST_BIT);
  auto ticket = cache->begin(hvac);
  query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
        std::to_string(hvac));
  supla_json_config empty;
  empty.set_user_config("{}");
  ASSERT_TRUE(
      cache->complete(hvac, ticket, &empty, SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
  // Hidden-source removal is not sent to a Client whose graph was already
  // empty.
  EXPECT_FALSE(graph.load(client.get_channels(), false));
  query("UPDATE supla_dev_channel SET hidden=0 WHERE id=" +
        std::to_string(resource));
  client.load();
  EXPECT_FALSE(graph.load(client.get_channels()));
}

TEST_F(SupLanHvacIntegrationTest, RelationIndexSourceMetadataAndAuxType) {
  auto user = supla_user::find(99124, true);
  auto cache = user->get_relation_index();
  auto json = "{\"auxThermometerChannelId\":" + std::to_string(resource) +
              ",\"auxThermometerType\":\"FLOOR\"}";
  query("UPDATE supla_dev_channel SET user_config='" + json + "' WHERE id=" +
        std::to_string(hvac));
  cache->invalidate();
  cache->refresh();
  auto before = cache->snapshot();
  auto relations = before->get(hvac, relation_with_sub_channel, 29);
  ASSERT_EQ(1u, relations.size());
  auto first_type = relations.front().get_relation_type();
  supla_json_config changed;
  json = "{\"auxThermometerChannelId\":" + std::to_string(resource) +
         ",\"auxThermometerType\":\"GENERIC_HEATER\"}";
  query("UPDATE supla_dev_channel SET user_config='" + json + "' WHERE id=" +
        std::to_string(hvac));
  changed.set_user_config(json.c_str());
  ASSERT_TRUE(cache->complete(hvac, cache->begin(hvac), &changed,
                             SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
  relations = cache->snapshot()->get(hvac, relation_with_sub_channel, 29);
  ASSERT_EQ(1u, relations.size());
  EXPECT_NE(first_type, relations.front().get_relation_type());
  // Consumer JSON stays unchanged when the remote Source changes function.
  query("UPDATE supla_dev_channel SET func=0 WHERE id=" +
        std::to_string(resource));
  cache->invalidate();
  ASSERT_TRUE(cache->refresh());
  EXPECT_TRUE(cache->snapshot()->get(hvac, relation_any, 29).empty());
  query("UPDATE supla_dev_channel SET func=" +
        std::to_string(SUPLA_CHANNELFNC_THERMOMETER) + " WHERE id=" +
        std::to_string(resource));
  cache->invalidate();
  ASSERT_TRUE(cache->refresh());
  EXPECT_EQ(1u, cache->snapshot()->get(hvac, relation_any, 29).size());
  query("DELETE FROM supla_dev_channel WHERE id=" + std::to_string(resource));
  cache->invalidate();
  ASSERT_TRUE(cache->refresh());
  EXPECT_TRUE(cache->snapshot()->get(hvac, relation_any, 29).empty());
  // Readers of the old immutable snapshot remain valid after replacement.
  EXPECT_EQ(1u, before->get(hvac, relation_any, 29).size());
}

TEST_F(SupLanCloudGrantIntegrationTest,
       ReconnectSharesOneRepositoryAcrossPeers) {
  for (int i = 0; i < 12; ++i) {
    int id = 710000 + i;
    execute(
        "INSERT INTO supla_iodevice "
        "(id,user_id,location_id,guid,enabled,reg_date,protocol_version,flags) "
        "SELECT " +
        std::to_string(id) + ",user_id,location_id,UNHEX(MD5('reconnect-peer-" +
        std::to_string(i) +
        "')),1,reg_date,29,flags FROM supla_iodevice "
        "WHERE id=" +
        std::to_string(destination));
    ASSERT_TRUE(factory()->accept_identity(id, {456, {}}));
  }
  int original_destination = destination;
  for (int i = 0; i < 12; ++i) {
    destination = 710000 + i;
    replace(USER_APPROVAL_ORIGIN, 810000 + i, 1);
  }
  destination = original_destination;
  unsigned repositories = 0;
  auto counted_factory = [&]() -> std::unique_ptr<repository> {
    ++repositories;
    return factory();
  };
  DeleteTransport transport;
  peer_provisioner peers(counted_factory, &transport);
  Sql004Probe probe;
  probe.restricted = true;
  auto started = Sql004Probe::clock_usec();
  peers.reconnect(source);
  auto elapsed = Sql004Probe::clock_usec() - started;
  EXPECT_EQ(12u, transport.sources.size());
  EXPECT_TRUE(transport.destinations.empty());
  for (const auto &request : transport.sources) {
    EXPECT_EQ(source, request.PeerContext.SourceNodeId);
    EXPECT_EQ(123u, request.PeerContext.RootEpoch);
    ASSERT_EQ(1, request.AclEntryCount);
    EXPECT_EQ(resource, request.Acl[0].ResourceId);
    EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_READ, request.Acl[0].Permissions);
    EXPECT_NE(0, request.Flags);
  }
  std::cout << "RECONNECT_SQL repositories=" << repositories
            << " connections=" << probe.connections
            << " selects=" << probe.selects << " calls=" << probe.calls
            << " dml=" << probe.dml << " elapsed_us=" << elapsed << std::endl;
  EXPECT_EQ(1u, repositories);
  EXPECT_EQ(1u, probe.connections);
  EXPECT_EQ(0u, probe.dml);
}

TEST_F(SupLanHvacIntegrationTest, RelationLoginRecoversLostCloudHint) {
  config(resource);
  auto user = supla_user::find(99124, true);
  auto index = user->get_relation_index();
  index->invalidate();
  index->refresh();
  ASSERT_EQ(1u, index->snapshot()->get(hvac, relation_any, 29).size());
  query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
        std::to_string(hvac));
  supla_connection_test_access protocol;
  ASSERT_TRUE(protocol.open());
  HvacPerfClient client(protocol.get(), user);
  client
      .load();  // Production login path, with no Cloud IPC or Device reconnect.
  EXPECT_TRUE(index->snapshot()->get(hvac, relation_any, 29).empty());
}

TEST_F(SupLanHvacIntegrationTest, RelationWriterWaitIsNotSqlFailure) {
  config(resource);
  auto index = supla_user::find(99124, true)->get_relation_index();
  index->invalidate();
  index->refresh();
  auto before = index->snapshot();
  auto ticket = index->begin(hvac);
  index->invalidate();
  EXPECT_EQ(0u, index->time_left_usec());
  {
    Sql004Probe probe;
    EXPECT_FALSE(index->refresh());
    EXPECT_EQ(0u, probe.executions);
    EXPECT_EQ(0u, probe.connections);
  }
  index->cancel(hvac, ticket);
  EXPECT_EQ(1u, index->time_left_usec());
  index->refresh();
  EXPECT_EQ(0u, index->time_left_usec());
  auto clean = index->snapshot();
  index->cancel(hvac, index->begin(hvac));  // Rejected input, not a SQL fault.
  EXPECT_EQ(clean, index->snapshot());
  EXPECT_EQ(0u, index->time_left_usec());
  supla_json_config empty;
  empty.set_user_config("{}");
  EXPECT_FALSE(index->complete(999999, index->begin(999999), &empty, -1));
  EXPECT_EQ(1u, index->time_left_usec());
  EXPECT_EQ(clean, index->snapshot());
}

TEST_F(SupLanHvacIntegrationTest, RelationReloadAcceptsMaximumTextColumns) {
  // Match Cloud Version20240906073433; the old Core base fixture uses VARCHAR.
  query(
      "ALTER TABLE supla_dev_channel MODIFY user_config TEXT "
      "CHARACTER SET utf8mb4 DEFAULT NULL, MODIFY properties TEXT "
      "CHARACTER SET utf8mb4 DEFAULT NULL");
  auto id = std::to_string(resource);
  query(
      "UPDATE supla_dev_channel SET user_config=JSON_OBJECT("
      "'mainThermometerChannelId'," +
      id +
      ", 'ignored',REPEAT('x',65535-"
      "OCTET_LENGTH(JSON_OBJECT('mainThermometerChannelId'," +
      id +
      ",'ignored','')))), properties=JSON_OBJECT('ignored',REPEAT('y',65535-"
      "OCTET_LENGTH(JSON_OBJECT('ignored','')))) WHERE id=" +
      std::to_string(hvac));
  EXPECT_EQ(65535, scalar("SELECT OCTET_LENGTH(user_config) FROM "
                          "supla_dev_channel WHERE id=" +
                          std::to_string(hvac)));
  EXPECT_EQ(65535, scalar("SELECT OCTET_LENGTH(properties) FROM "
                          "supla_dev_channel WHERE id=" +
                          std::to_string(hvac)));
  auto index = supla_user::find(99124, true)->get_relation_index();
  index->invalidate();
  index->refresh();
  ASSERT_TRUE(index->snapshot());
  auto rows = index->snapshot()->get(hvac, relation_any, 29);
  ASSERT_EQ(1u, rows.size());
  EXPECT_EQ(resource, rows[0].get_id());
  EXPECT_EQ(0u, index->time_left_usec());
}

TEST_F(SupLanHvacIntegrationTest, ClientWeeklyMergeRefreshesRelationRoot) {
  config(resource);
  auto user = supla_user::find(99124, true);
  auto index = user->get_relation_index();
  index->invalidate();
  index->refresh();
  supla_connection_test_access protocol;
  ASSERT_TRUE(protocol.open());
  auto client = std::make_shared<M4ConfigClient>(protocol.get(), user);
  char value[SUPLA_CHANNELVALUE_SIZE]{};
  ASSERT_TRUE(client->get_channels()->add(new supla_client_channel(
      client->get_channels(), hvac, 240, destination, 0, SUPLA_CHANNELTYPE_HVAC,
      SUPLA_CHANNELFNC_HVAC_THERMOSTAT, 0, 0, 0, 0, nullptr, nullptr, nullptr,
      "", 0, 0, 0, 0, 29, SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE, value, 0, "{}",
      "{}", false)));
  // A committed Cloud change is learned through the successful Client merge.
  config(other_resource);
  TChannelConfig_WeeklySchedule weekly{};
  TSCS_ChannelConfig request{};
  request.ChannelId = hvac;
  request.ConfigType = SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE;
  request.ConfigSize = sizeof(weekly);
  memcpy(request.Config, &weekly, sizeof(weekly));
  TsrpcReceivedData rd{};
  rd.data.scs_channel_config = &request;
  testing::SrpcAdapterMock reply;
  EXPECT_CALL(reply, sc_async_channel_config_update_or_result(testing::_))
      .WillOnce([](TSC_ChannelConfigUpdateOrResult *r) {
        EXPECT_EQ(SUPLA_CONFIG_RESULT_TRUE, r->Result);
        return 1;
      });
  ASSERT_TRUE(client->get_srpc_call_handler_collection()->handle_call(
      client, &reply, &rd, SUPLA_CS_CALL_SET_CHANNEL_CONFIG, 29));
  EXPECT_EQ(other_resource, index->snapshot()->nodes.at(hvac).hvac[0]);
  EXPECT_EQ(0u, index->time_left_usec());
}

TEST_F(SupLanHvacIntegrationTest,
       RelationReloadChurnRetriesWithoutFiveSecondDelay) {
  config(resource);
  auto index = supla_user::find(99124, true)->get_relation_index();
  index->invalidate();
  index->refresh();
  bool changed = false;
  {
    Sql004Probe probe;
    probe.after_relation_read = [&] {
      auto ticket = index->begin(hvac);
      config(other_resource);
      supla_json_config current;
      current.set_user_config(("{\"mainThermometerChannelId\":" +
                               std::to_string(other_resource) + "}")
                                  .c_str());
      EXPECT_TRUE(index->complete(hvac, ticket, &current, -1));
      changed = true;
    };
    index->invalidate();
    EXPECT_FALSE(index->refresh());
    EXPECT_TRUE(changed);
  }
  EXPECT_EQ(other_resource, index->snapshot()->nodes.at(hvac).hvac[0]);
  EXPECT_EQ(1u, index->time_left_usec());
  index->refresh();
  EXPECT_EQ(0u, index->time_left_usec());
  EXPECT_EQ(other_resource, index->snapshot()->nodes.at(hvac).hvac[0]);
}

TEST_F(SupLanHvacIntegrationTest, StaleFallbackCannotReplacePublishedIndex) {
  config(resource);
  query(
      "INSERT INTO supla_user "
      "(id,short_unique_id,long_unique_id,salt,email,enabled,reg_date,"
      "timezone,home_latitude,home_longitude) SELECT 99303,REPEAT('9',32),"
      "'fallback-test',salt,'fallback@test.invalid',enabled,reg_date,"
      "timezone,home_latitude,home_longitude FROM supla_user WHERE id=99124");
  query("UPDATE supla_iodevice SET user_id=99303 WHERE user_id=99124");
  query("UPDATE supla_dev_channel SET user_id=99303 WHERE user_id=99124");
  auto user = supla_user::find(99303, true);
  auto index = user->get_relation_index();
  ASSERT_FALSE(index->snapshot());
  supla_connection_test_access protocol;
  ASSERT_TRUE(protocol.open());
  HvacPerfClient client(protocol.get(), user);
  class PausedChannel : public supla_client_channel {
   public:
    using supla_client_channel::supla_client_channel;
    std::promise<void> reached, release;
    supla_json_config *get_json_config() override {
      auto copy = supla_client_channel::get_json_config();
      reached.set_value();
      release.get_future().wait();
      return copy;
    }
  };
  class Projection : public supla_client_channel_reactions {
   public:
    using supla_client_channel_reactions::supla_client_channel_reactions;
    using supla_dobjects::lock;
    using supla_dobjects::unlock;
  };
  auto channels = client.get_channels();
  char value[SUPLA_CHANNELVALUE_SIZE]{};
  auto json = "{\"mainThermometerChannelId\":" + std::to_string(resource) + "}";
  auto channel = new PausedChannel(
      channels, hvac, 240, destination, 0, SUPLA_CHANNELTYPE_HVAC,
      SUPLA_CHANNELFNC_HVAC_THERMOSTAT, 0, 0, 0, 0, nullptr, nullptr, nullptr,
      "", 0, 0, 0, 0, 29, 0, value, 0, json.c_str(), "{}", false);
  ASSERT_TRUE(channels->add(channel));
  ASSERT_TRUE(channels->add(new supla_client_channel(
      channels, resource, 0, source, 0, SUPLA_CHANNELTYPE_THERMOMETER,
      SUPLA_CHANNELFNC_THERMOMETER, 0, 0, 0, 0, nullptr, nullptr, nullptr, "",
      0, 0, 0, 0, 29, 0, value, 0, "{}", "{}", false)));
  testing::NiceMock<testing::SrpcAdapterMock> adapter;
  ON_CALL(adapter, get_proto_version()).WillByDefault(testing::Return(29));
  TSC_SuplaChannelRelationPack captured{};
  EXPECT_CALL(adapter, sc_async_channel_relation_pack_update(testing::_))
      .WillRepeatedly([&](TSC_SuplaChannelRelationPack *pack) {
        captured = *pack;
        return 1;
      });
  supla_client_channel_relation_remote_updater updater(&adapter);
  Projection graph(&updater);
  auto old =
      std::async(std::launch::async, [&] { return graph.load(channels); });
  channel->reached.get_future().wait();
  graph.lock();
  channel->release.set_value();
  // Wait for the old metadata capture to release the channel collection.
  channels->for_each([](supla_client_channel *, bool *) {});
  query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
        std::to_string(hvac));
  EXPECT_TRUE(index->refresh());
  EXPECT_TRUE(graph.load(channels));  // Recursive existing list mutex.
  graph.unlock();
  EXPECT_FALSE(old.get());
  ASSERT_TRUE(graph.update_remote());
  ASSERT_EQ(1, captured.count);
  EXPECT_EQ(0, captured.items[0].Id);
}

TEST_F(SupLanHvacIntegrationTest, RejectedDeviceUploadDoesNotScheduleIndexSql) {
  config(resource);
  auto user = supla_user::find(99124, true);
  auto index = user->get_relation_index();
  index->invalidate();
  index->refresh();
  supla_connection_test_access protocol;
  ASSERT_TRUE(protocol.open());
  auto device = std::make_shared<testing::DeviceStub>(protocol.get());
  device->set_id(destination);
  device->set_user(user);
  supla_mariadb_access_provider db;
  ASSERT_TRUE(db.connect());
  supla_device_dao dao(&db);
  device->set_channels(
      new supla_device_channels(&dao, device.get(), nullptr, nullptr, 0));
  auto previous = index->snapshot();
  TSDS_SetChannelConfig request{};
  request.ChannelNumber = 240;
  request.ConfigType = 255;
  TsrpcReceivedData rd{};
  rd.data.sds_set_channel_config_request = &request;
  testing::SrpcAdapterMock reply;
  EXPECT_CALL(reply, sd_async_set_channel_config_result(testing::_))
      .WillOnce([](TSDS_SetChannelConfigResult *r) {
        EXPECT_EQ(SUPLA_CONFIG_RESULT_NOT_ALLOWED, r->Result);
        return 1;
      });
  Sql004Probe probe;
  HvacPerfHandler handler;
  handler.handle_call(device, &reply, &rd, SUPLA_DS_CALL_SET_CHANNEL_CONFIG,
                      29);
  EXPECT_EQ(previous, index->snapshot());
  EXPECT_EQ(0u, index->time_left_usec());
  EXPECT_EQ(0u, probe.executions);
}

TEST_F(SupLanHvacIntegrationTest, RelayJsonIpcUpdatesOnlyChangedRelations) {
  auto relay = add_channel(destination, 239, SUPLA_CHANNELTYPE_RELAY,
                           SUPLA_CHANNELFNC_POWERSWITCH);
  query("UPDATE supla_dev_channel SET type=" +
        std::to_string(SUPLA_CHANNELTYPE_ELECTRICITY_METER) +
        ",func=" + std::to_string(SUPLA_CHANNELFNC_ELECTRICITY_METER) +
        " WHERE id=" + std::to_string(resource));
  auto user = supla_user::find(99124, true);
  auto index = user->get_relation_index();
  index->invalidate();
  index->refresh();
  auto hint = [&] {
    testing::IpcSocketAdapterMock socket(-1);
    EXPECT_CALL(socket, send_data(std::string("OK:99124\n"))).Times(1);
    supla_on_channel_config_changed_command ipc(&socket);
    auto input = std::string("USER-ON-CHANNEL-CONFIG-CHANGED:99124,") +
                 std::to_string(destination) + "," + std::to_string(relay) +
                 "," + std::to_string(SUPLA_CHANNELTYPE_RELAY) + "," +
                 std::to_string(SUPLA_CHANNELFNC_POWERSWITCH) + "," +
                 std::to_string(CONFIG_CHANGE_SCOPE_JSON_DEFAULT) + "\n";
    std::vector<char> buffer(input.begin(), input.end());
    buffer.push_back(0);
    EXPECT_TRUE(
        ipc.process_command(buffer.data(), buffer.size(), input.size()));
  };
  query(
      "UPDATE supla_dev_channel SET user_config="
      "JSON_OBJECT('relatedMeterChannelId'," +
      std::to_string(resource) + ") WHERE id=" + std::to_string(relay));
  hint();
  auto rows = index->snapshot()->get(relay, relation_with_sub_channel, 29);
  ASSERT_EQ(1u, rows.size());
  EXPECT_EQ(resource, rows[0].get_id());
  auto builds = index->build_count();
  hint();
  EXPECT_EQ(builds, index->build_count());
  query("UPDATE supla_dev_channel SET user_config='{}' WHERE id=" +
        std::to_string(relay));
  hint();
  EXPECT_TRUE(index->snapshot()->get(relay, relation_any, 29).empty());
}

TEST_F(SupLanHvacWireIntegrationTest, WarmRegistrationDefersRelationReload) {
  auto index = supla_user::find(99126, true)->get_relation_index();
  index->invalidate();
  index->refresh();
  auto previous = index->snapshot();
  ASSERT_TRUE(previous);
  std::atomic<bool> replied{false};
  publication_adapter->before_registration_reply = [&] {
    EXPECT_EQ(previous, index->snapshot());
    EXPECT_EQ(1u, index->time_left_usec());
    replied = true;
  };
  publication_adapter->registration_probe_ready = true;
  register_family(SUPLA_CHANNELTYPE_HVAC, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  EXPECT_TRUE(replied);
  publication_adapter->registration_probe_ready = false;
  EXPECT_EQ(0u, index->time_left_usec());
}
