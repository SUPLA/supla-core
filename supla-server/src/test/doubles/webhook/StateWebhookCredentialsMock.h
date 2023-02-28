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

#ifndef STATE_WEBHOOK_CREDENTIALS_MOCK_H_
#define STATE_WEBHOOK_CREDENTIALS_MOCK_H_

#include <gmock/gmock.h>

#include "webhook/state_webhook_credentials2.h"

namespace testing {

class StateWebhookCredentialsMock : public supla_state_webhook_credentials2 {
 private:
 protected:
 public:
  StateWebhookCredentialsMock();
  StateWebhookCredentialsMock(supla_user *user);
  virtual ~StateWebhookCredentialsMock(void);

  MOCK_METHOD0(refresh_lock, void(void));
  MOCK_METHOD0(refresh_unlock, void(void));
  MOCK_METHOD0(get_user_short_unique_id, std::string(void));
  MOCK_METHOD0(get_url, std::string(void));
  MOCK_METHOD0(get_access_token, std::string(void));
  MOCK_METHOD0(get_refresh_token, std::string(void));
  MOCK_METHOD0(is_access_token_exists, bool(void));
  MOCK_METHOD0(is_refresh_token_exists, bool(void));
  MOCK_METHOD0(expires_in, int(void));
  MOCK_METHOD0(get_set_time, struct timeval(void));
  MOCK_METHOD3(update, void(const std::string access_token,
                            const std::string refresh_token, int expires_in));
  MOCK_METHOD0(remove, void(void));
  MOCK_METHOD0(get_function_ids, std::vector<int>(void));
};

} /* namespace testing */

#endif /* STATE_WEBHOOK_CREDENTIALS_MOCK_H_ */
