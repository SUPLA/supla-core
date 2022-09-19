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

#ifndef SUPLA_CH_ABSTRACT_REGISTER_CLIENT_H_
#define SUPLA_CH_ABSTRACT_REGISTER_CLIENT_H_

#include <memory>

#include "client/abstract_client_dao.h"
#include "conn/abstract_connection_dao.h"
#include "conn/call_handler/abstract_register_object.h"
#include "db/abstract_db_access_provider.h"
#include "proto.h"
#include "srpc/abstract_srpc_adapter.h"

class supla_user;
class supla_client;
class supla_abstract_register_client
    : protected supla_abstract_register_object {
 private:
  std::weak_ptr<supla_client> client;
  supla_abstract_client_dao *client_dao;
  TCS_SuplaRegisterClient_B *register_client_b;
  TCS_SuplaRegisterClient_D *register_client_d;
  int access_id;
  int client_id;
  bool accessid_enabled;
  bool accessid_active;
  bool client_enabled;
  bool pwd_is_set;

  void send_result(int resultcode);
  bool client_auth(void);
  bool add_client(void);
  bool update_client(void);

 protected:
  void register_client(std::weak_ptr<supla_client> client,
                       TCS_SuplaRegisterClient_B *register_client_b,
                       TCS_SuplaRegisterClient_D *register_client_d,
                       supla_abstract_srpc_adapter *srpc_adapter,
                       supla_abstract_db_access_provider *dba,
                       supla_abstract_connection_dao *conn_dao,
                       supla_abstract_client_dao *client_dao, int client_sd,
                       int client_ipv4, unsigned char activity_timeout);

  std::weak_ptr<supla_client> get_client(void);
  supla_abstract_client_dao *get_client_dao(void);
  int get_client_id(void);
  int get_access_id(void);

  virtual void superuser_authorize(
      int user_id, const char email[SUPLA_EMAIL_MAXSIZE],
      const char password[SUPLA_PASSWORD_MAXSIZE]) = 0;

  virtual bool is_superuser_authorized(void) = 0;

  virtual void revoke_superuser_authorization(void) = 0;

  virtual void on_registraction_success(void) = 0;

  virtual int get_location_count(void) = 0;

  virtual int get_channel_count(void) = 0;

  virtual int get_channel_group_count(void) = 0;

  virtual int get_scene_count(void) = 0;

  virtual void remote_update_lists(void) = 0;

 public:
  supla_abstract_register_client(void);
  virtual ~supla_abstract_register_client();
  __useconds_t get_hold_time_on_failure_usec(void);
};

#endif /* SUPLA_CH_ABSTRACT_REGISTER_DEVICE_H_*/
