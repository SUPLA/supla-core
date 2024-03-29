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

#ifndef PUSH_NOTIFICATION_RECIPIENT_DAO_H_
#define PUSH_NOTIFICATION_RECIPIENT_DAO_H_

#include <mysql.h>

#include <vector>

#include "db/abstract_db_access_provider.h"
#include "push/pn_recipient.h"
#include "push/pn_recipients.h"

class supla_pn_recipient_dao {
 private:
  supla_abstract_db_access_provider *dba;
  void get_recipients(const char *sql, MYSQL_BIND *pbind, int pbind_size,
                      supla_pn_recipients *recipients);

 public:
  explicit supla_pn_recipient_dao(supla_abstract_db_access_provider *dba);
  virtual ~supla_pn_recipient_dao(void);
  void remove(int user_id, supla_pn_recipient *recipient);
  void get_recipients(int user_id, int push_notification_id,
                      supla_pn_recipients *recipients);
  void get_recipients(int user_id, const std::vector<int> &aids,
                      const std::vector<int> &cids,
                      supla_pn_recipients *recipients);
};

#endif /* PUSH_NOTIFICATION_RECIPIENT_DAO_H_ */
