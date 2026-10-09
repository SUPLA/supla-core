// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <tuple>
#include <vector>

#include "suplan/peer_provisioner.h"
#include "test/doubles/suplan/PeerProvisionerTestAccess.h"
using namespace supla_suplan;  // NOLINT
namespace {
struct Store {
  std::recursive_mutex mutex;
  Association a;
  std::vector<Grant> grants;
  Identity identity{123, {10, 11}};
  int writes = 0;
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
    store->mutex.lock();
    transaction = true;
    before = store->a;
    before_grants = store->grants;
    before_writes = store->writes;
    return load(id, a) && a->id;
  }
  bool load(uint64_t id, Association *a) override {
    std::lock_guard<std::recursive_mutex> lock(store->mutex);
    *a = store->a.id == id ? store->a : Association{};
    return true;
  }
  bool find(int source, int destination, Association *a) override {
    std::lock_guard<std::recursive_mutex> lock(store->mutex);
    *a = store->a.source == source && store->a.destination == destination
             ? store->a
             : Association{};
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
    store->a = a;
    ++store->writes;
    return true;
  }
  bool erase(uint64_t) override {
    store->a = {};
    return true;
  }
  bool commit() override {
    transaction = false;
    store->mutex.unlock();
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
    *out = store->identity;
    return true;
  }
  bool accept_identity(int, const Identity &i) override {
    store->identity = i;
    return true;
  }
  bool owner(uint8_t type, uint32_t resource, int *device) override {
    if ((type == 1 && (resource == 10 || resource == 11)) ||
        (type == 2 && resource == 1)) {
      *device = 1;
      return true;
    }
    if ((type == 1 && resource == 20) || (type == 2 && resource == 2)) {
      *device = 2;
      return true;
    }
    *device = 0;
    return true;
  }
  bool supports_batch() const override { return true; }
  bool channel(uint32_t id, ChannelInfo *out) override {
    *out = {};
    return owner(SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL, id, &out->device);
  }
  bool device_exists(int d) override { return d == 1 || d == 2; }
  bool for_device(int d, bool dormant, std::vector<uint64_t> *out) override {
    out->clear();
    if (store->a.id && (store->a.source == d || store->a.destination == d) &&
        (dormant || store->a.lifecycle != Lifecycle::Dormant))
      out->push_back(1);
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
  bool ready(int d) override { return online[d]; }
  bool owner_ready(int, uint32_t) override { return owner_gate; }
  bool send(int d, TSDS_SuplaSetSuplanSourceAssociation *q) override {
    order.push_back(d);
    sources.push_back(*q);
    return true;
  }
  bool send(int d, TSDS_SuplaSetSuplanDestinationAssociation *q) override {
    order.push_back(d);
    destinations.push_back({q->PeerContext, q->AclRevision, 0});
    destination_key_sizes.push_back(q->PeerKeySize);
    if (q->PeerKeySize) {
      key_seen = true;
      for (auto b : q->PeerKey) EXPECT_EQ(0xA5, b);
    }
    return true;
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
  PeerProvisioner peers{factory, &transport};
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
    store.a.destination = 2;
    store.a.lifecycle = Lifecycle::Active;
    store.a.acl = {{1, 10, 1}};
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
  destination_result();
  ASSERT_EQ(4095u, PeerProvisionerTestAccess::count(peers));
  store.a = next;
  peers.reconcile(next.id);
  ASSERT_EQ(4097u, transport.sources.size());
  EXPECT_EQ(4096u, PeerProvisionerTestAccess::count(peers));
  source_ok();
  destination_result();
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
  ASSERT_EQ(456u, transport.sources.back().PeerContext.RootEpoch);
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
  EXPECT_EQ(123u, transport.destinations.back().PeerContext.RootEpoch);
  destination_result();
  EXPECT_EQ(Lifecycle::Dormant, store.a.lifecycle);
  add();
  EXPECT_EQ(1u, store.a.generation);
  EXPECT_EQ(1u, store.a.revision);
  EXPECT_EQ(456u, transport.sources.back().PeerContext.RootEpoch);
}
TEST_F(SupLanPeerTest,
       FinalRemovalDuringRootReprovisionCleansCurrentNamespace) {
  installed();
  ASSERT_TRUE(peers.accepted(1, {456, {10, 11}}));
  peers.reconnect(1);
  auto old_source = transport.sources.size() - 1;
  ASSERT_EQ(456u, transport.sources.back().PeerContext.RootEpoch);
  ASSERT_EQ(123u, store.a.root);  // New-root Source ACK has not arrived.
  remove();
  ASSERT_EQ(old_source + 2, transport.sources.size());
  EXPECT_EQ(456u, store.a.root);
  EXPECT_EQ(456u, transport.sources.back().PeerContext.RootEpoch);
  EXPECT_EQ(0u, transport.sources.back().AclEntryCount);
  EXPECT_EQ(456u, transport.destinations.back().PeerContext.RootEpoch);
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
  EXPECT_EQ(456u, transport.sources.back().PeerContext.RootEpoch);
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
  EXPECT_EQ(456u, q.PeerContext.RootEpoch);
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
TEST_F(SupLanPeerTest, CurrentStaleConflictDiagnosedAndStopped) {
  for (auto code : {SUPLA_SUPLAN_RESULT_STALE_REVISION,
                    SUPLA_SUPLAN_RESULT_REVISION_CONFLICT,
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
  r = peers.grants()->ensure_access(2, q);
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_UNSUPPORTED, r.Result);
  EXPECT_EQ(SUPLA_RESOURCE_DELIVERY_SERVER_STREAM, r.DeliveryMode);
  q.DeliveryMode = SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER;
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
  EXPECT_EQ(SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED,
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
