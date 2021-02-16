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

#include "ChannelOrientedAsyncTaskMock.h"

ChannelOrientedAsyncTaskMock::ChannelOrientedAsyncTaskMock(
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    short priority, bool release_immediately)
    : AsyncTaskMock(queue, pool, priority, release_immediately) {}

ChannelOrientedAsyncTaskMock::ChannelOrientedAsyncTaskMock(
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool)
    : AsyncTaskMock(queue, pool) {}

bool ChannelOrientedAsyncTaskMock::_execute(bool *execute_again) {
  return true;
}

void ChannelOrientedAsyncTaskMock::set_channel_id(int channel_id) {
  lock();
  this->channel_id = channel_id;
  unlock();
}

int ChannelOrientedAsyncTaskMock::get_channel_id(void) {
  lock();
  int result = channel_id;
  unlock();

  return result;
}
