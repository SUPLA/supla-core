// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SUPLA_SUPLAN_PEER_DAO_H_
#define SUPLA_SUPLAN_PEER_DAO_H_
#include <memory>
#include <string>
#include <vector>

#include "db/mariadb_access_provider.h"
#include "suplan/peer_service.h"
namespace supla_suplan {
class peer_dao : public repository {
  supla_mariadb_access_provider dba;
  int user;
  bool transaction = false;
  association_t locked;
  bool execute(const std::string &sql);
  bool select(const std::string &sql, std::vector<std::vector<uint64_t>> *rows,
              size_t columns);
  bool read(const std::string &where, association_t *a, bool lock);
  std::string scope() const;

 public:
  explicit peer_dao(int user);
  ~peer_dao() override;
  bool begin(uint64_t id, association_t *a) override;
  bool load(uint64_t id, association_t *a) override;
  bool save(const association_t &a) override;
  bool commit() override;
  void rollback() override;
  bool identity(int device, identity_t *out) override;
  bool accept_identity(int device, const identity_t &identity) override;
  bool owner(uint8_t type, uint32_t resource, int *device) override;
  bool reference(uint32_t channel, uint8_t field, uint32_t *out) override;
  bool channel(uint32_t id, channel_info_t *out) override;
  bool device(int id, device_info_t *out) override;
  bool approval_pairs(uint32_t resource, int device,
                      std::vector<approval_pair_t> *out) override;
  bool approval_decisions(uint64_t pair,
                          std::vector<approval_decision_t> *out) override;
  bool stream_permissions(int destination, uint32_t resource,
                          uint8_t *permissions) override;
  bool binding_granted(int source, int destination, uint32_t resource,
                       uint32_t target, bool *out) override;
  bool stream_access(int destination, uint32_t resource,
                     channel_info_t *channel, uint8_t *permissions,
                     bool *ready) override;
  bool permissions(int destination, uint8_t type, uint32_t resource,
                    bool peer, uint8_t *out) override;
  bool request_pending(int destination, uint32_t resource,
                       uint8_t permissions, uint8_t mode,
                       bool allow = true) override;
  bool project_share(int destination, uint32_t resource,
                     uint32_t target) override;
  bool project_peer(int destination, uint32_t resource) override;
  bool cleanup_device(int device) override;
  bool cleanup_resource(uint32_t resource) override;
  bool observe_root(uint64_t id) override;
  bool changed_pairs(uint64_t after, size_t limit,
                     std::vector<std::pair<uint64_t, uint64_t>> *out) override;
  bool ack_changed(uint64_t id, uint64_t revision) override;
  bool stream_metadata(uint32_t channel, TDS_SuplaDeviceChannel_E *out);
  bool has_master_dependents(uint32_t channel, bool *out) override;
  bool device_exists(int device) override;
  bool owns_acl(int source, int destination, const acl_t &acl) override;
  bool check_acl_ownership(int source, int destination, const acl_t &acl,
                           bool *owned) override;
  bool for_device(int device, bool include_dormant,
                  std::vector<uint64_t> *out) override;
  bool find(int source, int destination, association_t *a) override;
};
}  // namespace supla_suplan
#endif  // SUPLA_SUPLAN_PEER_DAO_H_
