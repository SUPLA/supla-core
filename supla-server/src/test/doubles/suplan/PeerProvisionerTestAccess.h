// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SUPLA_TEST_PEER_PROVISIONER_TEST_ACCESS_H_
#define SUPLA_TEST_PEER_PROVISIONER_TEST_ACCESS_H_
#include "suplan/peer_provisioner.h"
namespace supla_suplan {
class PeerProvisionerTestAccess {
 public:
  static size_t count(peer_provisioner &peers) {
    std::lock_guard<std::mutex> lock(peers.flows_mutex);
    return peers.flows.size();
  }
  static peer_transport *transport(peer_provisioner &peers,
                                   peer_transport *next) {
    peer_transport *previous = peers.transport;
    peers.transport = next;
    return previous;
  }
};
}  // namespace supla_suplan
#endif  // SUPLA_TEST_PEER_PROVISIONER_TEST_ACCESS_H_
