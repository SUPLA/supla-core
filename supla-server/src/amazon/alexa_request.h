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

#ifndef ALEXA_REQUEST_H_
#define ALEXA_REQUEST_H_

#include <string>

#include "amazon/alexa_credentials.h"
#include "http/asynctask_http_request.h"

class supla_alexa_request : public supla_asynctask_http_request {
 private:
  supla_amazon_alexa_credentials *credentials;
  std::string zulu_time;
  std::string message_id;

 protected:
  supla_amazon_alexa_credentials *get_credentials(void);

 public:
  supla_alexa_request(const supla_caller &caller, int user_id, int device_id,
                      int channel_id, supla_asynctask_queue *queue,
                      supla_abstract_asynctask_thread_pool *pool,
                      supla_abstract_channel_property_getter *property_getter,
                      supla_amazon_alexa_credentials *credentials);

  std::string get_zulu_time(void);
  void set_zulu_time(const std::string &zulu_time);
  std::string get_message_id(void);
  void set_message_id(const std::string &message_id);
};

#endif /* ALEXA_REQUEST_H_ */
