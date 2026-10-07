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
class PeerDao : public Repository {
  supla_mariadb_access_provider dba;
  int user;
  bool transaction = false;
  Association locked;
  bool execute(const std::string &sql);
  bool select(const std::string &sql, std::vector<std::vector<uint64_t>> *rows,
              size_t columns);
  bool read(const std::string &where, Association *a, bool lock);
  std::string scope() const;

 public:
  explicit PeerDao(int user);
  ~PeerDao() override;
  bool begin(int source, int destination, Association *a) override;
  bool begin(uint64_t id, Association *a) override;
  bool load(uint64_t id, Association *a) override;
  bool grants(uint64_t id, std::vector<Grant> *out) override;
  bool put(uint64_t id, const Grant &g) override;
  bool erase_origin(uint64_t id, uint16_t type, uint64_t origin) override;
  bool erase_resource(uint64_t id, uint8_t type, uint32_t resource) override;
  bool erase_grants(uint64_t id) override;
  bool save(const Association &a) override;
  bool erase(uint64_t id) override;
  bool commit() override;
  void rollback() override;
  bool identity(int device, Identity *out) override;
  bool accept_identity(int device, const Identity &identity) override;
  bool owner(uint8_t type, uint32_t resource, int *device) override;
  bool device_exists(int device) override;
  bool owns_acl(int source, int destination, const Acl &acl) override;
  bool for_device(int device, bool include_dormant,
                  std::vector<uint64_t> *out) override;
  bool for_origin(uint16_t type, uint64_t origin,
                  std::vector<uint64_t> *out) override;
  bool for_resource(uint8_t type, uint32_t resource,
                    std::vector<uint64_t> *out) override;
  bool find(int source, int destination, Association *a) override;
};
}  // namespace supla_suplan
#endif  // SUPLA_SUPLAN_PEER_DAO_H_
