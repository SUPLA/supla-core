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

#include "doubles/device/DeviceDaoMock.h"

namespace testing {
DeviceDaoMock::DeviceDaoMock(void) : supla_abstract_device_dao() {}

DeviceDaoMock::~DeviceDaoMock(void) {}

int DeviceDaoMock::add_device(int location_id, const char guid[SUPLA_GUID_SIZE],
                              const char *authkey, const char *name,
                              unsigned int ipv4, const char *softver,
                              int proto_version, short manufacturer_id,
                              short product_id, int flags, int user_id) {
  add_device(location_id, guid, authkey, name, ipv4, softver, proto_version,
             manufacturer_id, product_id, flags);

  return add_device(user_id);
}

int DeviceDaoMock::add_channel(int device_id, int channel_number, int type,
                               int func, int param1, int param2, int flist,
                               unsigned _supla_int64_t flags, int alt_icon,
                               unsigned short sub_device_id, int user_id) {
  int result = add_channel_a(device_id, channel_number, type, func, param1,
                             param2, flist, flags, alt_icon, user_id);

  result += add_channel_b(device_id, channel_number, sub_device_id, user_id);
  return result;
}

}  // namespace testing
