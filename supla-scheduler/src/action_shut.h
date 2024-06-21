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

#ifndef ACTION_SHUT_H_
#define ACTION_SHUT_H_

#include "abstract_action_shut_partially.h"

class s_worker_action_shut : public s_abstract_action_shut_partially {
 protected:
  virtual bool do_action(void);

 public:
  explicit s_worker_action_shut(s_abstract_worker *worker);
  virtual ~s_worker_action_shut(void);
  virtual bool get_expected(char *percentage, bool *percentage_as_delta,
                            char *tilt, bool *tilt_as_delta);
};

#endif /*ACTION_SHUT_H_*/
