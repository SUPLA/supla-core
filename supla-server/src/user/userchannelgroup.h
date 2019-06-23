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

#ifndef USERCHANNELGROUP_H_
#define USERCHANNELGROUP_H_

#include "objcontaineritem.h"
#include "userchannelgroups.h"

class supla_user_channelgroups;
class supla_user_channelgroup : public supla_objcontainer_item {
 private:
  int ChannelId;
  int DeviceId;

 public:
  explicit supla_user_channelgroup(supla_user_channelgroups *Container,
                                   int GroupId, int ChannelId, int DeviceId);

  bool remote_update_is_possible(void);
  int getGroupId();
  int getChannelId();
  int getDeviceId();
  int getExtraId();
};

#endif /* USERCHANNELGROUP_H_ */
