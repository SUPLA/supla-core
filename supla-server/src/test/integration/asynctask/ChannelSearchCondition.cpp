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

#include "ChannelSearchCondition.h"
#include "ChannelOrientedAsyncTaskMock.h"

namespace testing {

ChannelSearchCondition::ChannelSearchCondition(void) { this->any_id = false; }

ChannelSearchCondition::~ChannelSearchCondition() {}

void ChannelSearchCondition::set_channels(std::vector<int> channels) {
  this->channels = channels;
}

void ChannelSearchCondition::set_any_id(bool any_id) { this->any_id = any_id; }

bool ChannelSearchCondition::condition_met(supla_abstract_asynctask *task) {
  ChannelOrientedAsyncTaskMock *ctask =
      dynamic_cast<ChannelOrientedAsyncTaskMock *>(task);
  if (ctask) {
    if (any_id) {
      return true;
    }

    for (std::vector<int>::iterator it = channels.begin(); it != channels.end();
         ++it) {
      if (*it == ctask->get_channel_id()) {
        return true;
      }
    }
  }

  return false;
}

}  // namespace testing
