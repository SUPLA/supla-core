// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <cstring>
#include <future>
#include <map>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <utility>
#include <thread>
#include <tuple>
#include <vector>

#include "suplan/peer_provisioner.h"
#include "suplan/server_stream.h"
#include "eh.h"
#include "test/doubles/suplan/PeerProvisionerTestAccess.h"
using namespace supla_suplan;  // NOLINT
namespace {
struct Store {
  std::recursive_mutex mutex;
  Association a;
  std::vector<Grant> grants;
  Identity identity{123, {10, 11}};
  int writes = 0;
  bool fail_save = false;
  bool fail_commit = false;
  bool fail_find = false;
  bool fail_begin = false;
  bool fail_identity = false;
  unsigned reads = 0;
  std::map<uint32_t, int> resource_owners;
  std::function<void()> before_begin, after_commit;
  bool fail_load = false;
  uint32_t fail_owner_resource = 0;
  bool fail_load_after_commit = false;
  std::map<uint64_t, Association> additional;
  ChannelInfo channel_info{1, SUPLA_CHANNELTYPE_THERMOMETER,
                           SUPLA_CHANNELFNC_THERMOMETER,
                           SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED, 0};
  DeviceInfo device_info{SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED, 29};
};
class MemoryRepo : public Repository {
  Store *store;
  bool transaction = false;
  Association before;
  std::vector<Grant> before_grants;
  int before_writes = 0;

