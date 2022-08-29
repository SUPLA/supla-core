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

#ifndef SUPLA_CH_ABSTRACT_REGISTER_DEVICE_H_
#define SUPLA_CH_ABSTRACT_REGISTER_DEVICE_H_

#include "db/abstract_db_access_provider.h"
#include "device/abstract_device_dao.h"
#include "proto.h"

class supla_ch_abstract_register_device {
 private:
  bool authkey_auth(const char guid[SUPLA_GUID_SIZE],
                    const char email[SUPLA_EMAIL_MAXSIZE],
                    const char authkey[SUPLA_AUTHKEY_SIZE], int *user_id,
                    supla_abstract_device_dao *dao);

 protected:
  char register_device(supla_abstract_db_access_provider *dba,
                       supla_abstract_device_dao *dao,
                       TDS_SuplaRegisterDevice_C *register_device_c,
                       TDS_SuplaRegisterDevice_E *register_device_e,
                       unsigned char proto_version);

  virtual void set_flags(int flags) = 0;
  virtual bool set_guid(const char GUID[SUPLA_GUID_SIZE]) = 0;
  virtual bool set_authkey(const char AuthKey[SUPLA_AUTHKEY_SIZE]) = 0;
  virtual int get_client_sd(void) = 0;

 public:
  supla_ch_abstract_register_device(void);
  virtual ~supla_ch_abstract_register_device();
};

#endif /* SUPLA_CH_ABSTRACT_REGISTER_DEVICE_H_*/
