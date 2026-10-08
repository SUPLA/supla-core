// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SUPLA_SUPLAN_SERVER_PEER_TRANSPORT_H_
#define SUPLA_SUPLAN_SERVER_PEER_TRANSPORT_H_
#include <memory>

#include "suplan/peer_provisioner.h"
class supla_user;
class supla_json_config;
class supla_suplan_server_peers : public supla_suplan::PeerTransport {
  supla_user *user;
  supla_suplan::PeerProvisioner provisioner;

 public:
  // Durable delete handling without constructing a runtime user or transport.
  static bool reconcile_dependencies(int user_id, int source_channel_id);
  static bool reconcile_hvac(
      int user_id, int channel_id,
      std::unique_ptr<supla_json_config> *current = nullptr);
  static bool channel_deleted(int user_id, int channel_id);
  static bool device_deleted(int user_id, int device_id);
  explicit supla_suplan_server_peers(supla_user *user);
  supla_suplan::PeerProvisioner *peers() { return &provisioner; }
  bool ready(int device) override;
  bool owner_ready(int source, uint32_t root) override;
  bool send(int device, TSDS_SuplaSetSuplanSourceAssociation *q) override;
  bool send(int device, TSDS_SuplaSetSuplanDestinationAssociation *q) override;
  void rebootstrap(int source) override;
  void diagnostic(uint64_t association, int device, uint8_t result) override;
};
#endif  // SUPLA_SUPLAN_SERVER_PEER_TRANSPORT_H_