 public:
  explicit MemoryRepo(Store *s) : store(s) {}
  ~MemoryRepo() override { rollback(); }
  bool begin(int source, int destination, Association *a) override {
    store->mutex.lock();
    transaction = true;
    before = store->a;
    before_grants = store->grants;
    before_writes = store->writes;
    if (!store->a.id) {
      store->a.id = 1;
      store->a.source = source;
      store->a.destination = destination;
    }
    *a = store->a;
    return true;
  }
  bool begin(uint64_t id, Association *a) override {
    if (store->fail_begin) return false;
    if (store->before_begin) {
      auto change = std::move(store->before_begin);
      store->before_begin = nullptr;
      change();
    }
    store->mutex.lock();
    transaction = true;
    before = store->a;
    before_grants = store->grants;
    before_writes = store->writes;
    return load(id, a) && a->id;
  }
  bool load(uint64_t id, Association *a) override {
    std::lock_guard<std::recursive_mutex> lock(store->mutex);
    ++store->reads;
    if (store->fail_load) return false;
    *a = store->a.id == id ? store->a : Association{};
    auto it = store->additional.find(id);
    if (it != store->additional.end()) *a = it->second;
    return true;
  }
  bool find(int source, int destination, Association *a) override {
    std::lock_guard<std::recursive_mutex> lock(store->mutex);
    if (store->fail_find) return false;
    *a = store->a.source == source && store->a.destination == destination
             ? store->a
             : Association{};
    for (const auto &item : store->additional)
      if (item.second.source == source &&
          item.second.destination == destination) *a = item.second;
    return true;
  }
  bool grants(uint64_t, std::vector<Grant> *out) override {
    *out = store->grants;
    return true;
  }
  bool put(uint64_t, const Grant &g) override {
    for (auto &old : store->grants)
      if (old.resource_type == g.resource_type &&
          old.resource_id == g.resource_id &&
          old.origin_type == g.origin_type && old.origin_id == g.origin_id) {
        old.permissions = g.permissions;
        return true;
      }
    store->grants.push_back(g);
    return true;
  }
  bool erase_origin(uint64_t, uint16_t type, uint64_t origin) override {
    auto &g = store->grants;
    g.erase(std::remove_if(g.begin(), g.end(),
                           [&](const Grant &v) {
                             return v.origin_type == type &&
                                    v.origin_id == origin;
                           }),
            g.end());
    return true;
  }
  bool erase_resource(uint64_t, uint8_t type, uint32_t resource) override {
    auto &g = store->grants;
    g.erase(std::remove_if(g.begin(), g.end(),
                           [&](const Grant &v) {
                             return v.resource_type == type &&
                                    v.resource_id == resource;
                           }),
            g.end());
    return true;
  }
  bool erase_grants(uint64_t) override {
    store->grants.clear();
    return true;
  }
  bool save(const Association &a) override {
    if (store->fail_save) return false;
    if (store->additional.count(a.id)) store->additional[a.id] = a;
    if (store->a.id == a.id) store->a = a;
    ++store->writes;
    return true;
  }
  bool erase(uint64_t) override {
    store->a = {};
    return true;
  }
  bool commit() override {
    if (store->fail_commit) return false;
    transaction = false;
    if (store->fail_load_after_commit) store->fail_load = true;
    store->mutex.unlock();
    if (store->after_commit) {
      auto change = std::move(store->after_commit);
      store->after_commit = nullptr;
      change();
    }
    return true;
  }
  void rollback() override {
    if (transaction) {
      store->a = before;
      store->grants = before_grants;
      store->writes = before_writes;
      transaction = false;
      store->mutex.unlock();
    }
  }
  bool identity(int, Identity *out) override {
    if (store->fail_identity) return false;
    *out = store->identity;
    return true;
  }
  bool accept_identity(int, const Identity &i) override {
    store->identity = i;
    return true;
  }
  bool owner(uint8_t type, uint32_t resource, int *device) override {
    if (resource == store->fail_owner_resource) return false;
    if (type == 1 && store->resource_owners.count(resource)) {
      *device = store->resource_owners.at(resource);
      return true;
    }
    if ((type == 1 && (resource == 10 || resource == 11)) ||
        (type == 2 && resource == 1)) {
      *device = 1;
      return true;
    }
    if ((type == 1 && resource == 20) || (type == 2 && resource == 2)) {
      *device = 2;
      return true;
    }
    if (type == SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE) {
      for (const auto &entry : store->additional) {
        if (resource == static_cast<uint32_t>(entry.second.source) ||
            resource == static_cast<uint32_t>(entry.second.destination)) {
          *device = resource;
          return true;
        }
      }
    }
    *device = 0;
    return true;
  }
  bool supports_batch() const override { return true; }
  bool channel(uint32_t id, ChannelInfo *out) override {
    *out = store->channel_info;
    return owner(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, id, &out->device);
  }
  bool device(int, DeviceInfo *out) override {
    *out = store->device_info;
    return true;
  }
  bool device_exists(int d) override { return d > 0 && d <= 5000; }
  bool check_acl_ownership(int source, int destination, const Acl &acl,
                           bool *owned) override {
    *owned = false;
    if (store->fail_owner_resource == static_cast<uint32_t>(source) ||
        store->fail_owner_resource == static_cast<uint32_t>(destination))
      return false;
    if (!device_exists(source) || !device_exists(destination)) return true;
    for (const auto &entry : acl) {
      int current_owner = 0;
      if (!owner(entry.ResourceType, entry.ResourceId, &current_owner))
        return false;
      if (current_owner != source) return true;
    }
    *owned = true;
    return true;
  }
  bool for_device(int d, bool dormant, std::vector<uint64_t> *out) override {
    out->clear();
    if (store->a.id && (store->a.source == d || store->a.destination == d) &&
        (dormant || store->a.lifecycle != Lifecycle::Dormant))
      out->push_back(store->a.id);
    for (const auto &item : store->additional)
      if (item.second.source == d || item.second.destination == d)
        out->push_back(item.first);
    return true;
  }
  bool for_origin(uint16_t type, uint64_t origin,
                  std::vector<uint64_t> *out) override {
    out->clear();
    for (auto &g : store->grants)
      if (g.origin_type == type && g.origin_id == origin) {
        out->push_back(1);
        break;
      }
    return true;
  }
  bool for_resource(uint8_t type, uint32_t resource,
                    std::vector<uint64_t> *out) override {
    out->clear();
    for (auto &g : store->grants)
      if (g.resource_type == type && g.resource_id == resource) {
        out->push_back(1);
        break;
      }
    return true;
  }
};
class Transport : public PeerTransport {
 public:
  bool online[3] = {false, true, true};
  bool owner_gate = true;
  int refreshes = 0;
  std::vector<int> order;
  std::vector<TSDS_SuplaSetSuplanSourceAssociation> sources;
  // Deliberately retain only non-secret Destination metadata in the double.
  std::vector<TDS_SuplaSetSuplanDestinationAssociationResult> destinations;
  std::vector<int> destination_key_sizes;
  std::vector<int> errors;
  bool key_seen = false;
  std::map<int, uint64_t> sessions;
  std::vector<int> forced;
  std::vector<int> wakes;
  SendCode source_failure = SendCode::EnqueueFailed;
  SendCode destination_failure = SendCode::EnqueueFailed;
  uint64_t session(int d) override {
    return sessions.count(d) ? sessions[d] : static_cast<uint64_t>(d);
  }
  int recovery_device() override { return online[1] ? 1 : online[2] ? 2 : 0; }
  void wake(int d, uint64_t) override { wakes.push_back(d); }
  bool reconnect_session(int d, uint64_t expected) override {
    if (!reconnectable || !ready(d) || session(d) != expected) return false;
    forced.push_back(d);
    return true;
  }
  bool reconnectable = true;
  bool source_send_ok = true;
  bool destination_send_ok = true;
  bool ready(int d) override { return d > 2 || online[d]; }
  bool owner_ready(int, uint32_t) override { return owner_gate; }
  SendResult send(int d, TSDS_SuplaSetSuplanSourceAssociation *q) override {
    order.push_back(d);
    sources.push_back(*q);
    return {source_send_ok ? SendCode::Queued : source_failure, session(d)};
  }
  SendResult send(int d,
                  TSDS_SuplaSetSuplanDestinationAssociation *q) override {
    order.push_back(d);
    destinations.push_back({q->PeerContext, q->AclRevision, 0});
    destination_key_sizes.push_back(q->PeerKeySize);
    if (q->PeerKeySize) {
      key_seen = true;
      for (auto b : q->PeerKey) EXPECT_EQ(0xA5, b);
    }
    return {destination_send_ok ? SendCode::Queued : destination_failure,
            session(d)};
  }
  void rebootstrap(int) override { ++refreshes; }
  void diagnostic(uint64_t, int, uint8_t result) override {
    errors.push_back(result);
  }
};
class SupLanPeerTest : public testing::Test {
 protected:
  Store store;
  Transport transport;
  PeerService::Factory factory = [this] {
    return std::make_unique<MemoryRepo>(&store);
  };
  std::chrono::steady_clock::time_point now{};
  PeerProvisioner peers{factory, &transport, [this] { return now; }};
  void advance(int seconds) {
    now += std::chrono::seconds(seconds);
    peers.iterate();
  }
  Grant grant{1, 10, 1, 100, 1};
  void add() { ASSERT_TRUE(peers.grants()->upsert(1, 2, grant)); }
  void source_ok(int index = -1, uint8_t result = 0) {
    if (index < 0) index = transport.sources.size() - 1;
    auto q = transport.sources.at(index);
    TDS_SuplaSetSuplanSourceAssociationResult r = {};
    r.PeerContext = q.PeerContext;
    r.AclRevision = q.AclRevision;
    r.Result = result;
    if (!result && q.Flags) {
      r.PeerKeySize = 32;
      memset(r.PeerKey, 0xA5, 32);
    }
    peers.on_source(1, &r);
    for (auto b : r.PeerKey) EXPECT_EQ(0, b);
  }
  void destination_result(uint8_t code = 0, int index = -1) {
    if (index < 0) index = transport.destinations.size() - 1;
    auto r = transport.destinations.at(index);
    r.Result = code;
    peers.on_destination(2, &r);
  }
  void installed() {
    add();
    source_ok();
    destination_result();
  }
  void remove() { ASSERT_TRUE(peers.grants()->delete_origin(1, 100)); }
};
TEST_F(SupLanPeerTest, SendOutcomeClassifiesOfflineRootRaceAndEnqueue) {
  for (auto code : {PeerTransport::SendCode::NotReady,
                    PeerTransport::SendCode::ContextChanged,
                    PeerTransport::SendCode::EnqueueFailed}) {
    store.a = {};
    store.grants.clear();
    PeerProvisioner isolated(factory, &transport, [&] { return now; });
    transport.source_send_ok = false;
    transport.source_failure = code;
    transport.forced.clear();
    ASSERT_TRUE(isolated.grants()->upsert(1, 2, grant));
    EXPECT_EQ(code == PeerTransport::SendCode::EnqueueFailed ? 1u : 0u,
              transport.forced.size());
    EXPECT_EQ(0u, isolated.time_left_usec(1));
  }
}
TEST_F(SupLanPeerTest, MatchedResultDatabaseErrorsCancelDeviceWatchdog) {
  for (int stage = 0; stage < 3; ++stage) {
    store.a = {};
    store.grants.clear();
    PeerProvisioner isolated(factory, &transport, [&] { return now; });
    ASSERT_TRUE(isolated.grants()->upsert(1, 2, grant));
    auto request = transport.sources.back();
    TDS_SuplaSetSuplanSourceAssociationResult reply{};
    reply.PeerContext = request.PeerContext;
    reply.AclRevision = request.AclRevision;
    reply.PeerKeySize = request.Flags ? 32 : 0;
    memset(reply.PeerKey, 0xA5, sizeof(reply.PeerKey));
    store.fail_find = stage == 0;
    store.fail_load = stage == 1;
    store.fail_identity = stage == 2;
    isolated.on_source(1, &reply);
    for (auto byte : reply.PeerKey) EXPECT_EQ(0, byte);
    EXPECT_EQ(5000000u, isolated.time_left_usec(1));
    store.fail_load = true;
    now += std::chrono::seconds(16);
    isolated.iterate();
    EXPECT_TRUE(transport.forced.empty());
    EXPECT_EQ(5000000u, isolated.time_left_usec(1));
    store.fail_find = store.fail_load = store.fail_identity = false;
    now += std::chrono::seconds(5);
    auto count = transport.sources.size();
    isolated.iterate();
    ASSERT_EQ(count + 1, transport.sources.size());
    EXPECT_NE(0, transport.sources.back().Flags);
    EXPECT_EQ(0u, store.a.root);
  }
}
TEST_F(SupLanPeerTest, SourceCommitFailureDoesNotAcknowledgeOrForwardKey) {
  add();
  store.fail_commit = true;
  source_ok();
  EXPECT_EQ(0u, store.a.root);
  EXPECT_TRUE(transport.destinations.empty());
  EXPECT_TRUE(transport.forced.empty());
  EXPECT_EQ(5000000u, peers.time_left_usec(1));
  store.fail_commit = false;
  advance(5);
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest,
       DestinationResultDatabaseFailureRecoversWithoutReconnect) {
  add();
  source_ok();
  store.fail_find = true;
  destination_result();
  EXPECT_EQ(5000000u, peers.time_left_usec(1));
  EXPECT_EQ(0u, peers.time_left_usec(2));
  EXPECT_TRUE(transport.forced.empty());
  store.fail_find = false;
  advance(5);
  ASSERT_EQ(2u, transport.sources.size());
  EXPECT_NE(0, transport.sources.back().Flags);
  source_ok();
  destination_result();
  EXPECT_TRUE(transport.forced.empty());
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, OldSessionTimeoutAndReplyCannotAffectReplacement) {
  add();
  auto request = transport.sources.back();
  transport.sessions[1] = 100;
  advance(16);
  EXPECT_TRUE(transport.forced.empty());
  peers.reconnect(1);
  TDS_SuplaSetSuplanSourceAssociationResult reply{};
  reply.PeerContext = request.PeerContext;
  reply.AclRevision = request.AclRevision;
  reply.PeerKeySize = 32;
  memset(reply.PeerKey, 0xA5, sizeof(reply.PeerKey));
  peers.on_source(1, &reply, 1);
  EXPECT_EQ(0u, store.a.root);
  EXPECT_TRUE(transport.destinations.empty());
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, OtherAcknowledgementsCannotHideSilentFlowOrResetBudget) {
  add();  // Keep association 1 silent.
  unsigned results = 0;
  for (uint64_t id = 2; id <= 170; ++id) {
    Association a = store.a;
    a.id = id;
    a.destination = id + 2;
    a.root = store.identity.root;
    a.provisioned_generation = a.generation;
    store.additional[id] = a;
    peers.reconcile(id);
    now += std::chrono::milliseconds(100);
    auto request = transport.sources.back();
    TDS_SuplaSetSuplanSourceAssociationResult reply{};
    reply.PeerContext = request.PeerContext;
    reply.AclRevision = request.AclRevision;
    peers.on_source(1, &reply);
    auto destination = transport.destinations.back();
    peers.on_destination(a.destination, &destination);
    results += 2;
    peers.iterate();
  }
  EXPECT_EQ(std::vector<int>({1}), transport.forced);
  store.additional.clear();  // Other incidents already completed independently.
  transport.sessions[1] = 100;
  peers.reconnect(1);
  advance(16);
  EXPECT_EQ(std::vector<int>({1}), transport.forced);
  std::cout << "SILENT source_requests=" << transport.sources.size()
            << " destination_requests=" << transport.destinations.size()
            << " results=" << results
            << " reconnects=" << transport.forced.size()
            << " wakes=" << transport.wakes.size() << std::endl;
}
TEST_F(SupLanPeerTest, CoalescedIncidentSurvivesUnrelatedCompletion) {
  add();
  auto other = store.a;
  other.id = 2;
  other.destination = 3;
  other.root = store.identity.root;
  other.provisioned_generation = other.generation;
  store.additional[2] = other;
  peers.reconcile(2);
  auto reply = transport.destinations.back();
  peers.on_destination(3, &reply);
  advance(16);
  EXPECT_EQ(std::vector<int>({1}), transport.forced);
  transport.sessions[1] = 100;
  auto before = transport.sources.size();
  peers.reconnect(1);
  source_ok(before);  // Association 1 completes; association 2 is still silent.
  destination_result();
  reply = transport.destinations.back();
  for (auto candidate : transport.destinations)
    if (candidate.PeerContext.DestinationNodeId == 3) reply = candidate;
  peers.on_destination(3, &reply);
  advance(16);
  EXPECT_EQ(std::vector<int>({1}), transport.forced);
  ++store.additional[2]
        .revision;  // Meaningful desired change resets its budget.
  peers.reconcile(2);
  reply = transport.destinations.back();
  peers.on_destination(3, &reply);
  advance(16);
  EXPECT_EQ(std::vector<int>({1, 1}), transport.forced);
}
TEST_F(SupLanPeerTest, IneligibleReconnectDoesNotSpendIncidentBudget) {
  // Ready but intentionally non-reconnectable (e.g. sleep mode).
  transport.reconnectable = false;
  add();
  advance(16);
  EXPECT_TRUE(transport.forced.empty());
  transport.reconnectable = true;
  transport.sessions[1] = 100;
  peers.reconnect(1);  // A natural registration may become eligible.
  advance(16);
  EXPECT_EQ(std::vector<int>({1}), transport.forced);
}
TEST_F(SupLanPeerTest, IdleDevicesHaveNoDeadlineWakeOrDatabaseWork) {
  for (int count : {1, 10, 100}) {
    const auto reads = store.reads;
    const auto wakes = transport.wakes.size();
    for (int second = 0; second < 120; ++second) {
      now += std::chrono::seconds(1);
      for (int d = 1; d <= count; ++d) {
        EXPECT_EQ(0u, peers.time_left_usec(d));
        peers.iterate(d);  // Even incidental loop entry must do no SQL.
      }
    }
    EXPECT_EQ(reads, store.reads);
    EXPECT_EQ(wakes, transport.wakes.size());
  }
}
TEST_F(SupLanPeerTest, OptimisticDevicePersistenceErrorDoesNotReplay) {
  add();
  source_ok(-1, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
  advance(7);
  advance(600);
  EXPECT_EQ(1u, transport.sources.size());
  EXPECT_TRUE(transport.destinations.empty());
  EXPECT_EQ(0u, store.a.root);
}
TEST_F(SupLanPeerTest, OptimisticLostResultNeverResendsInSameSession) {
  add();
  advance(16);
  advance(600);
  EXPECT_EQ(1u, transport.sources.size());
  EXPECT_EQ(0u, store.a.root);
}
TEST_F(SupLanPeerTest, EnqueueFailureReconnectsOnceAndRederivesKey) {
  transport.source_send_ok = false;
  add();
  ASSERT_EQ(std::vector<int>({1}), transport.forced);
  advance(600);
  EXPECT_EQ(1u, transport.sources.size());
  transport.source_send_ok = true;
  transport.sessions[1] = 100;
  peers.reconnect(1);
  ASSERT_EQ(2u, transport.sources.size());
  transport.destination_send_ok = false;
  source_ok();
  ASSERT_EQ(std::vector<int>({1, 2}), transport.forced);
  transport.destination_send_ok = true;
  transport.sessions[2] = 200;
  peers.reconnect(2);
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  source_ok();
  ASSERT_EQ(4u, transport.sources.size());
  EXPECT_NE(0, transport.sources.back().Flags);
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  EXPECT_EQ(1u, store.a.revision);
  EXPECT_EQ(1u, store.a.generation);
}

TEST_F(SupLanPeerTest, LostResultsReconnectOnlyPendingEndpointOnce) {
  add();
  EXPECT_EQ(15000000u, peers.time_left_usec(1));
  EXPECT_EQ(0u, peers.time_left_usec(2));
  advance(14);
  EXPECT_TRUE(transport.forced.empty());
  advance(2);
  ASSERT_EQ(std::vector<int>({1}), transport.forced);
  EXPECT_EQ(0u, peers.time_left_usec(1));
  transport.sessions[1] = 100;
  peers.reconnect(1);
  advance(16);
  EXPECT_EQ(std::vector<int>({1}), transport.forced);
  EXPECT_EQ(2u, transport.sources.size());
  // A natural reconnect is still eligible to replay and complete.
  transport.sessions[1] = 101;
  peers.reconnect(1);
  source_ok();
  EXPECT_EQ(15000000u, peers.time_left_usec(2));
  advance(16);
  EXPECT_EQ(std::vector<int>({1, 2}), transport.forced);
  transport.sessions[2] = 200;
  peers.reconnect(2);
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}

TEST_F(SupLanPeerTest, CoreDbRecoversWhileDevicePersistenceBlocks) {
  add();
  store.fail_save = true;
  source_ok();
  EXPECT_EQ(0u, store.a.root);
  EXPECT_TRUE(transport.destinations.empty());
  EXPECT_TRUE(transport.forced.empty());
  EXPECT_EQ(5000000u, peers.time_left_usec(1));
  store.fail_save = false;
  advance(5);
  ASSERT_EQ(2u, transport.sources.size());
  source_ok();
  destination_result(SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
  advance(600);
  EXPECT_EQ(1u, transport.destinations.size());
  EXPECT_TRUE(transport.forced.empty());
  peers.reconnect(2);
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}

TEST_F(SupLanPeerTest,
       EmptyCleanupReconnectsDroppedResultWithOtherSideOffline) {
  installed();
  transport.online[2] = false;
  remove();
  ASSERT_EQ(0, transport.sources.back().AclEntryCount);
  advance(16);
  EXPECT_EQ(std::vector<int>({1}), transport.forced);
  EXPECT_FALSE(store.a.source_acked);
  transport.sessions[1] = 100;
  peers.reconnect(1);
  ASSERT_EQ(3u, transport.sources.size());
  EXPECT_EQ(0, transport.sources.back().AclEntryCount);
  source_ok();
  EXPECT_TRUE(store.a.source_acked);
  EXPECT_EQ(Lifecycle::Draining, store.a.lifecycle);
  transport.online[2] = true;
  peers.reconnect(2);
  advance(16);
  EXPECT_EQ(std::vector<int>({1, 2}), transport.forced);
  EXPECT_FALSE(store.a.destination_acked);
  transport.sessions[2] = 200;
  peers.reconnect(2);
  destination_result();
  EXPECT_EQ(Lifecycle::Dormant, store.a.lifecycle);
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}

TEST_F(SupLanPeerTest, RevisionConflictSurvivesTimerAndReconnectUntilChange) {
  add();
  source_ok(-1, SUPLA_SUPLAN_RESULT_REVISION_CONFLICT);
  for (int i = 0; i < 10; ++i) {
    advance(60);
    peers.reconnect(1);
    peers.reconnect(2);
  }
  EXPECT_EQ(1u, transport.sources.size());
  EXPECT_TRUE(transport.destinations.empty());
  EXPECT_EQ(1u, store.a.generation);
  grant.permissions = SUPLA_SUPLAN_PERMISSION_CONTROL;
  ASSERT_TRUE(peers.grants()->upsert(1, 2, grant));
  ASSERT_EQ(2u, transport.sources.size());
  EXPECT_EQ(2u, static_cast<uint32_t>(transport.sources.back().AclRevision));
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, WatchdogReloadsDesiredStateAfterMissedEvent) {
  add();
  // Emulate a committed desired change whose notification was lost.
  ++store.a.revision;
  store.a.acl = {{1, 11, SUPLA_SUPLAN_PERMISSION_READ}};
  advance(16);
  advance(7);
  ASSERT_EQ(2u, transport.sources.size());
  EXPECT_EQ(2u, static_cast<uint32_t>(transport.sources.back().AclRevision));
  EXPECT_EQ(11u, static_cast<uint32_t>(
                     transport.sources.back().Acl[0].ResourceId));
  source_ok(0);
  EXPECT_TRUE(transport.destinations.empty());
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, FailedAuthoritativeReadRecoversWithoutStaleSend) {
  installed();
  store.fail_load = true;
  peers.reconcile(1, true);
  advance(16);
  EXPECT_EQ(1u, transport.sources.size());
  EXPECT_EQ(1u, transport.destinations.size());
  store.fail_load = false;
  advance(16);
  ASSERT_EQ(2u, transport.sources.size());
  ASSERT_EQ(2u, transport.destinations.size());
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, CleanupAckCommitFailuresRecoverIndependently) {
  installed();
  remove();
  store.fail_save = true;
  source_ok();
  destination_result();
  EXPECT_FALSE(store.a.source_acked);
  EXPECT_FALSE(store.a.destination_acked);
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  store.fail_save = false;
  advance(7);
  ASSERT_EQ(3u, transport.sources.size());
  ASSERT_EQ(3u, transport.destinations.size());
  destination_result();
  EXPECT_TRUE(store.a.destination_acked);
  EXPECT_FALSE(store.a.source_acked);
  source_ok();
  EXPECT_EQ(Lifecycle::Dormant, store.a.lifecycle);
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, PermanentResultsAndRepeatedPersistenceDoNotTimerLoop) {
  for (auto code : {SUPLA_SUPLAN_RESULT_CAPACITY_EXCEEDED,
                    SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED}) {
    peers.reconnect(1);
    if (!store.a.id) add();
    source_ok(-1, code);
    auto sends = transport.order.size();
    for (int i = 0; i < 10; ++i) advance(600);
    EXPECT_EQ(sends, transport.order.size());
    EXPECT_EQ(1u, store.a.generation);
  }
  peers.reconnect(1);
  for (int i = 0; i < 3; ++i) {
    source_ok(-1, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
    advance(100);
  }
  auto sends = transport.order.size();
  for (int i = 0; i < 10; ++i) advance(600);
  EXPECT_EQ(sends, transport.order.size());
  EXPECT_EQ(0u, store.a.root);
  EXPECT_TRUE(transport.destinations.empty());
}
TEST_F(SupLanPeerTest, DestinationConflictKeepsIndependentSourceConvergence) {
  installed();
  grant.permissions = SUPLA_SUPLAN_PERMISSION_CONTROL;
  ASSERT_TRUE(peers.grants()->upsert(1, 2, grant));
  destination_result(SUPLA_SUPLAN_RESULT_REVISION_CONFLICT);
  auto sends = transport.destinations.size();
  source_ok();
  advance(16);
  peers.reconnect(2);
  advance(600);
  EXPECT_EQ(sends, transport.destinations.size());
  EXPECT_EQ(1u, store.a.generation);
  grant.permissions = SUPLA_SUPLAN_PERMISSION_READ;
  ASSERT_TRUE(peers.grants()->upsert(1, 2, grant));
  EXPECT_EQ(sends + 1, transport.destinations.size());
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, LateNoKeyAckCannotBlockKeyRecoveryAfterReconnect) {
  transport.online[2] = false;
  add();
  advance(16);
  transport.online[2] = true;
  transport.sessions[1] = 100;
  peers.reconnect(1);
  ASSERT_EQ(2u, transport.sources.size());
  EXPECT_NE(0, transport.sources.back().Flags);
  source_ok(0);
  EXPECT_TRUE(transport.destinations.empty());
  EXPECT_TRUE(transport.errors.empty());
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}

TEST_F(SupLanPeerTest, Serial100msBatchExposesFixedWatchdogLimit) {
  for (uint64_t id = 1; id <= 170; ++id) {
    Association a;
    a.id = id;
    a.source = 1;
    a.destination = id + 2;
    a.lifecycle = Lifecycle::Active;
    a.root = store.identity.root;
    a.provisioned_generation = a.generation;
    a.acl = {{1, 10, 1}};
    store.additional[id] = a;
    peers.reconcile(id);
  }
  ASSERT_EQ(170u, transport.sources.size());
  // Separate Destinations process their keyless replacements concurrently.
  for (auto reply : transport.destinations)
    peers.on_destination(reply.PeerContext.DestinationNodeId, &reply);
  for (size_t index = 0; index < 170; ++index) {
    ASSERT_GT(transport.sources.size(), index);
    now += std::chrono::milliseconds(100);
    source_ok(index);
    peers.iterate();
  }
  EXPECT_EQ(std::chrono::seconds(17), now.time_since_epoch());
  // Deliberately synthetic: twenty replies fall beyond the fixed 15 s bound.
  // Document the limitation; do not disguise it by an admission window.
  EXPECT_EQ(std::vector<int>({1}), transport.forced);
  EXPECT_EQ(20u, PeerProvisionerTestAccess::count(peers));
}

TEST_F(SupLanPeerTest, MalformedResultTableNeverCommitsOrForwardsCredential) {
  add();
  const auto request = transport.sources.back();
  for (int field = 0; field < 9; ++field) {
    TDS_SuplaSetSuplanSourceAssociationResult r = {};
    r.PeerContext = request.PeerContext;
    r.AclRevision = request.AclRevision;
    r.PeerKeySize = SUPLA_SUPLAN_PEER_KEY_SIZE;
    memset(r.PeerKey, 0xA5, sizeof(r.PeerKey));
    switch (field) {
      case 0: ++r.PeerContext.AuthorityType; break;
      case 1: ++r.PeerContext.AuthorityId; break;
      case 2: ++r.PeerContext.SourceNodeIdNamespace; break;
      case 3: ++r.PeerContext.DestinationNodeIdNamespace; break;
      case 4: ++r.PeerContext.RootEpoch; break;
      case 5: ++r.PeerContext.PeerGeneration; break;
      case 6: ++r.AclRevision; break;
      case 7: ++r.PeerContext.DestinationNodeId; break;
      case 8: ++r.PeerContext.SourceNodeId; break;
    }
    peers.on_source(field == 0 ? 2 : 1, &r);
    for (auto b : r.PeerKey) EXPECT_EQ(0, b);
    EXPECT_EQ(0u, store.a.root);
    EXPECT_TRUE(transport.destinations.empty());
  }
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, ForcedReconnectBudgetSurvivesFlowResetAndRegistration) {
  transport.source_send_ok = false;
  add();
  for (int i = 0; i < 100; ++i) {
    advance(600);
    peers.reconcile(1);
  }
  EXPECT_EQ(1u, transport.sources.size());
  EXPECT_EQ(std::vector<int>({1}), transport.forced);
  for (int i = 0; i < 3; ++i) {
    transport.sessions[1] = 100 + i;
    peers.reconnect(1);
    advance(600);
  }
  EXPECT_EQ(4u, transport.sources.size());
  EXPECT_EQ(std::vector<int>({1}), transport.forced);
  transport.source_send_ok = true;
  transport.sessions[1] = 200;
  peers.reconnect(1);
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}

TEST_F(SupLanPeerTest, SourceConflictOnKeylessReplayCannotBeReopened) {
  installed();
  peers.reconnect(1);
  ASSERT_EQ(0, transport.sources.back().Flags);
  source_ok(-1, SUPLA_SUPLAN_RESULT_REVISION_CONFLICT);
  destination_result();
  auto sends = transport.sources.size();
  peers.reconnect(1);
  peers.reconnect(2);
  advance(600);
  EXPECT_EQ(sends, transport.sources.size());
  EXPECT_EQ(1u, store.a.generation);
}
TEST_F(SupLanPeerTest, WatchdogReadsNewRootAndNeverForwardsOldKey) {
  add();
  store.identity.root = 456;
  advance(16);
  advance(7);
  ASSERT_EQ(2u, transport.sources.size());
  EXPECT_EQ(456u, static_cast<uint32_t>(
                     transport.sources.back().PeerContext.RootEpoch));
  source_ok(0);
  EXPECT_TRUE(transport.destinations.empty());
  source_ok();
  destination_result();
  EXPECT_EQ(456u, store.a.root);
  EXPECT_EQ(1u, store.a.generation);
  EXPECT_EQ(1u, store.a.revision);
}
TEST_F(SupLanPeerTest, ConcurrentTimerMutationAndStaleResultsConverge) {
  std::atomic<int> seconds{0};
  PeerProvisioner concurrent(factory, &transport, [&] {
    return std::chrono::steady_clock::time_point() +
           std::chrono::seconds(seconds.load());
  });
  ASSERT_TRUE(concurrent.grants()->upsert(1, 2, grant));
  const auto first = transport.sources.back();
  auto reply = [&](const TSDS_SuplaSetSuplanSourceAssociation &q) {
    TDS_SuplaSetSuplanSourceAssociationResult r = {};
    r.PeerContext = q.PeerContext;
    r.AclRevision = q.AclRevision;
    if (q.Flags) {
      r.PeerKeySize = SUPLA_SUPLAN_PEER_KEY_SIZE;
      memset(r.PeerKey, 0xA5, sizeof(r.PeerKey));
    }
    concurrent.on_source(1, &r);
    for (auto b : r.PeerKey) EXPECT_EQ(0, b);
  };
  std::thread timer([&] {
    for (int i = 0; i < 200; ++i) {
      ++seconds;
      concurrent.iterate();
    }
  });
  std::thread mutation([&] {
    for (int i = 0; i < 200; ++i) {
      auto g = grant;
      g.permissions = i % 2 ? SUPLA_SUPLAN_PERMISSION_CONTROL
                            : SUPLA_SUPLAN_PERMISSION_READ;
      EXPECT_TRUE(concurrent.grants()->upsert(1, 2, g));
    }
  });
  std::thread stale([&] {
    for (int i = 0; i < 200; ++i) reply(first);
  });
  timer.join();
  mutation.join();
  stale.join();
  concurrent.reconnect(1);
  reply(transport.sources.back());
  ASSERT_FALSE(transport.destinations.empty());
  auto destination = transport.destinations.back();
  concurrent.on_destination(2, &destination);
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(concurrent));
  EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_CONTROL, store.a.acl[0].Permissions);
  EXPECT_EQ(1u, store.a.generation);
}
TEST_F(SupLanPeerTest, KeyRequiredCannotReplayConflictedSourceSnapshot) {
  installed();
  grant.permissions = SUPLA_SUPLAN_PERMISSION_CONTROL;
  ASSERT_TRUE(peers.grants()->upsert(1, 2, grant));
  source_ok(-1, SUPLA_SUPLAN_RESULT_REVISION_CONFLICT);
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  EXPECT_EQ(2u, transport.sources.size());
  peers.reconnect(2);
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  advance(600);
  EXPECT_EQ(2u, transport.sources.size());
  EXPECT_EQ(0, transport.destination_key_sizes.back());
  EXPECT_EQ(1u, store.a.generation);
}
TEST_F(SupLanPeerTest, ProvisionedBookkeepingCannotRepairRevisionConflict) {
  add();
  source_ok(-1, SUPLA_SUPLAN_RESULT_REVISION_CONFLICT);
  ASSERT_TRUE(peers.grants()->provisioned(store.a, store.identity.root));
  advance(600);
  peers.reconnect(1);
  EXPECT_EQ(1u, transport.sources.size());
  EXPECT_EQ(1u, store.a.revision);
  EXPECT_EQ(1u, store.a.generation);
}
TEST_F(SupLanPeerTest, PostCommitReadFailureRederivesInsteadOfLosingFlow) {
  add();
  store.fail_load_after_commit = true;
  source_ok();
  EXPECT_EQ(store.identity.root, store.a.root);
  EXPECT_TRUE(transport.destinations.empty());
  store.fail_load = store.fail_load_after_commit = false;
  advance(7);
  ASSERT_EQ(2u, transport.sources.size());
  EXPECT_NE(0, transport.sources.back().Flags);
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  EXPECT_EQ(1u, store.a.revision);
  EXPECT_EQ(1u, store.a.generation);
}
TEST_F(SupLanPeerTest, OwnershipReadFailureCannotLoseControlRevocationRetry) {
  installed();
  for (uint32_t failed_resource : {1u, 2u, 10u}) {
    auto extra = grant;
    extra.origin_id = 200;
    extra.permissions = SUPLA_SUPLAN_PERMISSION_CONTROL;
    ASSERT_TRUE(peers.grants()->upsert(1, 2, extra));
    source_ok();
    destination_result();
    ASSERT_TRUE(peers.grants()->delete_origin(1, 200));
    destination_result();
    const auto revision = store.a.revision;
    const auto attempts = transport.sources.size();
    store.fail_owner_resource = failed_resource;
    source_ok();
    EXPECT_EQ(attempts, transport.sources.size());
    EXPECT_TRUE(transport.forced.empty());
    advance(5);
    EXPECT_EQ(attempts, transport.sources.size());
    store.fail_owner_resource = 0;
    advance(5);
    ASSERT_EQ(attempts + 1, transport.sources.size());
    EXPECT_EQ(SUPLA_SUPLAN_PERMISSION_READ,
              transport.sources.back().Acl[0].Permissions);
    source_ok();
    EXPECT_EQ(revision, store.a.revision);
    EXPECT_EQ(1u, store.a.generation);
    EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  }
}

TEST_F(SupLanPeerTest, OwnershipReadFailureBeforeFlowCreationDefersSafely) {
  installed();
  auto attempts = transport.order.size();
  store.fail_owner_resource = 10;
  peers.reconcile(1, true);
  advance(16);
  EXPECT_EQ(attempts, transport.order.size());
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  store.fail_owner_resource = 0;
  advance(16);
  EXPECT_GT(transport.order.size(), attempts);
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, InvalidTopologyDoesNotBecomeTransientRetry) {
  installed();
  store.identity.channels.clear();
  auto attempts = transport.order.size();
  peers.reconcile(1, true);
  for (int i = 0; i < 10; ++i) advance(600);
  EXPECT_EQ(attempts, transport.order.size());
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_FOUND, transport.errors.back());
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, OneGrantSourceKeyThenDestination) {
  add();
  EXPECT_EQ(Lifecycle::Active, store.a.lifecycle);
  EXPECT_EQ(1u, store.a.revision);
  ASSERT_EQ(1u, transport.order.size());
  EXPECT_EQ(1, transport.order[0]);
  EXPECT_EQ(0u, store.a.root);
  source_ok();
  ASSERT_EQ(2u, transport.order.size());
  EXPECT_EQ(2, transport.order[1]);
  EXPECT_TRUE(transport.key_seen);
  EXPECT_EQ(123u, store.a.root);
  EXPECT_EQ(1u, store.a.provisioned_generation);
  destination_result();
  EXPECT_TRUE(transport.errors.empty());
}
TEST_F(SupLanPeerTest, CompletedNewContextRetiresFlow) {
  add();
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  source_ok();
  // Destination still pending.
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  EXPECT_EQ(Lifecycle::Active, store.a.lifecycle);
  EXPECT_EQ(123u, store.a.root);
}
TEST_F(SupLanPeerTest, CompletedAclOnlyUpdateRetiresFlow) {
  installed();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  grant.permissions = 2;
  ASSERT_TRUE(peers.grants()->upsert(1, 2, grant));
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  EXPECT_EQ(0, transport.sources.back().Flags);
  EXPECT_EQ(0, transport.destination_key_sizes.back());
  destination_result();
  // Source still pending.
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  source_ok();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  EXPECT_EQ(Lifecycle::Active, store.a.lifecycle);
  EXPECT_EQ(2u, store.a.revision);
}
TEST_F(SupLanPeerTest, SourceOnlyRetiresAndDestinationReconstructsKeyRecovery) {
  transport.online[2] = false;
  add();
  source_ok();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  EXPECT_TRUE(transport.destinations.empty());
  EXPECT_EQ(123u, store.a.root);
  transport.online[2] = true;
  peers.reconnect(2);
  auto no_key = transport.sources.size() - 1;
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  EXPECT_EQ(0, transport.sources.back().Flags);
  EXPECT_EQ(0, transport.destination_key_sizes.back());
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  source_ok(no_key);
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  EXPECT_EQ(SUPLA_SUPLAN_SOURCE_FLAG_RETURN_PEER_KEY,
            transport.sources.back().Flags);
  source_ok(no_key);  // Duplicate must not retire the pending rederive.
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  source_ok();
  EXPECT_EQ(32, transport.destination_key_sizes.back());
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  EXPECT_TRUE(transport.errors.empty());
}
TEST_F(SupLanPeerTest, RetiredCleanupCannotReplayNonemptyOrAckReactivatedFlow) {
  installed();
  auto nonempty = transport.sources.size() - 1;
  transport.online[2] = false;
  remove();
  auto empty = transport.sources.size() - 1;
  source_ok();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  EXPECT_EQ(Lifecycle::Draining, store.a.lifecycle);
  EXPECT_TRUE(store.a.source_acked);
  auto writes = store.writes;
  auto sends = transport.order.size();
  source_ok(nonempty);
  peers.reconcile(store.a.id);
  peers.reconnect(1);
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  EXPECT_EQ(writes, store.writes);
  EXPECT_EQ(sends, transport.order.size());
  transport.online[2] = true;
  peers.reconnect(2);
  auto destination_empty = transport.destinations.size() - 1;
  EXPECT_EQ(0, transport.destination_key_sizes.back());
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  destination_result();
  EXPECT_EQ(Lifecycle::Dormant, store.a.lifecycle);
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  add();
  writes = store.writes;
  sends = transport.order.size();
  source_ok(empty);
  destination_result(0, destination_empty);
  EXPECT_EQ(writes, store.writes);
  EXPECT_EQ(sends, transport.order.size());
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  EXPECT_EQ(Lifecycle::Active, store.a.lifecycle);
  EXPECT_EQ(2u, store.a.generation);
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, DelayedRepliesAfterRetirementCannotMutateDb) {
  installed();
  ASSERT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  auto writes = store.writes;
  auto sends = transport.order.size();
  source_ok();  // Helper also verifies that the ignored key is wiped.
  source_ok(-1, SUPLA_SUPLAN_RESULT_ROOT_EPOCH_MISMATCH);
  destination_result();
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  EXPECT_EQ(writes, store.writes);
  EXPECT_EQ(sends, transport.order.size());
  EXPECT_EQ(0, transport.refreshes);
  EXPECT_TRUE(transport.errors.empty());
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, SequentialCompletedAssociationsExceedFormerLifetimeCap) {
  // Advance the repository double through independent ACTIVE association IDs.
  // Completed rows remain logically ACTIVE; no DORMANT/erase frees their Flow.
  for (uint64_t id = 1; id <= 5000; ++id) {
    store.a = {};
    store.a.id = id;
    store.a.source = 1;
    store.a.destination = 2;
    store.a.lifecycle = Lifecycle::Active;
    store.a.acl = {{1, 10, 1}};
    peers.reconcile(id);
    ASSERT_EQ(id, transport.sources.size());
    source_ok();
    ASSERT_EQ(id, transport.destinations.size());
    destination_result();
    ASSERT_EQ(0u, PeerProvisionerTestAccess::count(peers))
        << "association=" << id;
    ASSERT_TRUE(transport.errors.empty());
    ASSERT_EQ(Lifecycle::Active, store.a.lifecycle);
  }
}
TEST_F(SupLanPeerTest, ConcurrentLiveWorkStillHonorsSafetyCapAndReusesSlot) {
  Association first;
  for (uint64_t id = 1; id <= 4097; ++id) {
    store.a = {};
    store.a.id = id;
    store.a.source = 1;
    store.a.destination = id + 2;
    store.a.lifecycle = Lifecycle::Active;
    store.a.acl = {{1, 10, 1}};
    store.additional[id] = store.a;
    if (id == 1) first = store.a;
    peers.reconcile(id);  // Leave each Source response pending.
  }
  ASSERT_EQ(4096u, PeerProvisionerTestAccess::count(peers));
  ASSERT_EQ(4096u, transport.sources.size());
  ASSERT_EQ(1u, transport.errors.size());
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_CAPACITY_EXCEEDED, transport.errors.back());
  auto next = store.a;
  store.a = first;
  source_ok(0);
  auto reply = transport.destinations.back();
  peers.on_destination(reply.PeerContext.DestinationNodeId, &reply);
  ASSERT_EQ(4095u, PeerProvisionerTestAccess::count(peers));
  store.a = next;
  peers.reconcile(next.id);
  ASSERT_EQ(4097u, transport.sources.size());
  peers.iterate();  // No admission queue and no duplicate sends.
  ASSERT_EQ(4097u, transport.sources.size());
  EXPECT_EQ(4096u, PeerProvisionerTestAccess::count(peers));
  source_ok();
  reply = transport.destinations.back();
  peers.on_destination(reply.PeerContext.DestinationNodeId, &reply);
  EXPECT_EQ(4095u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, OfflineUnsentWorkDoesNotNeedResidentFlow) {
  transport.online[1] = transport.online[2] = false;
  add();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
  EXPECT_TRUE(transport.order.empty());
  transport.online[1] = transport.online[2] = true;
  peers.reconnect(1);
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
TEST_F(SupLanPeerTest, OriginsUnionControlNormalizationAndNoopRemoval) {
  installed();
  auto second = grant;
  second.origin_id = 200;
  int writes = store.writes;
  auto sends = transport.order.size();
  ASSERT_TRUE(peers.grants()->upsert(1, 2, second));
  EXPECT_EQ(writes, store.writes);
  EXPECT_EQ(sends, transport.order.size());
  ASSERT_TRUE(peers.grants()->delete_origin(1, 100));
  EXPECT_EQ(writes, store.writes);
  EXPECT_EQ(1u, store.a.revision);
  second.permissions = 3;
  ASSERT_TRUE(peers.grants()->upsert(1, 2, second));
  EXPECT_EQ(2, store.a.acl[0].Permissions);
  EXPECT_EQ(2u, store.a.revision);
  EXPECT_EQ(writes + 1, store.writes);
  auto third = second;
  third.origin_id = 300;
  third.permissions = 4;
  ASSERT_TRUE(peers.grants()->upsert(1, 2, third));
  EXPECT_EQ(6, store.a.acl[0].Permissions);
  EXPECT_EQ(3u, store.a.revision);
  EXPECT_EQ(1u, store.a.generation);
}
TEST_F(SupLanPeerTest, StableOriginUpdatesInPlace) {
  installed();
  grant.permissions = 4;
  ASSERT_TRUE(peers.grants()->upsert(1, 2, grant));
  EXPECT_EQ(1u, store.grants.size());
  EXPECT_EQ(2u, store.a.revision);
}
TEST_F(SupLanPeerTest, UnsupportedBitsAndWrongOwnerRejected) {
  grant.permissions = 8;
  EXPECT_FALSE(peers.grants()->upsert(1, 2, grant));
  grant.permissions = 1;
  grant.resource_id = 20;
  EXPECT_FALSE(peers.grants()->upsert(1, 2, grant));
  EXPECT_FALSE(peers.grants()->upsert(1, 1, grant));
}
TEST_F(SupLanPeerTest, FinalRemovalPartialDurableCleanupAndDormant) {
  installed();
  remove();
  EXPECT_EQ(Lifecycle::Draining, store.a.lifecycle);
  EXPECT_EQ(2u, store.a.revision);
  EXPECT_EQ(1u, store.a.generation);
  source_ok();
  EXPECT_TRUE(store.a.source_acked);
  EXPECT_FALSE(store.a.destination_acked);
  transport.online[1] = false;
  PeerProvisioner restarted(factory, &transport);
  restarted.reconnect(2);
  auto r = transport.destinations.back();
  restarted.on_destination(2, &r);
  EXPECT_EQ(Lifecycle::Dormant, store.a.lifecycle);
  auto sends = transport.order.size();
  restarted.reconnect(1);
  restarted.reconnect(2);
  EXPECT_EQ(sends, transport.order.size());
}
TEST_F(SupLanPeerTest, DrainingAndDormantReactivationFreshGeneration) {
  installed();
  remove();
  auto old = store.a;
  add();
  EXPECT_EQ(2u, store.a.generation);
  EXPECT_EQ(1u, store.a.revision);
  peers.grants()->cleanup_ack(old, true);
  peers.grants()->cleanup_ack(old, false);
  EXPECT_EQ(Lifecycle::Active, store.a.lifecycle);
  source_ok();
  destination_result();
  remove();
  source_ok();
  destination_result();
  EXPECT_EQ(Lifecycle::Dormant, store.a.lifecycle);
  add();
  EXPECT_EQ(3u, store.a.generation);
  EXPECT_EQ(1u, store.a.revision);
}
TEST_F(SupLanPeerTest, RootChangeReprovisionsWithoutAclOrGenerationBump) {
  installed();
  int writes = store.writes;
  store.identity.root = 456;
  peers.reconnect(1);
  EXPECT_EQ(123u, store.a.root);
  EXPECT_EQ(writes, store.writes);
  EXPECT_EQ(1u, store.a.generation);
  EXPECT_EQ(1u, store.a.revision);
  ASSERT_EQ(456u,
            static_cast<uint32_t>(
                transport.sources.back().PeerContext.RootEpoch));
  source_ok();
  EXPECT_EQ(456u, store.a.root);
  EXPECT_EQ(1u, store.grants.size());
}
TEST_F(SupLanPeerTest, RootChangeDuringDrainingSatisfiesOnlySource) {
  installed();
  remove();
  store.identity.root = 456;
  peers.reconnect(1);
  EXPECT_TRUE(store.a.source_acked);
  EXPECT_FALSE(store.a.destination_acked);
  EXPECT_EQ(123u,
            static_cast<uint32_t>(
                transport.destinations.back().PeerContext.RootEpoch));
  destination_result();
  EXPECT_EQ(Lifecycle::Dormant, store.a.lifecycle);
  add();
  EXPECT_EQ(1u, store.a.generation);
  EXPECT_EQ(1u, store.a.revision);
  EXPECT_EQ(456u,
            static_cast<uint32_t>(
                transport.sources.back().PeerContext.RootEpoch));
}
TEST_F(SupLanPeerTest,
       FinalRemovalDuringRootReprovisionCleansCurrentNamespace) {
  installed();
  ASSERT_TRUE(peers.accepted(1, {456, {10, 11}}));
  peers.reconnect(1);
  auto old_source = transport.sources.size() - 1;
  ASSERT_EQ(456u,
            static_cast<uint32_t>(
                transport.sources.back().PeerContext.RootEpoch));
  ASSERT_EQ(123u, store.a.root);  // New-root Source ACK has not arrived.
  remove();
  ASSERT_EQ(old_source + 2, transport.sources.size());
  EXPECT_EQ(456u, store.a.root);
  EXPECT_EQ(456u,
            static_cast<uint32_t>(
                transport.sources.back().PeerContext.RootEpoch));
  EXPECT_EQ(0u, transport.sources.back().AclEntryCount);
  EXPECT_EQ(456u,
            static_cast<uint32_t>(
                transport.destinations.back().PeerContext.RootEpoch));
  EXPECT_EQ(0, transport.destination_key_sizes.back());
  auto sends = transport.order.size();
  source_ok(old_source);  // Late non-empty ACK cannot forward the old key.
  EXPECT_EQ(sends, transport.order.size());
  destination_result();
  EXPECT_EQ(Lifecycle::Draining, store.a.lifecycle);
  EXPECT_FALSE(store.a.source_acked);
  source_ok();
  EXPECT_EQ(Lifecycle::Dormant, store.a.lifecycle);
  add();
  EXPECT_EQ(2u, store.a.generation);
  EXPECT_EQ(1u, store.a.revision);
  EXPECT_EQ(456u,
            static_cast<uint32_t>(
                transport.sources.back().PeerContext.RootEpoch));
}
TEST_F(SupLanPeerTest, RootReprovisionCleanupSurvivesRestartAndPartialAck) {
  installed();
  ASSERT_TRUE(peers.accepted(1, {456, {10, 11}}));
  peers.reconnect(1);
  remove();
  destination_result();
  EXPECT_TRUE(store.a.destination_acked);
  EXPECT_FALSE(store.a.source_acked);
  PeerProvisioner restarted(factory, &transport);
  auto destination_count = transport.destinations.size();
  restarted.reconnect(1);
  EXPECT_EQ(destination_count, transport.destinations.size());
  EXPECT_FALSE(store.a.source_acked);
  auto q = transport.sources.back();
  EXPECT_EQ(456u, static_cast<uint32_t>(q.PeerContext.RootEpoch));
  EXPECT_EQ(0u, q.AclEntryCount);
  TDS_SuplaSetSuplanSourceAssociationResult r = {};
  r.PeerContext = q.PeerContext;
  r.AclRevision = q.AclRevision;
  restarted.on_source(1, &r);
  EXPECT_EQ(Lifecycle::Dormant, store.a.lifecycle);
}
TEST_F(SupLanPeerTest,
       ChangedRootDrainingReactivationDoesNotCompareGenerations) {
  installed();
  store.a.generation = 10;
  remove();
  store.identity.root = 456;
  add();
  EXPECT_EQ(10u, store.a.generation);
  EXPECT_EQ(1u, store.a.revision);
}
TEST_F(SupLanPeerTest, DestinationOfflineDoesNotRequestOrRetainKey) {
  transport.online[2] = false;
  add();
  ASSERT_EQ(1u, transport.sources.size());
  EXPECT_EQ(0, transport.sources.back().Flags);
  source_ok();
  EXPECT_TRUE(transport.destinations.empty());
  EXPECT_FALSE(transport.key_seen);
  transport.online[2] = true;
  peers.reconnect(2);
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  source_ok();
  EXPECT_FALSE(transport.key_seen);
  source_ok();
  EXPECT_TRUE(transport.key_seen);
  EXPECT_EQ(32, transport.destination_key_sizes.back());
}
TEST_F(SupLanPeerTest, SourceOfflineWaitsForFirstCredential) {
  transport.online[1] = false;
  add();
  EXPECT_TRUE(transport.order.empty());
  transport.online[1] = true;
  peers.reconnect(1);
  source_ok();
  EXPECT_TRUE(transport.key_seen);
}
TEST_F(SupLanPeerTest, AclOnlyDestinationConvergesWithSourceOffline) {
  installed();
  transport.online[1] = false;
  grant.permissions = 2;
  auto source_count = transport.sources.size();
  ASSERT_TRUE(peers.grants()->upsert(1, 2, grant));
  EXPECT_EQ(source_count, transport.sources.size());
  EXPECT_EQ(0, transport.destination_key_sizes.back());
  EXPECT_EQ(2u, store.a.revision);
}
TEST_F(SupLanPeerTest, PeerKeyRequiredBoundedRecoveryAndDuplicateIgnored) {
  installed();
  peers.reconnect(2);
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  auto n = transport.sources.size();
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  EXPECT_EQ(n, transport.sources.size());
  source_ok();
  source_ok();
  destination_result();
  auto sends = transport.order.size();
  source_ok();
  destination_result();
  EXPECT_EQ(sends, transport.order.size());
}
TEST_F(SupLanPeerTest, KeyRecoveryWaitsForThePendingNoKeySourceReply) {
  installed();
  peers.reconnect(2);
  auto no_key = transport.sources.size() - 1;
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  EXPECT_EQ(no_key + 1, transport.sources.size());
  source_ok(no_key);
  ASSERT_EQ(no_key + 2, transport.sources.size());
  EXPECT_EQ(SUPLA_SUPLAN_SOURCE_FLAG_RETURN_PEER_KEY,
            transport.sources.back().Flags);
  source_ok();
  EXPECT_TRUE(transport.errors.empty());
  EXPECT_EQ(32, transport.destination_key_sizes.back());
}
TEST_F(SupLanPeerTest, DuplicateNoKeyAckCannotCancelPendingKeyRecovery) {
  installed();
  peers.reconnect(2);
  auto no_key = transport.sources.size() - 1;
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  source_ok(no_key);
  ASSERT_EQ(SUPLA_SUPLAN_SOURCE_FLAG_RETURN_PEER_KEY,
            transport.sources.back().Flags);
  auto sends = transport.order.size();
  source_ok(no_key);
  EXPECT_EQ(sends, transport.order.size());
  EXPECT_TRUE(transport.errors.empty());
  source_ok();
  EXPECT_EQ(32, transport.destination_key_sizes.back());
  destination_result();
  EXPECT_TRUE(transport.errors.empty());
}
TEST_F(SupLanPeerTest, DuplicateKeyRequiredCannotCancelKeyBearingDestination) {
  installed();
  peers.reconnect(2);
  auto keyless_destination = transport.destinations.size() - 1;
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  source_ok();
  source_ok();
  ASSERT_EQ(32, transport.destination_key_sizes.back());
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED,
                     keyless_destination);
  EXPECT_TRUE(transport.errors.empty());
  destination_result(SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
  ASSERT_EQ(1u, transport.errors.size());
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR, transport.errors.back());
}
TEST_F(SupLanPeerTest, MissingKeyWithoutAnAcceptedNoKeyReplyIsDiagnosed) {
  add();
  auto q = transport.sources.back();
  TDS_SuplaSetSuplanSourceAssociationResult r = {};
  r.PeerContext = q.PeerContext;
  r.AclRevision = q.AclRevision;
  peers.on_source(1, &r);
  ASSERT_EQ(1u, transport.errors.size());
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT, transport.errors.back());
  EXPECT_TRUE(transport.destinations.empty());
}
TEST_F(SupLanPeerTest, DestinationReconnectWaitsForTheInitialNoKeyReply) {
  transport.online[2] = false;
  add();
  transport.online[2] = true;
  peers.reconnect(2);
  ASSERT_EQ(1u, transport.sources.size());
  source_ok(0);
  ASSERT_EQ(2u, transport.sources.size());
  EXPECT_EQ(SUPLA_SUPLAN_SOURCE_FLAG_RETURN_PEER_KEY,
            transport.sources.back().Flags);
  source_ok();
  EXPECT_TRUE(transport.errors.empty());
  EXPECT_EQ(32, transport.destination_key_sizes.back());
}
TEST_F(SupLanPeerTest, OldNonemptyKeyResponseFencedAfterEmptyCleanup) {
  add();
  auto old = transport.sources.back();
  remove();
  source_ok();
  destination_result();
  EXPECT_EQ(Lifecycle::Dormant, store.a.lifecycle);
  auto count = transport.order.size();
  TDS_SuplaSetSuplanSourceAssociationResult r = {};
  r.PeerContext = old.PeerContext;
  r.AclRevision = old.AclRevision;
  r.PeerKeySize = 32;
  memset(r.PeerKey, 0xA5, 32);
  peers.on_source(1, &r);
  EXPECT_EQ(count, transport.order.size());
  EXPECT_EQ(0, r.PeerKey[0]);
  peers.reconcile(1);
  EXPECT_EQ(count, transport.order.size());
}
TEST_F(SupLanPeerTest, StaleSourceAckCannotForwardKeyForNewAcl) {
  add();
  auto old_count = transport.destinations.size();
  grant.permissions = 2;
  ASSERT_TRUE(peers.grants()->upsert(1, 2, grant));
  source_ok(0);
  EXPECT_EQ(old_count, transport.destinations.size());
  source_ok();
  EXPECT_TRUE(transport.key_seen);
}
TEST_F(SupLanPeerTest, SourceAckCannotForwardKeyForAnObsoleteOwnerSnapshot) {
  add();
  store.identity.channels.clear();
  source_ok();
  EXPECT_TRUE(transport.destinations.empty());
  EXPECT_FALSE(transport.key_seen);
  EXPECT_EQ(0u, store.a.provisioned_generation);
}
TEST_F(SupLanPeerTest, ConcurrentOriginsNoLostUpdate) {
  add();
  std::vector<std::thread> threads;
  for (int i = 0; i < 12; ++i)
    threads.emplace_back([&, i] {
      auto g = grant;
      g.origin_id += i + 1;
      g.permissions = i % 2 ? 2 : 4;
      EXPECT_TRUE(peers.grants()->upsert(1, 2, g));
    });
  for (auto &thread : threads) thread.join();
  EXPECT_EQ(13u, store.grants.size());
  EXPECT_EQ(6, store.a.acl[0].Permissions);
  EXPECT_GE(2u, store.a.revision - 1);
  EXPECT_EQ(1u, store.a.generation);
}
TEST_F(SupLanPeerTest, CapacityAndPersistenceDoNotSpamUnchangedEnsure) {
  add();
  source_ok(-1, SUPLA_SUPLAN_RESULT_CAPACITY_EXCEEDED);
  auto sends = transport.order.size();
  for (int i = 0; i < 10; ++i) peers.reconcile(1);
  EXPECT_EQ(sends, transport.order.size());
  peers.reconnect(1);
  EXPECT_GT(transport.order.size(), sends);
  source_ok(-1, SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR);
  sends = transport.order.size();
  peers.reconcile(1);
  EXPECT_EQ(sends, transport.order.size());
}
TEST_F(SupLanPeerTest, CurrentStaleDiagnosedAndStopped) {
  for (auto code : {SUPLA_SUPLAN_RESULT_STALE_REVISION,
                    SUPLA_SUPLAN_RESULT_STALE_GENERATION}) {
    peers.reconnect(1);
    if (!store.a.id) add();
    source_ok(-1, code);
    auto sends = transport.order.size();
    peers.reconcile(1);
    EXPECT_EQ(sends, transport.order.size());
    EXPECT_EQ(code, transport.errors.back());
  }
}
TEST_F(SupLanPeerTest, RootMismatchRebootstraps) {
  add();
  source_ok(-1, SUPLA_SUPLAN_RESULT_ROOT_EPOCH_MISMATCH);
  EXPECT_EQ(1, transport.refreshes);
}
TEST_F(SupLanPeerTest, OwnerAcceptanceGateBlocksNewChannelProvisioning) {
  transport.owner_gate = false;
  add();
  EXPECT_TRUE(transport.order.empty());
  transport.owner_gate = true;
  store.identity.channels.clear();
  peers.reconnect(1);
  EXPECT_TRUE(transport.order.empty());
  store.identity.channels.push_back(10);
  peers.reconnect(1);
  EXPECT_FALSE(transport.order.empty());
}
TEST_F(SupLanPeerTest, RemovedEndpointRequiresSurvivorCleanup) {
  installed();
  ASSERT_TRUE(peers.grants()->remove_device(1));
  EXPECT_TRUE(store.a.source_removed);
  EXPECT_TRUE(store.a.source_acked);
  EXPECT_EQ(Lifecycle::Draining, store.a.lifecycle);
  destination_result();
  EXPECT_EQ(0u, store.a.id);
}
TEST_F(SupLanPeerTest, RemovedDestinationRequiresSourceCleanup) {
  installed();
  ASSERT_TRUE(peers.grants()->remove_device(2));
  EXPECT_TRUE(store.a.destination_removed);
  source_ok();
  EXPECT_EQ(0u, store.a.id);
}
TEST_F(SupLanPeerTest, EnsureValidationAndNoAuthorizationOrApprovalCreation) {
  TDS_SuplaEnsureResourceAccess q = {
      {1, 10}, 1, SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER, 0};
  auto r = peers.grants()->ensure_access(2, q);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, r.Result);
  EXPECT_EQ(0u, store.a.id);
  q.Flags = 2;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT,
            peers.grants()->ensure_access(2, q).Result);
  q.Flags = 0;
  q.DeliveryMode = 77;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT,
            peers.grants()->ensure_access(2, q).Result);
  q.DeliveryMode = SUPLA_RESOURCE_DELIVERY_SERVER_STREAM;
  q.Permissions = SUPLA_SUPLAN_PERMISSION_CONTROL;
  r = peers.grants()->ensure_access(2, q);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED, r.Result);
  EXPECT_EQ(SUPLA_RESOURCE_DELIVERY_SERVER_STREAM, r.DeliveryMode);
  q.DeliveryMode = SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER;
  q.Permissions = SUPLA_SUPLAN_PERMISSION_READ;
  q.Flags = 1;
  r = peers.grants()->ensure_access(2, q);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, r.Result);
  EXPECT_EQ(0u, store.a.id);
  installed();
  r = peers.grants()->ensure_access(2, q);
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, r.AccessStatus);
  EXPECT_EQ(SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER, r.DeliveryMode);
  remove();
  r = peers.grants()->ensure_access(2, q);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, r.Result);
  EXPECT_TRUE(store.grants.empty());
}
TEST_F(SupLanPeerTest, ShareRequiresAuthoritativeBindingNotAnUnrelatedGrant) {
  installed();
  TDS_SuplaEnsureResourceShare q = {{1, 10}, {1, 20}, 1};
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
            peers.grants()->ensure_share(1, q).Result);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
            peers.grants()->ensure_share(2, q).Result);
  q.Permissions = 8;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT,
            peers.grants()->ensure_share(1, q).Result);
}
TEST_F(SupLanPeerTest, RevisionExhaustionAndExplicitDistrustRotateGeneration) {
  installed();
  store.a.revision = UINT32_MAX;
  grant.permissions = 2;
  ASSERT_TRUE(peers.grants()->upsert(1, 2, grant));
  EXPECT_EQ(2u, store.a.generation);
  EXPECT_EQ(1u, store.a.revision);
  ASSERT_TRUE(peers.grants()->distrust(1));
  EXPECT_EQ(3u, store.a.generation);
}
TEST_F(SupLanPeerTest, CleanupBeforeFirstSourceAckStillFencesReactivation) {
  add();
  remove();
  source_ok();
  destination_result();
  EXPECT_EQ(Lifecycle::Dormant, store.a.lifecycle);
  EXPECT_EQ(123u, store.a.root);
  add();
  EXPECT_EQ(2u, store.a.generation);
  EXPECT_EQ(1u, store.a.revision);
}
TEST_F(SupLanPeerTest, SourceOfflineKeyRecoveryWaitsUntilReconnect) {
  installed();
  transport.online[1] = false;
  peers.reconnect(2);
  auto source_count = transport.sources.size();
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  EXPECT_EQ(source_count, transport.sources.size());
  peers.reconcile(1);
  EXPECT_EQ(source_count, transport.sources.size());
  transport.online[1] = true;
  peers.reconnect(1);
  destination_result(SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED);
  source_ok();
  source_ok();
  EXPECT_EQ(32, transport.destination_key_sizes.back());
}
TEST_F(SupLanPeerTest, RestartAfterSourceOkDoesNotAssumeDestinationHasKey) {
  add();
  source_ok();
  PeerProvisioner restarted(factory, &transport);
  restarted.reconnect(2);
  EXPECT_EQ(0, transport.destination_key_sizes.back());
  auto r = transport.destinations.back();
  r.Result = SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED;
  restarted.on_destination(2, &r);
  auto pending = transport.sources.back();
  EXPECT_EQ(0, pending.Flags);
  TDS_SuplaSetSuplanSourceAssociationResult no_key = {};
  no_key.PeerContext = pending.PeerContext;
  no_key.AclRevision = pending.AclRevision;
  restarted.on_source(1, &no_key);
  EXPECT_EQ(0, transport.destination_key_sizes.back());
  auto q = transport.sources.back();
  EXPECT_EQ(1, q.Flags);
  TDS_SuplaSetSuplanSourceAssociationResult key = {};
  key.PeerContext = q.PeerContext;
  key.AclRevision = q.AclRevision;
  key.PeerKeySize = 32;
  memset(key.PeerKey, 0xA5, sizeof(key.PeerKey));
  restarted.on_source(1, &key);
  EXPECT_EQ(32, transport.destination_key_sizes.back());
  EXPECT_EQ(0, key.PeerKey[0]);
}
TEST_F(SupLanPeerTest, MalformedKeyReplyIsWipedAndCannotReachDestination) {
  add();
  auto q = transport.sources.back();
  TDS_SuplaSetSuplanSourceAssociationResult r = {};
  r.PeerContext = q.PeerContext;
  r.AclRevision = q.AclRevision;
  r.PeerKeySize = 31;
  memset(r.PeerKey, 0xA5, sizeof(r.PeerKey));
  peers.on_source(1, &r);
  EXPECT_TRUE(transport.destinations.empty());
  EXPECT_EQ(0, r.PeerKey[0]);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT, transport.errors.back());
}
TEST_F(SupLanPeerTest, GenerationExhaustionRollsBackGrantMutation) {
  installed();
  store.a.generation = UINT32_MAX;
  store.a.revision = UINT32_MAX;
  grant.permissions = 2;
  int writes = store.writes;
  EXPECT_FALSE(peers.grants()->upsert(1, 2, grant));
  EXPECT_EQ(1, store.grants[0].permissions);
  EXPECT_EQ(1, store.a.acl[0].Permissions);
  EXPECT_EQ(writes, store.writes);
  EXPECT_EQ(UINT32_MAX, store.a.generation);
}
TEST_F(SupLanPeerTest, DeviceScopeUnionCoversLeafEnsure) {
  grant.resource_type = 2;
  grant.resource_id = 1;
  grant.permissions = 2;
  add();
  TDS_SuplaEnsureResourceAccess q = {
      {1, 11}, 1, SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER, 0};
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            peers.grants()->ensure_access(2, q).AccessStatus);
  q.Permissions = 4;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED,
            peers.grants()->ensure_access(2, q).Result);
}
TEST(SupLanCanonicalTest, CapacityIsExplicitAndNeverTruncates) {
  std::vector<Grant> grants;
  for (int i = 0; i < 90; ++i)
    grants.push_back({1, static_cast<uint32_t>(i + 1), 1, 1, 1});
  Acl acl;
  EXPECT_FALSE(canonicalize(grants, &acl));
  EXPECT_TRUE(acl.empty());
}
TEST(SupLanCanonicalTest, DeterministicByteEncodingAndRejectNoncanonicalInput) {
  Acl acl;
  ASSERT_TRUE(canonicalize(
      {{2, 1, 1, 3, 1}, {1, 11, 1, 2, 1}, {1, 10, 1, 1, 3}}, &acl));
  EXPECT_EQ((std::vector<unsigned char>{1, 0, 0, 0, 10, 2, 1, 0, 0, 0, 11, 1, 2,
                                        0, 0, 0, 1, 1}),
            encode_acl(acl));
  Acl decoded;
  auto data = encode_acl(acl);
  EXPECT_TRUE(decode_acl(data.data(), data.size(), &decoded));
  data[5] = 3;
  EXPECT_FALSE(decode_acl(data.data(), data.size(), &decoded));
  EXPECT_FALSE(decode_acl(data.data(), data.size() - 1, &decoded));
}
}  // namespace

