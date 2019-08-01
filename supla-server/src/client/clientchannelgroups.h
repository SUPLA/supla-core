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

#ifndef CLIENTCHANNELGROUPS_H_
#define CLIENTCHANNELGROUPS_H_

#include "clientchannelgroup.h"
#include "clientchannelgrouprelation.h"
#include "clientchannelgroupvalue.h"
#include "clientobjcontainer.h"

class supla_client_channelgroups : public supla_client_objcontainer {
 private:
  bool channelRelationExists(int ChannelId);
  bool get_data_for_remote(supla_client_channelgroup *cg, void **data);
  template <typename TSuplaDataPack, class TObjClass>
  bool get_datapack_for_remote(supla_client_objcontainer_item *obj, void **data,
                               int max_count);
  template <typename TSuplaDataPack>
  void set_pack_eol(void *data);

 protected:
  void _load(database *db, e_objc_scope scope);
  bool get_data_for_remote(supla_client_objcontainer_item *obj, void **data,
                           int data_type, bool *check_more, e_objc_scope scope);
  void send_data_to_remote_and_free(void *srpc, void *data, int data_type,
                                    e_objc_scope scope);
  int available_data_types_for_remote(e_objc_scope scope);

 public:
  explicit supla_client_channelgroups(supla_client *client);
  void load(void);
  virtual bool add(supla_client_objcontainer_item *obj, e_objc_scope scope);
  supla_client_channelgroup *findGroup(int Id);
  void on_channel_value_changed(void *srpc, int DeviceId, int ChannelId);
  bool set_device_channel_new_value(TCS_SuplaNewValue *new_value);
  bool device_calcfg_request(TCS_DeviceCalCfgRequest_B *request);
};

#endif /* CLIENTCHANNELGROUPS_H_ */
