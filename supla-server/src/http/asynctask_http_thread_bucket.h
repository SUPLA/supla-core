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

#ifndef ASYNCTASK_HTTP_THREAD_BUCKET_H_
#define ASYNCTASK_HTTP_THREAD_BUCKET_H_

#include "asynctask/asynctask_thread_bucket.h"
#include "http/abstract_curl_adapter.h"

class supla_asynctask_http_thread_bucket
    : public supla_asynctask_thread_bucket {
 private:
  supla_abstract_curl_adapter *curl_adapter;

 public:
  supla_asynctask_http_thread_bucket(supla_abstract_curl_adapter *curl_adapter);
  supla_abstract_curl_adapter *get_adapter(void);
  virtual ~supla_asynctask_http_thread_bucket(void);
};

#endif /*ASYNCTASK_HTTP_THREAD_BUCKET_H_*/
