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

#ifndef CLIENT_CLIENTCHANNELGROUPVALUE_H_
#define CLIENT_CLIENTCHANNELGROUPVALUE_H_

#include "clientobjcontaineritem.h"

// This class is used to send the channel value when the channel is hidden on
// the main list

class supla_client;
class supla_client_channelgroups;
class supla_client_channelgroup_value : public supla_client_objcontainer_item {
 private:
  int DeviceId;

 protected:
 public:
  explicit supla_client_channelgroup_value(
      supla_client_channelgroups *Container, int ChannelId, int DeviceId);
  bool remote_update_is_possible(void);
  int getChannelId(void);
  int getDeviceId(void);
  virtual int getExtraId();
  void proto_get(TSC_SuplaChannelValue *channel_value);
};

#endif /* CLIENT_CLIENTCHANNELGROUPVALUE_H_ */
