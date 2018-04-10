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
 protected:
  void _load(database *db, e_objc_scope scope);
  bool get_data_for_remote(supla_client_objcontainer_item *obj, void **data,
                           bool full, bool EOL, bool *check_more);
  void send_data_to_remote_and_free(void *srpc, void *data, bool full);

 public:
  explicit supla_client_channelgroups(supla_client *client);
  void load(void);
  virtual bool add(supla_client_objcontainer_item *obj, e_objc_scope scope);
};

#endif /* CLIENTCHANNELGROUPS_H_ */
