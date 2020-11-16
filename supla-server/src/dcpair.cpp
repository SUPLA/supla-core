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

#include <dcpair.h>

dcpair::dcpair(int DeviceId, int ChannelId) {
  this->DeviceId = DeviceId;
  this->ChannelId = ChannelId;
}

int dcpair::getDeviceId() { return DeviceId; }

int dcpair::getChannelId() { return ChannelId; }

// static
bool dcpair::popDeviceChannelIDs(std::list<dcpair> *pairs, int *deviceId,
                                 std::list<int> *cids) {
  if (pairs && deviceId && cids) {
    *deviceId = 0;
    cids->clear();

    std::list<dcpair>::iterator it = pairs->begin();
    while (it != pairs->end()) {
      if (*deviceId == 0) {
        *deviceId = it->getDeviceId();
      }

      if (*deviceId == it->getDeviceId()) {
        cids->push_back(it->getChannelId());
        it = pairs->erase(it);
      } else {
        ++it;
      }
    }
    return cids->size() > 0;
  }

  return false;
}

// static
bool dcpair::compare(const dcpair p1, const dcpair p2) {
  return p1.DeviceId < p2.DeviceId ||
         (p1.DeviceId == p2.DeviceId && p1.ChannelId <= p2.ChannelId);
}

// static
void dcpair::sort_by_device_id(std::list<dcpair> *pairs) {
  pairs->sort(dcpair::compare);
}

// static
bool dcpair::last_one(std::list<dcpair> *pairs,
                      std::list<dcpair>::iterator it) {
  if (pairs->end() == it) {
    return true;
  }

  int deviceID1 = it->getDeviceId();
  it++;
  int deviceID2 = it->getDeviceId();

  return deviceID1 != deviceID2;
}
