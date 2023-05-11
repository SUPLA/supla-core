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

#ifndef PUSH_NOTIFICATION_DAO_H_
#define PUSH_NOTIFICATION_DAO_H_

#include "db/abstract_db_access_provider.h"
#include "push/push_notification.h"

class supla_pn_dao {
 private:
  supla_abstract_db_access_provider *dba;

 public:
  explicit supla_pn_dao(supla_abstract_db_access_provider *dba);
  virtual ~supla_pn_dao(void);
  bool get(int user_id, supla_push_notification *push);

  int get_limit(int user_id);  // I have doubts whether geting limits of sent
                               // messages should be in pn_dao or user_dao.
  int get_device_managed_push_id(int user_id, int device_id, int channel_id);
  void register_device_managed_push(int user_id, int device_id,
                                         int channel_id, bool sm_title,
                                         bool sm_body, bool sm_sound);
};

#endif /* PUSH_NOTIFICATION_DAO_H_ */
