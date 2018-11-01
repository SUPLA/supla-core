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

#include <list>
#include "../cdcommon.h"
#include "devicechannel.h"

class supla_user;
class supla_device : public cdcommon {
 protected:
  supla_device_channels *channels;

  void load_config(void);
  static char channels_clean_cnd(void *channel);

 public:
  explicit supla_device(serverconnection *svrconn);
  char register_device(TDS_SuplaRegisterDevice_C *register_device_c,
                       TDS_SuplaRegisterDevice_E *register_device_e,
                       unsigned char proto_version);
  virtual ~supla_device();

  bool get_channel_value(int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE]);
  bool get_channel_extendedvalue(int ChannelID,
                                 TSuplaChannelExtendedValue *value);
  void set_device_channel_value(int SenderID, int ChannelID,
                                const char value[SUPLA_CHANNELVALUE_SIZE]);
  bool set_device_channel_char_value(int SenderID, int ChannelID,
                                     const char value);
  bool set_device_channel_rgbw_value(int SenderID, int ChannelID, int color,
                                     char color_brightness, char brightness);

  bool channel_exists(int ChannelID);
  void on_device_channel_value_changed(TDS_SuplaDeviceChannelValue *value);
  void on_device_channel_extendedvalue_changed(
      TDS_SuplaDeviceChannelExtendedValue *ev);
  void on_channel_set_value_result(TDS_SuplaChannelNewValueResult *result);
  std::list<int> master_channel(int ChannelID);
  std::list<int> slave_channel(int ChannelID);
  bool get_channel_double_value(int ChannelID, double *Value);
  bool get_channel_temperature_value(int ChannelID, double *Value);
  bool get_channel_humidity_value(int ChannelID, double *Value);
  void get_temp_and_humidity(void *tarr);
  void get_electricity_measurement(void *emarr);
  bool get_channel_char_value(int ChannelID, char *Value);
  bool get_channel_rgbw_value(int ChannelID, int *color, char *color_brightness,
                              char *brightness);
  void get_firmware_update_url(TDS_FirmwareUpdateParams *params);
  bool calcfg_request(int SenderID, bool SuperUserAuthorized,
                           TCS_DeviceCalCfgRequest *request);
  void on_calcfg_result(TDS_DeviceCalCfgResult *result);
};

#endif /* DEVICE_H_ */
