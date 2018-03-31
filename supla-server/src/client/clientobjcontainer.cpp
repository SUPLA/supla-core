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

#include "client.h"
#include "clientobjcontainer.h"
#include "../safearray.h"

// static
char supla_client_objcontainer::arr_delcnd(void *ptr) {
  delete (supla_client_location *)ptr;
  return 1;
}

void supla_client_objcontainer::arr_clean(void) {
  safe_array_lock(arr);
  safe_array_clean(arr, arr_delcnd);
  safe_array_unlock(arr);
}

supla_client_objcontainer::supla_client_objcontainer(supla_client *client) {
  this->client = client;
  this->arr = safe_array_init();
}

supla_client_objcontainer::~supla_client_objcontainer() {}

int supla_client_objcontainer::count(void) { return safe_array_count(arr); }
