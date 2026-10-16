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

#include "srpc/abstract_srpc_adapter.h"

// Connection-local barrier for future SupLAN-dependent provisioning.
// The owning Device serializes access; no identity becomes active on DB
// allocation.
class supla_suplan_identity_bootstrap {
 private:
  bool pending = false;
  unsigned _supla_int_t root_epoch = 0;

 public:
  void reset() {
    pending = false;
    root_epoch = 0;
  }

  enum class StartResult { Disabled, Pending, Failed };

  StartResult start(supla_abstract_srpc_adapter *srpc, int flags,
             TSD_SuplaDeviceIdentities *identities) {
    reset();
    if (!(flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) ||
        !(flags & SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED) ||
        srpc->get_proto_version() < 29) {
      return StartResult::Disabled;
    }
    if (identities->DeviceId <= 0 ||
        identities->ChannelCount < 0 ||
        identities->ChannelCount > SUPLA_CHANNELMAXCOUNT) {
      return StartResult::Failed;
    }
    for (int i = 0; i < identities->ChannelCount; i++) {
      if (identities->ChannelId[i] <= 0) return StartResult::Failed;
    }
    pending = srpc->sd_async_suplan_device_identities(identities) > 0;
    return pending ? StartResult::Pending : StartResult::Failed;
  }

  void on_result(const TDS_SuplaDeviceIdentitiesResult *result) {
    bool was_pending = pending;
    reset();
    if (was_pending && result && result->Result == SUPLA_SUPLAN_RESULT_OK &&
        result->RootEpoch != 0 && result->RootEpoch != 0xFFFFFFFFu) {
      root_epoch = result->RootEpoch;
    }
  }

  bool is_pending() const { return pending; }
  bool is_ready() const { return root_epoch != 0; }
  unsigned _supla_int_t get_root_epoch() const { return root_epoch; }
};

#endif  // SUPLAN_IDENTITY_BOOTSTRAP_H_
