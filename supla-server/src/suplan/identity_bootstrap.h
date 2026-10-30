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

#ifndef SUPLAN_IDENTITY_BOOTSTRAP_H_
#define SUPLAN_IDENTITY_BOOTSTRAP_H_

#include <chrono>

#include "srpc/abstract_srpc_adapter.h"

// Connection-local barrier for future SupLAN-dependent provisioning.
// The owning Device serializes access; no identity becomes active on DB
// allocation.
class supla_suplan_identity_bootstrap {
 public:
  using clock_t = std::chrono::steady_clock;
  using time_point_t = clock_t::time_point;

 private:
  time_point_t deadline{};
  bool pending = false;
  unsigned _supla_int_t root_epoch = 0;

 public:
  void reset() {
    pending = false;
    root_epoch = 0;
    deadline = {};
  }

  enum class start_result { disabled, pending, failed };

  start_result start(supla_abstract_srpc_adapter *srpc, int flags,
                     TSD_SuplaDeviceIdentities *identities,
                     time_point_t now = clock_t::now()) {
    reset();
    if (!(flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) ||
        !(flags & SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED) ||
        srpc->get_proto_version() < 29) {
      return start_result::disabled;
    }
    if (identities->DeviceId <= 0 ||
        identities->ChannelCount < 0 ||
        identities->ChannelCount > SUPLA_CHANNELMAXCOUNT) {
      return start_result::failed;
    }
    for (int i = 0; i < identities->ChannelCount; i++) {
      if (identities->ChannelId[i] <= 0) return start_result::failed;
    }
    pending = srpc->sd_async_suplan_device_identities(identities) > 0;
    if (pending) deadline = now + std::chrono::seconds(15);
    return pending ? start_result::pending : start_result::failed;
  }

  void on_result(const TDS_SuplaDeviceIdentitiesResult *result) {
    if (!pending) return;
    reset();
    if (result && result->Result == SUPLA_SUPLAN_RESULT_OK &&
        result->RootEpoch != 0 && result->RootEpoch != 0xFFFFFFFFu) {
      root_epoch = result->RootEpoch;
    }
  }

  uint64_t time_left_usec(time_point_t now = clock_t::now()) const {
    if (!pending) return 0;
    if (now >= deadline) return 1;
    return std::chrono::duration_cast<std::chrono::microseconds>(
        deadline - now).count() + 1;
  }

  bool expire(time_point_t now = clock_t::now()) {
    if (!pending || now < deadline) return false;
    reset();
    return true;
  }

  bool is_pending() const { return pending; }
  bool is_ready() const { return root_epoch != 0; }
  unsigned _supla_int_t get_root_epoch() const { return root_epoch; }
};

#endif  // SUPLAN_IDENTITY_BOOTSTRAP_H_
