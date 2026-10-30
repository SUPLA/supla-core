// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include "suplan/server_stream.h"

#include <algorithm>
#include <cstring>
#include <utility>
namespace supla_suplan {
stream_connection::stream_connection(size_t limit,
                                     authorize_callback_t authorize,
                                     read_callback_t read, send_callback_t send,
                                     std::function<bool()> ready, clock_t clock,
                                     std::function<void()> notify)
    // NOLINTNEXTLINE(whitespace/indent_namespace)
    : capacity(std::min(limit, size_t(4096))),
      authorize(authorize),
      read(read),
      send(send),
      ready(ready),
      clock(clock),
      notify(notify) {}  // NOLINT(whitespace/indent_namespace)
uint64_t stream_connection::serial(uint32_t id) {
  std::lock_guard<std::mutex> lock(dirty_mutex);
  auto it = serials.find(id);
  return it == serials.end() ? 0 : it->second;
}
void stream_connection::dirty(uint32_t id) {
  // Mutation hooks never acquire the publication lock, do SQL or send SRPC.
  // A Channel lock -> publication lock inversion is therefore impossible.
  bool signal = false;
  {
    std::lock_guard<std::mutex> lock(dirty_mutex);
    auto it = serials.find(id);
    if (it != serials.end()) {
      ++it->second;
      signal = !wake.exchange(true);
    }
  }
  if (signal && notify) notify();
}
void stream_connection::invalidate_access() {
  {
    // Serialize invalidation with publication, outside Channel reads.
    std::lock_guard<std::mutex> lock(access_mutex);
    ++access_revision;
  }
  bool signal;
  {
    std::lock_guard<std::mutex> lock(dirty_mutex);
    signal = !serials.empty() && !wake.exchange(true);
  }
  if (signal && notify) notify();
}
uint64_t stream_connection::time_left_usec() const {
  if (wake.load()) return 1;
  auto deadline = due.load();
  if (deadline == time_point_t::max()) return 0;
  auto now = clock();
  if (deadline <= now) return 1;
  return std::max<int64_t>(1, std::chrono::duration_cast<
      std::chrono::microseconds>(deadline - now).count());
}
void stream_connection::erase(uint32_t id) {
  interests.erase(id);
  std::lock_guard<std::mutex> lock(dirty_mutex);
  serials.erase(id);
}
void stream_connection::terminal(uint32_t id, interest_t *interest) {
  if (!interest->active) return;
  auto state = interest->last.state;  // No new protected lookup after revoke.
  state.ChannelId = id;
  state.Channel.Number = 0xFF;
  state.Channel.Offline = SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE;
  if (ready() && !send(state)) closed = true;
}
void stream_connection::suspend(interest_t *interest) {
  if (!interest->active || interest->last.state.Channel.Offline == 1) return;
  auto state = interest->last.state;
  state.Channel.Offline = SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE;
  if (ready() && send(state))
    interest->last.state = state;
  else
    closed = true;
}
void stream_connection::retry(interest_t *interest) {
  if (!closed) suspend(interest);
  interest->access_revision = 0;
  interest->retry_after = clock() + std::chrono::seconds(5);
}
bool stream_connection::publish(uint32_t id, interest_t *interest,
                                 const TDS_SuplaDeviceChannel_E &metadata) {
  auto observed = serial(id);
  snapshot_t snapshot;
  if (!read(id, &snapshot) || !ready()) return false;
  snapshot.state.Channel.FuncList = metadata.FuncList;
  snapshot.state.Channel.DefaultIcon = metadata.DefaultIcon;
  snapshot.state.Channel.SubDeviceId = metadata.SubDeviceId;
  snapshot.state.ChannelId = id;
  snapshot.state.Channel.Number = 0xFF;
  if (snapshot.expires != time_point_t() && clock() >= snapshot.expires)
    snapshot.state.Channel.Offline = SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE;
  auto comparable = snapshot.state;
  auto previous = interest->last.state;
  // TTL renewal/countdown is scheduling, not a remote-state change.
  comparable.Channel.ValueValidityTimeSec = 0;
  previous.Channel.ValueValidityTimeSec = 0;
  // No Channel locks or SQL under this fence. A later invalidation wakes us
  // again; a snapshot prepared before invalidation must not pass it.
  std::lock_guard<std::mutex> lock(access_mutex);
  if (interest->access_revision != access_revision) return true;
  if (!interest->active || memcmp(&comparable, &previous, sizeof(previous))) {
    if (!send(snapshot.state)) {
      closed = true;
      return false;
    }
  }
  interest->last = snapshot;
  interest->active = true;
  interest->serial = observed;
  interest->retry_after = time_point_t();
  return true;
}
void stream_connection::schedule() {
  auto next = time_point_t::max();
  auto now = clock();
  for (const auto &item : interests) {
    const auto &interest = item.second;
    auto deadline = time_point_t::max();
    auto expiry = interest.last.expires;
    if (interest.active && expiry != time_point_t() &&
        interest.last.state.Channel.Offline !=
            SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE)
      deadline = expiry;
    if (interest.retry_after != time_point_t())
      deadline = std::min(deadline, interest.retry_after);
    if (interest.access_revision != access_revision ||
        (interest.granted && serial(item.first) != interest.serial))
      deadline = std::min(deadline, now);
    deadline = std::max(deadline, interest.retry_after);
    next = std::min(next, deadline);
  }
  due = next;
}
void stream_connection::ensure(const TDS_SuplaEnsureResourceAccess &q,
                               reply_callback_t reply) {
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
  auto revision = access_revision.load();
  TDS_SuplaDeviceChannel_E metadata{};
  result = authorize(id, q.Flags != 0, true, &metadata);
  if (!reply(result)) {
    shutdown();
    return;
  }
  if (result.Result == SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR) {
    if (found != interests.end()) {
      retry(&found->second);
      if (closed) shutdown();
      else schedule();
    }
    return;
  }
  if (result.Result != SUPLA_SUPLAN_RESULT_OK ||
      (result.AccessStatus != SUPLA_SUPLAN_ACCESS_STATUS_GRANTED &&
       result.AccessStatus != SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL)) {
    if (found != interests.end()) {
      terminal(id, &found->second);
      erase(id);
      if (closed) shutdown();
      else schedule();
    }
    return;
  }
  if (found == interests.end()) {
    std::lock_guard<std::mutex> dirty_lock(dirty_mutex);
    serials[id] = 1;
    found = interests.emplace(id, interest_t{}).first;
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
    found->second = interest_t{};
    found->second.serial = serial(id);
  }
  auto &interest = found->second;
  interest.access_revision = revision;
  interest.retry_after = time_point_t();
  interest.granted =
      result.AccessStatus == SUPLA_SUPLAN_ACCESS_STATUS_GRANTED;
  if (interest.granted && !publish(id, &interest, metadata)) {
    retry(&interest);
    if (closed) {
      shutdown();
      return;
    }
  }
  schedule();
}
void stream_connection::iterate() {
  std::unique_lock<std::mutex> lock(mutex, std::try_to_lock);
  if (!lock || closed) return;
  if (!ready()) {
    shutdown();
    return;
  }
  if (!wake.exchange(false) && clock() < due.load()) return;
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
    bool expired = interest.active && interest.last.expires != time_point_t() &&
                   clock() >= interest.last.expires &&
                   interest.last.state.Channel.Offline !=
                       SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE;
    bool revalidate = interest.access_revision != access_revision;
    if (!revalidate && !expired && serial(id) == interest.serial &&
        interest.retry_after == time_point_t()) continue;
    auto metadata = interest.last.state.Channel;
    if (revalidate) {
      auto revision = access_revision.load();
      auto access = authorize(id, false, false, &metadata);
      if (access.Result == SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR) {
        retry(&interest);
      } else if (access.Result != SUPLA_SUPLAN_RESULT_OK ||
                 (access.AccessStatus != SUPLA_SUPLAN_ACCESS_STATUS_GRANTED &&
                  access.AccessStatus !=
                      SUPLA_SUPLAN_ACCESS_STATUS_PENDING_APPROVAL)) {
        terminal(id, &interest);
        erase(id);
      } else {
        interest.access_revision = revision;
        interest.retry_after = time_point_t();
        interest.granted =
            access.AccessStatus == SUPLA_SUPLAN_ACCESS_STATUS_GRANTED;
        if (!interest.granted) {
          terminal(id, &interest);
          interest.active = false;
          interest.serial = serial(id);
        }
      }
    }
    // erase() invalidates the reference. Failed reads retain retry-only work.
    auto found = interests.find(id);
    if (found != interests.end() &&
        found->second.retry_after == time_point_t() && found->second.granted &&
        !publish(id, &found->second, metadata))
      retry(&found->second);
    if (closed) {
      shutdown();
      return;
    }
    cursor = id;
    if (++work == 16) break;
  }
  schedule();
}
void stream_connection::shutdown() {
  closed = true;
  interests.clear();
  std::lock_guard<std::mutex> dirty_lock(dirty_mutex);
  serials.clear();
  wake = false;
  due = time_point_t::max();
}
void stream_connection::close() {
  std::lock_guard<std::mutex> lock(mutex);
  shutdown();
}
size_t stream_connection::size() {
  std::lock_guard<std::mutex> lock(mutex);
  return interests.size();
}
}  // namespace supla_suplan
