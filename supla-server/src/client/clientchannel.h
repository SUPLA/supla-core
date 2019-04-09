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

#include "clientchannels.h"
#include "clientobjcontaineritem.h"
#include "proto.h"

class supla_client;
class supla_client_channels;
class supla_client_channel : public supla_client_objcontainer_item {
 private:
  int DeviceId;
  int LocationId;
  int Type;
  int Func;
  int Param1;
  int Param2;
  int Param3;
  char *TextParam1;
  char *TextParam2;
  char *TextParam3;
  int AltIcon;
  int UserIcon;
  short ManufacturerID;
  short ProductID;
  unsigned char ProtocolVersion;
  unsigned int Flags;

  void get_cost_and_currency(char currency[3], _supla_int_t *total_cost,
                             _supla_int_t *price_per_unit, double count);
  _supla_int64_t get_calculated_value(_supla_int_t impulses_per_unit,
                                      unsigned _supla_int64_t counter);

  void proto_get_value(TSuplaChannelValue *value, char *online,
                       supla_client *client);

 public:
  supla_client_channel(supla_client_channels *Container, int Id, int DeviceId,
                       int LocationID, int Type, int Func, int Param1,
                       int Param2, int Param3, char *TextParam1,
                       char *TextParam2, char *TextParam3, const char *Caption,
                       int AltIcon, int UserIcon, short ManufacturerID,
                       short ProductID, unsigned char ProtocolVersion);
  virtual ~supla_client_channel(void);
  void mark_for_remote_update(int mark);
  bool remote_update_is_possible(void);
  void proto_get(TSC_SuplaChannel *channel, supla_client *client);
  void proto_get(TSC_SuplaChannel_B *channel, supla_client *client);
  void proto_get(TSC_SuplaChannel_C *channel, supla_client *client);
  void proto_get(TSC_SuplaChannelValue *channel_value, supla_client *client);
  bool proto_get(TSC_SuplaChannelExtendedValue *cev, supla_client *client);
  int getDeviceId();
  int getExtraId();
};

#endif /* CLIENTCHANNEL_H_ */
