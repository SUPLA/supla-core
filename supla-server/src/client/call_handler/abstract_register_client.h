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
class supla_ch_abstract_register_client
    : protected supla_ch_abstract_register_object {
 private:
  std::weak_ptr<supla_client> client;

  int access_id;
  supla_abstract_client_dao *client_dao;

 protected:
  char register_client(TCS_SuplaRegisterClient_B *register_client_b,
                       TCS_SuplaRegisterClient_D *register_client_d,
                       supla_abstract_srpc_adapter *srpc_adapter,
                       supla_abstract_db_access_provider *dba,
                       supla_abstract_connection_dao *conn_dao,
                       supla_abstract_client_dao *device_dao, int client_sd,
                       int client_ipv4, unsigned char activity_timeout);

  std::weak_ptr<supla_client> get_client(void);
  supla_abstract_client_dao *get_client_dao(void);

 public:
  supla_ch_abstract_register_client(void);
  virtual ~supla_ch_abstract_register_client();
  __useconds_t get_hold_time_on_failure_usec(void);
};

#endif /* SUPLA_CH_ABSTRACT_REGISTER_DEVICE_H_*/
