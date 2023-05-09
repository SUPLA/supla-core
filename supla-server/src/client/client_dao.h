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

#ifndef SUPLA_CLIENT_DAO_H_
#define SUPLA_CLIENT_DAO_H_

#include <string>

#include "client/abstract_client_dao.h"
#include "db/abstract_db_access_provider.h"

class supla_client_dao : public supla_abstract_client_dao {
 private:
  supla_abstract_db_access_provider *dba;

  int oauth_add_client_id(void);
  int oauth_get_client_id(bool create);
  bool exists(int client_id, int id, const char *query);

 public:
  explicit supla_client_dao(supla_abstract_db_access_provider *dba);
  virtual ~supla_client_dao();

  virtual bool access_id_auth(int access_id, char *access_id_pwd, int *user_id,
                              bool *is_enabled, bool *is_active);

  virtual bool get_authkey_hash(int id, char authkey_hash[BCRYPT_HASH_MAXSIZE],
                                bool *is_null);

  virtual bool oauth_get_token(TSC_OAuthToken *token, int user_id,
                               int access_id, bool *storage_connect_error);

  virtual bool set_reg_enabled(int user_id, int device_reg_time_sec,
                               int client_reg_time_sec);

  virtual bool get_client_id(int user_id, const char guid[SUPLA_GUID_SIZE],
                             int *id);

  virtual int get_client_id(int user_id, const char guid[SUPLA_GUID_SIZE]);

  virtual int get_client_access_id(int client_id, bool *accessid_enabled,
                                   bool *accessid_active);

  virtual bool get_client_variables(int client_id, bool *client_enabled,
                                    int *access_id, bool *accessid_enabled,
                                    bool *accessid_active,
                                    std::string *client_name);

  virtual bool get_client_reg_enabled(int user_id);

  virtual int get_client_limit_left(int user_id);

  virtual int get_client_count(int user_id);

  virtual int get_access_id(int user_id, bool enabled, bool active);

  virtual int add_client(int access_id, const char *guid, const char *authkey,
                         const char *name, unsigned int ipv4,
                         const char *softver, int proto_version, int user_id);

  virtual bool on_newclient(int client_id);

  virtual bool update_client(int client_id, int access_id, const char *authkey,
                             const char *name, unsigned int ipv4,
                             const char *softver, int proto_version);

  virtual bool channel_exists(int client_id, int channel_id);

  virtual bool channel_group_exists(int client_id, int channel_group_id);

  virtual bool scene_exists(int client_id, int scene_id);

  virtual void update_client_push_notification_token(int client_id,
                                                     const char *token,
                                                     int platform, int app_id,
                                                     bool development_env);
};

#endif /* SUPLA_CLIENT_DAO_H_ */
