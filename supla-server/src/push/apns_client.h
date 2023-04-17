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

#ifndef APNS_CLIENT_H_
#define APNS_CLIENT_H_

#include "http/abstract_curl_adapter.h"
#include "push/abstract_access_token_provider.h"
#include "push/push_notification.h"

class supla_apns_client {
 private:
  supla_abstract_access_token_provider *token_provider;
  supla_push_notification *push;
  supla_abstract_curl_adapter *curl_adapter;

 public:
  supla_apns_client(supla_abstract_curl_adapter *curl_adapter,
                    supla_abstract_access_token_provider *token_provider,
                    supla_push_notification *push);
  virtual ~supla_apns_client(void);
  bool send(void);
};

#endif /* APNS_CLIENT_H_ */
