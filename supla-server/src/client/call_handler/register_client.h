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

#ifndef SUPLA_CH_REGISTER_CLIENT_H_
#define SUPLA_CH_REGISTER_CLIENT_H_

#include <memory>

#include "client/call_handler/abstract_register_client.h"

class supla_ch_register_client : public supla_ch_abstract_register_client {
 protected:
  virtual supla_authkey_cache *get_authkey_cache(void);
  virtual int get_user_id_by_email(const char email[SUPLA_EMAIL_MAXSIZE]);
  virtual bool get_object_id(int user_id, const char guid[SUPLA_GUID_SIZE],
                             int *id);
  virtual bool get_authkey_hash(int id, char authkey_hash[BCRYPT_HASH_MAXSIZE],
                                bool *is_null);
  virtual void superuser_authorize(int user_id,
                                   const char email[SUPLA_EMAIL_MAXSIZE],
                                   const char password[SUPLA_PASSWORD_MAXSIZE]);
  virtual bool is_superuser_authorized(void);
  virtual void revoke_superuser_authorization(void);
  virtual void on_registraction_success(void);
  virtual int get_location_count(void);
  virtual int get_channel_count(void);
  virtual int get_channel_group_count(void);
  virtual int get_scene_count(void);
  virtual void remote_update_lists(void);

 public:
  supla_ch_register_client(void);
  virtual ~supla_ch_register_client();
};

#endif /* SUPLA_CH_REGISTER_CLIENT_H_*/
