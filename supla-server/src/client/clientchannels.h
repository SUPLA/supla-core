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

#ifndef CLIENTCHANNELS_H_
#define CLIENTCHANNELS_H_

#include "clientchannel.h"
#include "clientobjcontainer.h"

class supla_client_channel;
class supla_client_channels : public supla_client_objcontainer {
 private:
  supla_client *client;
  void *arr;

  static char arr_findcmp(void *ptr, void *id);

  supla_client_channel *get_marked(void);
  supla_client_channel *find_channel(int Id);
  bool remote_update_cv(void *srpc);
  bool remote_update_c(void *srpc);
  bool remote_update_c_b(void *srpc);

 public:
  explicit supla_client_channels(supla_client *client);
  void update_channel(int Id, int DeviceId, int LocationID, int Func,
                      int Param1, int Param2, const char *Caption, int AltIcon,
                      unsigned char ProtocolVersion);
  void update_device_channels(int DeviceId);
  void on_channel_value_changed(
      void *srpc, int DeviceId,
      int ChannelId = 0);  // ChannelId == 0 - All channels
  bool channel_exists(int ChannelID);
  void load(void);
  bool remote_update(void *srpc);

  bool set_device_channel_new_value(
      TCS_SuplaChannelNewValue_B *channel_new_value);
};

#endif /* CLIENTCHANNELS_H_ */
