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

#include <memory>

#include "clientchannels.h"
#include "clientobjcontaineritem.h"
#include "device/abstract_common_channel_properties.h"
#include "proto.h"

class supla_client;
class supla_client_channels;
class supla_device;
class supla_client_channel : public supla_client_objcontainer_item,
                             public supla_abstract_common_channel_properties {
 private:
  unsigned char channel_number;
  int DeviceId;
  int LocationId;
  int Type;
  int Func;
  int Param1;
  int Param2;
  int Param3;
  int Param4;
  char *TextParam1;
  char *TextParam2;
  char *TextParam3;
  int AltIcon;
  int UserIcon;
  short ManufacturerID;
  short ProductID;
  unsigned char ProtocolVersion;
  unsigned int Flags;
  channel_json_config *json_config;

  // during offline
  char value[SUPLA_CHANNELVALUE_SIZE];
  struct timeval value_valid_to;
  // --------------

  void get_cost_and_currency(char currency[3], _supla_int_t *total_cost,
                             _supla_int_t *price_per_unit, double count);
  _supla_int64_t get_calculated_value(_supla_int_t impulses_per_unit,
                                      unsigned _supla_int64_t counter);

  void proto_get_value(TSuplaChannelValue_B *value, char *online,
                       supla_client *client);
  void proto_get_value(TSuplaChannelValue *value, char *online,
                       supla_client *client);
  bool get_cs_extended_value(std::shared_ptr<supla_device> device,
                             int channel_id,
                             TSC_SuplaChannelExtendedValue *cev);
  virtual void for_each(
      std::function<void(supla_abstract_common_channel_properties *, bool *)>
          on_channel_properties);

 public:
  supla_client_channel(supla_client_channels *Container, int Id,
                       unsigned char channel_number, int DeviceId,
                       int LocationID, int Type, int Func, int Param1,
                       int Param2, int Param3, int Param4, char *TextParam1,
                       char *TextParam2, char *TextParam3, const char *Caption,
                       int AltIcon, int UserIcon, short ManufacturerID,
                       short ProductID, unsigned char ProtocolVersion,
                       unsigned int Flags, unsigned int EmSubcFlags,
                       const char value[SUPLA_CHANNELVALUE_SIZE],
                       unsigned _supla_int_t validity_time_sec,
                       const char *user_config,
                       const char *em_subc_user_config);
  virtual ~supla_client_channel(void);
  virtual unsigned char get_channel_number(void);
  void mark_for_remote_update(int mark);
  bool remote_update_is_possible(void);
  void proto_get(TSC_SuplaChannel *channel, supla_client *client);
  void proto_get(TSC_SuplaChannel_B *channel, supla_client *client);
  void proto_get(TSC_SuplaChannel_C *channel, supla_client *client);
  void proto_get(TSC_SuplaChannel_D *channel, supla_client *client);
  void proto_get(TSC_SuplaChannelValue *channel_value, supla_client *client);
  void proto_get(TSC_SuplaChannelValue_B *channel_value, supla_client *client);
  bool proto_get(TSC_SuplaChannelExtendedValue *cev, supla_client *client);
  bool get_basic_cfg(TSC_ChannelBasicCfg *basic_cfg);
  virtual int get_type();
  virtual int get_func();
  virtual int get_param1();
  virtual int get_param2();
  virtual int get_param3();
  virtual int get_param4();
  virtual channel_json_config *get_json_config(void);
  virtual void set_json_config(channel_json_config *json_config);
  void set_func(int Func);
  void setCaption(const char *Caption);
  virtual int get_user_id();
  virtual int get_device_id();
  short get_manufacturer_id();
  short get_product_id();
  virtual unsigned int get_flags();
  virtual int get_id();
  virtual int get_extra_id();

  void setValueValidityTimeSec(unsigned _supla_int_t validity_time_sec);
  bool isValueValidityTimeSet();
  unsigned _supla_int64_t getValueValidityTimeUSec(void);
  void resetValueValidityTime(void);
  void get_config(TSCS_ChannelConfig *config, unsigned char config_type,
                  unsigned _supla_int_t flags);
};

#endif /* CLIENTCHANNEL_H_ */
