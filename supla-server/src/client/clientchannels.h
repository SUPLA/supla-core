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

#include <functional>

#include "clientchannel.h"
#include "clientobjcontainer.h"
#include "device.h"

class supla_client_channel;
class supla_client_channels : public supla_client_objcontainer {
 private:
  supla_client_channel *find_channel(int Id);
  supla_client_channel *any_channel_with_deviceid(int DeviceId);
  template <typename TSuplaDataPack, class TObjClass>
  bool get_datapack_for_remote(supla_client_objcontainer_item *obj, void **data,
                               int max_count);
  bool get_ev_datapack_for_remote(supla_client_objcontainer_item *obj,
                                  void **data);

  template <typename TSuplaDataPack>
  void set_pack_eol(void *data);

 protected:
  bool get_data_for_remote(supla_client_objcontainer_item *obj, void **data,
                           int data_type, bool *check_more, e_objc_scope scope);
  void send_data_to_remote_and_free(void *srpc, void *data, int data_type,
                                    e_objc_scope scope);
  int available_data_types_for_remote(e_objc_scope scope);
  void _load(database *db, e_objc_scope scope);

 public:
  explicit supla_client_channels(supla_client *client);
  void update_device_channels(int DeviceId);
  void on_channel_value_changed(
      int DeviceId, int ChannelId = 0,
      bool Extended = false);  // ChannelId == 0 - All channels
  bool channel_exists(int ChannelID);

  void set_channel_function(int ChannelId, int Func);
  void set_channel_caption(int ChannelId, char *Caption);
  unsigned _supla_int64_t value_validity_time_usec(void);
  void update_expired(void);

  void channel_access(int channel_id,
                      std::function<void(supla_client_channel *)> on_channel);
  void device_access(int channel_id,
                     std::function<void(supla_device *)> method);
  int get_device_id(int channel_id);
  int get_any_channel_id_with_deviceid(int device_id);
  int get_channel_id(unsigned char channel_number);

  void for_each(std::function<void(supla_client_channel *, bool *will_continue)>
                    on_channel);
};

#endif /* CLIENTCHANNELS_H_ */
