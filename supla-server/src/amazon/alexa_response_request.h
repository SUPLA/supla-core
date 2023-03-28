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

#include "amazon/alexa_credentials2.h"
#include "amazon/alexa_request2.h"

class supla_alexa_response_request2 : public supla_alexa_request2 {
 private:
  std::string correlation_token;

 protected:
  virtual bool make_request(supla_abstract_curl_adapter *curl_adapter);
  virtual std::string get_name(void);

 public:
  supla_alexa_response_request2(
      const supla_caller &caller, int user_id, int device_id, int channel_id,
      supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
      supla_abstract_channel_property_getter *property_getter,
      supla_amazon_alexa_credentials2 *credentials,
      const std::string &correlation_token);

  static bool is_caller_allowed(const supla_caller &caller);
  static bool is_function_allowed(int func);

  static void new_request(const supla_caller &caller, supla_user *user,
                          int device_id, int channel_id);
};

#endif /* ALEXA_RESPONSE_REQUEST_H_ */
