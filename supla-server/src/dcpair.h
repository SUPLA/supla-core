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

#ifndef DCPAIR_H_
#define DCPAIR_H_
#include <list>

class dcpair {
 private:
  int DeviceId;
  int ChannelId;

 public:
  dcpair(int DeviceId, int ChannelId);
  int getDeviceId();
  int getChannelId();

  static bool popDeviceChannelIDs(std::list<dcpair> *pairs, int *deviceId,
                                  std::list<int> *cids);

  static bool compare(const dcpair p1, const dcpair p2);
  static void sort_by_device_id(std::list<dcpair> *pairs);
  static bool last_one(std::list<dcpair> *pairs,
                       std::list<dcpair>::iterator it);
};

#endif /* DCPAIR_H_ */
