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

#ifndef SUPLA_ABSTRACT_CLIENT_DAO_H_
#define SUPLA_ABSTRACT_CLIENT_DAO_H_

#include "proto.h"
#include "tools.h"

class supla_abstract_client_dao {
 public:
  supla_abstract_client_dao();
  virtual ~supla_abstract_client_dao();

  virtual bool access_id_auth(int access_id, char *access_id_pwd, int *user_id,
                              bool *is_enabled, bool *is_active) = 0;

  virtual bool get_authkey_hash(int id, char authkey_hash[BCRYPT_HASH_MAXSIZE],
                                bool *is_null) = 0;

  virtual bool oauth_get_token(TSC_OAuthToken *token, int user_id,
                               int access_id, bool *storage_connect_error) = 0;
  virtual bool set_reg_enabled(int user_id, int device_reg_time_sec,
                               int client_reg_time_sec) = 0;

  virtual bool get_client_id(int user_id, const char guid[SUPLA_GUID_SIZE],
                             int *id) = 0;

  virtual int get_client_id(int user_id, const char guid[SUPLA_GUID_SIZE]) = 0;

  virtual int get_client_access_id(int client_id, bool *accessid_enabled,
                                   bool *accessid_active) = 0;

  virtual bool get_client_variables(int client_id, bool *client_enabled,
                                    int *access_id, bool *accessid_enabled,
                                    bool *accessid_active) = 0;

  virtual bool get_client_reg_enabled(int user_id) = 0;

  virtual int get_client_limit_left(int user_id) = 0;

  virtual int get_client_count(int user_id) = 0;

  virtual int get_access_id(int user_id, bool enabled, bool active) = 0;

  virtual int add_client(int access_id, const char *guid, const char *authkey,
                         const char *name, unsigned int ipv4,
                         const char *softver, int proto_version,
                         int user_id) = 0;

  virtual bool on_newclient(int client_id) = 0;

  virtual bool update_client(int client_id, int access_id, const char *authkey,
                             const char *name, unsigned int ipv4,
                             const char *softver, int proto_version) = 0;

  virtual bool channel_exists(int client_id, int channel_id) = 0;

  virtual bool channel_group_exists(int client_id, int channel_group_id) = 0;

  virtual bool scene_exists(int client_id, int scene_id) = 0;
};

#endif /* SUPLA_ABSTRACT_CLIENT_DAO_H_ */
