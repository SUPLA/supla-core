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

#ifndef SUPLA_CH_GET_CHANNEL_VALUE_WITH_AUTH_H_
#define SUPLA_CH_GET_CHANNEL_VALUE_WITH_AUTH_H_

#include <memory>

#include "client/call_handler/abstract_client_srpc_call_handler.h"

class supla_mariadb_access_provider;
class supla_connection_dao;
class supla_client_dao;

class supla_ch_get_channel_value_with_auth
    : public supla_abstract_client_srpc_call_handler {
 protected:
  struct authentication_result {
    int result_code;
    int client_id;
    int user_id;
  };

  virtual void handle_call(std::shared_ptr<supla_client> client,
                           supla_abstract_srpc_adapter* srpc_adapter,
                           TsrpcReceivedData* rd, unsigned int call_id,
                           unsigned char proto_version);
  void handle_request(std::shared_ptr<supla_client> client,
                      supla_abstract_srpc_adapter* srpc_adapter,
                      TCS_GetChannelValueWithAuth* request);
  virtual authentication_result authenticate(
      std::shared_ptr<supla_client> client,
      TCS_ClientAuthorizationDetails* auth,
      supla_abstract_srpc_adapter* srpc_adapter,
      supla_mariadb_access_provider* dba, supla_connection_dao* conn_dao,
      supla_client_dao* client_dao);
  virtual bool channel_exists(supla_client_dao* client_dao, int client_id,
                              int channel_id);

 public:
  supla_ch_get_channel_value_with_auth(void);
  virtual ~supla_ch_get_channel_value_with_auth();
  virtual bool can_handle_call(unsigned int call_id);
  virtual bool is_registration_required(void);
};

#endif /* SUPLA_CH_GET_CHANNEL_VALUE_WITH_AUTH_H_*/
