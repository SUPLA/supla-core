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
class peer_transport {
 public:
  virtual ~peer_transport() = default;
  virtual bool ready(int device) = 0;
  // Offline Source may use its last accepted public identity. An online
  // Source whose new snapshot is pending must block topology-dependent replay.
  virtual bool owner_ready(int source, uint32_t root) = 0;
  enum class send_code { queued, not_ready, context_changed, enqueue_failed };
  struct send_result_t {
    send_code code;
    uint64_t session;
  };
  virtual uint64_t session(int device) = 0;
  virtual int recovery_device() = 0;
  virtual void wake(int device, uint64_t session) = 0;
  virtual bool reconnect_session(int device, uint64_t session) = 0;
  virtual send_result_t send(int device,
                             TSDS_SuplaSetSuplanSourceAssociation *q) = 0;
  virtual send_result_t send(int device,
                             TSDS_SuplaSetSuplanDestinationAssociation *q) = 0;
  virtual void rebootstrap(int source) = 0;
  virtual void diagnostic(uint64_t association, int device, uint8_t result) = 0;
  virtual void transient_diagnostic(uint64_t association, int device,
                                    bool timeout) {}
};
class peer_provisioner {
  friend class PeerProvisionerTestAccess;
  struct side_t {
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
  struct flow_t {
    association_t desired;
    uint32_t wire_root = 0;
    side_t source, destination;
    bool force_key = false;
    bool key_recovery_started = false;
    bool source_no_key_sent = false;
    std::chrono::steady_clock::time_point refresh_at;
    int recovery_owner = 0;
  };
  peer_service::factory_t factory;
  peer_transport *transport;
  peer_service service;
  std::function<std::chrono::steady_clock::time_point()> clock;
  std::mutex flows_mutex;
  std::map<uint64_t, std::shared_ptr<flow_t>> flows;
  std::map<std::pair<int, int>, uint64_t> pairs;
  using time_point_t = std::chrono::steady_clock::time_point;
  // Only outstanding result/DB work is indexed, never healthy associations.
  struct deadline_t {
    time_point_t due;
    uint64_t session;  // Zero for DB-only work, never a Device watchdog.
  };
  std::map<int, std::map<uint64_t, deadline_t>> deadlines;
  struct incident_t {
    uint64_t session;
    std::set<uint64_t> associations;
  };
  std::map<int, incident_t> incidents;
  void resolved(int device, uint64_t id);
  struct recovery_t {
    int owner;
    time_point_t due;
  };
  std::map<int, recovery_t> reconnect_due;
  void defer_reconnect(int device);
  struct schedule_guard {
    peer_provisioner *owner;
    uint64_t id;
    ~schedule_guard() { owner->schedule(id); }
  };
  void schedule(uint64_t id);
  void db_failure(flow_t *flow, bool source);
  void no_response(flow_t *flow, bool source, bool timeout = true);
  void sent(flow_t *flow, bool source, peer_transport::send_result_t result);
  uint64_t correlate(const TSuplaSuplanPeerContext &context);
  enum class current_state { yes, stale, database_error };
  std::shared_ptr<flow_t> flow(uint64_t id, bool create = true);
  void retire_completed(uint64_t id, const std::shared_ptr<flow_t> &flow);
  current_state current(repository *repo, const flow_t &flow, association_t *a);
  void send_source(flow_t *flow, bool key);
  void send_destination(flow_t *flow, const unsigned char *key);
  void result(flow_t *flow, bool source, uint8_t result);
  bool reconcile_locked(uint64_t id, bool meaningful_event);
  void finish_cleanup(const association_t &sent, bool source,
                      const std::shared_ptr<flow_t> &flow);

 public:
  using clock_t = std::function<std::chrono::steady_clock::time_point()>;
  peer_provisioner(peer_service::factory_t factory, peer_transport *transport,
                   clock_t clock = std::chrono::steady_clock::now);
  void iterate(int device = 0);
  uint64_t time_left_usec(int device);
  peer_service *grants() { return &service; }
  bool accepted(int device, const identity_t &identity);
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
