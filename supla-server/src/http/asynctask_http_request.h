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

#ifndef HTTP_ASYNCTASK_HTTP_REQUEST_H_
#define HTTP_ASYNCTASK_HTTP_REQUEST_H_

#include "asynctask/abstract_asynctask.h"
#include "caller.h"
#include "commontypes.h"
#include "http/abstract_curl_adapter.h"
#include "user/user.h"

class supla_asynctask_http_request : public supla_abstract_asynctask {
 private:
  supla_caller caller;
  supla_user *user;
  int device_id;
  int channel_id;
  event_type et;

 protected:
  const supla_caller &get_caller(void);
  supla_user *get_user(void);
  int get_device_id(void);
  int get_channel_id(void);
  event_type get_event_type(void);
  virtual bool _execute(bool *execute_again,
                        supla_asynctask_thread_bucket *bucket);
  virtual bool make_request(supla_abstract_curl_adapter *curl_adapter) = 0;

 public:
  supla_asynctask_http_request(const supla_caller &caller, supla_user *user,
                               int device_id, int channel_id, event_type et,
                               supla_asynctask_queue *queue,
                               supla_abstract_asynctask_thread_pool *pool);

  virtual ~supla_asynctask_http_request(void);
};

#endif /* HTTP_ASYNCTASK_HTTP_REQUEST_H_ */
