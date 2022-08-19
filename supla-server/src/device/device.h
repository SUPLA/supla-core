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

#ifndef DEVICE_H_
#define DEVICE_H_

#include <conn/abstract_connection_object.h>

#include <list>
#include <memory>

#include "commontypes.h"
#include "devicechannel.h"

class supla_user;
class supla_device : public supla_abstract_connection_object {
 private:
  int flags;

 protected:
  supla_device_channels *channels;

  void load_config(int UserID);
  static char channels_clean_cnd(void *channel);
  bool db_authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                       const char Email[SUPLA_EMAIL_MAXSIZE],
                       const char AuthKey[SUPLA_AUTHKEY_SIZE], int *UserID,
                       database *db);

 public:
  explicit supla_device(supla_connection *connection);
  virtual ~supla_device();

  std::shared_ptr<supla_device> get_shared_ptr(void);
  virtual supla_abstract_srpc_call_handler_collection *
  get_srpc_call_handler_collection(void);
  static bool funclist_contains_function(int funcList, int func);

  char register_device(TDS_SuplaRegisterDevice_C *register_device_c,
                       TDS_SuplaRegisterDevice_E *register_device_e,
                       unsigned char proto_version);

  supla_device_channels *get_channels(void);

  void on_device_channel_value_changed(TDS_SuplaDeviceChannelValue *value,
                                       TDS_SuplaDeviceChannelValue_B *value_b,
                                       TDS_SuplaDeviceChannelValue_C *value_c);
  void on_device_channel_extendedvalue_changed(
      TDS_SuplaDeviceChannelExtendedValue *ev);
  void on_channel_set_value_result(TDS_SuplaChannelNewValueResult *result);
  void on_channel_state_result(TDSC_ChannelState *state);
  void get_firmware_update_url(TDS_FirmwareUpdateParams *params);
  void on_calcfg_result(TDS_DeviceCalCfgResult *result);
  bool enter_cfg_mode(void);
};

#endif /* DEVICE_H_ */
