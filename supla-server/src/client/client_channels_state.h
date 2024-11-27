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

#ifndef SUPLA_CLIENT_CHANNELS_STATE_H_
#define SUPLA_CLIENT_CHANNELS_STATE_H_

#include "client/abstract_client_dao.h"
#include "distributedobjects/dobjects.h"
#include "proto.h"

class supla_client_channels;
class supla_client_channels_state : public supla_dobjects {
 private:
  supla_abstract_client_dao *dao;

 public:
  explicit supla_client_channels_state(
      supla_abstract_dobject_remote_updater *updater,
      supla_abstract_client_dao *dao);
  virtual ~supla_client_channels_state();
  void load(int client_protocol_version, int user_id, int client_id);
  virtual bool update_remote(void);
};

#endif /* SUPLA_CLIENT_CHANNELS_STATE_H_ */
