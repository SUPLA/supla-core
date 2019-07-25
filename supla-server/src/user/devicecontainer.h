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

#include "cdcontainer.h"

class supla_user_device_container : public cdcontainer {
 private:
  static char find_device_byid(void *ptr, void *ID);
  static char find_device_by_channelid(void *ptr, void *ID);
  static char find_device_byguid(void *ptr, void *GUID);

  supla_device *baseToDevice(cdbase *base);

 protected:
  virtual void cd_delete(cdbase *base);

 public:
  supla_user_device_container();
  virtual ~supla_user_device_container();

  supla_device *findByID(int DeviceID);
  supla_device *findByChannelID(int CahnnelID);
  supla_device *findByGUID(const char *GUID);
  supla_device *get(int idx);
};

#endif /* USER_DEVICECONTAINER_H_ */
