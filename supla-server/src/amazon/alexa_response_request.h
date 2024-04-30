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

#ifndef ALEXA_RESPONSE_REQUEST_H_
#define ALEXA_RESPONSE_REQUEST_H_

#include <string>

#include "amazon/alexa_correlation_token.h"
#include "amazon/alexa_credentials.h"
#include "amazon/alexa_request.h"
#include "device/value/channel_value.h"

class supla_alexa_response_request : public supla_alexa_request {
 private:
  supla_alexa_correlation_token *correlation_token;

 protected:
  virtual bool make_request(supla_abstract_curl_adapter *curl_adapter);
  virtual std::string get_name(void);

 public:
  supla_alexa_response_request(
      const supla_caller &caller, int user_id, int device_id, int channel_id,
      supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
      supla_abstract_channel_property_getter *property_getter,
      supla_amazon_alexa_credentials *credentials,
      supla_alexa_correlation_token *correlation_token);

  ~supla_alexa_response_request(void);

  static bool is_caller_allowed(const supla_caller &caller);
  static bool is_function_allowed(int func);

  static void new_request(const supla_caller &caller, supla_user *user,
                          int device_id, int channel_id,
                          supla_alexa_correlation_token *correlation_token);
};

#endif /* ALEXA_RESPONSE_REQUEST_H_ */
