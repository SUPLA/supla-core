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

#ifndef AMAZON_ALEXA_DISCOVER_PAYLOAD_OBTAINER_H_
#define AMAZON_ALEXA_DISCOVER_PAYLOAD_OBTAINER_H_

#include <string>

#include "amazon/alexa_credentials.h"
#include "amazon/alexa_credentials_dao.h"
#include "http/abstract_curl_adapter.h"
#include "json/cJSON.h"

class supla_alexa_discover_payload_obtainer {
 private:
  supla_amazon_alexa_credentials_dao *dao;
  virtual std::string get_message_id(void);
  cJSON *json_get_object(cJSON *root, const char name[]);
  void json_set_string(cJSON *root, const char name[], const char value[]);

 public:
  supla_alexa_discover_payload_obtainer(
      supla_amazon_alexa_credentials_dao *dao);
  virtual ~supla_alexa_discover_payload_obtainer(void);

  std::string obtain(int user_id, supla_amazon_alexa_credentials *credentials,
                     supla_abstract_curl_adapter *curl_adapter);
};

#endif /* AMAZON_ALEXA_DISCOVER_PAYLOAD_OBTAINER_H_ */
