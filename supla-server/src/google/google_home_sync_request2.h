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

#ifndef GOOGLE_HOME_SYNC_REQUEST2_H_
#define GOOGLE_HOME_SYNC_REQUEST2_H_

#include <string>

#include "google/google_home_credentials2.h"
#include "http/asynctask_http_request.h"

class supla_google_home_sync_request2 : public supla_asynctask_http_request {
 private:
  supla_google_home_credentials2 *credentials;

 protected:
  virtual bool make_request(supla_abstract_curl_adapter *curl_adapter);
  virtual std::string get_name(void);

 public:
  supla_google_home_sync_request2(const supla_caller &caller, int user_id,
                                  event_type et, supla_asynctask_queue *queue,
                                  supla_abstract_asynctask_thread_pool *pool,
                                  supla_google_home_credentials2 *credentials);

  static bool is_event_type_allowed(event_type et);
  static void new_request(const supla_caller &caller, supla_user *user,
                          event_type et);
};

#endif /* GOOGLE_HOME_SYNC_REQUEST2_H_ */
