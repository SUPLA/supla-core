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

#include <stdlib.h>
#include <string.h>

#include "clientlocation.h"
#include "../database.h"
#include "../lck.h"
#include "../log.h"
#include "../safearray.h"
#include "../srpc.h"

supla_client_location::supla_client_location(int Id, const char *Caption) {
  this->Id = Id;
  this->Caption = NULL;

  if (Caption) {
    this->Caption = strdup(Caption);
  }
}

supla_client_location::~supla_client_location() {
  if (Caption) {
    free(Caption);
  }
}

void supla_client_location::proto_get_location(TSC_SuplaLocation *location) {
  memset(location, 0, sizeof(TSC_SuplaLocation));
  location->Id = Id;

  if (Caption) {
    snprintf(location->Caption, SUPLA_LOCATION_CAPTION_MAXSIZE, "%s", Caption);
    location->CaptionSize =
        strnlen(location->Caption, SUPLA_LOCATION_CAPTION_MAXSIZE - 1) + 1;
  } else {
    location->CaptionSize = 1;
    location->Caption[0] = 0;
  }
}

int supla_client_location::getId(void) { return Id; }
// ------------------------------------------------------------

char supla_client_locations::arr_findcmp(void *ptr, void *id) {
  return ((supla_client_location *)ptr)->getId() == *((int *)id) ? 1
                                                                 : 0;
}

char supla_client_locations::arr_delcnd(void *ptr) {
  delete (supla_client_location *)ptr;
  return 1;
}

supla_client_locations::supla_client_locations() {
  this->arr = safe_array_init();

  this->lck = lck_init();
  ids = NULL;
  ids_count = 0;
}

supla_client_locations::~supla_client_locations() {
  ids_clean();
  arr_clean();
  safe_array_free(arr);
  lck_free(this->lck);
}

void supla_client_locations::ids_clean(void) {
  lck_lock(lck);

  if (ids) free(ids);

  ids_count = 0;

  lck_unlock(lck);
}

void supla_client_locations::arr_clean(void) {
  safe_array_lock(arr);
  safe_array_clean(arr, arr_delcnd);
  safe_array_unlock(arr);
}

int supla_client_locations::count() {
  int result = 0;

  lck_lock(lck);
  result = ids_count;
  lck_unlock(lck);

  return result;
}

void supla_client_locations::add_location(int Id, const char *Caption) {
  safe_array_lock(arr);

  if (safe_array_findcnd(arr, arr_findcmp, &Id) == 0) {
    supla_client_location *l = new supla_client_location(Id, Caption);

    if (l != NULL && safe_array_add(arr, l) == -1) {
      delete l;
      l = NULL;
    }
  }

  safe_array_unlock(arr);
}

void supla_client_locations::load(int ClientID) {
  database *db = new database();
  int a, n;

  if (db->connect() == true) {
    safe_array_lock(arr);
    arr_clean();

    db->get_client_locations(ClientID, this);

    lck_lock(lck);
    ids_clean();

    n = safe_array_count(arr);

    if (n > 0) {
      ids = (int *)malloc(sizeof(int) * n);
    }

    if (ids) {
      for (a = 0; a < n; a++) {
        supla_client_location *loc =
            (supla_client_location *)safe_array_get(arr, a);
        if (loc != NULL) {
          ids[ids_count] = loc->getId();
          ids_count++;
        }
      }
    }

    lck_unlock(lck);

    safe_array_unlock(arr);
  }

  delete db;
}

bool supla_client_locations::remote_update(void *srpc) {
  TSC_SuplaLocationPack location_pack;
  memset(&location_pack, 0, sizeof(TSC_SuplaLocationPack));

  safe_array_lock(arr);

  supla_client_location *loc = NULL;

  do {
    loc = (supla_client_location *)safe_array_pop(arr);

    if (loc && location_pack.count < SUPLA_LOCATIONPACK_MAXCOUNT) {
      loc->proto_get_location(&location_pack.items[location_pack.count]);
      location_pack.items[location_pack.count].EOL = 0;
      location_pack.count++;

      delete loc;

      if (location_pack.count >= SUPLA_LOCATIONPACK_MAXCOUNT) {
        loc = NULL;
      }

    } else {
      loc = NULL;
    }
  } while (loc != NULL);

  safe_array_unlock(arr);

  if (location_pack.count > 0) {
    location_pack.items[location_pack.count - 1].EOL = 1;

    srpc_sc_async_locationpack_update(srpc, &location_pack);
    return true;
  }

  return false;
}

bool supla_client_locations::location_exists(int Id) {
  bool result = false;

  lck_lock(lck);
  for (int a = 0; a < ids_count; a++)
    if (ids[a] == Id) {
      result = true;
      break;
    }
  lck_unlock(lck);

  return result;
}
