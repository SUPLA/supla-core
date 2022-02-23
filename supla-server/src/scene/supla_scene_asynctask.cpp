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

#include "supla_scene_asynctask.h"

supla_scene_asynctask::supla_scene_asynctask(
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    supla_abstract_action_executor *action_executor,
    supla_scene_operations *operations)
    : supla_abstract_asynctask(queue, pool) {
  this->action_executor = action_executor;
  this->operations = operations;
}

supla_scene_asynctask::~supla_scene_asynctask() {
  if (action_executor) {
    delete action_executor;
    action_executor = NULL;
  }

  if (operations) {
    delete operations;
    operations = NULL;
  }
}
