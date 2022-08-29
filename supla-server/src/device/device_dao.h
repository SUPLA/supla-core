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

#ifndef SUPLA_DEVICE_DAO_H_
#define SUPLA_DEVICE_DAO_H_

#include "db/abstract_db_access_provider.h"
#include "device/abstract_device_dao.h"

class supla_device_dao : public supla_abstract_device_dao {
 private:
  supla_abstract_db_access_provider *dba;

 public:
  explicit supla_device_dao(supla_abstract_db_access_provider *dba);
  virtual ~supla_device_dao();

  virtual bool get_device_firmware_update_url(
      int device_id, TDS_FirmwareUpdateParams *params,
      TSD_FirmwareUpdate_UrlResult *url);

  virtual bool location_auth(int location_id, char *location_pwd, int *user_id,
                             bool *is_enabled);

  virtual int get_location_id(int user_id, bool enabled);

  virtual int get_device_id(int user_id, const char guid[SUPLA_GUID_SIZE]);

  virtual bool get_device_reg_enabled(int user_id);

  virtual int get_device_limit_left(int user_id);

  virtual int get_device_variables(int device_id, bool *device_enabled,
                                   int *original_location_id, int *location_id,
                                   bool *location_enabled);
};

#endif /* SUPLA_DEVICE_DAO_H_ */
