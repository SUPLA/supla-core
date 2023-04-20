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

#ifndef FCM_CLIENT_H_
#define FCM_CLIENT_H_

#include <string>

#include "push/abstract_pn_gateway_client.h"

class supla_fcm_client : public supla_abstract_pn_gateway_client {
 protected:
  virtual _platform_e get_platform(void);
  char *get_payload(supla_pn_recipient *recipient);
  std::string get_message_id(const std::string &request_result);
  virtual bool _send(supla_pn_gateway_access_token *token,
                     supla_pn_recipient *recipient);

 public:
  supla_fcm_client(supla_abstract_curl_adapter *curl_adapter,
                   supla_pn_gateway_access_token_provider *token_provider,
                   supla_push_notification *push);
  virtual ~supla_fcm_client(void);
};

#endif /* FCM_CLIENT_H_ */