TEST(SupLanCanonicalTest, MissingApprovalRightsUseCanonicalControlAndAction) {
  struct Case { uint8_t requested, effective, missing; };
  const Case cases[] = {{1, 0, 1}, {2, 0, 2}, {3, 0, 2}, {1, 2, 0},
                        {3, 2, 0}, {2, 1, 2}, {7, 1, 6}, {7, 2, 4},
                        {7, 4, 2}, {4, 1, 4}, {1, 4, 1}, {7, 6, 0}};
  for (const auto &c : cases)
    EXPECT_EQ(c.missing, missing_permissions(c.requested, c.effective));
}

TEST_F(SupLanPeerTest, M5InvalidEnsureCannotReconcileOrTouchAuthorization) {
  installed();
  const auto before = store.a;
  const auto writes = store.writes;
  int reconciliations = 0;
  auto request = [&](int requester, const TDS_SuplaEnsureResourceAccess &q) {
    return peers.grants()->ensure_access(requester, q, [&] {
      ++reconciliations;
      return true;
    });
  };
  const TDS_SuplaEnsureResourceAccess valid = {
      {1, 10}, 1, SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER, 1};
  for (int field = 0; field < 7; ++field) {
    auto q = valid;
    switch (field) {
      case 0: q.Flags = 2; break;
      case 1: q.Permissions = 0; break;
      case 2: q.Permissions = 8; break;
      case 3: q.Resource.ResourceId = 0; break;
      case 4: q.Resource.ResourceId = UINT32_MAX; break;
      case 5: q.DeliveryMode = 77; break;
      case 6: q.Resource.ResourceType = 99; break;
    }
    EXPECT_NE(SUPLA_SUPLAN_RESULT_OK, request(2, q).Result);
  }
  for (int requester : {0, -1, 1, 99})
    EXPECT_NE(SUPLA_SUPLAN_RESULT_OK, request(requester, valid).Result);
  auto q = valid;
  q.Resource.ResourceId = 99;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_FOUND, request(2, q).Result);
  q = valid;
  q.Resource = {2, 1};
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED, request(2, q).Result);
  q = valid;
  q.DeliveryMode = SUPLA_RESOURCE_DELIVERY_SERVER_STREAM;
  q.Permissions = SUPLA_SUPLAN_PERMISSION_CONTROL;
  const auto stream = request(2, q);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED, stream.Result);
  EXPECT_EQ(SUPLA_RESOURCE_DELIVERY_SERVER_STREAM, stream.DeliveryMode);
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_INVALID, stream.AccessStatus);
  // The valid missing resource audits stale origins before NOT_FOUND.
  EXPECT_EQ(1, reconciliations);
  EXPECT_TRUE(same_state(before, store.a));
  EXPECT_EQ(writes, store.writes);
}

