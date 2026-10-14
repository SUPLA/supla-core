// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include "suplan/server_stream.h"

#include <algorithm>
#include <cstring>
#include <utility>
namespace supla_suplan {
StreamConnection::StreamConnection(size_t limit, Authorize authorize, Read read,
                                   Send send, std::function<bool()> ready,
                                   Clock clock)
    // NOLINTNEXTLINE(whitespace/indent_namespace)
    : capacity(std::min(limit, size_t(4096))),
      authorize(authorize),
      read(read),
      send(send),
      ready(ready),
      clock(clock) {}  // NOLINT(whitespace/indent_namespace)
uint64_t StreamConnection::serial(uint32_t id) {
  std::lock_guard<std::mutex> lock(dirty_mutex);
  auto it = serials.find(id);
  return it == serials.end() ? 0 : it->second;
}
void StreamConnection::dirty(uint32_t id) {
  // Mutation hooks never acquire the publication lock, do SQL or send SRPC.
  // A Channel lock -> publication lock inversion is therefore impossible.
  std::lock_guard<std::mutex> lock(dirty_mutex);
  auto it = serials.find(id);
  if (it != serials.end()) {
    ++it->second;
    wake = true;
  }
}
void StreamConnection::erase(uint32_t id) {
  interests.erase(id);
  std::lock_guard<std::mutex> lock(dirty_mutex);
  serials.erase(id);
}
void StreamConnection::terminal(uint32_t id, Interest *interest) {
  if (!interest->active) return;
  auto state = interest->last.state;  // No new protected lookup after revoke.
  state.ChannelId = id;
  state.Channel.Number = 0xFF;
  state.Channel.Offline = SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE;
  if (ready() && !send(state)) closed = true;
}
void StreamConnection::suspend(Interest *interest) {
  if (!interest->active || interest->last.state.Channel.Offline == 1) return;
  auto state = interest->last.state;
  state.Channel.Offline = SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE;
  if (ready() && send(state))
    interest->last.state = state;
  else
    closed = true;
}
bool StreamConnection::publish(uint32_t id, Interest *interest) {
  auto observed = serial(id);
  Snapshot snapshot;
  bool obtained = read(id, &snapshot);
  // Revalidate AFTER snapshot construction, including races with initial read.
  auto access = authorize(id, false, false);
  if (access.Result == SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR) return false;
  if (access.Result != SUPLA_SUPLAN_RESULT_OK ||
      access.AccessStatus != SUPLA_SUPLAN_ACCESS_STATUS_GRANTED) {
    terminal(id, interest);
    erase(id);
    return true;
  }
  if (!obtained || !ready()) return false;
  snapshot.state.ChannelId = id;
  snapshot.state.Channel.Number = 0xFF;
  if (snapshot.expires != Time() && clock() >= snapshot.expires)
    snapshot.state.Channel.Offline = SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE;
  auto comparable = snapshot.state;
  auto previous = interest->last.state;
  // TTL renewal/countdown is scheduling, not a remote-state change.
  comparable.Channel.ValueValidityTimeSec = 0;
  previous.Channel.ValueValidityTimeSec = 0;
  if (!interest->active || memcmp(&comparable, &previous, sizeof(previous))) {
    if (!send(snapshot.state)) {
      closed = true;
      return false;
    }
  }
  interest->last = snapshot;
  interest->active = true;
  interest->serial = observed;
  interest->check = clock() + std::chrono::seconds(30);
  interest->retry_after = Time();
  return true;
}
void StreamConnection::schedule() {
  due = Time::max();
  for (const auto &item : interests) {
    due = std::min(due, item.second.check);
    auto expiry = item.second.last.expires;
    if (item.second.active && expiry != Time() &&
        item.second.last.state.Channel.Offline !=
            SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE)
      due = std::min(due, expiry);
    if (item.second.retry_after <= clock() &&
        serial(item.first) != item.second.serial)
      wake = true;
  }
}
void StreamConnection::ensure(const TDS_SuplaEnsureResourceAccess &q,
                              Reply reply) {
  std::lock_guard<std::mutex> lock(mutex);
  TSD_SuplaEnsureResourceAccessResult result = {
      SUPLA_SUPLAN_RESULT_INVALID_ARGUMENT, SUPLA_SUPLAN_ACCESS_STATUS_INVALID,
      SUPLA_RESOURCE_DELIVERY_SERVER_STREAM};
  bool shape = q.Resource.ResourceId && q.Resource.ResourceId <= INT32_MAX &&
               q.Permissions && !(q.Permissions & ~7) &&
               !(q.Flags & ~SUPLA_SUPLAN_ENSURE_ACCESS_FLAG_ALLOW_APPROVAL);
  if (!shape) {
    reply(result);
    return;
  }
  if (q.DeliveryMode != SUPLA_RESOURCE_DELIVERY_SERVER_STREAM ||
      q.Resource.ResourceType != SUPLA_SUPLAN_RESOURCE_TYPE_CHANNEL ||
      q.Permissions != SUPLA_SUPLAN_PERMISSION_READ) {
    result.Result = SUPLA_SUPLAN_RESULT_UNSUPPORTED;
    reply(result);
    return;
  }
  if (closed || !ready()) {
    result.Result = SUPLA_SUPLAN_RESULT_NOT_AUTHORIZED;
    reply(result);
    return;
  }
  auto id = static_cast<uint32_t>(q.Resource.ResourceId);
  auto found = interests.find(id);
  if (found == interests.end() && interests.size() >= capacity) {
    result.Result = SUPLA_SUPLAN_RESULT_CAPACITY_EXCEEDED;
    reply(result);
    return;
  }
  result = authorize(id, q.Flags != 0, true);
  if (!reply(result)) {
    shutdown();
    return;
  }
  if (result.Result == SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR) {
    if (found != interests.end()) {
      suspend(&found->second);
      if (closed) {
        shutdown();
      } else {
        found->second.check = clock() + std::chrono::seconds(5);
        found->second.retry_after = found->second.check;
        schedule();
      }
    }
    return;
  }
  if (result.Result != SUPLA_SUPLAN_RESULT_OK ||
      (result.AccessStatus != SUPLA_SUPLAN_ACCESS_STATUS_GRANTED &&
       result.AccessStatus != SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL)) {
    if (found != interests.end()) {
      terminal(id, &found->second);
      erase(id);
    }
    return;
  }
  if (found == interests.end()) {
    std::lock_guard<std::mutex> dirty_lock(dirty_mutex);
    serials[id] = 1;
    found = interests.emplace(id, Interest{}).first;
    found->second.serial = 1;
  }
  if (result.AccessStatus == SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL &&
      found->second.active) {
    // End the old authorized stream immediately, but retain the current
    // non-delivering demand so a later approval needs no new ENSURE.
    terminal(id, &found->second);
    if (closed) {
      shutdown();
      return;
    }
    found->second = Interest{};
    found->second.serial = serial(id);
  }
  found->second.check = clock() + std::chrono::seconds(30);
  if (result.AccessStatus == SUPLA_SUPLAN_ACCESS_STATUS_GRANTED) {
    if (!publish(id, &found->second)) {
      suspend(&found->second);
      if (closed) {
        shutdown();
        return;
      }
      found->second.check = clock() + std::chrono::seconds(5);
      found->second.retry_after = found->second.check;
    }
  }
  schedule();
}
void StreamConnection::iterate() {
  std::unique_lock<std::mutex> lock(mutex, std::try_to_lock);
  if (!lock || closed) return;
  if (!ready()) {
    shutdown();
    return;
  }
  if (!wake.exchange(false) && clock() < due) return;
  // Fair, bounded work on this connection, without global Channel scans.
  unsigned work = 0;
  auto it = interests.upper_bound(cursor);
  size_t remaining = interests.size();
  while (remaining-- && !interests.empty()) {
    if (it == interests.end()) it = interests.begin();
    auto current = it++;
    auto id = current->first;
    auto &interest = current->second;
    if (clock() < interest.retry_after) continue;
    bool expired = interest.active && interest.last.expires != Time() &&
                   clock() >= interest.last.expires &&
                   interest.last.state.Channel.Offline !=
                       SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE;
    if (serial(id) == interest.serial && !expired && clock() < interest.check)
      continue;
    if (interest.active) {
      // One current permission check at the send boundary, not two SQL
      // authorization lookups per active recipient on the hot update path.
      if (!publish(id, &interest)) {
        if (closed) {
          shutdown();
          return;
        }
        suspend(&interest);
        if (closed) {
          shutdown();
          return;
        }
        interest.check = clock() + std::chrono::seconds(5);
        interest.retry_after = interest.check;
        interest.serial = serial(id);
      }
      cursor = id;
      if (++work == 16) break;
      continue;
    }
    auto access = authorize(id, false, false);
    if (access.Result == SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR) {
      suspend(&interest);
      if (closed) {
        shutdown();
        return;
      }
      interest.check = clock() + std::chrono::seconds(5);
      interest.retry_after = interest.check;
      // Fail closed during DB outages; dirty work is retried at the deadline.
      interest.serial = serial(id);
    } else if (access.Result == SUPLA_SUPLAN_RESULT_OK &&
               access.AccessStatus == SUPLA_SUPLAN_ACCESS_STATUS_GRANTED) {
      if (!publish(id, &interest)) {
        if (closed) {
          shutdown();
          return;
        }
        suspend(&interest);
        if (closed) {
          shutdown();
          return;
        }
        interest.check = clock() + std::chrono::seconds(5);
        interest.retry_after = interest.check;
        interest.serial = serial(id);
      }
    } else if (interest.active || access.Result != SUPLA_SUPLAN_RESULT_OK ||
               access.AccessStatus !=
                   SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL) {
      terminal(id, &interest);
      erase(id);
    } else {
      interest.serial = serial(id);
      interest.check = clock() + std::chrono::seconds(30);
    }
    cursor = id;
    if (++work == 16) break;
  }
  schedule();
}
void StreamConnection::shutdown() {
  closed = true;
  interests.clear();
  std::lock_guard<std::mutex> dirty_lock(dirty_mutex);
  serials.clear();
}
void StreamConnection::close() {
  std::lock_guard<std::mutex> lock(mutex);
  shutdown();
}
size_t StreamConnection::size() {
  std::lock_guard<std::mutex> lock(mutex);
  return interests.size();
}
}  // namespace supla_suplan
