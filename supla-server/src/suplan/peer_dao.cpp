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
  if (!dba.is_connected() && !dba.connect()) return false;
  if (sql.compare(0, 5, "CALL ") != 0) return !dba.query(sql.c_str(), true);
  MYSQL_STMT *statement = nullptr;
  if (!dba.stmt_execute(reinterpret_cast<void **>(&statement), sql.c_str(),
                        nullptr, 0, true)) return false;
  int status;
  do {
    mysql_stmt_free_result(statement);
    status = mysql_stmt_next_result(statement);
  } while (status == 0);
  mysql_stmt_close(statement);
  return status == -1;
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
bool PeerDao::begin(uint64_t id, Association *a) {
  if (!transaction && !execute("START TRANSACTION")) return false;
  transaction = true;
  std::vector<std::vector<uint64_t>> rows;
  return select("SELECT id FROM supla_user WHERE id=" + to_string(user) +
                " FOR UPDATE", &rows, 1) && rows.size() == 1 &&
         read("id=" + to_string(id), a, true) && a->id;
}
bool PeerDao::load(uint64_t id, Association *a) {
  return read("id=" + to_string(id), a, false);
}
bool PeerDao::find(int source, int destination, Association *a) {
  return read("source_device_id=" + to_string(source) +
                  " AND destination_device_id=" + to_string(destination),
              a, false);
}
bool PeerDao::save(const Association &a) {
  if (!transaction || locked.id != a.id ||
      a.source != locked.source || a.destination != locked.destination ||
      a.generation != locked.generation || a.revision != locked.revision ||
      !equal_acl(a.acl, locked.acl) ||
      a.source_removed != locked.source_removed ||
      a.destination_removed != locked.destination_removed) return false;
  auto args = to_string(user) + "," + to_string(a.id) + ",";
  if (a.root != locked.root ||
      a.provisioned_generation != locked.provisioned_generation) {
    // Invoked only after a matching Source OK, inside the existing transaction.
    if (!execute("CALL supla_suplan_peer_ack(" + args + to_string(a.root) +
                 "," + to_string(a.generation) + "," +
                 to_string(a.revision) + ",1,0)")) return false;
  }
  for (bool source : {true, false}) {
    if ((source ? a.source_acked && !locked.source_acked
                : a.destination_acked && !locked.destination_acked) &&
        !execute("CALL supla_suplan_peer_ack(" + args + to_string(a.root) +
                 "," + to_string(a.generation) + "," + to_string(a.revision) +
                 (source ? ",1,1)" : ",2,1)"))) return false;
  }
  Association current;
  if (!load(a.id, &current)) return false;
  // The ACK procedure deletes only terminal cleanup for removed endpoints.
  if (!current.id && a.lifecycle == Lifecycle::Dormant &&
      a.source_acked && a.destination_acked &&
      (a.source_removed || a.destination_removed)) {
    locked = {};
    return true;
  }
  if (!same_state(a, current) ||
      a.source_acked != current.source_acked ||
      a.destination_acked != current.destination_acked) return false;
  locked = current;
  return true;
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
  if (!transaction && !execute("START TRANSACTION")) return false;
  transaction = true;
  return execute("CALL supla_suplan_accept_identity(" + to_string(user) +
                 "," + to_string(device) + "," + to_string(identity.root) +
                 "," + hex(blob) + ")") && commit();
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
bool PeerDao::approval_decisions(uint64_t pair,
                                 std::vector<ApprovalDecision> *out) {
  std::vector<std::vector<uint64_t>> rows;
  if (!select(
      "SELECT id,permissions,revision,CASE status WHEN 'PENDING' THEN 1 "
      "WHEN 'ACCEPTED' THEN 2 WHEN 'REJECTED' THEN 3 ELSE 4 END," +
      std::string("requested_delivery_mode ") +
      "FROM supla_resource_access_decision WHERE pair_id=" + to_string(pair) +
      " ORDER BY id", &rows, 5)) return false;
  out->clear();
  for (const auto &r : rows) {
    if (!valid_permissions(r[1]) || !r[2]) return false;
    out->push_back(
        {r[0], static_cast<uint8_t>(r[1]), static_cast<uint32_t>(r[2]),
         static_cast<ApprovalStatus>(r[3]), static_cast<uint8_t>(r[4])});
  }
  return true;
}
bool PeerDao::binding_granted(int source, int destination, uint32_t resource,
                               uint32_t target, bool *out) {
  *out = false;
  std::vector<std::vector<uint64_t>> rows;
  if (!select("SELECT COUNT(*) FROM supla_resource_access_grant_valid WHERE " +
              scope() + " AND source_device_id=" + to_string(source) +
              " AND destination_device_id=" + to_string(destination) +
              " AND resource_type=1 AND resource_id=" + to_string(resource) +
              " AND origin_type=1 AND (origin_id & 255) IN (5,6) "
              "AND peer_eligible=1 AND requires_peer=1 "
              "AND binding_target_channel_id=" + to_string(target),
              &rows, 1) || rows.size() != 1) return false;
  *out = rows[0][0] != 0;
  return true;
}
bool PeerDao::project_share(int destination, uint32_t resource,
                             uint32_t target) {
  if (!execute("START TRANSACTION")) return false;
  transaction = true;
  return execute("CALL supla_suplan_share_demand(" + to_string(user) + "," +
                 to_string(destination) + "," + to_string(resource) + "," +
                 to_string(target) + ")") && commit();
}
bool PeerDao::stream_access(int destination, uint32_t resource,
                             ChannelInfo *channel, uint8_t *permissions,
                             bool *ready) {
  *channel = {};
  *permissions = 0;
  *ready = false;
  std::vector<std::vector<uint64_t>> rows;
  if (!select(
          "SELECT "
          "c.iodevice_id,c.type,c.func,COALESCE(BIT_OR(g.permissions),0),"
          "d.protocol_version,d.flags,d.is_virtual,COALESCE(i.current_root_"
          "epoch,0) "
          "FROM supla_dev_channel c JOIN supla_iodevice d ON d.id=" +
              to_string(destination) +
              " AND d.user_id=c.user_id "
              "LEFT JOIN supla_suplan_device_state i ON i.device_id=d.id "
              "AND i.user_id=d.user_id LEFT JOIN "
              "supla_resource_access_grant_valid g "
              "ON g.user_id=c.user_id AND g.destination_device_id=d.id "
              "AND g.resource_type=1 AND g.resource_id=c.id WHERE c.user_id=" +
              to_string(user) + " AND c.id=" + to_string(resource) +
              " GROUP BY "
              "c.iodevice_id,c.type,c.func,d.protocol_version,d.flags,"
              "d.is_virtual,i.current_root_epoch",
          &rows, 8))
    return false;
  if (!rows.empty()) {
    const auto &row = rows.front();
    if (row[3] > 7) return false;
    channel->device = row[0];
    channel->type = row[1];
    channel->function = row[2];
    *permissions = row[3];
    *ready = row[4] >= 29 && (row[5] & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) &&
             !row[6] && row[7] && row[7] < UINT32_MAX;
  }
  return true;
}
bool PeerDao::permissions(int destination, uint8_t type, uint32_t resource,
                          bool peer, uint8_t *out) {
  *out = 0;
  std::vector<std::vector<uint64_t>> rows;
  if (!select("SELECT COALESCE(BIT_OR(permissions),0) FROM "
              "supla_resource_access_grant_valid WHERE " + scope() +
              " AND destination_device_id=" + to_string(destination) +
              " AND resource_type=" + to_string(type) +
              " AND resource_id=" + to_string(resource) +
              (peer ? " AND peer_eligible=1" : ""), &rows, 1) ||
      rows.size() != 1 || rows[0][0] > 7) return false;
  *out = rows[0][0];
  return true;
}
bool PeerDao::stream_permissions(int destination, uint32_t resource,
                                  uint8_t *out) {
  return permissions(destination, SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL,
                     resource, false, out);
}
bool PeerDao::request_pending(int destination, uint32_t resource,
                               uint8_t permissions, uint8_t mode, bool allow) {
  if (!execute("START TRANSACTION")) return false;
  transaction = true;
  return execute("CALL supla_resource_access_pending(" + to_string(user) +
                 "," + to_string(destination) + "," + to_string(resource) +
                 "," + to_string(permissions) + "," + to_string(mode) +
                 "," + to_string(allow) + ")") && commit();
}
bool PeerDao::cleanup_device(int device) {
  if (!execute("START TRANSACTION")) return false;
  transaction = true;
  return execute("CALL supla_suplan_cleanup(" + to_string(user) + "," +
                 to_string(device) + ")") && commit();
}
bool PeerDao::cleanup_resource(uint32_t resource) {
  if (!execute("START TRANSACTION")) return false;
  transaction = true;
  return execute("CALL supla_suplan_resource_cleanup(" + to_string(user) +
                 "," + to_string(resource) + ")") && commit();
}
bool PeerDao::observe_root(uint64_t id) {
  if (!execute("START TRANSACTION")) return false;
  transaction = true;
  return execute("CALL supla_suplan_observe_root(" + to_string(user) + "," +
                 to_string(id) + ")") && commit();
}
bool PeerDao::project_peer(int destination, uint32_t resource) {
  if (!execute("START TRANSACTION")) return false;
  transaction = true;
  return execute("CALL supla_suplan_peer_demand(" + to_string(user) + "," +
                 to_string(destination) + "," + to_string(resource) + ")") &&
         commit();
}
bool PeerDao::changed_pairs(uint64_t after, size_t limit,
                             std::vector<std::pair<uint64_t, uint64_t>> *out) {
  out->clear();
  std::vector<std::vector<uint64_t>> rows;
  if (!limit || limit > 128 || !select(
      "SELECT association_id,revision FROM supla_suplan_changed_pair WHERE " +
      scope() + " AND association_id>" + to_string(after) +
      " ORDER BY association_id LIMIT " + to_string(limit), &rows, 2))
    return false;
  for (const auto &row : rows) out->push_back({row[0], row[1]});
  return true;
}
bool PeerDao::ack_changed(uint64_t id, uint64_t revision) {
  if (!execute("START TRANSACTION")) return false;
  transaction = true;
  return execute("CALL supla_suplan_changed_ack(" + to_string(user) + "," +
                 to_string(id) + "," + to_string(revision) + ")") && commit();
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

// Read-only local config validation; these lookups never authorize a Grant.
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
  // Local config validation must not coerce strings/fractions into IDs.
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

}  // namespace supla_suplan