TEST_F(SupLanPeerTest,
       M5ManualEnsureChecksCapabilityFunctionAndAcceptedIdentity) {
  installed();
  const TDS_SuplaEnsureResourceAccess q = {
      {1, 10}, 1, SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER, 1};
  int reconciliations = 0;
  auto ensure = [&] {
    return peers.grants()->ensure_access(2, q, [&] {
      ++reconciliations;
      return true;
    });
  };
  store.device_info.flags = 0;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED, ensure().Result);
  store.device_info.flags = SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED;
  store.device_info.protocol = 28;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED, ensure().Result);
  store.device_info.protocol = 29;
  store.channel_info.function = 0;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED, ensure().Result);
  store.channel_info.function = SUPLA_CHANNELFNC_THERMOMETER;
  store.identity.channels = {11};
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, ensure().Result);
  store.identity.channels = {10, 11};
  EXPECT_EQ(0, reconciliations);
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, ensure().AccessStatus);
  EXPECT_EQ(1, reconciliations);
}

TEST_F(SupLanPeerTest,
       M5UnsupportedActionPreservesExistingReadEvenWithBroadAcl) {
  installed();
  Grant broad{2, 1, 7, 700, 4};
  ASSERT_TRUE(peers.grants()->upsert(1, 2, broad));
  const auto before = store.a;
  const auto writes = store.writes;
  TDS_SuplaEnsureResourceAccess q = {
      {1, 10}, 5, SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER, 1};
  const auto rejected = peers.grants()->ensure_access(2, q);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED, rejected.Result);
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_INVALID, rejected.AccessStatus);
  EXPECT_TRUE(same_state(before, store.a));
  EXPECT_EQ(writes, store.writes);
  q.Permissions = 1;
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            peers.grants()->ensure_access(2, q).AccessStatus);
}

