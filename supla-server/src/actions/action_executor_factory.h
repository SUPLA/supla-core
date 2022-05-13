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

#ifndef ACTION_EXECUTOR_FACTORY_H_
#define ACTION_EXECUTOR_FACTORY_H_

#include "actions/abstract_action_executor_factory.h"

class supla_action_executor_factory : supla_abstract_action_executor_factory {
 private:
  static supla_action_executor_factory *_global_instance;

 public:
  static supla_action_executor_factory *global_instance(void);
  static void global_instance_release(void);

  supla_action_executor_factory(void);
  virtual ~supla_action_executor_factory(void);
  virtual supla_abstract_action_executor *get_action_executor(void);
};

#endif /*ABSTRACT_ACTION_EXECUTOR_FACTORY_H_*/
