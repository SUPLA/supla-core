// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include "suplan/peer_dao.h"

#include <algorithm>
#include <string>
#include <vector>
namespace supla_suplan {
using std::to_string;
static std::string hex(const std::vector<unsigned char> &data) {
  static const char digits[] = "0123456789ABCDEF";
  std::string out = "X'";
  for (auto b : data) {
    out += digits[b >> 4];
    out += digits[b & 15];
  }
  return out + "'";
}
PeerDao::PeerDao(int user) : user(user) {}
PeerDao::~PeerDao() { rollback(); }
std::string PeerDao::scope() const { return "user_id=" + to_string(user); }
bool PeerDao::execute(const std::string &sql) {
  return (dba.is_connected() || dba.connect()) && !dba.query(sql.c_str(), true);
}
bool PeerDao::select(const std::string &sql,
                     std::vector<std::vector<uint64_t>> *rows, size_t columns) {
  rows->clear();
  if (!dba.is_connected() && !dba.connect()) return false;
  MYSQL_STMT *stmt = nullptr;
  if (!dba.stmt_execute(reinterpret_cast<void **>(&stmt), sql.c_str(), nullptr,
                        0, true))
    return false;
  std::vector<uint64_t> values(columns);
  std::vector<MYSQL_BIND> binds(columns);
  for (size_t i = 0; i < columns; ++i) {
    binds[i] = {};
    binds[i].buffer_type = MYSQL_TYPE_LONGLONG;
    binds[i].buffer = &values[i];
    binds[i].is_unsigned = true;
  }
  bool ok = !mysql_stmt_bind_result(stmt, binds.data());
  int status = MYSQL_NO_DATA;
  while (ok && (status = mysql_stmt_fetch(stmt)) == 0) rows->push_back(values);
  ok = ok && status == MYSQL_NO_DATA;
  mysql_stmt_close(stmt);
  return ok;
}
bool PeerDao::read(const std::string &where, Association *a, bool lock) {
  *a = {};
  if (!dba.is_connected() && !dba.connect()) return false;
  const std::string sql =
      "SELECT "
      "id,source_device_id,destination_device_id,lifecycle,peer_generation,"
      "acl_revision,provisioned_root_epoch,provisioned_peer_generation,"
      "source_empty_acked,destination_empty_acked,source_removed,destination_"
      "removed,"
      "canonical_acl FROM supla_suplan_peer_association WHERE " +
      scope() + " AND " + where + (lock ? " FOR UPDATE" : "");
  MYSQL_STMT *stmt = nullptr;
  if (!dba.stmt_execute(reinterpret_cast<void **>(&stmt), sql.c_str(), nullptr,
                        0, true))
    return false;
  uint64_t values[12] = {};
  unsigned char blob[SUPLA_SUPLAN_MAX_ACL_ENTRIES * 6] = {};
  unsigned long size = 0;
  MYSQL_BIND binds[13] = {};
  for (size_t i = 0; i < 12; ++i) {
    binds[i].buffer_type = MYSQL_TYPE_LONGLONG;
    binds[i].buffer = &values[i];
    binds[i].is_unsigned = true;
  }
  binds[12].buffer_type = MYSQL_TYPE_BLOB;
  binds[12].buffer = blob;
  binds[12].buffer_length = sizeof(blob);
  binds[12].length = &size;
  bool ok = !mysql_stmt_bind_result(stmt, binds);
  int status = ok ? mysql_stmt_fetch(stmt) : 1;
  if (status == 0) {
    a->id = values[0];
    a->source = values[1];
    a->destination = values[2];
    a->lifecycle = static_cast<Lifecycle>(values[3]);
    a->generation = values[4];
    a->revision = values[5];
    a->root = values[6];
    a->provisioned_generation = values[7];
    a->source_acked = values[8];
    a->destination_acked = values[9];
    a->source_removed = values[10];
    a->destination_removed = values[11];
    ok = size <= sizeof(blob) && decode_acl(blob, size, &a->acl) &&
         a->generation && a->revision && values[3] >= 1 && values[3] <= 3;
  } else {
    ok = status == MYSQL_NO_DATA;
  }
  mysql_stmt_close(stmt);
  if (ok && lock && a->id) locked = *a;
  return ok;
}
bool PeerDao::begin(int source, int destination, Association *a) {
  if (!execute("START TRANSACTION")) return false;
  transaction = true;
  // Unique ordered pair serializes concurrent first creation as well.
  return execute(
             "INSERT INTO supla_suplan_peer_association (user_id,"
             "source_device_id,destination_device_id,canonical_acl) VALUES (" +
             to_string(user) + "," + to_string(source) + "," +
             to_string(destination) + ",X'') ON DUPLICATE KEY UPDATE id=id") &&
         read("source_device_id=" + to_string(source) +
                  " AND destination_device_id=" + to_string(destination),
              a, true) &&
         a->id;
}
bool PeerDao::begin(uint64_t id, Association *a) {
  if (!execute("START TRANSACTION")) return false;
  transaction = true;
  return read("id=" + to_string(id), a, true) && a->id;
}
bool PeerDao::load(uint64_t id, Association *a) {
  return read("id=" + to_string(id), a, false);
}
bool PeerDao::find(int source, int destination, Association *a) {
  return read("source_device_id=" + to_string(source) +
                  " AND destination_device_id=" + to_string(destination),
              a, false);
}
bool PeerDao::grants(uint64_t id, std::vector<Grant> *out) {
  std::vector<std::vector<uint64_t>> rows;
  if (!select(
          "SELECT resource_type,resource_id,origin_type,origin_id,permissions "
          "FROM supla_suplan_grant WHERE association_id=" +
              to_string(id),
          &rows, 5))
    return false;
  out->clear();
  for (const auto &r : rows)
    out->push_back({static_cast<uint8_t>(r[0]), static_cast<uint32_t>(r[1]),
                    static_cast<uint16_t>(r[2]), r[3],
                    static_cast<uint8_t>(r[4])});
  return true;
}
bool PeerDao::put(uint64_t id, const Grant &g) {
  return execute(
      "INSERT INTO supla_suplan_grant (association_id,resource_type,"
      "resource_id,origin_type,origin_id,permissions) VALUES (" +
      to_string(id) + "," + to_string(g.resource_type) + "," +
      to_string(g.resource_id) + "," + to_string(g.origin_type) + "," +
      to_string(g.origin_id) + "," + to_string(g.permissions) +
      ") ON DUPLICATE KEY UPDATE permissions=VALUES(permissions)");
}
bool PeerDao::erase_origin(uint64_t id, uint16_t type, uint64_t origin) {
  return execute("DELETE FROM supla_suplan_grant WHERE association_id=" +
                 to_string(id) + " AND origin_type=" + to_string(type) +
                 " AND origin_id=" + to_string(origin));
}
bool PeerDao::erase_resource(uint64_t id, uint8_t type, uint32_t resource) {
  return execute("DELETE FROM supla_suplan_grant WHERE association_id=" +
                 to_string(id) + " AND resource_type=" + to_string(type) +
                 " AND resource_id=" + to_string(resource));
}
bool PeerDao::erase_grants(uint64_t id) {
  return execute("DELETE FROM supla_suplan_grant WHERE association_id=" +
                 to_string(id));
}
bool PeerDao::save(const Association &a) {
  if (!transaction || locked.id != a.id) return false;
  std::string changes;
  auto add = [&](const char *column, uint64_t value, uint64_t previous) {
    if (value != previous) {
      if (!changes.empty()) changes += ",";
      changes += std::string(column) + "=" + to_string(value);
    }
  };
  add("lifecycle", static_cast<uint8_t>(a.lifecycle),
      static_cast<uint8_t>(locked.lifecycle));
  add("peer_generation", a.generation, locked.generation);
  add("acl_revision", a.revision, locked.revision);
  add("provisioned_root_epoch", a.root, locked.root);
  add("provisioned_peer_generation", a.provisioned_generation,
      locked.provisioned_generation);
  add("source_empty_acked", a.source_acked, locked.source_acked);
  add("destination_empty_acked", a.destination_acked, locked.destination_acked);
  add("source_removed", a.source_removed, locked.source_removed);
  add("destination_removed", a.destination_removed, locked.destination_removed);
  if (!equal_acl(a.acl, locked.acl)) {
    if (!changes.empty()) changes += ",";
    changes += "canonical_acl=" + hex(encode_acl(a.acl));
  }
  if (changes.empty()) return true;
  if (!execute("UPDATE supla_suplan_peer_association SET " + changes +
               " WHERE " + scope() + " AND id=" + to_string(a.id)))
    return false;
  locked = a;
  return true;
}
bool PeerDao::erase(uint64_t id) {
  return erase_grants(id) &&
         execute("DELETE FROM supla_suplan_peer_association WHERE " + scope() +
                 " AND id=" + to_string(id));
}
bool PeerDao::commit() {
  if (!transaction || !execute("COMMIT")) return false;
  transaction = false;
  return true;
}
void PeerDao::rollback() {
  if (transaction) {
    execute("ROLLBACK");
    transaction = false;
  }
}
bool PeerDao::identity(int device, Identity *out) {
  *out = {};
  if (!dba.is_connected() && !dba.connect()) return false;
  MYSQL_STMT *stmt = nullptr;
  std::string sql =
      "SELECT current_root_epoch,accepted_channels FROM "
      "supla_suplan_device_state WHERE " +
      scope() + " AND device_id=" + to_string(device);
  if (!dba.stmt_execute(reinterpret_cast<void **>(&stmt), sql.c_str(), nullptr,
                        0, true))
    return false;
  unsigned char data[SUPLA_CHANNELMAXCOUNT * 4] = {};
  unsigned long size = 0;
  MYSQL_BIND binds[2] = {};
  binds[0].buffer_type = MYSQL_TYPE_LONG;
  binds[0].buffer = &out->root;
  binds[0].is_unsigned = true;
  binds[1].buffer_type = MYSQL_TYPE_BLOB;
  binds[1].buffer = data;
  binds[1].buffer_length = sizeof(data);
  binds[1].length = &size;
  bool ok = !mysql_stmt_bind_result(stmt, binds);
  int result = ok ? mysql_stmt_fetch(stmt) : 1;
  if (result == MYSQL_NO_DATA) {
    mysql_stmt_close(stmt);
    return true;
  }
  ok = result == 0 && size <= sizeof(data) && size % 4 == 0 && out->root &&
       out->root != UINT32_MAX;
  if (ok) {
    for (size_t i = 0; i < size; i += 4) {
      uint32_t id = 0;
      for (size_t j = 0; j < 4; ++j) id = (id << 8) | data[i + j];
      if (!id || id > INT32_MAX ||
          (!out->channels.empty() && id <= out->channels.back())) {
        ok = false;
        break;
      }
      out->channels.push_back(id);
    }
  }
  mysql_stmt_close(stmt);
  return ok;
}
bool PeerDao::accept_identity(int device, const Identity &identity) {
  Identity previous;
  if (!identity.root || identity.root == UINT32_MAX ||
      identity.channels.size() > SUPLA_CHANNELMAXCOUNT ||
      !this->identity(device, &previous))
    return false;
  uint32_t last = 0;
  for (auto id : identity.channels) {
    if (!id || id > INT32_MAX || id <= last) return false;
    last = id;
  }
  if (previous.root == identity.root && previous.channels == identity.channels)
    return true;
  std::vector<unsigned char> blob;
  for (auto id : identity.channels)
    for (int shift = 24; shift >= 0; shift -= 8) blob.push_back(id >> shift);
  return execute(
      "INSERT INTO supla_suplan_device_state (device_id,user_id,"
      "current_root_epoch,accepted_channels) VALUES (" +
      to_string(device) + "," + to_string(user) + "," +
      to_string(identity.root) + "," + hex(blob) +
      ") ON DUPLICATE KEY UPDATE user_id=VALUES(user_id),"
      "current_root_epoch=VALUES(current_root_epoch),accepted_"
      "channels=VALUES(accepted_channels)");
}
bool PeerDao::owner(uint8_t type, uint32_t resource, int *device) {
  std::vector<std::vector<uint64_t>> rows;
  std::string sql;
  if (!resource || resource > INT32_MAX) return false;
  if (type == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL)
    sql = "SELECT iodevice_id FROM supla_dev_channel WHERE user_id=" +
          to_string(user);
  else if (type == SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE)
    sql = "SELECT id FROM supla_iodevice WHERE user_id=" + to_string(user);
  else
    return false;
  if (!select(sql + " AND id=" + to_string(resource), &rows, 1)) return false;
  *device = rows.empty() ? 0 : rows[0][0];
  return true;
}
bool PeerDao::device_exists(int device) {
  int owner_id = 0;
  return owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, device, &owner_id) &&
         owner_id == device;
}
bool PeerDao::owns_acl(int source, int destination, const Acl &acl) {
  std::vector<std::vector<uint64_t>> rows;
  if (source == destination ||
      !select("SELECT COUNT(*) FROM supla_iodevice WHERE user_id=" +
                  to_string(user) + " AND id IN (" + to_string(source) + "," +
                  to_string(destination) + ")",
              &rows, 1) ||
      rows.size() != 1 || rows[0][0] != 2)
    return false;
  std::string channels;
  size_t count = 0;
  for (const auto &entry : acl) {
    if (entry.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE) {
      if (entry.ResourceId != static_cast<uint32_t>(source)) return false;
    } else if (entry.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL) {
      if (count++) channels += ",";
      channels += to_string(entry.ResourceId);
    } else {
      return false;
    }
  }
  if (!count) return true;
  return select("SELECT COUNT(*) FROM supla_dev_channel WHERE user_id=" +
                    to_string(user) + " AND iodevice_id=" + to_string(source) +
                    " AND id IN (" + channels + ")",
                &rows, 1) &&
         rows.size() == 1 && rows[0][0] == count;
}
static void ids(const std::vector<std::vector<uint64_t>> &rows,
                std::vector<uint64_t> *out) {
  out->clear();
  for (const auto &r : rows) out->push_back(r[0]);
}
bool PeerDao::for_device(int device, bool include_dormant,
                         std::vector<uint64_t> *out) {
  std::vector<std::vector<uint64_t>> rows;
  std::string filter = include_dormant ? "" : " AND lifecycle IN (1,2)";
  std::string base =
      "SELECT id FROM supla_suplan_peer_association WHERE " + scope();
  bool ok = select(
      base + " AND source_device_id=" + to_string(device) + filter + " UNION " +
          base + " AND destination_device_id=" + to_string(device) + filter,
      &rows, 1);
  ids(rows, out);
  return ok;
}
bool PeerDao::for_origin(uint16_t type, uint64_t origin,
                         std::vector<uint64_t> *out) {
  std::vector<std::vector<uint64_t>> rows;
  bool ok = select(
      "SELECT DISTINCT g.association_id FROM supla_suplan_grant g "
      "JOIN supla_suplan_peer_association a ON a.id=g.association_id WHERE a." +
          scope() + " AND g.origin_type=" + to_string(type) +
          " AND g.origin_id=" + to_string(origin),
      &rows, 1);
  ids(rows, out);
  return ok;
}
bool PeerDao::for_resource(uint8_t type, uint32_t resource,
                           std::vector<uint64_t> *out) {
  std::vector<std::vector<uint64_t>> rows;
  bool ok = select(
      "SELECT DISTINCT g.association_id FROM supla_suplan_grant g "
      "JOIN supla_suplan_peer_association a ON a.id=g.association_id WHERE a." +
          scope() + " AND g.resource_type=" + to_string(type) +
          " AND g.resource_id=" + to_string(resource),
      &rows, 1);
  ids(rows, out);
  return ok;
}
}  // namespace supla_suplan
