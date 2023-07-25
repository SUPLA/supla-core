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

#include "client/client_channel_relations.h"

#include <vector>

#include "client/clientchannel.h"
#include "device/channel_relation.h"

using std::vector;

supla_client_channel_reactions::supla_client_channel_reactions(
    supla_abstract_dobject_remote_updater *updater,
    supla_client_channels *channels)
    : supla_dobjects(updater) {
  vector<supla_channel_relation> relations;

  channels->for_each(
      [&](supla_client_channel *channel, bool *will_continue) -> void {
        channel->get_channel_relations(&relations, relation_any);
      });

  for (auto it = relations.begin(); it != relations.end(); ++it) {
    add(new supla_channel_relation(&(*it), true));
  }
}

supla_client_channel_reactions::~supla_client_channel_reactions() {}
