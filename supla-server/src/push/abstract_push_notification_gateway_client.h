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

#ifndef ABSTRACT_PUSH_NOTIFICATION_GATEWAY_CLIENT_H_
#define ABSTRACT_PUSH_NOTIFICATION_GATEWAY_CLIENT_H_

#include <string>

#include "http/abstract_curl_adapter.h"
#include "push/access_token_providers.h"
#include "push/push_notification.h"

class supla_abstract_push_notification_gateway_client {
 private:
  supla_access_token_providers *token_providers;
  supla_push_notification *push;
  supla_abstract_curl_adapter *curl_adapter;

 protected:
  supla_push_notification *get_push_notification(void);
  supla_abstract_curl_adapter *get_curl_adapter(void);

  virtual _platform_e get_platform(void) = 0;
  virtual bool _send(const std::string &url, const std::string &token,
                     supla_pn_recipient *recipient) = 0;

 public:
  supla_abstract_push_notification_gateway_client(
      supla_abstract_curl_adapter *curl_adapter,
      supla_access_token_providers *token_providers,
      supla_push_notification *push);
  virtual ~supla_abstract_push_notification_gateway_client(void);
  bool send(void);
};

#endif /* ABSTRACT_PUSH_NOTIFICATION_GATEWAY_CLIENT_H_ */
