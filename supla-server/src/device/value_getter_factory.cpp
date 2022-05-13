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

#include "value_getter_factory.h"

#include <string.h>

#include "value_getter.h"

supla_value_getter_factory *supla_value_getter_factory::_global_instance = NULL;

// static
supla_value_getter_factory *supla_value_getter_factory::global_instance(void) {
  if (!_global_instance) {
    _global_instance = new supla_value_getter_factory();
  }
  return _global_instance;
}

// static
void supla_value_getter_factory::global_instance_release(void) {
  if (_global_instance) {
    delete _global_instance;
    _global_instance = NULL;
  }
}

supla_value_getter_factory::supla_value_getter_factory()
    : supla_abstract_value_getter_factory() {}

supla_value_getter_factory::~supla_value_getter_factory() {}

supla_abstract_value_getter *supla_value_getter_factory::get_value_getter(
    void) {
  return new supla_value_getter();
}
