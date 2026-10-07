// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SUPLA_SUPLAN_PEER_PROVISIONER_H_
#define SUPLA_SUPLAN_PEER_PROVISIONER_H_
#include <chrono>
#include <map>
#include <memory>
#include <mutex>

#include "suplan/peer_service.h"
namespace supla_suplan {
class PeerTransport {
 public:
  virtual ~PeerTransport() = default;
  virtual bool ready(int device) = 0;
  // Offline Source may use its last accepted public identity. An online
  // Source whose new snapshot is pending must block topology-dependent replay.
  virtual bool owner_ready(int source, uint32_t root) = 0;
  virtual bool send(int device, TSDS_SuplaSetSuplanSourceAssociation *q) = 0;
  virtual bool send(int device,
                    TSDS_SuplaSetSuplanDestinationAssociation *q) = 0;
  virtual void rebootstrap(int source) = 0;
  virtual void diagnostic(uint64_t association, int device, uint8_t result) = 0;
};
class PeerProvisioner {
  struct Side {
    bool sent = false;
    bool pending = false;
    bool blocked = false;
    bool acknowledged = false;
    bool key_requested = false;
    std::chrono::steady_clock::time_point deadline;
  };
  struct Flow {
    Association desired;
    uint32_t wire_root = 0;
    Side source, destination;
    bool force_key = false;
    bool key_recovery_started = false;
    bool source_no_key_acked = false;
  };
  PeerService::Factory factory;
  PeerTransport *transport;
  PeerService service;
  std::mutex flows_mutex;
  std::map<uint64_t, std::shared_ptr<Flow>> flows;
  std::shared_ptr<Flow> flow(uint64_t id, bool create = true);
  void retire_completed(uint64_t id, const std::shared_ptr<Flow> &flow);
  bool current(Repository *repo, const Flow &flow, Association *a);
  void send_source(Flow *flow, bool key);
  void send_destination(Flow *flow, const unsigned char *key);
  void result(Flow *flow, bool source, uint8_t result);

 public:
  PeerProvisioner(PeerService::Factory factory, PeerTransport *transport);
  PeerService *grants() { return &service; }
  bool accepted(int device, const Identity &identity);
  void reconnect(int device);
  void reconcile(uint64_t id, bool meaningful_event = false);
  void on_source(int device, TDS_SuplaSetSuplanSourceAssociationResult *result);
  void on_destination(
      int device, const TDS_SuplaSetSuplanDestinationAssociationResult *result);
  void disconnected(int device);
  size_t transient_flow_count();
};
void wipe_key(void *buffer, size_t size);
}  // namespace supla_suplan
#endif  // SUPLA_SUPLAN_PEER_PROVISIONER_H_
