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
  supla_client_channel *find_channel(int Id);

 protected:
  bool get_data_for_remote(supla_client_objcontainer_item *obj, void **data,
                           bool full, bool EOL, bool *check_more);
  void send_data_to_remote_and_free(void *srpc, void *data, bool full);

  void _load(database *db, e_objc_scope scope);
 public:
  explicit supla_client_channels(supla_client *client);
  void update_device_channels(int DeviceId);
  void on_channel_value_changed(
      void *srpc, int DeviceId,
      int ChannelId = 0);  // ChannelId == 0 - All channels
  bool channel_exists(int ChannelID);

  bool set_device_channel_new_value(
      TCS_SuplaChannelNewValue_B *channel_new_value);
};

#endif /* CLIENTCHANNELS_H_ */