TEST_F(SupLanPeerTest,
       M5ExistingAtActionIsIndependentAndNeedsNoApprovalOrigin) {
  store.channel_info.type = SUPLA_CHANNELTYPE_ACTIONTRIGGER;
  store.channel_info.function = SUPLA_CHANNELFNC_ACTIONTRIGGER;
  store.channel_info.action_capabilities = 2;
  grant.permissions = 4;
  installed();
  const auto before = store.a;
  const auto writes = store.writes;
  const auto sends = transport.order.size();
  TDS_SuplaEnsureResourceAccess q = {
      {1, 10}, 4, SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER, 1};
  for (int i = 0; i < 20; ++i)
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
              peers.grants()->ensure_access(2, q).AccessStatus);
  EXPECT_EQ(writes, store.writes);
  EXPECT_EQ(sends, transport.order.size());
  EXPECT_TRUE(same_state(before, store.a));
  store.channel_info.action_capabilities = 0;
  q.Flags = 0;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED,
            peers.grants()->ensure_access(2, q).Result);
  EXPECT_TRUE(same_state(before, store.a));
  store.channel_info.action_capabilities = 2;
  q.Flags = 1;
  q.Permissions = 1;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED,
            peers.grants()->ensure_access(2, q).Result);
}

TEST_F(SupLanPeerTest, M5MissingExtensionFailsClosedWithoutCloudSchema) {
  store.channel_info.type = SUPLA_CHANNELTYPE_RELAY;
  store.channel_info.function = SUPLA_CHANNELFNC_POWERSWITCH;
  installed();
  const auto before = store.a;
  const auto writes = store.writes;
  const auto sends = transport.order.size();
  TDS_SuplaEnsureResourceAccess q = {
      {1, 10}, 2, SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER, 1};
  for (int i = 0; i < 20; ++i) {
    const auto result = peers.grants()->ensure_access(2, q);
    EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, result.Result);
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_INVALID, result.AccessStatus);
  }
  EXPECT_TRUE(same_state(before, store.a));
  EXPECT_EQ(writes, store.writes);
  EXPECT_EQ(sends, transport.order.size());
  ASSERT_EQ(1u, store.grants.size());
  EXPECT_EQ(1, store.grants.front().permissions);
  q.Permissions = 1;
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED,
            peers.grants()->ensure_access(2, q).AccessStatus);
}

