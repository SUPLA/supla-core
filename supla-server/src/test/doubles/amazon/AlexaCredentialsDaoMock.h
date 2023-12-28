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

#ifndef ALEXA_CREDENTIALS_DAO_MOCK_H_
#define ALEXA_CREDENTIALS_DAO_MOCK_H_

#include <gmock/gmock.h>

#include <string>

#include "amazon/alexa_credentials_dao.h"

namespace testing {

class AlexaCredentialsDaoMock : public supla_amazon_alexa_credentials_dao {
 private:
 protected:
 public:
  explicit AlexaCredentialsDaoMock(supla_abstract_db_access_provider *dba);
  AlexaCredentialsDaoMock(supla_abstract_db_access_provider *dba,
                          bool release_dba);

  virtual ~AlexaCredentialsDaoMock(void);

  MOCK_METHOD5(get, bool(int user_id, std::string *access_token,
                         std::string *refresh_token, int *expires_in,
                         std::string *region));
  MOCK_METHOD4(set, void(int user_id, const std::string &access_token,
                         const std::string &refresh_token, int expires_in));
  MOCK_METHOD1(remove, void(int user_id));
  MOCK_METHOD1(get_cloud_access_token, std::string(int user_id));
};

} /* namespace testing */

#endif /* ALEXA_CREDENTIALS_DAO_MOCK_H_ */
