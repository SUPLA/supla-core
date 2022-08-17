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

#include "test/doubles/conn/ConnectionObjectsMock.h"

#include "log.h"

ConnectionObjectsMock::ConnectionObjectsMock() : supla_connection_objects() {}

ConnectionObjectsMock::~ConnectionObjectsMock() {}

void ConnectionObjectsMock::for_each(
    std::function<bool(std::shared_ptr<supla_connection_object> obj)>
        on_object) {
  supla_connection_objects::for_each(on_object);
}

std::vector<std::shared_ptr<supla_connection_object> >
ConnectionObjectsMock::get_all(void) {
  return supla_connection_objects::get_all();
}

bool ConnectionObjectsMock::add(std::shared_ptr<supla_connection_object> obj) {
  return supla_connection_objects::add(obj);
}

std::shared_ptr<supla_connection_object> ConnectionObjectsMock::get(int id) {
  return supla_connection_objects::get(id);
}
