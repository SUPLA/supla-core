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

#ifndef GOOGLE_HOME_SYNC_SEARCH_CONDITION_H_
#define GOOGLE_HOME_SYNC_SEARCH_CONDITION_H_

#include "asynctask/abstract_asynctask_search_condition.h"

class supla_google_home_sync_search_condition
    : public supla_abstract_asynctask_search_condition {
 private:
  int user_id;

 public:
  explicit supla_google_home_sync_search_condition(int user_id);

  virtual bool condition_met(supla_abstract_asynctask *task);
};

#endif /*GOOGLE_HOME_SYNC_SEARCH_CONDITION_H_*/
