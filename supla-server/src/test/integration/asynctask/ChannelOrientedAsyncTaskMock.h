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

#ifndef CHANNEL_ORIENTED_ASYNCTASK_MOCK_H_
#define CHANNEL_ORIENTED_ASYNCTASK_MOCK_H_

#include "asynctask/AsyncTaskMock.h"

class ChannelOrientedAsyncTaskMock : public AsyncTaskMock {
 private:
  int channel_id;

 protected:
  virtual bool _execute(bool *execute_again);

 public:
  ChannelOrientedAsyncTaskMock(supla_asynctask_queue *queue,
                               supla_abstract_asynctask_thread_pool *pool,
                               short priority, bool release_immediately);
  ChannelOrientedAsyncTaskMock(supla_asynctask_queue *queue,
                               supla_abstract_asynctask_thread_pool *pool);

  void set_channel_id(int channel_id);
  int get_channel_id(void);
};

#endif /*CHANNEL_ORIENTED_ASYNCTASK_MOCK_H_*/
