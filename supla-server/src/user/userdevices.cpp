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
#include <sys/time.h>
#include <userdevices.h>

#include "db/db_access_provider.h"
#include "device/device.h"
#include "device/device_dao.h"
#include "user/user.h"
#include "user/user_dao.h"

using std::dynamic_pointer_cast;
using std::function;
using std::shared_ptr;
using std::vector;
using std::weak_ptr;

supla_user_devices::supla_user_devices(supla_user *user)
    : supla_connection_objects() {
  virtual_channels_update_time = {};
  this->user = user;
}

supla_user_devices::~supla_user_devices() {}

bool supla_user_devices::add(shared_ptr<supla_device> device) {
  bool result = supla_connection_objects::add(device);

  vector<supla_channel_fragment> fragments =
      device->get_channels()->get_fragments();

  int device_id = device->get_id();

  lock();
  for (auto it = channel_fragments.begin(); it != channel_fragments.end();
       ++it) {
    if (it->get_device_id() == device_id) {
      it = channel_fragments.erase(it);
      --it;
    }
  }

  for (auto it = fragments.begin(); it != fragments.end(); ++it) {
    channel_fragments.push_back(*it);
  }
  unlock();

  return result;
}

std::shared_ptr<supla_device> supla_user_devices::get(int device_id) {
  return dynamic_pointer_cast<supla_device>(
      supla_connection_objects::get(device_id));
}

std::shared_ptr<supla_device> supla_user_devices::get(int device_id,
                                                      int channel_id) {
  if (device_id) {
    return get(device_id);
  } else if (channel_id) {
    shared_ptr<supla_device> result;

    for_each([&result, channel_id](shared_ptr<supla_device> device,
                                   bool *will_continue) -> void {
      if (device->get_channels()->channel_exists(channel_id)) {
        result = device;
        *will_continue = false;
      }
    });

    return result;
  }

  return nullptr;
}

supla_channel_fragment supla_user_devices::get_channel_fragment_with_number(
    int device_id, unsigned char channel_number,
    bool load_from_database_if_necessary) {
  supla_channel_fragment result;

  lock();
  for (auto it = channel_fragments.begin(); it != channel_fragments.end();
       ++it) {
    if (it->get_device_id() == device_id &&
        it->get_channel_number() == channel_number) {
      result = *it;
      break;
    }
  }
  unlock();

  if (result.get_channel_id() == 0 && load_from_database_if_necessary) {
    supla_db_access_provider dba;
    supla_device_dao dao(&dba);
    result = dao.get_channel_fragment(device_id, channel_number);

    if (result.get_channel_id()) {
      lock();
      for (auto it = channel_fragments.begin(); it != channel_fragments.end();
           ++it) {
        if (it->get_channel_id() == result.get_channel_id()) {
          it = channel_fragments.erase(it);
          break;
        }
      }

      channel_fragments.push_back(result);

      unlock();
    }
  }

  return result;
}

supla_channel_fragment supla_user_devices::get_channel_fragment(
    int channel_id) {
  supla_channel_fragment result;
  lock();
  for (auto it = channel_fragments.begin(); it != channel_fragments.end();
       ++it) {
    if (it->get_channel_id() == channel_id) {
      result = *it;
      break;
    }
  }
  unlock();
  return result;
}

void supla_user_devices::update_function_of_channel_fragment(int channel_id,
                                                             int func) {
  lock();
  for (auto it = channel_fragments.begin(); it != channel_fragments.end();
       ++it) {
    if (it->get_channel_id() == channel_id) {
      it->set_function(func);
      break;
    }
  }
  unlock();
}

void supla_user_devices::for_each(
    function<void(shared_ptr<supla_device> device, bool *will_continue)>
        on_device) {
  supla_connection_objects::for_each(
      [on_device](shared_ptr<supla_abstract_connection_object> obj,
                  bool *will_continue) -> void {
        shared_ptr<supla_device> device =
            dynamic_pointer_cast<supla_device>(obj);
        if (device) {
          on_device(device, will_continue);
        }
      });
}

supla_virtual_channel supla_user_devices::get_virtual_channel(int channel_id) {
  update_virtual_channels_if_never_updated();

  supla_virtual_channel result;

  lock();
  for (auto it = virtual_channels.begin(); it != virtual_channels.end(); ++it) {
    if (it->get_channel_id() == channel_id) {
      result = *it;
      break;
    }
  }
  unlock();

  return result;
}

void supla_user_devices::update_virtual_channels_if_never_updated(void) {
  lock();

  if (!virtual_channels_update_time.tv_sec) {
    update_virtual_channels();
  }

  unlock();
}

void supla_user_devices::update_virtual_channels(void) {
  lock();

  supla_db_access_provider dba;
  supla_user_dao dao(&dba);

  std::vector<supla_virtual_channel> virtual_channels =
      dao.get_virtual_channels(user, virtual_channels_update_time.tv_sec);

  for (auto tit = this->virtual_channels.begin();
       tit != this->virtual_channels.end(); ++tit) {
    bool exists = false;
    for (auto it = virtual_channels.begin(); it != virtual_channels.end();
         ++it) {
      if (it->get_channel_id() == tit->get_channel_id()) {
        exists = true;
        break;
      }
    }

    if (!exists) {
      tit = this->virtual_channels.erase(tit);
      --tit;
    }
  }

  for (auto it = virtual_channels.begin(); it != virtual_channels.end(); ++it) {
    for (auto tit = this->virtual_channels.begin();
         tit != this->virtual_channels.end(); ++tit) {
      if (it->get_channel_id() == tit->get_channel_id()) {
        tit->apply_changes(user, &(*it));

        it = virtual_channels.erase(it);
        --it;

        break;
      }
    }
  }

  for (auto it = virtual_channels.begin(); it != virtual_channels.end(); ++it) {
    this->virtual_channels.push_back(*it);
  }

  gettimeofday(&virtual_channels_update_time, nullptr);

  unlock();
}

supla_channel_availability_status
supla_user_devices::get_channel_availability_status(int device_id,
                                                    int channel_id) {
  supla_channel_availability_status result(true);

  shared_ptr<supla_device> device = get(device_id, channel_id);
  if (device != nullptr) {
    result =
        device->get_channels()->get_channel_availability_status(channel_id);
  } else {
    supla_virtual_channel vchannel = get_virtual_channel(channel_id);
    if (vchannel.get_channel_id()) {
      result = vchannel.get_availability_status();
    }
  }

  return result;
}

bool supla_user_devices::is_online(int id) {
  update_virtual_channels_if_never_updated();

  bool result = supla_connection_objects::is_online(id);
  if (!result) {
    lock();
    for (auto it = virtual_channels.begin(); it != virtual_channels.end();
         ++it) {
      if (it->get_device_id() == id &&
          it->get_availability_status().is_online()) {
        result = true;
        break;
      }
    }
    unlock();
  }

  return result;
}
