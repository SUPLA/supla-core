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

#ifndef CLIENT_DAO_MOCK_H_
#define CLIENT_DAO_MOCK_H_

#include <gmock/gmock.h>

#include "client/abstract_client_dao.h"

namespace testing {

class ClientDaoMock : public supla_abstract_client_dao {
 public:
  ClientDaoMock(void);
  virtual ~ClientDaoMock(void);

  MOCK_METHOD5(access_id_auth,
               bool(int access_id, char *access_id_pwd, int *user_id,
                    bool *is_enabled, bool *is_active));

  MOCK_METHOD4(oauth_get_token,
               bool(TSC_OAuthToken *token, int user_id, int access_id,
                    bool *storage_connect_error));

  MOCK_METHOD3(oauth_get_token, bool(int user_id, int device_reg_time_sec,
                                     int client_reg_time_sec));

  MOCK_METHOD3(set_reg_enabled, bool(int user_id, int device_reg_time_sec,
                                     int client_reg_time_sec));

  MOCK_METHOD3(get_client_id,
               bool(int user_id, const char guid[SUPLA_GUID_SIZE], int *id));

  MOCK_METHOD2(get_client_id,
               int(int user_id, const char guid[SUPLA_GUID_SIZE]));

  MOCK_METHOD3(get_client_access_id, int(int client_id, bool *accessid_enabled,
                                         bool *accessid_active));

  MOCK_METHOD5(get_client_variables,
               bool(int client_id, bool *client_enabled, int *access_id,
                    bool *accessid_enabled, bool *accessid_active));

  MOCK_METHOD1(get_client_reg_enabled, bool(int user_id));

  MOCK_METHOD1(get_client_limit_left, int(int user_id));

  MOCK_METHOD1(get_client_count, int(int user_id));

  MOCK_METHOD3(get_access_id, int(int user_id, bool enabled, bool active));

  MOCK_METHOD8(add_client,
               int(int access_id, const char *guid, const char *authkey,
                   const char *name, unsigned int ipv4, const char *softver,
                   int proto_version, int user_id));

  MOCK_METHOD1(on_newclient, bool(int client_id));

  MOCK_METHOD7(update_client,
               bool(int client_id, int access_id, const char *authkey,
                    const char *name, unsigned int ipv4, const char *softver,
                    int proto_version));
};

} /* namespace testing */

#endif /* CLIENT_DAO_MOCK_H_ */
