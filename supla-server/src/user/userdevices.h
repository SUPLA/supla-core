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

#ifndef USERDEVICES_H_
#define USERDEVICES_H_

#include <functional>
#include <list>
#include <memory>
#include <vector>

#include "conn/connection_objects.h"
#include "device/channel_fragment.h"
#include "device/device.h"
#include "user/virtualchannel.h"

class supla_user;
class supla_user_devices : public supla_connection_objects {
 private:
  supla_user *user;
  std::list<supla_channel_fragment>
      channel_fragments;  // Fragments remain in memory even after the device is
                          // freed.

  std::vector<supla_virtual_channel> virtual_channels;
  struct timeval virtual_channels_update_time;
  void update_virtual_channels_if_never_updated(void);

 public:
  explicit supla_user_devices(supla_user *user);
  virtual ~supla_user_devices();
  bool add(std::shared_ptr<supla_device> device);

  std::shared_ptr<supla_device> get(int device_id);
  std::shared_ptr<supla_device> get(int device_id,
                                    int channel_id);  // device_id or channel_id
  supla_virtual_channel get_virtual_channel(int channel_id);
  supla_channel_fragment get_channel_fragment_with_number(
      int device_id, unsigned char channel_number,
      bool load_from_database_if_necessary);
  supla_channel_fragment get_channel_fragment(int channel_id);
  void update_function_of_channel_fragment(int channel_id, int func);

  void for_each(std::function<void(std::shared_ptr<supla_device> device,
                                   bool *will_continue)>
                    on_device);
  void update_virtual_channels(void);
  supla_channel_availability_status get_channel_availability_status(
      int device_id, int channel_id);

  virtual bool is_online(int id);
  void on_channel_deleted(int device_id, int channel_id);
};

#endif /* USERDEVICES_H_ */
