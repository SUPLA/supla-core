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
  bool set_device_channel_new_value(int ChannelId, char *value);

 public:
  explicit supla_client_channels(supla_client *client);
  void update_device_channels(int DeviceId);
  void on_channel_value_changed(
      void *srpc, int DeviceId, int ChannelId = 0,
      bool Extended = false);  // ChannelId == 0 - All channels
  bool channel_exists(int ChannelID);

  bool set_device_channel_new_value(
      TCS_SuplaChannelNewValue_B *channel_new_value);
  bool set_device_channel_new_value(TCS_SuplaNewValue *new_value);
  bool device_calcfg_request(TCS_DeviceCalCfgRequest_B *request);
  bool device_get_channel_state(TCSD_ChannelStateRequest *request);
  void get_channel_basic_cfg(void *srpc, TCS_ChannelBasicCfgRequest *request);
  void set_channel_function(int ChannelId, int Func);
  void set_channel_caption(int ChannelId, char *Caption);
  unsigned _supla_int64_t value_validity_time_usec(void);
  void update_expired(void *srpc);
};

#endif /* CLIENTCHANNELS_H_ */
