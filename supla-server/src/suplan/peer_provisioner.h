// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SUPLA_SUPLAN_PEER_PROVISIONER_H_
#define SUPLA_SUPLAN_PEER_PROVISIONER_H_
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <utility>

#include "suplan/peer_service.h"
namespace supla_suplan {
class PeerTransport {
 public:
  virtual ~PeerTransport() = default;
  virtual bool ready(int device) = 0;
  // Offline Source may use its last accepted public identity. An online
  // Source whose new snapshot is pending must block topology-dependent replay.
  virtual bool owner_ready(int source, uint32_t root) = 0;
  enum class SendCode { Queued, NotReady, ContextChanged, EnqueueFailed };
  struct SendResult {
    SendCode code;
    uint64_t session;
  };
  virtual uint64_t session(int device) = 0;
  virtual int recovery_device() = 0;
  virtual void wake(int device, uint64_t session) = 0;
  virtual bool reconnect_session(int device, uint64_t session) = 0;
  virtual SendResult send(int device,
                          TSDS_SuplaSetSuplanSourceAssociation *q) = 0;
  virtual SendResult send(int device,
                          TSDS_SuplaSetSuplanDestinationAssociation *q) = 0;
  virtual void rebootstrap(int source) = 0;
  virtual void diagnostic(uint64_t association, int device, uint8_t result) = 0;
  virtual void transient_diagnostic(uint64_t association, int device,
                                    bool timeout) {}
};
class PeerProvisioner {
  friend class PeerProvisionerTestAccess;
  struct Side {
    bool sent = false;
    bool pending = false;
    bool blocked = false;
    bool acknowledged = false;
    bool key_requested = false;
    bool committing = false;
    bool conflict = false;
    uint64_t session = 0;
    std::chrono::steady_clock::time_point deadline;
  };
  struct Flow {
    Association desired;
    uint32_t wire_root = 0;
    Side source, destination;
    bool force_key = false;
    bool key_recovery_started = false;
    bool source_no_key_sent = false;
    std::chrono::steady_clock::time_point refresh_at;
    int recovery_owner = 0;
  };
  PeerService::Factory factory;
  PeerTransport *transport;
  PeerService service;
  std::function<std::chrono::steady_clock::time_point()> clock;
  std::mutex flows_mutex;
  std::map<uint64_t, std::shared_ptr<Flow>> flows;
  std::map<std::pair<int, int>, uint64_t> pairs;
  using Time = std::chrono::steady_clock::time_point;
  // Only outstanding result/DB work is indexed, never healthy associations.
  struct Deadline {
    Time due;
    uint64_t session;  // Zero for DB-only work, never a Device watchdog.
  };
  std::map<int, std::map<uint64_t, Deadline>> deadlines;
  struct Incident {
    uint64_t session;
    std::set<uint64_t> associations;
  };
  std::map<int, Incident> incidents;
  void resolved(int device, uint64_t id);
  struct Recovery {
    int owner;
    Time due;
  };
  std::map<int, Recovery> reconnect_due;
  void defer_reconnect(int device);
  struct Schedule {
    PeerProvisioner *owner;
    uint64_t id;
    ~Schedule() { owner->schedule(id); }
  };
  void schedule(uint64_t id);
  void db_failure(Flow *flow, bool source);
  void no_response(Flow *flow, bool source, bool timeout = true);
  void sent(Flow *flow, bool source, PeerTransport::SendResult result);
  uint64_t correlate(const TSuplaSuplanPeerContext &context);
  enum class Current { Yes, Stale, DatabaseError };
  std::shared_ptr<Flow> flow(uint64_t id, bool create = true);
  void retire_completed(uint64_t id, const std::shared_ptr<Flow> &flow);
  Current current(Repository *repo, const Flow &flow, Association *a);
  void send_source(Flow *flow, bool key);
  void send_destination(Flow *flow, const unsigned char *key);
  void result(Flow *flow, bool source, uint8_t result);
  bool reconcile_locked(uint64_t id, bool meaningful_event);
  void finish_cleanup(const Association &sent, bool source,
                      const std::shared_ptr<Flow> &flow);

 public:
  using Clock = std::function<std::chrono::steady_clock::time_point()>;
  PeerProvisioner(PeerService::Factory factory, PeerTransport *transport,
                  Clock clock = std::chrono::steady_clock::now);
  void iterate(int device = 0);
  uint64_t time_left_usec(int device);
  PeerService *grants() { return &service; }
  bool accepted(int device, const Identity &identity);
  void reconnect(int device);
  bool reconcile(uint64_t id, bool meaningful_event = false);
  void on_source(int device, TDS_SuplaSetSuplanSourceAssociationResult *result,
                 uint64_t session = 0);
  void on_destination(
      int device, const TDS_SuplaSetSuplanDestinationAssociationResult *result,
      uint64_t session = 0);
  void disconnected(int device, uint64_t session = 0);
};
void wipe_key(void *buffer, size_t size);
}  // namespace supla_suplan
#endif  // SUPLA_SUPLAN_PEER_PROVISIONER_H_
