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

#ifndef CLIENTCHANNELGROUP_H_
#define CLIENTCHANNELGROUP_H_

#include "clientchannelgrouprelation.h"
#include "clientobjcontaineritem.h"

class supla_client;
class supla_client_channelgroups;
class supla_client_channelgroup : public supla_client_objcontainer_item {
 private:
  int LocationID;
  int Func;
  int AltIcon;
  unsigned int Flags;
  void *relarr;  // This is just index. Don't free related objects

 protected:
 public:
  supla_client_channelgroup(supla_client_channelgroups *Container, int Id,
                            int LocationID, int Func, const char *Caption,
                            int AltIcon);

  virtual ~supla_client_channelgroup(void);
  bool add_relation(supla_client_channelgroup_relation *cg_rel);
  bool remote_update_is_possible(void);
};

#endif /* CLIENTCHANNELGROUP_H_ */
