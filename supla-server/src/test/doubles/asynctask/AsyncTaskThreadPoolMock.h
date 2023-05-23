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

#ifndef ASYNCTASK_THREAD_POOL_MOCK_H_
#define ASYNCTASK_THREAD_POOL_MOCK_H_

#include <gmock/gmock.h>

#include "abstract_asynctask_thread_pool.h"

class AsyncTaskThreadPoolMock : public supla_abstract_asynctask_thread_pool {
 public:
  explicit AsyncTaskThreadPoolMock(supla_asynctask_queue *queue);
  virtual ~AsyncTaskThreadPoolMock(void);

  MOCK_METHOD0(thread_count_limit, unsigned int(void));
  MOCK_METHOD0(pool_name, std::string(void));
  MOCK_METHOD0(get_bucket, supla_asynctask_thread_bucket *(void));
};

#endif /*ASYNCTASK_THREAD_POOL_MOCK_H_*/
