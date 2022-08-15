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

#ifndef USER_DEVICECONTAINER_H_
#define USER_DEVICECONTAINER_H_

#include "conn/connection_objects.h"
#include "device.h"

class supla_user_device_container : public supla_connection_objects {
 private:
  static char find_device_byid(void *ptr, void *ID);
  static char find_device_by_channelid(void *ptr, void *ID);
  static char find_device_byguid(void *ptr, void *GUID);

  supla_device *baseToDevice(supla_connection_object *base);

 public:
  supla_user_device_container();
  virtual ~supla_user_device_container();
  bool add(std::shared_ptr<supla_device> device);

  std::shared_ptr<supla_device> find_by_id(int device_id);
  std::shared_ptr<supla_device> find_by_guid(const char *guid);
  std::shared_ptr<supla_device> find_by_channel_id(int channel_id);
  std::vector<std::shared_ptr<supla_device> > get_all(void);
  bool get_channel_double_value(int device_id, int channel_id, double *value,
                                char type);
  bool get_channel_char_value(int device_id, int channel_id, char *value);
  bool get_channel_rgbw_value(int device_id, int channel_id, int *color,
                              char *color_brightness, char *brightness,
                              char *on_off);
  bool get_channel_valve_value(int device_id, int channel_id,
                               TValve_Value *value);
  void set_channel_function(int channel_id, int func);
};

#endif /* USER_DEVICECONTAINER_H_ */
