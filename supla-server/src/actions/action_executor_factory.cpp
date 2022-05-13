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

#include "actions/action_executor_factory.h"

#include "actions/action_executor.h"

supla_action_executor_factory* supla_action_executor_factory::_global_instance =
    NULL;

// static
supla_action_executor_factory* supla_action_executor_factory::global_instance(
    void) {
  if (!_global_instance) {
    _global_instance = new supla_action_executor_factory();
  }
  return _global_instance;
}

// static
void supla_action_executor_factory::global_instance_release(void) {
  if (_global_instance) {
    delete _global_instance;
    _global_instance = NULL;
  }
}

supla_action_executor_factory::supla_action_executor_factory(void)
    : supla_abstract_action_executor_factory() {}

supla_action_executor_factory::~supla_action_executor_factory(void) {}

supla_abstract_action_executor*
supla_action_executor_factory::get_action_executor(void) {
  return new supla_action_executor();
}
