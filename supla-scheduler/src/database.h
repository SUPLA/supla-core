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

#ifndef DATABASE_H_
#define DATABASE_H_

#include "db.h"
#include "supla-scheduler.h"

class database : public dbcommon {
 private:
  virtual char *cfg_get_host(void);
  virtual char *cfg_get_user(void);
  virtual char *cfg_get_password(void);
  virtual char *cfg_get_database(void);
  virtual int cfg_get_port(void);

 public:
  void get_s_executions(void *s_exec_arr, int limit);
  bool get_channel(supla_channel *channel);

  void set_expired_result(int expired_time);
  void set_zombie_result(int zombie_time);
  bool set_result(int id, int result);

  bool set_fetched(int id);
  bool set_retry(int id, int sec);
  void set_unfetched(int id);
};

#endif /* DATABASE_H_ */
