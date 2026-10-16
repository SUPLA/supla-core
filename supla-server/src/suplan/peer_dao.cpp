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
  if (!transaction && !execute("START TRANSACTION")) return false;
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
  if (!transaction && !execute("START TRANSACTION")) return false;
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
bool PeerDao::reference(uint32_t channel, uint8_t field, uint32_t *out) {
  std::string path;
  static const char *hvac_keys[] = {
      "mainThermometerChannelId", "auxThermometerChannelId",
      "binarySensorChannelId",    "masterThermostatChannelId",
      "pumpSwitchChannelId",      "heatOrColdSourceSwitchChannelId"};
  if (field >= 1 && field <= 6) {
    path = "$." + std::string(hvac_keys[field - 1]);
  } else if (field >= 32 && field < 42) {
    path = "$.sensors[" + to_string(field - 32) + "].channelId";
  } else if (field >= 64 && field < 84) {
    path = "$.floodSensorChannelIds[" + to_string(field - 64) + "]";
  } else {
    return false;
  }
  std::vector<std::vector<uint64_t>> rows;
  // ENSURE must not coerce strings or fractional JSON into a different ID.
  std::string value = "JSON_VALUE(user_config,'" + path + "')";
  std::string type = "JSON_TYPE(JSON_EXTRACT(user_config,'" + path + "'))";
  if (!select("SELECT CASE WHEN " + type + " IN ('INTEGER','DOUBLE') AND " +
                  value + ">0 AND " + value + "<=2147483647 AND " + value +
                  "=FLOOR(" + value + ") THEN " + value +
                  " ELSE 0 END FROM supla_dev_channel WHERE user_id=" +
                  to_string(user) + " AND id=" + to_string(channel),
              &rows, 1)) {
    return false;
  }
  *out = rows.empty() || rows[0][0] > INT32_MAX ? 0 : rows[0][0];
  return true;
}

