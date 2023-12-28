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

#ifndef ALEXA_DELETE_REQUEST_H_
#define ALEXA_DELETE_REQUEST_H_

#include <string>
#include <vector>

#include "amazon/alexa_credentials.h"
#include "amazon/alexa_request.h"

class supla_alexa_delete_request : public supla_alexa_request {
 private:
  std::vector<int> scene_ids;
  std::vector<int> channel_ids;

 protected:
  virtual bool make_request(supla_abstract_curl_adapter *curl_adapter);
  virtual std::string get_name(void);
  static void new_request(supla_user *user, std::vector<int> ids, bool scene);

 public:
  supla_alexa_delete_request(int user_id, supla_asynctask_queue *queue,
                             supla_abstract_asynctask_thread_pool *pool,
                             supla_amazon_alexa_credentials *credentials);

  void add(std::vector<int> ids, bool scene);

  static bool is_function_allowed(int func);

  static void new_request(supla_user *user, int channel_id, int func);
  static void new_request(supla_user *user, int device_or_scene_id, bool scene);
};

#endif /* ALEXA_DELETE_REQUEST_H_ */