namespace {
class SupLanStreamConnectionTest : public testing::Test {
 protected:
  StreamConnection::Time now{};
  bool connected = true, database = true, allowed = true, pending = false;
  bool send_ok = true;
  unsigned checks = 0, approval_requests = 0, reads = 0;
  std::vector<TSD_SuplaRemoteChannelState> messages;
  std::vector<unsigned> order;
  StreamConnection::Snapshot current;
  std::function<void()> during_read;
  unsigned notifications = 0;
  std::function<void()> on_notify;
  StreamConnection connection{
      2,
      [this](uint32_t, bool create, bool) {
        ++checks;
        if (create && !allowed) {
          ++approval_requests;
          pending = true;
        }
        return TSD_SuplaEnsureResourceAccessResult{
            static_cast<unsigned char>(
                database ? SUPLA_SUPLAN_RESULT_OK
                         : SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR),
            static_cast<unsigned char>(
                allowed   ? SUPLA_SUPLAN_ACCESS_STATUS_GRANTED
                : pending ? SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL
                          : SUPLA_SUPLAN_ACCESS_STATUS_REJECTED),
            SUPLA_RESOURCE_DELIVERY_SERVER_STREAM};
      },
      [this](uint32_t, StreamConnection::Snapshot *snapshot) {
        ++reads;
        *snapshot = current;
        if (during_read) {
          auto callback = during_read;
          during_read = nullptr;
          callback();
        }
        return true;
      },
      [this](const TSD_SuplaRemoteChannelState &snapshot) {
        order.push_back(1380);
        messages.push_back(snapshot);
        return send_ok;
      },
      [this] { return connected; },
      [this] { return now; },
      [this] {
        ++notifications;
        if (on_notify) on_notify();
      }};
  TSD_SuplaEnsureResourceAccessResult ensure(uint32_t id = 10,
                                             bool approval = false,
                                             unsigned char permissions = 1) {
    TDS_SuplaEnsureResourceAccess q{
        {1, id}, permissions, 2, static_cast<unsigned char>(approval)};
    TSD_SuplaEnsureResourceAccessResult result{};
    connection.ensure(q, [&](const TSD_SuplaEnsureResourceAccessResult &r) {
      order.push_back(1350);
      result = r;
      return true;
    });
    return result;
  }
  void advance(unsigned seconds) {
    now += std::chrono::seconds(seconds);
    connection.iterate();
  }
  void SetUp() override {
    current.state.Channel.Type = SUPLA_CHANNELTYPE_THERMOMETER;
    current.state.Channel.Default = SUPLA_CHANNELFNC_THERMOMETER;
    current.state.Channel.value[0] = 42;
  }
};
TEST_F(SupLanStreamConnectionTest,
       InitialSnapshotOrderedAndDuplicateIdempotent) {
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, ensure().AccessStatus);
  ASSERT_EQ(2u, order.size());
  EXPECT_EQ(1350u, order[0]);
  EXPECT_EQ(1380u, order[1]);
  EXPECT_EQ(10, messages[0].ChannelId);
  EXPECT_EQ(255, messages[0].Channel.Number);
  EXPECT_EQ(42, messages[0].Channel.value[0]);
  ensure();
  ensure();
  EXPECT_EQ(1u, connection.size());
  EXPECT_EQ(1u, messages.size());
}
TEST_F(SupLanStreamConnectionTest, DirtySignalsOnlySubscribedAndOutsideLock) {
  connection.dirty(10);
  EXPECT_EQ(0u, notifications);
  ensure();
  on_notify = [&] { connection.dirty(999); };  // Reenters dirty_mutex.
  auto before = checks;
  connection.dirty(999);
  connection.dirty(10);
  connection.dirty(10);
  EXPECT_EQ(1u, notifications);
  EXPECT_EQ(before, checks);  // No authorization/read/send on Source thread.
  EXPECT_EQ(1u, messages.size());
  current.state.Channel.value[0] = 43;
  connection.iterate();
  EXPECT_EQ(2u, messages.size());
  connection.dirty(10);
  EXPECT_EQ(2u, notifications);
  connection.close();
  connection.dirty(10);
  EXPECT_EQ(2u, notifications);
  EXPECT_EQ(0u, connection.time_left_usec());
}
TEST_F(SupLanStreamConnectionTest, QuietCheckRetryAndTtlExposeNearestDeadline) {
  EXPECT_EQ(0u, connection.time_left_usec());
  ensure();
  EXPECT_EQ(30000000u, connection.time_left_usec());
  now += std::chrono::seconds(7);
  EXPECT_EQ(23000000u, connection.time_left_usec());
  current.expires = now + std::chrono::seconds(3);
  connection.dirty(10);
  EXPECT_EQ(1u, connection.time_left_usec());
  connection.iterate();
  EXPECT_EQ(3000000u, connection.time_left_usec());
  now += std::chrono::seconds(3);
  EXPECT_EQ(1u, connection.time_left_usec());
  connection.iterate();
  EXPECT_EQ(1, messages.back().Channel.Offline);
  EXPECT_EQ(30000000u, connection.time_left_usec());
  database = false;
  connection.dirty(10);
  connection.iterate();
  EXPECT_EQ(5000000u, connection.time_left_usec());
  // Dirty while backing off may wake once, but cannot cause a busy loop.
  connection.dirty(10);
  connection.iterate();
  EXPECT_EQ(5000000u, connection.time_left_usec());
  auto before = checks;
  connection.iterate();
  EXPECT_EQ(before, checks);
  database = true;
  now += std::chrono::seconds(5);
  connection.iterate();
  EXPECT_EQ(30000000u, connection.time_left_usec());
  allowed = false;
  connection.dirty(10);
  connection.iterate();
  EXPECT_EQ(0u, connection.time_left_usec());
}
TEST_F(SupLanStreamConnectionTest, EarliestInterestDeadline) {
  current.expires = now + std::chrono::seconds(8);
  ensure(10);
  current.expires = now + std::chrono::seconds(2);
  ensure(11);
  EXPECT_EQ(2000000u, connection.time_left_usec());
  connection.close();
  EXPECT_EQ(0u, connection.time_left_usec());
}
TEST_F(SupLanStreamConnectionTest, PendingHasThirtySecondDeadline) {
  allowed = false;
  ensure(10, true);
  EXPECT_TRUE(messages.empty());
  EXPECT_EQ(30000000u, connection.time_left_usec());
  now += std::chrono::seconds(30);
  connection.iterate();
  EXPECT_EQ(30000000u, connection.time_left_usec());
}
TEST_F(SupLanStreamConnectionTest, DirtyLatchesBeforeAndDuringEventWait) {
  std::unique_ptr<TEventHandler, decltype(&eh_free)> event(eh_init(), eh_free);
  ASSERT_NE(nullptr, event);
  on_notify = [&] { eh_raise_event(event.get()); };
  ensure();
  for (bool before_wait : {true, false}) {
    std::promise<void> entering;
    int result = 0;
    ++current.state.Channel.value[0];
    if (before_wait) connection.dirty(10);
    std::thread waiter([&] {
      entering.set_value();
      result = eh_wait(event.get(), 2000000);
    });
    entering.get_future().wait();
    if (!before_wait) connection.dirty(10);
    waiter.join();
    EXPECT_GT(result, 0);
    connection.iterate();
  }
  EXPECT_EQ(3u, messages.size());
  connection.close();
}
TEST_F(SupLanStreamConnectionTest,
       ConcurrentDirtyCloseCannotPublishOrDeadlock) {
  ensure();
  std::atomic<bool> start{false};
  // Only dirty/close run concurrently; the injected clock is not mutated.
  on_notify = [&] { connection.dirty(999); };
  std::thread source([&] {
    while (!start.load()) std::this_thread::yield();
    for (unsigned i = 0; i < 1000; ++i) connection.dirty(10);
  });
  start = true;
  connection.close();
  source.join();
  auto before = notifications;
  connection.dirty(10);
  connection.iterate();
  EXPECT_EQ(before, notifications);
  EXPECT_EQ(0u, connection.size());
  EXPECT_EQ(1u, messages.size());
}
TEST_F(SupLanStreamConnectionTest,
       PendingActivationAndLostHintWithoutNewEnsure) {
  allowed = false;
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            ensure(10, true).AccessStatus);
  EXPECT_TRUE(messages.empty());
  EXPECT_EQ(0u, reads);
  allowed = true;
  advance(29);
  EXPECT_TRUE(messages.empty());
  advance(1);
  ASSERT_EQ(1u, messages.size());
  EXPECT_EQ(1u, approval_requests);
}
TEST_F(SupLanStreamConnectionTest,
       GrantedToPendingInvalidatesOnceAndKeepsLiveDemand) {
  ensure();
  ASSERT_EQ(1u, messages.size());
  current.state.Channel.value[0] = 99;
  allowed = false;
  const auto previous_reads = reads;
  EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
            ensure(10, true).AccessStatus);
  ASSERT_EQ(2u, messages.size());
  EXPECT_EQ(1, messages.back().Channel.Offline);
  EXPECT_EQ(42, messages.back().Channel.value[0]);
  EXPECT_EQ(previous_reads, reads);

  // Neither repeated requests nor dirty work may postpone invalidation or
  // deliver newly protected data while approval is pending.
  for (unsigned i = 0; i < 10; ++i) {
    advance(29);
    EXPECT_EQ(SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL,
              ensure(10, true).AccessStatus);
    connection.dirty(10);
    connection.iterate();
  }
  advance(30);
  EXPECT_EQ(2u, messages.size());
  EXPECT_EQ(previous_reads, reads);
  EXPECT_EQ(1u, connection.size());

  allowed = true;
  connection.dirty(10);  // Cloud approval hint, without another ENSURE.
  connection.iterate();
  ASSERT_EQ(3u, messages.size());
  EXPECT_EQ(0, messages.back().Channel.Offline);
  EXPECT_EQ(99, messages.back().Channel.value[0]);
}
TEST_F(SupLanStreamConnectionTest,
       GrantedToPendingRecoversApprovalWithoutHintOrNewEnsure) {
  ensure();
  allowed = false;
  ensure(10, true);
  advance(30);
  ASSERT_EQ(2u, messages.size());
  EXPECT_EQ(1u, connection.size());
  allowed = true;
  advance(30);
  ASSERT_EQ(3u, messages.size());
  EXPECT_EQ(0, messages.back().Channel.Offline);
}
TEST_F(SupLanStreamConnectionTest,
       GrantedToPendingTerminalSendFailureEndsConnectionDemand) {
  ensure();
  allowed = false;
  send_ok = false;
  ensure(10, true);
  EXPECT_EQ(0u, connection.size());
  allowed = true;
  send_ok = true;
  connection.dirty(10);
  advance(60);
  EXPECT_EQ(2u, messages.size());
}
TEST_F(SupLanStreamConnectionTest, InitDirtyCannotLoseConcurrentLatestState) {
  during_read = [&] {
    current.state.Channel.value[0] = 88;
    connection.dirty(10);
  };
  ensure();
  ASSERT_EQ(1u, messages.size());
  EXPECT_EQ(42, messages[0].Channel.value[0]);
  advance(1);
  ASSERT_EQ(2u, messages.size());
  EXPECT_EQ(88, messages[1].Channel.value[0]);
}
TEST_F(SupLanStreamConnectionTest, RevocationDuringInitialReadLeaksNoSnapshot) {
  during_read = [&] {
    allowed = false;
    pending = false;
  };
  ensure();
  EXPECT_TRUE(messages.empty());
  EXPECT_EQ(0u, connection.size());
}
TEST_F(SupLanStreamConnectionTest, TerminalFencesDirtyAndCannotResurrect) {
  ensure();
  current.state.Channel.value[0] = 99;
  allowed = false;
  connection.dirty(10);
  advance(1);
  ASSERT_EQ(2u, messages.size());
  EXPECT_EQ(1, messages.back().Channel.Offline);
  EXPECT_EQ(42, messages.back().Channel.value[0]);
  allowed = true;
  connection.dirty(10);
  advance(60);
  EXPECT_EQ(2u, messages.size());
  EXPECT_EQ(0u, connection.size());
}
TEST_F(SupLanStreamConnectionTest, ExpiryRenewalNoExtraSnapshotAndRecovery) {
  current.expires = now + std::chrono::seconds(5);
  current.state.Channel.ValueValidityTimeSec = 5;
  ensure();
  advance(4);
  current.expires = now + std::chrono::seconds(5);
  connection.dirty(10);
  connection.iterate();
  EXPECT_EQ(1u, messages.size());
  advance(1);
  EXPECT_EQ(1u, messages.size());
  advance(4);
  ASSERT_EQ(2u, messages.size());
  EXPECT_EQ(1, messages.back().Channel.Offline);
  current.expires = now + std::chrono::seconds(5);
  connection.dirty(10);
  connection.iterate();
  ASSERT_EQ(3u, messages.size());
  EXPECT_EQ(0, messages.back().Channel.Offline);
}
TEST_F(SupLanStreamConnectionTest, MetadataChangeAndUnrelatedChannelIsolation) {
  ensure();
  current.state.Channel.Flags = 456;
  connection.dirty(11);
  advance(1);
  EXPECT_EQ(1u, messages.size());
  connection.dirty(10);
  connection.iterate();
  ASSERT_EQ(2u, messages.size());
  EXPECT_EQ(456, static_cast<int64_t>(messages.back().Channel.Flags));
}
TEST_F(SupLanStreamConnectionTest,
       CapacityPrecedesDurableApprovalAndNoEviction) {
  ensure(10);
  ensure(11);
  unsigned before = checks;
  allowed = false;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_CAPACITY_EXCEEDED, ensure(12, true).Result);
  EXPECT_EQ(before, checks);
  EXPECT_EQ(0u, approval_requests);
  EXPECT_EQ(2u, connection.size());
  EXPECT_EQ(2u, messages.size());
}
TEST_F(SupLanStreamConnectionTest,
       InvalidShapeAndIneligibleConnectionAllocateNothing) {
  for (unsigned char mask : {0, 2, 3, 4, 5, 7, 128})
    EXPECT_NE(SUPLA_SUPLAN_RESULT_OK, ensure(10, true, mask).Result);
  connected = false;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, ensure(10, true).Result);
  EXPECT_EQ(0u, checks);
  EXPECT_EQ(0u, connection.size());
}
TEST_F(SupLanStreamConnectionTest, DatabaseOutageFailsClosedThenRecovers) {
  ensure();
  database = false;
  current.state.Channel.value[0] = 99;
  connection.dirty(10);
  advance(1);
  ASSERT_EQ(2u, messages.size());
  EXPECT_EQ(1, messages.back().Channel.Offline);
  auto failed_checks = checks;
  for (int second = 0; second < 4; ++second) {
    for (int update = 0; update < 1000; ++update) connection.dirty(10);
    advance(1);
    EXPECT_EQ(failed_checks, checks);  // Dirty floods cannot bypass DB backoff.
  }
  database = true;
  advance(1);
  ASSERT_EQ(3u, messages.size());
  EXPECT_EQ(99, messages.back().Channel.value[0]);
}
TEST_F(SupLanStreamConnectionTest, CloseDropsOnlyEphemeralDemand) {
  ensure();
  connection.close();
  connection.dirty(10);
  advance(60);
  EXPECT_EQ(1u, messages.size());
  EXPECT_EQ(0u, connection.size());
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED, ensure().Result);
}
TEST_F(SupLanStreamConnectionTest,
       DuplicateEnsureDbFailureRetainsSafeRecovery) {
  ensure();
  database = false;
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR, ensure().Result);
  EXPECT_EQ(1u, connection.size());
  ASSERT_EQ(2u, messages.size());
  EXPECT_EQ(1, messages.back().Channel.Offline);
  database = true;
  advance(5);
  ASSERT_EQ(3u, messages.size());
  EXPECT_EQ(0, messages.back().Channel.Offline);
}
}  // namespace

