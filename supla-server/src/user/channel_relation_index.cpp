// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include "user/channel_relation_index.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "db/mariadb_access_provider.h"
#include "jsonconfig/channel/container_config.h"
#include "jsonconfig/channel/hvac_config.h"
#include "jsonconfig/channel/power_switch_config.h"
#include "jsonconfig/channel/valve_config.h"

namespace {
bool hvac_function(int f) {
  return f == SUPLA_CHANNELFNC_HVAC_THERMOSTAT ||
         f == SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL ||
         f == SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL ||
         f == SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER;
}
bool thermometer(int f) {
  return f == SUPLA_CHANNELFNC_THERMOMETER ||
         f == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE;
}
bool meter(int f) {
  return f == SUPLA_CHANNELFNC_ELECTRICITY_METER ||
         f == SUPLA_CHANNELFNC_IC_ELECTRICITY_METER ||
         f == SUPLA_CHANNELFNC_IC_GAS_METER ||
         f == SUPLA_CHANNELFNC_IC_WATER_METER ||
         f == SUPLA_CHANNELFNC_IC_HEAT_METER;
}
bool power(int f) {
  return f == SUPLA_CHANNELFNC_POWERSWITCH ||
         f == SUPLA_CHANNELFNC_LIGHTSWITCH ||
         f == SUPLA_CHANNELFNC_STAIRCASETIMER;
}
}  // namespace
bool supla_relation_node::opening_sensor(int f) {
  return f == SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY ||
         f == SUPLA_CHANNELFNC_OPENINGSENSOR_GATE ||
         f == SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR ||
         f == SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR ||
         f == SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER ||
         f == SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW;
}
bool supla_relation_node::opening_control(int f) {
  return partial_opening_control(f) ||
         f == SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER ||
         f == SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW;
}
bool supla_relation_node::partial_opening_control(int f) {
  return f == SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK ||
         f == SUPLA_CHANNELFNC_CONTROLLINGTHEGATE ||
         f == SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR ||
         f == SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK;
}
supla_relation_node supla_relation_node::read(
    supla_abstract_common_channel_properties *p) {
  supla_relation_node n;
  n.id = p->get_id();
  n.type = p->get_type();
  n.function = p->get_func();
  const int f = n.function;
  if (supla_relation_node::opening_control(f)) {
    n.param2 = p->get_param2();
    if (supla_relation_node::partial_opening_control(f))
      n.param3 = p->get_param3();
  }
  if (supla_relation_node::opening_sensor(f)) {
    n.param1 = p->get_param1();
    n.param2 = p->get_param2();
  }
  if (hvac_function(f) || power(f) || n.type == SUPLA_CHANNELTYPE_CONTAINER ||
      f == SUPLA_CHANNELFNC_VALVE_OPENCLOSE) {
    std::unique_ptr<supla_json_config> config(p->get_json_config());
    n.parse(config.get());
  }
  return n;
}
void supla_relation_node::parse(supla_json_config *config) {
  hvac = {};
  sensors.clear();
  aux_type = 0;
  meter = 0;
  if (!config) return;
  if (hvac_function(function)) {
    TChannelConfig_HVAC value{};
    if (hvac_config(config).get_config(&value)) {
      hvac = {static_cast<unsigned>(value.MainThermometerChannelId),
              static_cast<unsigned>(value.AuxThermometerChannelId),
              static_cast<unsigned>(value.BinarySensorChannelId),
              static_cast<unsigned>(value.MasterThermostatChannelId),
              static_cast<unsigned>(value.PumpSwitchChannelId),
              static_cast<unsigned>(value.HeatOrColdSourceSwitchChannelId)};
      aux_type = value.AuxThermometerType;
    }
  }
  if (power(function))
    meter = power_switch_config(config).get_related_meter_channel_id();
  if (function == SUPLA_CHANNELFNC_VALVE_OPENCLOSE) {
    TChannelConfig_Valve value{};
    if (valve_config(config).get_config(&value))
      for (const auto &entry : value.SensorInfo)
        if (entry.ChannelId) sensors.push_back(entry.ChannelId);
  } else if (type == SUPLA_CHANNELTYPE_CONTAINER) {
    TChannelConfig_Container value{};
    if (container_config(config).get_config(&value))
      for (const auto &entry : value.SensorInfo)
        if (entry.ChannelId) sensors.push_back(entry.ChannelId);
  }
}
bool supla_relation_node::operator==(const supla_relation_node &b) const {
  return std::tie(id, device, type, function, number, param1, param2, param3,
                  meter, aux_type, hvac, sensors) ==
         std::tie(b.id, b.device, b.type, b.function, b.number, b.param1,
                  b.param2, b.param3, b.meter, b.aux_type, b.hvac, b.sensors);
}
bool supla_relation_edge::operator==(const supla_relation_edge &b) const {
  return std::tie(child, parent, type, version, directions) ==
         std::tie(b.child, b.parent, b.type, b.version, b.directions);
}
void supla_relation_snapshot::build() {
  edges.clear();
  by_parent.clear();
  by_child.clear();
  auto add = [&](int child, int parent, short type, int version,
                 int direction) {
    if (!child || !parent || !nodes.count(child) || !nodes.count(parent))
      return;
    size_t i = edges.size();
    edges.push_back({child, parent, type, static_cast<unsigned char>(version),
                     static_cast<unsigned char>(direction)});
    if (direction & 1) by_parent[parent].push_back(i);
    if (direction & 2) by_child[child].push_back(i);
  };
  std::vector<const supla_relation_node *> ordered;
  for (const auto &entry : nodes) ordered.push_back(&entry.second);
  // Preserve the first-parent sensor inverse rule in Client SQL order.
  std::stable_sort(ordered.begin(), ordered.end(), [](auto a, auto b) {
    return std::tie(a->device, a->number, a->id) <
           std::tie(b->device, b->number, b->id);
  });

  for (auto node : ordered) {
    const auto &n = *node;
    int f = n.function;
    if (supla_relation_node::opening_control(f)) {
      add(n.param2, n.id, CHANNEL_RELATION_TYPE_OPENING_SENSOR, 0, 1);
      if (supla_relation_node::partial_opening_control(f))
        add(n.param3, n.id, CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR, 0, 1);
    }
    if (supla_relation_node::opening_sensor(f)) {
      add(n.id, n.param1, CHANNEL_RELATION_TYPE_OPENING_SENSOR, 0, 2);
      add(n.id, n.param2, CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR, 0, 2);
    }
    if (n.meter) {
      auto child = nodes.find(n.meter);
      add(n.meter, n.id, CHANNEL_RELATION_TYPE_METER, 0,
          1 | ((child != nodes.end() && meter(child->second.function)) ? 2
                                                                       : 0));
    }
    for (auto id : n.sensors) {
      auto child = nodes.find(id);
      int target = f == SUPLA_CHANNELFNC_VALVE_OPENCLOSE
                       ? SUPLA_CHANNELFNC_FLOOD_SENSOR
                       : SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR;
      if (child == nodes.end() || child->second.function != target) continue;
      // Pick the first inverse parent AFTER the reader's visibility filter.
      add(id, n.id, CHANNEL_RELATION_TYPE_DEFAULT, 27, 7);
    }
    if (!hvac_function(f)) continue;
    for (size_t field : {0u, 1u, 2u, 4u, 5u}) {
      auto c = nodes.find(n.hvac[field]);
      if (c == nodes.end() || c->first == n.id) continue;
      const auto &child = c->second;
      short type = CHANNEL_RELATION_TYPE_DEFAULT;
      int version = 0;
      bool ok = false;
      if (field <= 1) {
        ok = thermometer(child.function);
        if (field == 1)
          ok = ok && n.aux_type >= SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR &&
               n.aux_type <= SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER;
        type =
            field == 0 ? CHANNEL_RELATION_TYPE_MAIN_TERMOMETER : n.aux_type + 3;
      } else if (field == 2) {
        ok = child.type == SUPLA_CHANNELTYPE_BINARYSENSOR;
      } else {
        ok = child.function == (field == 4
                                    ? SUPLA_CHANNELFNC_PUMPSWITCH
                                    : SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH);
        version = 25;
        type = field == 4 ? CHANNEL_RELATION_TYPE_PUMP_SWITCH
                          : CHANNEL_RELATION_TYPE_HEAT_OR_COLD_SOURCE_SWITCH;
      }
      if (ok)
        add(child.id, n.id, type, version,
            child.type == SUPLA_CHANNELTYPE_BINARYSENSOR && field != 2 ? 1 : 3);
    }
    auto master = nodes.find(n.hvac[3]);
    if (master != nodes.end())
      add(n.id, master->first, CHANNEL_RELATION_TYPE_MASTER_THERMOSTAT, 25,
          hvac_function(master->second.function) ? 3 : 2);
  }
  auto order = [&](int id) {
    const auto &n = nodes.at(id);
    return std::make_tuple(n.device, n.number, n.id);
  };
  for (auto &entry : by_parent) {
    if (!hvac_function(nodes.at(entry.first).function)) continue;
    // Legacy HVAC scans visited referenced channels in Client SQL order.
    std::stable_sort(entry.second.begin(), entry.second.end(),
                     [&](size_t a, size_t b) {
                       return order(edges[a].child) < order(edges[b].child);
                     });
  }
  for (auto &entry : by_child) {
    auto rank = [&](const supla_relation_edge &e) {
      if (e.directions & 4) return 0;
      return supla_relation_node::opening_sensor(nodes.at(e.child).function) &&
                     (e.type == CHANNEL_RELATION_TYPE_OPENING_SENSOR ||
                      e.type == CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR)
                 ? 1
                 : 2;
    };
    std::stable_sort(entry.second.begin(), entry.second.end(),
                     [&](size_t a, size_t b) {
                       const auto &x = edges[a], &y = edges[b];
                       if (rank(x) != rank(y)) return rank(x) < rank(y);
                       if (rank(x) == 1) return x.type < y.type;
                       return order(x.parent) < order(y.parent);
                     });
  }
}
std::vector<supla_channel_relation> supla_relation_snapshot::get(
    int id, e_relation_kind kind, unsigned char version,
    std::function<bool(const supla_relation_edge &)> visible) const {
  std::vector<supla_channel_relation> result;
  auto append = [&](const auto &index) {
    auto found = index.find(id);
    if (found == index.end()) return;
    bool first_sensor = false;
    for (auto i : found->second) {
      const auto &e = edges[i];
      if (e.version > version || (visible && !visible(e))) continue;
      if (&index == &by_child && (e.directions & 4)) {
        if (first_sensor) continue;
        first_sensor = true;
      }
      bool duplicate = false;
      for (auto &r : result)
        if (r.get_relation_type() == e.type &&
            ((r.get_id() == e.child && r.get_parent_id() == e.parent) ||
             (r.get_id() == e.parent && r.get_parent_id() == e.child)))
          duplicate = true;
      if (!duplicate) result.emplace_back(e.child, e.parent, e.type);
    }
  };
  if (kind != relation_with_parent_channel) append(by_parent);
  if (kind != relation_with_sub_channel) append(by_child);
  return result;
}
size_t supla_relation_snapshot::memory_bytes() const {
  // Allocated payload estimate; benchmark also measures allocator live bytes.
  size_t bytes = sizeof(*this) + edges.capacity() * sizeof(supla_relation_edge);
  for (const auto &n : nodes)
    bytes += sizeof(n) + 4 * sizeof(void *) +
             n.second.sensors.capacity() * sizeof(unsigned);
  for (auto index : {&by_parent, &by_child})
    for (const auto &e : *index)
      bytes +=
          sizeof(e) + 4 * sizeof(void *) + e.second.capacity() * sizeof(size_t);
  return bytes;
}
std::shared_ptr<const supla_relation_snapshot>
supla_channel_relation_index::snapshot() const {
  return std::atomic_load(&current);
}
uint64_t supla_channel_relation_index::begin(int channel) {
  std::lock_guard<std::mutex> guard(mutex);
  return pending[channel] = ++epoch;
}
void supla_channel_relation_index::retry_locked() {
  reload = true;
  retry_at = clock::now() + std::chrono::seconds(5);
}
void supla_channel_relation_index::failed(int channel, uint64_t ticket) {
  std::lock_guard<std::mutex> guard(mutex);
  auto found = pending.find(channel);
  if (found != pending.end() && found->second == ticket) {
    pending.erase(found);
    ++epoch;
    retry_locked();
  }
}
void supla_channel_relation_index::cancel(int channel, uint64_t ticket) {
  std::lock_guard<std::mutex> guard(mutex);
  auto found = pending.find(channel);
  if (found != pending.end() && found->second == ticket) {
    pending.erase(found);
  }
}
void supla_channel_relation_index::invalidate() {
  std::lock_guard<std::mutex> guard(mutex);
  ++epoch;
  reload = true;
  retry_at = clock::time_point{};
}
bool supla_channel_relation_index::complete(int channel, uint64_t ticket,
                                            supla_json_config *config,
                                            int function) {
  if (!config) {
    failed(channel, ticket);
    return false;
  }
  std::lock_guard<std::mutex> guard(mutex);
  auto p = pending.find(channel);
  if (p == pending.end() || p->second != ticket) return false;
  auto old = snapshot();
  if (!old || !old->nodes.count(channel)) {
    pending.erase(p);
    ++epoch;
    reload = true;
    retry_at = clock::time_point{};
    return false;
  }
  auto node = old->nodes.at(channel);
  if (function >= 0) node.function = function;
  node.parse(config);
  if (node == old->nodes.at(channel)) {
    pending.erase(p);
    if (reload && pending.empty()) retry_at = clock::time_point{};
    return false;
  }
  auto next = std::make_shared<supla_relation_snapshot>();
  next->nodes = old->nodes;
  next->nodes[channel] = std::move(node);
  next->build();
  // Publication is fenced by the same per-index mutex as begin(), including
  // the post-build revision check. SQL and callbacks never hold this mutex.
  if (pending.at(channel) != ticket) return false;
  pending.erase(channel);
  ++epoch;
  if (reload && pending.empty()) retry_at = clock::time_point{};
  next->revision = ++builds;
  std::atomic_store(&current,
                    std::shared_ptr<const supla_relation_snapshot>(next));
  return next->edges != old->edges;
}
uint64_t supla_channel_relation_index::time_left_usec() const {
  std::lock_guard<std::mutex> guard(mutex);
  // A writer's completion/cancellation will wake the existing event loops.
  if (!reload || !pending.empty()) return 0;
  auto now = clock::now();
  if (now >= retry_at) return 1;
  return std::chrono::duration_cast<std::chrono::microseconds>(retry_at - now)
             .count() +
         1;
}
uint64_t supla_channel_relation_index::build_count() const {
  std::lock_guard<std::mutex> guard(mutex);
  return builds;
}
bool supla_channel_relation_index::refresh() {
  std::lock_guard<std::mutex> loader(reload_mutex);
  uint64_t ticket;
  {
    std::lock_guard<std::mutex> guard(mutex);
    if (!reload || !pending.empty() || clock::now() < retry_at) return false;
    ticket = epoch;
  }
  // One coherent SQL statement, including offline/virtual channel metadata.
  supla_mariadb_access_provider db;
  auto next = std::make_shared<supla_relation_snapshot>();
  bool ok = db.connect();
  MYSQL_STMT *stmt = nullptr;
  std::string sql =
      "SELECT "
      "c.id,c.iodevice_id,c.type,COALESCE(c.func,0),COALESCE(c.param1,0),"
      "COALESCE(c.param2,0),COALESCE(c.param3,0),c.channel_number,c.user_"
      "config,c.properties "
      "FROM supla_dev_channel c JOIN supla_iodevice d ON d.id=c.iodevice_id "
      "AND d.user_id=c.user_id WHERE c.user_id=" +
      std::to_string(user);
  ok = ok && db.stmt_execute(reinterpret_cast<void **>(&stmt), sql.c_str(),
                             nullptr, 0, true);
  if (ok) {
    supla_relation_node node;
    std::vector<char> json(65536), properties(65536);
    unsigned long sizes[2]{};
    my_bool nulls[2]{};
    MYSQL_BIND bind[10]{};
    int *values[] = {&node.id,     &node.device, &node.type,   &node.function,
                     &node.param1, &node.param2, &node.param3, &node.number};
    for (int i = 0; i < 8; ++i) {
      bind[i].buffer_type = MYSQL_TYPE_LONG;
      bind[i].buffer = values[i];
    }
    for (int i = 0; i < 2; ++i) {
      bind[i + 8].buffer_type = MYSQL_TYPE_STRING;
      bind[i + 8].buffer = i ? properties.data() : json.data();
      bind[i + 8].buffer_length = json.size() - 1;
      bind[i + 8].length = &sizes[i];
      bind[i + 8].is_null = &nulls[i];
    }
    ok = !mysql_stmt_bind_result(stmt, bind);
    int status = MYSQL_NO_DATA;
    // Cloud's TEXT columns hold at most 65535 bytes each, plus our NUL.
    while (ok && (status = mysql_stmt_fetch(stmt)) == 0) {
      json[std::min<size_t>(sizes[0], json.size() - 1)] = 0;
      properties[std::min<size_t>(sizes[1], properties.size() - 1)] = 0;
      // Use the existing JSON parser's legacy unset/null semantics. A SQL
      // failure still rejects the entire candidate snapshot.
      supla_json_config config;
      if (!nulls[0]) config.set_user_config(json.data());
      if (!nulls[1]) config.set_properties(properties.data());
      node.parse(&config);
      next->nodes[node.id] = node;
    }
    ok = ok && status == MYSQL_NO_DATA;
  }
  if (stmt) mysql_stmt_close(stmt);
  auto observed = snapshot();
  bool unchanged = ok && observed && observed->nodes == next->nodes;
  if (ok && !unchanged) next->build();
  std::lock_guard<std::mutex> guard(mutex);
  if (!ok) {
    retry_locked();
    return false;
  }
  if (epoch != ticket || !pending.empty()) {
    // An authority change is not a database failure. Pending writers wake us
    // when done; otherwise the next event-loop turn reloads the latest root.
    reload = true;
    retry_at = clock::time_point{};
    return false;
  }
  auto old = snapshot();
  if (unchanged) {
    reload = false;
    return false;
  }
  next->revision = ++builds;
  reload = false;
  std::atomic_store(&current,
                    std::shared_ptr<const supla_relation_snapshot>(next));
  return !old || next->edges != old->edges;
}
