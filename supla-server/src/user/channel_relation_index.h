// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SUPLA_CHANNEL_RELATION_INDEX_H_
#define SUPLA_CHANNEL_RELATION_INDEX_H_
#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "device/abstract_common_channel_properties.h"

// Presentation metadata only. Never an authorization or availability cache.
struct supla_relation_node {
  int id = 0, device = 0, type = 0, function = 0, number = 0;
  int param1 = 0, param2 = 0, param3 = 0, meter = 0, aux_type = 0;
  std::array<unsigned int, 6> hvac{};
  std::vector<unsigned int> sensors;
  static supla_relation_node read(supla_abstract_common_channel_properties *p);
  static bool opening_sensor(int function);
  static bool opening_control(int function);
  static bool partial_opening_control(int function);
  void parse(supla_json_config *config);
  bool operator==(const supla_relation_node &other) const;
};
struct supla_relation_edge {
  int child, parent;
  short type;
  unsigned char version, directions;
  bool operator==(const supla_relation_edge &other) const;
};
struct supla_relation_snapshot {
  uint64_t revision = 0;
  std::map<int, supla_relation_node> nodes;
  std::vector<supla_relation_edge> edges;
  std::map<int, std::vector<size_t>> by_parent, by_child;
  void build();
  std::vector<supla_channel_relation> get(
      int id, e_relation_kind kind, unsigned char version,
      std::function<bool(const supla_relation_edge &)> visible = {}) const;
  size_t memory_bytes() const;
};
class supla_channel_relation_index {
  using clock = std::chrono::steady_clock;
  int user;
  mutable std::mutex mutex;
  std::mutex reload_mutex;
  std::shared_ptr<const supla_relation_snapshot> current;
  uint64_t epoch = 0, builds = 0;
  std::map<int, uint64_t> pending;
  bool reload = true;
  clock::time_point retry_at{};
  void retry_locked();

 public:
  explicit supla_channel_relation_index(int user) : user(user) {}
  std::shared_ptr<const supla_relation_snapshot> snapshot() const;
  uint64_t begin(int channel);
  // Config must come from a successful authoritative read/protected merge.
  bool complete(int channel, uint64_t ticket, supla_json_config *config,
                int function);
  void failed(int channel, uint64_t ticket);
  void cancel(int channel, uint64_t ticket);
  void invalidate();
  bool refresh();
  uint64_t time_left_usec() const;
  uint64_t build_count() const;
};
#endif
