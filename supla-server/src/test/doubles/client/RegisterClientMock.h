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

#ifndef REGISTER_CLIENT_MOCK_H_
#define REGISTER_CLIENT_MOCK_H_

#include <gmock/gmock.h>

#include "client/call_handler/abstract_register_client.h"

namespace testing {

class RegisterClientMock : public supla_ch_abstract_register_client {
 public:
  RegisterClientMock(void);
  virtual ~RegisterClientMock(void);

  char register_client(TCS_SuplaRegisterClient_B *register_client_b,
                       TCS_SuplaRegisterClient_D *register_client_d,
                       supla_abstract_srpc_adapter *srpc_adapter,
                       supla_abstract_db_access_provider *dba,
                       supla_abstract_connection_dao *conn_dao,
                       supla_abstract_client_dao *client_dao, int client_sd,
                       int client_ipv4, unsigned char activity_timeout);

  MOCK_METHOD0(get_authkey_cache, supla_authkey_cache *(void));

  MOCK_METHOD1(get_user_id_by_email,
               int(const char email[SUPLA_EMAIL_MAXSIZE]));

  MOCK_METHOD3(get_object_id,
               bool(int user_id, const char guid[SUPLA_GUID_SIZE], int *id));

  MOCK_METHOD3(get_authkey_hash,
               bool(int id, char authkey_hash[BCRYPT_HASH_MAXSIZE],
                    bool *is_null));

  MOCK_METHOD3(superuser_authorize,
               void(int user_id, const char email[SUPLA_EMAIL_MAXSIZE],
                    const char password[SUPLA_PASSWORD_MAXSIZE]));

  MOCK_METHOD0(is_superuser_authorized, bool(void));

  MOCK_METHOD0(revoke_superuser_authorization, void(void));

  MOCK_METHOD0(on_registraction_success, void(void));

  MOCK_METHOD0(get_location_count, int(void));

  MOCK_METHOD0(get_channel_count, int(void));

  MOCK_METHOD0(get_channel_group_count, int(void));

  MOCK_METHOD0(get_scene_count, int(void));

  MOCK_METHOD0(remote_update_lists, void(void));

  void set_hold_time_on_failure_usec(__useconds_t usec);
};

} /* namespace testing */

#endif /* REGISTER_CLIENT_MOCK_H_ */
