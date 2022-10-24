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

#ifndef ABSTRACT_AUTO_GATE_CLOSING_DAO_H_
#define ABSTRACT_AUTO_GATE_CLOSING_DAO_H_

#include <vector>

#include "db/abstract_db_access_provider.h"

#define ATTEMPT_RETRY_TIME_SEC 900

class supla_abstract_auto_gate_closing_dao {
 private:
  supla_abstract_db_access_provider *dba;

 protected:
  supla_abstract_db_access_provider *get_dba(void);

 public:
  typedef struct {
    int user_id;
    int device_id;
    int channel_id;
    int seconds_since_last_attempt;
    bool seen_open;
  } item_t;

  explicit supla_abstract_auto_gate_closing_dao(
      supla_abstract_db_access_provider *dba);
  virtual ~supla_abstract_auto_gate_closing_dao();
  virtual std::vector<item_t> get_all_active(void) = 0;
  virtual int mark_gate_open(int channel_id) = 0;
  virtual void mark_gate_closed(int channel_id) = 0;
  virtual void set_closing_attemtp(int channel_id) = 0;
};

#endif /* ABSTRACT_AUTO_GATE_CLOSING_DAO_H_ */
