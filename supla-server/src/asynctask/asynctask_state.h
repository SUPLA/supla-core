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

#ifndef ASYNCTASK_STATE_H_
#define ASYNCTASK_STATE_H_

#include "abstract_asynctask.h"

class supla_asynctask_state {
 public:
  enum _state_e {
    INIT,
    WAITING,
    PICKED,
    EXECUTING,
    SUCCESS,
    FAILURE,
    TIMEOUT,
    CANCELED,
  };

 private:
  _state_e state;

 public:
  supla_asynctask_state(void);
  virtual ~supla_asynctask_state(void);
  void set_state(_state_e state);
  _state_e get_state(void) const;

  supla_asynctask_state &operator=(const _state_e state);
  operator _state_e();
  bool operator==(const _state_e state) const;
  bool operator!=(const _state_e state) const;
  virtual bool operator==(const supla_asynctask_state &state) const;
  virtual bool operator!=(const supla_asynctask_state &state) const;
};

#endif /*ASYNCTASK_STATE_H_*/
