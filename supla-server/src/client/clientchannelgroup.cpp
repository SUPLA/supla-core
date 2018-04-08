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

#include "clientchannelgroup.h"

supla_client_channelgroup::supla_client_channelgroup(int Id, int LocationID,
                                                     int Func,
                                                     const char *Caption,
                                                     int AltIcon)
    : supla_client_objcontainer_item(Id, Caption) {
  this->LocationID = LocationID;
  this->Func = Func;
  this->AltIcon = AltIcon;
  this->Flags = 0;
}

supla_client_channelgroup::supla_client_channelgroup(
    supla_client_channelgroup *cg)
    : supla_client_objcontainer_item(cg) {
  update(cg);
}

void supla_client_channelgroup::update(supla_client_channelgroup *cg) {
  supla_client_objcontainer_item::update(cg);

  this->LocationID = cg->LocationID;
  this->Func = cg->Func;
  this->AltIcon = cg->AltIcon;
  this->Flags = cg->Flags;
}

bool supla_client_channelgroup::remote_update_is_possible(void) { return true; }
