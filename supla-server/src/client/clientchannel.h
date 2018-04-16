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

#ifndef CLIENTCHANNEL_H_
#define CLIENTCHANNEL_H_

#include "proto.h"
#include "clientchannels.h"
#include "clientobjcontaineritem.h"

class supla_client;
class supla_client_channels;
class supla_client_channel : public supla_client_objcontainer_item {
 private:
  int DeviceId;
  int LocationId;
  int Func;
  int Param1;
  int Param2;
  int AltIcon;
  unsigned char ProtocolVersion;
  unsigned int Flags;

 public:
  supla_client_channel(supla_client_channels *Container, int Id, int DeviceId,
                       int LocationID, int Func, int Param1, int Param2,
                       const char *Caption, int AltIcon,
                       unsigned char ProtocolVersion);

  bool remote_update_is_possible(void);
  void proto_get(TSC_SuplaChannel *channel, supla_client *client);
  void proto_get(TSC_SuplaChannel_B *channel, supla_client *client);
  void proto_get(TSC_SuplaChannelValue *channel_value, supla_client *client);
  int getDeviceId();
  int getExtraId();
};

#endif /* CLIENTCHANNEL_H_ */
