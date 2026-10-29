/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "client/client_channel_relations.h"

#include <algorithm>
#include <set>
#include <tuple>
#include <vector>

#include "client/client.h"
#include "client/clientchannel.h"
#include "device/channel_relation.h"
#include "user.h"
#include "user/channel_relation_index.h"

using std::vector;

supla_client_channel_reactions::supla_client_channel_reactions(
    supla_abstract_dobject_remote_updater *updater)
    : supla_dobjects(updater) {}

supla_client_channel_reactions::~supla_client_channel_reactions() {}

bool supla_client_channel_reactions::load(supla_client_channels *channels,
                                          bool force) {
  auto client = channels->getClient();
  auto user = client ? client->get_user() : nullptr;
  auto snapshot = user ? user->get_relation_index()->snapshot() : nullptr;
  // Capture one immutable graph and one visibility set; no list lock is
  // held while reading the Client channel collection (or doing any SQL).
  {
    lock();
    bool unchanged =
        !force && snapshot && loaded && revision == snapshot->revision;
    unlock();
    if (unchanged) return false;
  }
  vector<supla_channel_relation> relations;
  std::set<std::tuple<int, int, short>> seen;
  auto append = [&](supla_channel_relation &r) {
    int a = r.get_id(), b = r.get_parent_id();
    if (seen.emplace(std::min(a, b), std::max(a, b), r.get_relation_type())
            .second)
      relations.push_back(r);
  };
  if (snapshot) {
    std::set<int> visible;
    std::vector<int> order;
    channels->for_each([&](supla_client_channel *channel, bool *) {
      visible.insert(channel->get_id());
      order.push_back(channel->get_id());
    });
    auto accept = [&](const supla_relation_edge &e) {
      return visible.count(e.child) && visible.count(e.parent);
    };
    auto version = client->get_protocol_version();
    for (int id : order) {
      for (auto &relation : snapshot->get(id, relation_any, version, accept)) {
        append(relation);
      }
    }
  } else {
    supla_relation_snapshot local;
    std::vector<std::pair<int, unsigned char>> readers;
    channels->for_each([&](supla_client_channel *channel, bool *) {
      auto node = supla_relation_node::read(channel);
      node.device = 0;
      node.number = readers.size();
      local.nodes[node.id] = node;
      readers.emplace_back(node.id, channel->get_relation_protocol_version());
    });
    local.build();
    for (auto reader : readers)
      for (auto &r : local.get(reader.first, relation_any, reader.second)) {
        append(r);
      }
  }

  // A temporary solution to the problem of clearing the list on the application
  // side when the channel relations has been deleted.
  relations.push_back(supla_channel_relation(0, 0, relation_any));

  lock();
  // A slower publisher must never replace a newer projection.
  if (snapshot ? revision > snapshot->revision : revision != 0) {
    unlock();
    return false;
  }
  bool equal = loaded && previous.size() == relations.size();
  for (size_t i = 0; equal && i < relations.size(); ++i)
    equal = previous[i].child == relations[i].get_id() &&
            previous[i].parent == relations[i].get_parent_id() &&
            previous[i].type == relations[i].get_relation_type();
  if (equal) {
    revision = snapshot ? snapshot->revision : 0;
    unlock();
    return false;
  }
  previous.clear();
  previous.reserve(relations.size());
  for (auto &r : relations)
    previous.push_back(
        {r.get_id(), r.get_parent_id(), r.get_relation_type(), 0, 0});
  loaded = true;
  revision = snapshot ? snapshot->revision : 0;
  clear();

  for (auto it = relations.begin(); it != relations.end(); ++it) {
    add(new supla_channel_relation(&(*it), true));
  }

  unlock();
  return true;
}
