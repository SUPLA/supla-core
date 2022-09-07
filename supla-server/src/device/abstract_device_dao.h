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

#ifndef SUPLA_ABSTRACT_DEVICE_DAO_H_
#define SUPLA_ABSTRACT_DEVICE_DAO_H_

#include "proto.h"

class supla_abstract_device_dao {
 public:
  supla_abstract_device_dao();
  virtual ~supla_abstract_device_dao();

  virtual bool get_device_firmware_update_url(
      int device_id, TDS_FirmwareUpdateParams *params,
      TSD_FirmwareUpdate_UrlResult *url) = 0;

  virtual bool location_auth(int location_id, char *location_pwd, int *user_id,
                             bool *is_enabled) = 0;

  virtual int get_location_id(int user_id, bool enabled) = 0;

  virtual bool get_device_id(int user_id, const char guid[SUPLA_GUID_SIZE],
                             int *id) = 0;

  virtual int get_device_id(int user_id, const char guid[SUPLA_GUID_SIZE]) = 0;

  virtual bool get_device_reg_enabled(int user_id) = 0;

  virtual int get_device_limit_left(int user_id) = 0;

  virtual int get_device_variables(int device_id, bool *device_enabled,
                                   int *original_location_id, int *location_id,
                                   bool *location_enabled) = 0;

  virtual int get_channel_id_and_type(int device_id, int channel_number,
                                      int *type) = 0;

  virtual int get_device_channel_count(int device_id) = 0;

  virtual int add_device(int location_id, const char guid[SUPLA_GUID_SIZE],
                         const char *authkey, const char *name,
                         unsigned int ipv4, const char *softver,
                         int proto_version, short manufacturer_id,
                         short product_id, int flags, int user_id) = 0;

  virtual int update_device(int device_od, int original_location_id,
                            const char *authkey, const char *name,
                            unsigned int ipv4, const char *softver,
                            int proto_version, int flags) = 0;

  virtual int add_channel(int device_id, int channel_number, int type, int func,
                          int param1, int param2, int flist, int flags,
                          int user_id) = 0;

  virtual bool on_new_device(int device_id) = 0;

  virtual bool on_channel_added(int device_id, int channel_id) = 0;
};

#endif /* SUPLA_ABSTRACT_DEVICE_DAO_H_ */
