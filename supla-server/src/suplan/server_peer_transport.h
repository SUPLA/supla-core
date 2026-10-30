// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SUPLA_SUPLAN_SERVER_PEER_TRANSPORT_H_
#define SUPLA_SUPLAN_SERVER_PEER_TRANSPORT_H_
#include <memory>

#include "suplan/peer_provisioner.h"
#include "suplan/server_stream.h"
#include <mutex>
#include <vector>
class supla_user;
class supla_device;
class supla_json_config;
class supla_suplan_server_peers : public supla_suplan::peer_transport {
  supla_user *user;
  std::mutex stream_mutex;
  std::vector<std::weak_ptr<supla_suplan::stream_connection>> streams;
  std::vector<std::shared_ptr<supla_suplan::stream_connection>> live_streams();
  std::mutex changes_mutex;
  supla_suplan::peer_provisioner provisioner;

 public:
  std::shared_ptr<supla_suplan::stream_connection> new_stream(supla_device *d);
  void stream_dirty(uint32_t channel);
  // Authorization-relevant change: every interest revalidates once.
  void stream_access_changed();
  bool drain_changes();
  static bool channel_deleted(int user_id, int channel_id);
  static bool device_deleted(int user_id, int device_id);
  explicit supla_suplan_server_peers(supla_user *user);
  ~supla_suplan_server_peers();
  supla_suplan::peer_provisioner *peers() { return &provisioner; }
  bool ready(int device) override;
  uint64_t session(int device) override;
  int recovery_device() override;
  void wake(int device, uint64_t session) override;
  bool reconnect_session(int device, uint64_t session) override;
  bool owner_ready(int source, uint32_t root) override;
  send_result_t send(int device,
                     TSDS_SuplaSetSuplanSourceAssociation *q) override;
  send_result_t send(int device,
                     TSDS_SuplaSetSuplanDestinationAssociation *q) override;
  void rebootstrap(int source) override;
  void diagnostic(uint64_t association, int device, uint8_t result) override;
  void transient_diagnostic(uint64_t association, int device,
                            bool timeout) override;
};
#endif  // SUPLA_SUPLAN_SERVER_PEER_TRANSPORT_H_
