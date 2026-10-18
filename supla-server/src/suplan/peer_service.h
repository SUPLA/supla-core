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
constexpr uint16_t USER_APPROVAL_ORIGIN = 2;
// Low-byte field codes are stable; Main=1 remains unchanged from M3B.
inline uint64_t channel_config_origin(uint32_t channel, uint8_t field) {
  return (static_cast<uint64_t>(channel) << 8) | field;
}
struct ChannelInfo {
  int device = 0;
  int type = 0;
  int function = 0;
  uint32_t device_flags = 0;
  unsigned char number = 0;
  uint32_t action_capabilities = 0;
  bool is_virtual = false;
};
enum class Lifecycle : uint8_t { Active = 1, Draining = 2, Dormant = 3 };
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
struct DeviceInfo {
  uint32_t flags = 0;
  unsigned char protocol = 0;
  bool is_virtual = false;
};
enum class ApprovalStatus : uint8_t {
  Pending = 1, Accepted = 2, Rejected = 3, Removed = 4
};
struct ApprovalPair {
  uint64_t id = 0;
  int source = 0;
  int destination = 0;
  uint32_t resource = 0;
};
struct ApprovalDecision {
  uint64_t id = 0;
  uint8_t permissions = 0;
  uint32_t revision = 0;
  ApprovalStatus status = ApprovalStatus::Removed;
  uint8_t delivery_mode = SUPLA_RESOURCE_DELIVERY_SUPLAN_PEER;
};

// Each instance owns one existing MariaDB access provider/transaction. Errors
// must fail closed; an absent row is distinguished from a failed SELECT.
class Repository {
 public:
  virtual ~Repository() = default;
  virtual bool begin(uint64_t id, Association *a) = 0;
  virtual bool load(uint64_t id, Association *a) = 0;
  virtual bool save(const Association &a) = 0;
  virtual bool commit() = 0;
  virtual void rollback() = 0;
  virtual bool identity(int device, Identity *out) = 0;
  virtual bool accept_identity(int device, const Identity &identity) = 0;
  virtual bool owner(uint8_t type, uint32_t resource, int *device) = 0;
  virtual bool channel(uint32_t id, ChannelInfo *out) { return false; }
  virtual bool device(int id, DeviceInfo *out) { return false; }
  // Resource + device selects that Destination only; device alone selects
  // both endpoint roles for reconnect. Resource alone selects all Destinations.
  virtual bool approval_pairs(uint32_t resource, int device,
                              std::vector<ApprovalPair> *out) {
    out->clear();
    return true;
  }
  virtual bool approval_decisions(uint64_t pair,
                                  std::vector<ApprovalDecision> *out) {
    return false;
  }
  virtual bool stream_access(int destination, uint32_t resource,
                             ChannelInfo *channel, uint8_t *permissions,
                             bool *ready) { return false; }
  virtual bool permissions(int destination, uint8_t type, uint32_t resource,
                           bool peer, uint8_t *out) { return false; }
  virtual bool request_pending(int destination, uint32_t resource,
                              uint8_t permissions, uint8_t mode,
                              bool allow = true) {
    return false;
  }
  virtual bool binding_granted(int source, int destination, uint32_t resource,
                               uint32_t target, bool *out) { return false; }
  virtual bool project_share(int destination, uint32_t resource,
                             uint32_t target) { return false; }
  virtual bool project_peer(int destination, uint32_t resource) {
    return false;
  }
  virtual bool cleanup_device(int device) { return false; }
  virtual bool cleanup_resource(uint32_t resource) { return false; }
  virtual bool observe_root(uint64_t id);
  virtual bool changed_pairs(uint64_t after, size_t limit,
      std::vector<std::pair<uint64_t, uint64_t>> *out) { return false; }
  virtual bool ack_changed(uint64_t id, uint64_t revision) { return false; }
  virtual bool stream_permissions(int destination, uint32_t resource,
                                  uint8_t *permissions) { return false; }
  // Local-only protected merge validation, never a permission source.
  virtual bool reference(uint32_t channel, uint8_t field, uint32_t *out) {
    *out = 0;
    return true;
  }
  virtual bool has_master_dependents(uint32_t channel, bool *out) {
    *out = false;
    return true;
  }
  virtual bool device_exists(int device) = 0;
  virtual bool owns_acl(int source, int destination, const Acl &acl);
  // Return query success separately from the authoritative ownership verdict.
  virtual bool check_acl_ownership(int source, int destination, const Acl &acl,
                                   bool *owned);
  virtual bool for_device(int device, bool include_dormant,
                          std::vector<uint64_t> *out) = 0;
  virtual bool find(int source, int destination, Association *a) = 0;
};

bool stream_read_supported(const ChannelInfo &channel);
bool valid_permissions(uint8_t p);
uint8_t normalize(uint8_t p);
// Effective CONTROL covers READ; only still-missing canonical rights can
// enter a later approval workflow. This helper never creates authorization.
uint8_t missing_permissions(uint8_t requested, uint8_t effective);
bool canonicalize(const Acl &entries, Acl *acl);
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
  bool mutate(uint64_t id,
              const std::function<bool(Repository *, Association *)> &fn);

 public:
  PeerService(Factory factory, Changed changed);
  bool cleanup_ack(const Association &sent, bool source);
  bool observe_root(uint64_t id);
  // Persist only after Source OK; no speculative/mass root writes.
  bool provisioned(const Association &sent, uint32_t root);
  TSD_SuplaEnsureResourceAccessResult ensure_access(
      int destination, const TDS_SuplaEnsureResourceAccess &request,
      const std::function<bool()> &reconcile = nullptr);
  TSD_SuplaEnsureResourceAccessResult ensure_stream(
      int destination, uint32_t resource, bool allow_approval,
      const std::function<bool()> &reconcile = nullptr,
      bool record_request = false);
  TSD_SuplaEnsureResourceShareResult ensure_share(
      int source, const TDS_SuplaEnsureResourceShare &request);
};

}  // namespace supla_suplan
#endif  // SUPLA_SUPLAN_PEER_SERVICE_H_
