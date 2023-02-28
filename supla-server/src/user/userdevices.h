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

class supla_user_devices : public supla_connection_objects {
 private:
  std::list<supla_channel_fragment>
      channel_fragments;  // Fragments remain in memory even after the device is
                          // freed.

 public:
  supla_user_devices();
  virtual ~supla_user_devices();
  bool add(std::shared_ptr<supla_device> device);

  std::shared_ptr<supla_device> get(int device_id);
  std::shared_ptr<supla_device> get(int device_id,
                                    int channel_id);  // device_id or channel_id
  supla_channel_fragment get_channel_fragment(int channel_id);
  void update_function_of_channel_fragment(int channel_id, int func);

  void for_each(std::function<void(std::shared_ptr<supla_device> device,
                                   bool *will_continue)>
                    on_device);
};

#endif /* USERDEVICES_H_ */
