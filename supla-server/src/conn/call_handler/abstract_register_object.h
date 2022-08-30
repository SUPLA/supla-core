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

#ifndef SUPLA_CH_ABSTRACT_REGISTER_OBJECT_H_
#define SUPLA_CH_ABSTRACT_REGISTER_OBJECT_H_

#include "conn/authkey_cache.h"
#include "proto.h"

class supla_ch_abstract_register_object {
 protected:
  bool is_valid_guid(char guid[SUPLA_GUID_SIZE]);
  bool is_valid_authkey(char authkey[SUPLA_AUTHKEY_SIZE]);

  bool authkey_auth(const char guid[SUPLA_GUID_SIZE],
                    const char email[SUPLA_EMAIL_MAXSIZE],
                    const char authkey[SUPLA_AUTHKEY_SIZE], int *user_id);

  virtual supla_authkey_cache *get_authkey_cache(void) = 0;
  virtual int get_user_id_by_email(const char email[SUPLA_EMAIL_MAXSIZE]) = 0;
  virtual bool get_object_id(int user_id, const char guid[SUPLA_GUID_SIZE],
                             int *id) = 0;
  virtual bool get_authkey_hash(int id, char *buffer, unsigned int buffer_size,
                                bool *is_null) = 0;

 public:
  supla_ch_abstract_register_object(void);
  virtual ~supla_ch_abstract_register_object();
};

#endif /* SUPLA_CH_ABSTRACT_REGISTER_OBJECT_H_*/
