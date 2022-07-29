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

#ifndef ABSTRACT_ASYNCTASK_OBSERVER_H_
#define ABSTRACT_ASYNCTASK_OBSERVER_H_

#include "asynctask/abstract_asynctask.h"

class supla_asynctask_queue;
class supla_abstract_asynctask_observer {
 private:
 protected:
  friend class supla_asynctask_queue;
  virtual void on_asynctask_started(supla_abstract_asynctask *asynctask) = 0;
  virtual void on_asynctask_finished(supla_abstract_asynctask *asynctask) = 0;

 public:
  supla_abstract_asynctask_observer(void);
  virtual ~supla_abstract_asynctask_observer(void);
};

#endif /*ABSTRACT_ASYNCTASK_OBSERVER_H_*/