bool PeerDao::channel(uint32_t id, ChannelInfo *out) {
  *out = {};
  std::vector<std::vector<uint64_t>> rows;
  if (!select("SELECT c.iodevice_id,c.type,c.func,d.flags,c.channel_number,"
              "CASE WHEN JSON_VALID(c.properties) THEN COALESCE(JSON_LENGTH("
              "JSON_EXTRACT(c.properties,'$.actionTriggerCapabilities')),0) "
              "ELSE 0 END,c.is_virtual OR d.is_virtual "
              "FROM supla_dev_channel c JOIN supla_iodevice d "
              "ON d.id=c.iodevice_id AND d.user_id=c.user_id "
              "WHERE c.user_id=" +
                  to_string(user) + " AND c.id=" + to_string(id),
              &rows, 7))
    return false;
  if (!rows.empty()) {
    const auto &r = rows.front();
    out->device = r[0];
    out->type = r[1];
    out->function = r[2];
    out->device_flags = r[3];
    out->number = r[4];
    out->action_capabilities = r[5];
    out->is_virtual = r[6];
  }
  return true;
}
bool PeerDao::device_exists(int device) {
  int owner_id = 0;
  return owner(SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE, device, &owner_id) &&
         owner_id == device;
}
bool PeerDao::device(int id, DeviceInfo *out) {
  *out = {};
  std::vector<std::vector<uint64_t>> rows;
  if (!select("SELECT flags,protocol_version,is_virtual FROM supla_iodevice "
              "WHERE " +
                  scope() + " AND id=" + to_string(id),
              &rows, 3))
    return false;
  if (!rows.empty()) {
    out->flags = rows.front()[0];
    out->protocol = rows.front()[1];
    out->is_virtual = rows.front()[2];
  }
  return true;
}
bool PeerDao::owns_acl(int source, int destination, const Acl &acl) {
  return Repository::owns_acl(source, destination, acl);
}
bool PeerDao::check_acl_ownership(int source, int destination, const Acl &acl,
                                  bool *owned) {
  *owned = false;
  std::vector<std::vector<uint64_t>> rows;
  if (source == destination) return true;
  if (!select("SELECT COUNT(*) FROM supla_iodevice WHERE user_id=" +
                  to_string(user) + " AND id IN (" + to_string(source) + "," +
                  to_string(destination) + ")",
              &rows, 1) || rows.size() != 1)
    return false;
  if (rows[0][0] != 2) return true;
  std::string channels;
  size_t count = 0;
  for (const auto &entry : acl) {
    if (entry.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_DEVICE) {
      if (entry.ResourceId != static_cast<uint32_t>(source)) return true;
    } else if (entry.ResourceType == SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL) {
      if (count++) channels += ",";
      channels += to_string(entry.ResourceId);
    } else {
      return true;
    }
  }
  if (!count) {
    *owned = true;
    return true;
  }
  if (!select("SELECT COUNT(*) FROM supla_dev_channel WHERE user_id=" +
                    to_string(user) + " AND iodevice_id=" + to_string(source) +
                    " AND id IN (" + channels + ")",
                &rows, 1) || rows.size() != 1)
    return false;
  *owned = rows[0][0] == count;
  return true;
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
bool PeerDao::has_master_dependents(uint32_t channel, bool *out) {
  std::vector<std::vector<uint64_t>> rows;
  if (!select(
          "SELECT COUNT(*) FROM supla_dev_channel WHERE user_id=" +
              to_string(user) +
              " AND type=" + to_string(SUPLA_CHANNELTYPE_HVAC) + " AND id<>" +
              to_string(channel) +
              " AND JSON_VALUE(user_config,'$.masterThermostatChannelId')=" +
              to_string(channel),
          &rows, 1) ||
      rows.size() != 1) {
    return false;
  }
  *out = rows[0][0] != 0;
  return true;
}

// Rare deletion audit: the referenced target is not the READ resource and
// Cloud may already have cleared its config key. Active binding origins remain
// discoverable without relying on an orphan business reference.
bool PeerDao::binding_consumers(std::vector<uint64_t> *out) {
  std::vector<std::vector<uint64_t>> rows;
  bool ok = select(
      "SELECT DISTINCT (g.origin_id >> 8) FROM supla_suplan_grant g "
      "JOIN supla_suplan_peer_association a ON a.id=g.association_id WHERE a." +
          scope() + " AND g.origin_type=" + to_string(CHANNEL_CONFIG_ORIGIN) +
          " AND (g.origin_id & 255) IN (5,6)",
      &rows, 1);
  ids(rows, out);
  return ok;
}

bool PeerDao::config_origin_fields(uint32_t consumer,
                                   std::vector<uint64_t> *out) {
  std::vector<std::vector<uint64_t>> rows;
  bool ok = select(
      "SELECT DISTINCT (g.origin_id & 255) FROM supla_suplan_grant g "
      "JOIN supla_suplan_peer_association a ON a.id=g.association_id WHERE a." +
          scope() + " AND g.origin_type=" + to_string(CHANNEL_CONFIG_ORIGIN) +
          " AND (g.origin_id >> 8)=" + to_string(consumer),
      &rows, 1);
  ids(rows, out);
  return ok;
}

// Business references survive a temporarily invalid Source and revoked Grant.
bool PeerDao::hvac_dependents(uint32_t source, std::vector<uint64_t> *out) {
  std::vector<std::vector<uint64_t>> rows;
  std::string predicates;
  for (uint8_t field = 1; field <= 6; ++field) {
    static const char *keys[] = {
        "mainThermometerChannelId", "auxThermometerChannelId",
        "binarySensorChannelId",    "masterThermostatChannelId",
        "pumpSwitchChannelId",      "heatOrColdSourceSwitchChannelId"};
    if (!predicates.empty()) {
      predicates += " OR ";
    }
    predicates += "JSON_VALUE(user_config,'$." + std::string(keys[field - 1]) +
                  "')=" + to_string(source);
  }
  for (unsigned slot = 0; slot < 20; ++slot) {
    predicates += " OR JSON_VALUE(user_config,'$.floodSensorChannelIds[" +
                  to_string(slot) + "]')=" + to_string(source);
    if (slot < 10) {
      predicates += " OR JSON_VALUE(user_config,'$.sensors[" + to_string(slot) +
                    "].channelId')=" + to_string(source);
    }
  }
  bool ok = select("SELECT id FROM supla_dev_channel WHERE user_id=" +
                       to_string(user) + " AND (" + predicates + ")",
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

bool PeerDao::approval_schema(bool *available) {
  std::vector<std::vector<uint64_t>> rows;
  if (!select("SELECT COUNT(*) FROM information_schema.tables WHERE "
              "table_schema=DATABASE() AND table_name IN "
              "('supla_resource_access_pair','supla_resource_access_decision')",
              &rows, 1) || rows.size() != 1) return false;
  *available = rows[0][0] == 2;
  return true;
}
bool PeerDao::approval_pairs(uint32_t resource, int device,
                              std::vector<ApprovalPair> *out) {
  std::vector<std::vector<uint64_t>> rows;
  std::string filter = resource
                           ? " AND source_channel_id=" + to_string(resource)
                               : " AND (source_device_id=" + to_string(device) +
                                 " OR destination_device_id=" +
                                 to_string(device) + ")";
  if (resource && device)
    filter += " AND destination_device_id=" + to_string(device);
  if (!select("SELECT id,source_device_id,destination_device_id,"
              "source_channel_id "
              "FROM supla_resource_access_pair WHERE " + scope() + filter +
              " ORDER BY id", &rows, 4)) return false;
  out->clear();
  for (const auto &r : rows)
    out->push_back({r[0], static_cast<int>(r[1]), static_cast<int>(r[2]),
                    static_cast<uint32_t>(r[3])});
  return true;
}
bool PeerDao::approval_lock(ApprovalPair *pair, bool create) {
  if (!execute("SET SESSION innodb_lock_wait_timeout=5") ||
      !execute("SET TRANSACTION ISOLATION LEVEL READ COMMITTED") ||
      !execute("START TRANSACTION")) return false;
  transaction = true;
  std::vector<std::vector<uint64_t>> rows;
  const std::string where = scope() + " AND destination_device_id=" +
      to_string(pair->destination) + " AND source_channel_id=" +
      to_string(pair->resource);
  // Plain discovery avoids taking an absent-key gap lock before concurrent
  // unique INSERT. Existing requests issue no no-op UPDATE/write statement.
  if (!select("SELECT id FROM supla_resource_access_pair WHERE " + where,
              &rows, 1)) return false;
  if (rows.empty() && create && !execute(
      "INSERT INTO supla_resource_access_pair (user_id,destination_device_id,"
      "source_channel_id,source_device_id) VALUES (" + to_string(user) + "," +
      to_string(pair->destination) + "," + to_string(pair->resource) + "," +
      to_string(pair->source) + ") ON DUPLICATE KEY UPDATE id=id"))
    return false;
  if (!select("SELECT id,source_device_id,destination_device_id,"
              "source_channel_id FROM supla_resource_access_pair WHERE " +
              where + " FOR UPDATE", &rows, 4) || rows.size() != 1)
    return false;
  const auto &r = rows.front();
  *pair = {r[0], static_cast<int>(r[1]), static_cast<int>(r[2]),
            static_cast<uint32_t>(r[3])};
  return true;
}
bool PeerDao::approval_decisions(uint64_t pair,
                                 std::vector<ApprovalDecision> *out) {
  std::vector<std::vector<uint64_t>> rows;
  bool stream_available = false;
  if (!stream_schema(&stream_available)) return false;
  if (!transaction || !select(
      "SELECT id,permissions,revision,CASE status WHEN 'PENDING' THEN 1 "
      "WHEN 'ACCEPTED' THEN 2 WHEN 'REJECTED' THEN 3 ELSE 4 END," +
      std::string(stream_available ? "requested_delivery_mode " : "1 ") +
      "FROM supla_resource_access_decision WHERE pair_id=" + to_string(pair) +
      " ORDER BY id FOR UPDATE", &rows, 5)) return false;
  out->clear();
  for (const auto &r : rows) {
    if (!valid_permissions(r[1]) || !r[2]) return false;
    out->push_back(
        {r[0], static_cast<uint8_t>(r[1]), static_cast<uint32_t>(r[2]),
         static_cast<ApprovalStatus>(r[3]), static_cast<uint8_t>(r[4])});
  }
  return true;
}
bool PeerDao::approval_pending(uint64_t pair, uint8_t permissions,
                               const ApprovalDecision *pending) {
  if (!transaction || !valid_permissions(permissions)) return false;
  if (pending) {
    if (pending->permissions == permissions) return true;
    bool stream_available = false;
    if (!stream_schema(&stream_available)) return false;
    return execute("UPDATE supla_resource_access_decision SET " +
                   std::string(stream_available ?
                       "requested_delivery_mode=1," : "") + "permissions=" +
                   to_string(permissions) + " WHERE id=" +
                   to_string(pending->id) + " AND pair_id=" + to_string(pair) +
                   " AND status='PENDING' AND revision=" +
                   to_string(pending->revision));
  }
  return execute("INSERT INTO supla_resource_access_decision "
                 "(pair_id,open_pair_id,status,permissions,revision) VALUES (" +
                 to_string(pair) + "," + to_string(pair) + ",'PENDING'," +
                 to_string(permissions) + ",1)");
}
bool PeerDao::approval_context(const ApprovalPair &pair, ChannelInfo *info,
                               DeviceInfo *source, DeviceInfo *destination,
                               Identity *accepted) {
  // Pair -> sorted Devices -> Source Channel -> identity -> association.
  // Match Cloud's order; missing rows are invalid context, not SELECT failure.
  std::vector<std::vector<uint64_t>> rows;
  std::vector<int> devices{pair.source, pair.destination};
  std::sort(devices.begin(), devices.end());
  for (auto id : devices) {
    if (!select("SELECT id FROM supla_iodevice WHERE id=" + to_string(id) +
                " FOR UPDATE", &rows, 1)) return false;
  }
  if (!select("SELECT id FROM supla_dev_channel WHERE id=" +
              to_string(pair.resource) + " FOR UPDATE", &rows, 1) ||
      !select("SELECT device_id FROM supla_suplan_device_state "
              "WHERE device_id=" +
              to_string(pair.source) + " FOR UPDATE", &rows, 1)) return false;
  return channel(pair.resource, info) && device(pair.source, source) &&
         device(pair.destination, destination) &&
         identity(pair.source, accepted);
}

bool PeerDao::stream_schema(bool *available) {
  std::vector<std::vector<uint64_t>> rows;
  if (!select("SELECT COUNT(*) FROM information_schema.columns WHERE "
              "table_schema=DATABASE() AND table_name="
              "'supla_resource_access_decision' AND column_name="
              "'requested_delivery_mode'", &rows, 1) || rows.size() != 1)
    return false;
  *available = rows[0][0] == 1;
  return true;
}
bool PeerDao::stream_permissions(int destination, uint32_t resource,
                                  uint8_t *permissions) {
  *permissions = 0;
  std::vector<std::vector<uint64_t>> rows;
  // Read actual origins, never the materialized ACL/provisioned status.
  if (!select("SELECT COALESCE(BIT_OR(g.permissions),0) "
              "FROM supla_suplan_grant g JOIN supla_suplan_peer_association a "
              "ON a.id=g.association_id JOIN supla_dev_channel c "
              "ON c.iodevice_id=a.source_device_id AND c.user_id=a.user_id "
              "WHERE a.user_id=" + to_string(user) +
              " AND a.destination_device_id=" + to_string(destination) +
              " AND c.id=" + to_string(resource) +
              " AND a.source_removed=0 AND a.destination_removed=0 "
              "AND g.origin_type<>" + to_string(USER_APPROVAL_ORIGIN) +
              " AND ((g.resource_type=1 AND g.resource_id=c.id) OR "
              "(g.resource_type=2 AND g.resource_id=c.iodevice_id))",
              &rows, 1) || rows.size() != 1) return false;
  *permissions = rows[0][0];
  bool available = false;
  if (!approval_schema(&available)) return false;
  if (!available) return true;
  if (!select("SELECT COALESCE(BIT_OR(d.permissions),0) "
              "FROM supla_resource_access_decision d "
              "JOIN supla_resource_access_pair p ON p.id=d.pair_id "
              "JOIN supla_dev_channel c ON c.id=p.source_channel_id "
              "AND c.user_id=p.user_id AND c.iodevice_id=p.source_device_id "
              "WHERE p.user_id=" + to_string(user) +
              " AND p.destination_device_id=" + to_string(destination) +
              " AND p.source_channel_id=" + to_string(resource) +
              " AND d.status='ACCEPTED'", &rows, 1) || rows.size() != 1)
    return false;
  *permissions |= rows[0][0];
  return true;
}
bool PeerDao::approval_stream_pending(uint64_t pair,
                                       const ApprovalDecision *pending) {
  if (pending) return true;  // Existing pair workflow is transport-neutral.
  return transaction &&
         execute(
             "INSERT INTO supla_resource_access_decision "
             "(pair_id,open_pair_id,status,permissions,revision,requested_"
             "delivery_mode)"
             " VALUES (" +
             to_string(pair) + "," + to_string(pair) + ",'PENDING',1,1,2)");
}
bool PeerDao::approval_stream_mode(const ApprovalDecision &decision) {
  return transaction && execute(
      "UPDATE supla_resource_access_decision SET requested_delivery_mode=2 "
      "WHERE id=" + to_string(decision.id) + " AND revision=" +
      to_string(decision.revision) + " AND requested_delivery_mode<>2");
}
bool PeerDao::stream_metadata(uint32_t channel,
                              TDS_SuplaDeviceChannel_E *out) {
  *out = {};
  std::vector<std::vector<uint64_t>> rows;
  if (!select("SELECT type,func,COALESCE(flist,0),COALESCE(flags,0),"
              "COALESCE(alt_icon,0),sub_device_id FROM supla_dev_channel "
              "WHERE user_id=" + to_string(user) + " AND id=" +
              to_string(channel), &rows, 6)) return false;
  if (rows.empty()) return false;
  out->Number = 0xFF;
  out->Type = rows[0][0];
  out->Default = rows[0][1];
  out->FuncList = rows[0][2];
  out->Flags = rows[0][3];
  out->DefaultIcon = rows[0][4];
  out->SubDeviceId = rows[0][5];
  return true;
}

}  // namespace supla_suplan
