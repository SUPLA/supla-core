// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SUPLA_SUPLAN_PEER_SERVICE_H_
#define SUPLA_SUPLAN_PEER_SERVICE_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "proto.h"

namespace supla_suplan {

// Internal business origin namespace; never a wire discriminator.
constexpr uint16_t CHANNEL_CONFIG_ORIGIN = 1;
inline uint64_t main_thermometer_origin(uint32_t channel) {
  return (static_cast<uint64_t>(channel) << 8) | 1;
}
struct ChannelInfo {
  int device = 0;
  int type = 0;
  int function = 0;
  uint32_t device_flags = 0;
  unsigned char number = 0;
};
enum class Lifecycle : uint8_t { Active = 1, Draining = 2, Dormant = 3 };
struct Grant {
  uint8_t resource_type;
  uint32_t resource_id;
  uint16_t origin_type;
  uint64_t origin_id;
  uint8_t permissions;
};
using Acl = std::vector<TSuplaSuplanAclEntry>;
struct Association {
  uint64_t id = 0;
  int source = 0;
  int destination = 0;
  Lifecycle lifecycle = Lifecycle::Dormant;
  uint32_t generation = 1;
  uint32_t revision = 1;
  uint32_t root = 0;  // Provisioned context, never the current Device root.
  uint32_t provisioned_generation = 0;
  bool source_acked = false;
  bool destination_acked = false;
  bool source_removed = false;
  bool destination_removed = false;
  Acl acl;
};
struct Identity {
  uint32_t root = 0;
  std::vector<uint32_t> channels;
};

// Each instance owns one existing MariaDB access provider/transaction. Errors
// must fail closed; an absent row is distinguished from a failed SELECT.
class Repository {
 public:
  virtual ~Repository() = default;
  virtual bool begin(int source, int destination, Association *a) = 0;
  virtual bool begin(uint64_t id, Association *a) = 0;
  virtual bool load(uint64_t id, Association *a) = 0;
  virtual bool grants(uint64_t id, std::vector<Grant> *out) = 0;
  virtual bool put(uint64_t id, const Grant &g) = 0;
  virtual bool erase_origin(uint64_t id, uint16_t type, uint64_t origin) = 0;
  virtual bool erase_resource(uint64_t id, uint8_t type, uint32_t resource) = 0;
  virtual bool erase_grants(uint64_t id) = 0;
  virtual bool save(const Association &a) = 0;
  virtual bool erase(uint64_t id) = 0;
  virtual bool commit() = 0;
  virtual void rollback() = 0;
  virtual bool identity(int device, Identity *out) = 0;
  virtual bool accept_identity(int device, const Identity &identity) = 0;
  virtual bool owner(uint8_t type, uint32_t resource, int *device) = 0;
  virtual bool channel(uint32_t id, ChannelInfo *out) { return false; }
  virtual bool supports_batch() const { return false; }
  virtual bool device_exists(int device) = 0;
  virtual bool owns_acl(int source, int destination, const Acl &acl);
  virtual bool for_device(int device, bool include_dormant,
                          std::vector<uint64_t> *out) = 0;
  virtual bool for_origin(uint16_t type, uint64_t origin,
                          std::vector<uint64_t> *out) = 0;
  virtual bool for_resource(uint8_t type, uint32_t resource,
                            std::vector<uint64_t> *out) = 0;
  virtual bool find(int source, int destination, Association *a) = 0;
};

bool valid_permissions(uint8_t p);
uint8_t normalize(uint8_t p);
bool canonicalize(const std::vector<Grant> &grants, Acl *acl);
bool equal_acl(const Acl &a, const Acl &b);
bool same_state(const Association &a, const Association &b);
TSuplaSuplanPeerContext context(const Association &a, uint32_t root);
bool matches(const TSuplaSuplanPeerContext &p, const Association &a);
std::vector<unsigned char> encode_acl(const Acl &acl);
bool decode_acl(const unsigned char *data, size_t size, Acl *acl);

// One short-lived lock per association (not a global work lock). Runtime sends
// and mutations use the same gate; transactions end before transport enqueue.
std::shared_ptr<std::mutex> association_gate(uint64_t id);

class PeerService {
 public:
  using Factory = std::function<std::unique_ptr<Repository>()>;
  using Changed = std::function<void(uint64_t)>;

 private:
  Factory factory;
  Changed changed;
  bool reconcile_origin_locked(uint16_t type, uint64_t origin, int source,
                               int destination, const Grant *desired,
                               std::unique_lock<std::mutex> &origin_lock);
  bool recompute(Repository *repo, Association *a, uint32_t current_root);
  bool mutate(uint64_t id,
              const std::function<bool(Repository *, Association *)> &fn);

 public:
  PeerService(Factory factory, Changed changed);
  bool upsert(int source, int destination, const Grant &grant);
  // Replace one origin atomically, then notify M2 after the shared commit.
  bool reconcile_origin(uint16_t type, uint64_t origin, int source,
                        int destination, const Grant *desired);
  bool reconcile_main_thermometer(uint32_t destination_channel,
                                  uint32_t source_channel);
  bool reconcile_main_thermometer(
      uint32_t destination_channel,
      const std::function<bool(uint32_t *)> &read_current_source);
  bool delete_origin(uint16_t type, uint64_t origin);
  bool delete_resource(uint8_t type, uint32_t resource);
  bool delete_resource(uint64_t association, uint8_t type, uint32_t resource);
  bool remove_device(int device);
  bool distrust(uint64_t id);
  bool cleanup_ack(const Association &sent, bool source);
  bool observe_root(uint64_t id);
  // Persist only after Source OK; no speculative/mass root writes.
  bool provisioned(const Association &sent, uint32_t root);
  TSD_SuplaEnsureResourceAccessResult ensure_access(
      int destination, const TDS_SuplaEnsureResourceAccess &request);
  TSD_SuplaEnsureResourceShareResult ensure_share(
      int source, const TDS_SuplaEnsureResourceShare &request);
};

}  // namespace supla_suplan
#endif  // SUPLA_SUPLAN_PEER_SERVICE_H_