TEST_F(SupLanStreamConnectionTest, SendFailureDropsConnectionInterests) {
  send_ok = false;
  ensure();
  EXPECT_EQ(0u, connection.size());
  send_ok = true;
  advance(60);
  EXPECT_EQ(1u, messages.size());
}

TEST(SupLanStreamConcurrencyTest, MutationAndRevocationFenceAllLateWork) {
  std::atomic<bool> allowed{true};
  std::atomic<int> value{0};
  std::vector<TSD_SuplaRemoteChannelState> messages;
  StreamConnection stream(
      256,
      [&](uint32_t, bool, bool) {
        return TSD_SuplaEnsureResourceAccessResult{
            0,
            static_cast<unsigned char>(
                allowed ? SUPLA_SUPLAN_ACCESS_STATUS_GRANTED
                        : SUPLA_SUPLAN_ACCESS_STATUS_REJECTED),
            2};
      },
      [&](uint32_t, StreamConnection::Snapshot *s) {
        s->state.Channel.Type = SUPLA_CHANNELTYPE_THERMOMETER;
        s->state.Channel.value[0] = value.load();
        return true;
      },
      [&](const TSD_SuplaRemoteChannelState &s) {
        messages.push_back(s);
        return true;
      },
      [] { return true; });
  stream.ensure(
      {{1, 10}, 1, 2, 0},
      [](const TSD_SuplaEnsureResourceAccessResult &) { return true; });
  std::thread producer([&] {
    for (int i = 0; i < 10000; ++i) {
      value = i;
      stream.dirty(10);
    }
  });
  std::thread reader([&] {
    for (int i = 0; i < 10000; ++i) stream.iterate();
  });
  producer.join();
  reader.join();
  allowed = false;
  stream.dirty(10);
  stream.iterate();
  ASSERT_FALSE(messages.empty());
  EXPECT_EQ(1, messages.back().Channel.Offline);
  const auto terminal_count = messages.size();
  for (int i = 0; i < 100; ++i) {
    stream.dirty(10);
    stream.iterate();
  }
  EXPECT_EQ(terminal_count, messages.size());
  EXPECT_EQ(0u, stream.size());
}
TEST(SupLanStreamLoadTest, FanoutCoalescesFloodWithinConfiguredBounds) {
  std::vector<std::unique_ptr<StreamConnection>> destinations;
  unsigned reads = 0, authorizations = 0, messages = 0;
  int value = 0;
  auto began = std::chrono::steady_clock::now();
  for (unsigned n = 0; n < 64; ++n) {
    destinations.emplace_back(new StreamConnection(
        256,
        [&](uint32_t, bool, bool) {
          ++authorizations;
          return TSD_SuplaEnsureResourceAccessResult{
              0, SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, 2};
        },
        [&](uint32_t, StreamConnection::Snapshot *s) {
          ++reads;
          s->state.Channel.Type = SUPLA_CHANNELTYPE_THERMOMETER;
          s->state.Channel.value[0] = value;
          return true;
        },
        [&](const TSD_SuplaRemoteChannelState &) {
          ++messages;
          return true;
        },
        [] { return true; }));
    for (uint32_t id = 1; id <= 64; ++id)
      destinations.back()->ensure(
          {{1, id}, 1, 2, 0},
          [](const TSD_SuplaEnsureResourceAccessResult &) { return true; });
  }
  EXPECT_EQ(4096u, messages);
  const auto baseline = authorizations;
  value = 1;
  for (unsigned i = 0; i < 10000; ++i)
    for (auto &stream : destinations) stream->dirty(10);
  for (auto &stream : destinations) stream->iterate();
  EXPECT_EQ(4096u + 64, messages);
  EXPECT_EQ(messages, reads);
  EXPECT_EQ(baseline + 64, authorizations);
  for (auto &stream : destinations) EXPECT_EQ(64u, stream->size());
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::steady_clock::now() - began)
                     .count();
  std::cout << "STREAM load: 64 connections/4096 interests, 640000 dirty "
               "marks -> 64 updates; elapsed_ms="
            << elapsed << std::endl;
}

