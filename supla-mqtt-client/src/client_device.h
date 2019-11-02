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

#ifndef CLIENT_DEVICE_H
#define CLIENT_DEVICE_H

#include <string.h>
#include <vector>
#include "client_state.h"
#include "supla-client-lib/devicechannel.h"
#include "supla-client-lib/safearray.h"

class client_device_channel : public supla_device_channel {
 private:
  std::vector<client_state *> states;
  char *Caption;

 public:
  client_device_channel(int Id, int Number, int Type, int Func, int Param1,
                        int Param2, int Param3, char *TextParam1,
                        char *TextParam2, char *TextParam3, bool Hidden,
                        char *Caption, std::vector<client_state *> &States);
  ~client_device_channel();
  char *getCaption(void);
  const std::vector<client_state *> getStates(void) const;
};

class client_device_channels : public supla_device_channels {
 private:
 public:
  void add_channel(int Id, int Number, int Type, int Func, int Param1,
                   int Param2, int Param3, char *TextParam1, char *TextParam2,
                   char *TextParam3, bool Hidden, char *Caption,
                   std::vector<client_state *> &States);
  client_device_channel *find_channel(int ChannelId);
};

#endif
