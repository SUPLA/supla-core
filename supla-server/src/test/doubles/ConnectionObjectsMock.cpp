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

#include "test/doubles/ConnectionObjectsMock.h"

#include "log.h"

// static
char ConnectionObjectsMock::find_by_ptr(void *ptr1, void *ptr2) {
  return ptr1 == ptr2 ? 1 : 0;
}

ConnectionObjectsMock::ConnectionObjectsMock() : supla_connection_objects() {
  del_count = 0;
}

void ConnectionObjectsMock::cd_delete(supla_connection_object *cd) {
  delete cd;
  del_count++;
}

ConnectionObjectsMock::~ConnectionObjectsMock() {}

int ConnectionObjectsMock::delCount(void) { return del_count; }

supla_connection_object *ConnectionObjectsMock::findByPtr(void *ptr) {
  return find(find_by_ptr, ptr);
}
