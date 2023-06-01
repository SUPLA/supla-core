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

#ifndef ALEXA_CREDENTIALS_MOCK_H_
#define ALEXA_CREDENTIALS_MOCK_H_

#include <gmock/gmock.h>

#include <string>

#include "amazon/alexa_credentials.h"

namespace testing {

class AlexaCredentialsMock : public supla_amazon_alexa_credentials {
 private:
 protected:
 public:
  AlexaCredentialsMock();
  explicit AlexaCredentialsMock(supla_user *user);
  virtual ~AlexaCredentialsMock(void);

  MOCK_METHOD0(refresh_lock, void(void));
  MOCK_METHOD0(refresh_unlock, void(void));
  MOCK_METHOD0(get_user_short_unique_id, std::string(void));
  MOCK_METHOD0(get_access_token, std::string(void));
  MOCK_METHOD0(get_refresh_token, std::string(void));
  MOCK_METHOD0(is_access_token_exists, bool(void));
  MOCK_METHOD0(is_refresh_token_exists, bool(void));
  MOCK_METHOD0(expires_in, int(void));
  MOCK_METHOD0(get_set_time, struct timeval(void));
  MOCK_METHOD3(update, void(const std::string &access_token,
                            const std::string &refresh_token, int expires_in));
  MOCK_METHOD0(remove, void(void));
  MOCK_METHOD0(get_region, std::string(void));
};

} /* namespace testing */

#endif /* STATE_WEBHOOK_CREDENTIALS_MOCK_H_ */
