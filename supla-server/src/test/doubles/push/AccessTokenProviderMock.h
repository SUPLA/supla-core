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

#ifndef ACCESS_TOKEN_PROVIDER_MOCK_H_
#define ACCESS_TOKEN_PROVIDER_MOCK_H_

#include <gmock/gmock.h>

#include "push/abstract_access_token_provider.h"

namespace testing {

class AccessTokenProviderMock : public supla_abstract_access_token_provider {
 public:
  AccessTokenProviderMock(void);
  virtual ~AccessTokenProviderMock();

  MOCK_METHOD0(min_secs_between_refresh_attempts, int(void));
  MOCK_METHOD0(refresh_time_margin_secs, int(void));
  MOCK_METHOD0(service_tick_time_usec, long long(void));

  MOCK_METHOD3(new_token, bool(supla_abstract_curl_adapter **curl_adapter,
                               std::string *token, int *expires_in_secs));
  MOCK_METHOD0(get_url, std::string(void));
  MOCK_METHOD0(get_platform, _platform_e(void));
  MOCK_METHOD0(get_app_id, _app_id_e(void));
};

} /* namespace testing */

#endif /* ACCESS_TOKEN_PROVIDER_MOCK_H_ */
