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

#include "doubles/client/RegisterClientMock.h"

namespace testing {
RegisterClientMock::RegisterClientMock(void)
    : supla_abstract_register_client() {}

RegisterClientMock::~RegisterClientMock(void) {}

void RegisterClientMock::set_hold_time_on_failure_usec(__useconds_t usec) {
  supla_abstract_register_client::set_hold_time_on_failure_usec(usec);
}

void RegisterClientMock::register_client(
    TCS_SuplaRegisterClient_B *register_client_b,
    TCS_SuplaRegisterClient_D *register_client_d,
    supla_abstract_srpc_adapter *srpc_adapter,
    supla_abstract_db_access_provider *dba,
    supla_abstract_connection_dao *conn_dao,
    supla_abstract_client_dao *client_dao, int client_sd, int client_ipv4,
    unsigned char activity_timeout) {
  std::weak_ptr<supla_client> client;
  supla_abstract_register_client::register_client(
      client, register_client_b, register_client_d, srpc_adapter, dba, conn_dao,
      client_dao, client_sd, client_ipv4, activity_timeout);
}

}  // namespace testing
