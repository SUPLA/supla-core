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

#include "gate_state_getter.h"
#include <list>
#include "device.h"

gate_state_getter::gate_state_getter(void)
    : supla_abstract_gate_state_getter() {}

gate_state_getter::~gate_state_getter(void) {}

bool gate_state_getter::get_openingsensor_state(supla_user *user,
                                                int channel_id,
                                                bool *is_closed) {
  bool result = false;
  supla_device *device = user->device_by_channelid(channel_id);
  if (device) {
    supla_device_channels *channels = device->get_channels();

    if (channels->is_channel_online(channel_id)) {
      int func = channels->get_channel_func(channel_id);

      switch (func) {
        case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
          char v = 0;
          if (channels->get_channel_char_value(channel_id, &v)) {
            *is_closed = v != 0;
            result = true;
          }
          break;
      }
    }

    device->releasePtr();
  }
  return result;
}

bool gate_state_getter::get_related_openingsensor_state(int user_id,
                                                        int master_device_id,
                                                        int master_channel_id,
                                                        bool *is_closed) {
  if (is_closed == NULL) {
    return false;
  }

  static supla_user *user = supla_user::find(user_id, false);
  if (!user) {
    return false;
  }

  bool result = false;

  supla_device *device = user->get_device(master_device_id);
  if (device) {
    supla_device_channels *channels = device->get_channels();
    if (channels->is_channel_online(master_channel_id)) {
      int func = channels->get_channel_func(master_channel_id);

      switch (func) {
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
          std::list<int> rel =
              device->get_channels()->related_channel(master_channel_id);

          if (rel.size()) {
            result = get_openingsensor_state(user, rel.front(), is_closed);
          }
          break;
      }
    }
    device->releasePtr();
  }

  return result;
}
