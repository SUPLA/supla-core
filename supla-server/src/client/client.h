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

#ifndef CLIENT_H_
#define CLIENT_H_

#include "cdcommon.h"
#include "clientchannelgroups.h"
#include "clientchannels.h"
#include "clientlocation.h"

class supla_user;
class supla_client : public cdcommon {
 private:
  char name[SUPLA_CLIENT_NAME_MAXSIZE];

 protected:
  supla_client_locations *locations;
  supla_client_channels *channels;
  supla_client_channelgroups *cgroups;

  void loadIODevices(void);
  void loadConfig(void);

  void remote_update_lists(void);
  void setName(const char *name);

 public:
  explicit supla_client(serverconnection *svrconn);

  char register_client(TCS_SuplaRegisterClient_B *register_client_b,
                       TCS_SuplaRegisterClient_C *register_client_c,
                       unsigned char proto_version);
  void update_device_channels(int LocationID, int DeviceID);
  void on_channel_value_changed(int DeviceId, int ChannelId = 0,
                                bool Extended = false);
  void get_next(void);
  void set_device_channel_new_value(
      TCS_SuplaChannelNewValue_B *channel_new_value);
  void set_new_value(TCS_SuplaNewValue *new_value);
  int getName(char *buffer, int size);

  void call_event(TSC_SuplaEvent *event);

  virtual ~supla_client();
};

#endif /* CLIENT_H_ */