TEST(SupLanStreamLoadTest, ExpiryDrainsAcrossBoundedBatchesWithoutNewTraffic) {
  StreamConnection::Time now{};
  unsigned online = 0, offline = 0;
  StreamConnection stream(
      64,
      [](uint32_t, bool, bool) {
        return TSD_SuplaEnsureResourceAccessResult{
            0, SUPLA_SUPLAN_ACCESS_STATUS_GRANTED, 2};
      },
      [&](uint32_t, StreamConnection::Snapshot *snapshot) {
        snapshot->expires = StreamConnection::Time() + std::chrono::seconds(5);
        return true;
      },
      [&](const TSD_SuplaRemoteChannelState &snapshot) {
        if (snapshot.Channel.Offline)
          ++offline;
        else
          ++online;
        return true;
      },
      [] { return true; }, [&] { return now; });
  for (uint32_t id = 1; id <= 33; ++id)
    stream.ensure(
        {{1, id}, 1, 2, 0},
        [](const TSD_SuplaEnsureResourceAccessResult &) { return true; });
  ASSERT_EQ(33u, online);
  now += std::chrono::seconds(5);
  stream.iterate();
  EXPECT_EQ(16u, offline);
  stream.iterate();
  EXPECT_EQ(32u, offline);
  stream.iterate();
  EXPECT_EQ(33u, offline);
  stream.iterate();
  EXPECT_EQ(33u, offline);
}

TEST_F(SupLanPeerTest, BatchProvisioningMessageAccounting) {
  for (bool many_sources : {true, false}) {
    for (bool existing : {false, true}) {
      Store data;
      Transport wire;
      auto time = std::chrono::steady_clock::time_point{};
      PeerProvisioner batch([&] { return std::make_unique<MemoryRepo>(&data); },
                            &wire, [&] { return time; });
      const auto started = std::chrono::steady_clock::now();
      for (unsigned id = 1; id <= 170; ++id) {
        Association a;
        a.id = id;
        a.source = many_sources ? id + 2 : 1;
        a.destination = many_sources ? 2 : id + 2;
        a.lifecycle = Lifecycle::Active;
        if (existing) {
          a.root = data.identity.root;
          a.provisioned_generation = a.generation;
        }
        a.acl = {{1, 1000 + id, 1}};
        data.resource_owners[1000 + id] = a.source;
        data.identity.channels.push_back(1000 + id);
        data.additional[id] = a;
        batch.reconcile(id);
      }
      size_t source_results = 0, destination_results = 0, required = 0;
      std::set<std::pair<int, int>> credentials;
      for (unsigned turn = 0; turn < 1000 &&
           PeerProvisionerTestAccess::count(batch); ++turn) {
        while (source_results < wire.sources.size()) {
          auto q = wire.sources[source_results++];
          TDS_SuplaSetSuplanSourceAssociationResult r{};
          r.PeerContext = q.PeerContext;
          r.AclRevision = q.AclRevision;
          if (q.Flags) {
            r.PeerKeySize = 32;
            memset(r.PeerKey, 0xA5, sizeof(r.PeerKey));
          }
          time += std::chrono::milliseconds(1);  // Host profile, not flash.
          batch.on_source(q.PeerContext.SourceNodeId, &r);
          for (auto byte : r.PeerKey) EXPECT_EQ(0, byte);
        }
        while (destination_results < wire.destinations.size()) {
          auto r = wire.destinations[destination_results];
          auto pair = std::make_pair(
              static_cast<int>(r.PeerContext.SourceNodeId),
              static_cast<int>(r.PeerContext.DestinationNodeId));
          if (existing || wire.destination_key_sizes[destination_results])
            credentials.insert(pair);
          if (!credentials.count(pair)) {
            r.Result = SUPLA_SUPLAN_RESULT_PEER_KEY_REQUIRED;
            ++required;
          }
          ++destination_results;
          time += std::chrono::milliseconds(1);
          batch.on_destination(r.PeerContext.DestinationNodeId, &r);
        }
        batch.iterate();
      }
      EXPECT_EQ(0u, PeerProvisionerTestAccess::count(batch));
      EXPECT_TRUE(wire.forced.empty());
      EXPECT_EQ(170u, wire.sources.size());
      EXPECT_EQ(170u, wire.destinations.size());
      EXPECT_EQ(0u, required);
      auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now() - started).count();
      std::cout << "BATCH sources=" << many_sources << " existing=" << existing
                << " source_requests=" << wire.sources.size()
                << " destination_requests=" << wire.destinations.size()
                << " results=" << source_results + destination_results
                << " key_required=" << required
                << " reconnects=" << wire.forced.size()
                << " wakes=" << wire.wakes.size()
                << " model_ms=" << std::chrono::duration_cast<
                       std::chrono::milliseconds>(
                       time.time_since_epoch()).count()
                << " host_us=" << elapsed << std::endl;
    }
  }
}

TEST_F(SupLanPeerTest, SourceBeginStaleReconcilesWithoutDatabaseRetry) {
  add();
  store.before_begin = [&] { ++store.a.revision; };
  source_ok();
  ASSERT_EQ(2u, transport.sources.size());
  EXPECT_EQ(2u, static_cast<uint32_t>(transport.sources.back().AclRevision));
  EXPECT_TRUE(transport.errors.empty());
  EXPECT_EQ(15000000u, peers.time_left_usec(1));
  EXPECT_TRUE(transport.destinations.empty());
}
TEST_F(SupLanPeerTest, SourcePostCommitStaleNeverForwardsOldKeyOrDefers) {
  add();
  store.after_commit = [&] { ++store.a.revision; };
  source_ok();
  ASSERT_EQ(2u, transport.sources.size());
  EXPECT_EQ(2u, static_cast<uint32_t>(transport.sources.back().AclRevision));
  EXPECT_TRUE(transport.errors.empty());
  EXPECT_EQ(15000000u, peers.time_left_usec(1));
  for (auto size : transport.destination_key_sizes) EXPECT_EQ(0, size);
}
TEST_F(SupLanPeerTest, LongGateJoinsOnceAndUnrelatedDisconnectDoesNotWait) {
  add();
  auto gate = association_gate(1);
  std::unique_lock<std::mutex> held(*gate);
  now += std::chrono::seconds(16);
  auto wakes = transport.wakes.size();
  auto reads = store.reads;
  auto timeout = std::async(std::launch::async, [&] { peers.iterate(1); });
  EXPECT_EQ(std::future_status::timeout,
            timeout.wait_for(std::chrono::seconds(2)));
  // No failed try_lock/rearm loop and no DB operation while the gate is held.
  EXPECT_EQ(wakes, transport.wakes.size());
  EXPECT_EQ(reads, store.reads);
  auto unrelated = std::async(std::launch::async,
                              [&] { peers.disconnected(99, 99); });
  auto status = unrelated.wait_for(std::chrono::milliseconds(300));
  held.unlock();  // Always release before future destructors / assertions.
  EXPECT_EQ(std::future_status::ready, status);
  unrelated.get();
  timeout.get();
  EXPECT_EQ(std::vector<int>({1}), transport.forced);
  EXPECT_EQ(0u, peers.time_left_usec(1));
}

TEST_F(SupLanPeerTest, SourceBeginFailureIsDatabaseOnlyRecovery) {
  add();
  store.fail_begin = true;
  source_ok();
  EXPECT_TRUE(transport.destinations.empty());
  EXPECT_TRUE(transport.forced.empty());
  EXPECT_EQ(5000000u, peers.time_left_usec(1));
  store.fail_begin = false;
  advance(5);
  ASSERT_EQ(2u, transport.sources.size());
  EXPECT_NE(0, transport.sources.back().Flags);
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}

TEST_F(SupLanPeerTest, OfflineReplayCannotRetireUnresolvedReconnectIncident) {
  add();
  advance(16);
  ASSERT_EQ(std::vector<int>({1}), transport.forced);
  transport.online[1] = false;
  peers.disconnected(1, transport.session(1));
  peers.reconnect(2);  // Source is absent: no new key can be obtained.
  EXPECT_EQ(1u, PeerProvisionerTestAccess::count(peers));
  EXPECT_EQ(0u, peers.time_left_usec(2));
  transport.online[1] = true;
  transport.sessions[1] = 100;
  peers.reconnect(1);
  advance(16);
  EXPECT_EQ(std::vector<int>({1}), transport.forced);
  transport.sessions[1] = 200;
  peers.reconnect(1);
  source_ok();
  destination_result();
  EXPECT_EQ(0u, PeerProvisionerTestAccess::count(peers));
}
