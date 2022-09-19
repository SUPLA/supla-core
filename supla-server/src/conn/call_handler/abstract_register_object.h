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

#include "conn/abstract_connection_dao.h"
#include "conn/authkey_cache.h"
#include "conn/call_handler/abstract_register_object.h"
#include "db/abstract_db_access_provider.h"
#include "proto.h"
#include "srpc/abstract_srpc_adapter.h"
#include "tools.h"

class supla_abstract_register_object {
 private:
  char *guid;
  char *authkey;
  char *name;
  char *softver;

  int client_sd;
  int client_ipv4;
  unsigned char activity_timeout;

  int user_id;
  bool should_rollback;

  supla_abstract_srpc_adapter *srpc_adapter;
  supla_abstract_db_access_provider *dba;
  supla_abstract_connection_dao *conn_dao;

  __useconds_t hold_time_on_failure_usec;

 protected:
  bool is_valid_guid(void);
  bool is_valid_authkey(void);

  bool authkey_auth(const char guid[SUPLA_GUID_SIZE],
                    const char email[SUPLA_EMAIL_MAXSIZE],
                    const char authkey[SUPLA_AUTHKEY_SIZE], int *user_id);

  virtual supla_authkey_cache *get_authkey_cache(void) = 0;
  virtual int get_user_id_by_email(const char email[SUPLA_EMAIL_MAXSIZE]) = 0;
  virtual bool get_object_id(int user_id, const char guid[SUPLA_GUID_SIZE],
                             int *id) = 0;
  virtual bool get_authkey_hash(int id, char authkey_hash[BCRYPT_HASH_MAXSIZE],
                                bool *is_null) = 0;
  void set_guid(char *guid);
  char *get_guid(void);
  void set_authkey(char *authkey);
  char *get_authkey(void);
  void set_name(char *name);
  char *get_name(void);
  void set_softver(char *softver);
  char *get_softver(void);

  void set_client_sd(int client_sd);
  int get_client_sd(void);
  void set_client_ipv4(int client_ipv4);
  int get_client_ipv4(void);
  void set_activity_timeout(unsigned char activity_timeout);
  unsigned char get_activity_timeout(void);
  void set_user_id(int user_id);
  int get_user_id(void);
  int *get_user_id_ptr(void);

  void set_should_rollback(bool should_rollback);
  bool get_should_rollback(void);

  void set_srpc_adapter(supla_abstract_srpc_adapter *srpc_adapter);
  supla_abstract_srpc_adapter *get_srpc_adapter(void);
  void set_dba(supla_abstract_db_access_provider *dba);
  supla_abstract_db_access_provider *get_dba(void);
  void set_conn_dao(supla_abstract_connection_dao *conn_dao);
  supla_abstract_connection_dao *get_conn_dao(void);

  void set_hold_time_on_failure_usec(__useconds_t hold_time_on_failure_usec);

 public:
  supla_abstract_register_object(void);
  virtual ~supla_abstract_register_object();
  __useconds_t get_hold_time_on_failure_usec(void);
};

#endif /* SUPLA_CH_ABSTRACT_REGISTER_OBJECT_H_*/
