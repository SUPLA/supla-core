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

#include "AsyncTaskThreadPoolMock.h"

AsyncTaskThreadPoolMock::AsyncTaskThreadPoolMock(supla_asynctask_queue *queue)
    : supla_abstract_asynctask_thread_pool(queue) {
  limit = 1;
}

AsyncTaskThreadPoolMock::~AsyncTaskThreadPoolMock(void) {}

unsigned int AsyncTaskThreadPoolMock::thread_count_limit(void) { return limit; }

std::string AsyncTaskThreadPoolMock::pool_name(void) {
  return std::string("TestPool");
}

void AsyncTaskThreadPoolMock::set_thread_count_limit(unsigned int limit) {
  this->limit = limit;
}
