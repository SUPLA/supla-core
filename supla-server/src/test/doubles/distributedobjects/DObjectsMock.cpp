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

#include "doubles/distributedobjects/DObjectsMock.h"

namespace testing {

using std::function;

DObjectsMock::DObjectsMock(supla_abstract_dobject_remote_updater *updater)
    : supla_dobjects(updater) {}

DObjectsMock::~DObjectsMock(void) {}

void DObjectsMock::add(supla_dobject *object) { supla_dobjects::add(object); }

void DObjectsMock::access_object(
    int id, function<void(supla_dobject *object)> on_access) {
  supla_dobjects::access_object(id, on_access);
}

}  // namespace testing
