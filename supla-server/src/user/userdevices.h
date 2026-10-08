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
#include <map>
#include <memory>
#include <vector>

#include "conn/connection_objects.h"
#include "device/channel_fragment.h"
#include "device/device.h"
#include "ocpp/ocpp_devices.h"
#include "user/virtualchannel.h"

class supla_user;
class supla_electricity_analyzer;
class supla_user_devices : public supla_connection_objects {
 private:
  supla_user *user;
  std::list<supla_channel_fragment>
      channel_fragments;  // Fragments remain in memory even after the device is
                          // freed.

  std::vector<supla_virtual_channel> virtual_channels;
  struct timeval virtual_channels_update_time;
  void update_virtual_channels_if_never_updated(void);
  // OCPP stations also use virtual devices and channels in the database, but
  // have a separate runtime for connection state, validity and commands.
  supla_ocpp_devices ocpp_devices;

 public:
  explicit supla_user_devices(supla_user *user);
  virtual ~supla_user_devices();
  bool add(std::shared_ptr<supla_device> device,
           std::map<int, supla_channel_availability_status> *previous_statuses);

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
  std::shared_ptr<supla_ocpp_device> get_ocpp_device(int device_id,
                                                     int channel_id = 0);
  supla_ocpp_channel get_ocpp_channel(int channel_id);
  void reload_ocpp_devices(int device_id = 0);
  void access_ocpp_data_analyzers(
      std::function<void(supla_electricity_analyzer *)> callback);
  void get_ocpp_meter_values(
      std::vector<supla_abstract_channel_extended_value_envelope *> *values);
  void on_channel_config_changed(int device_id, int channel_id);
  supla_channel_availability_status get_channel_availability_status(
      int device_id, int channel_id);

  virtual bool is_online(int id);
  void on_channel_added(int device_id, int channel_id);
  void on_device_deleted(int device_id);
  void on_channel_deleted(int device_id, int channel_id);
};

#endif /* USERDEVICES_H_ */
