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

#ifndef CLIENTLOCATION_H_
#define CLIENTLOCATION_H_

#include "proto.h"

class supla_client_location {
 private:
  int Id;
  char *Caption;

 public:
  supla_client_location(int Id, const char *Caption);
  virtual ~supla_client_location();
  int getId(void);
  void proto_get_location(TSC_SuplaLocation *location);
};

class supla_client_locations {
 private:
  void *arr;
  // -------------

  void *lck;
  int *ids;
  int ids_count;

  void ids_clean();
  void arr_clean(void);
  static char arr_findcmp(void *ptr, void *id);
  static char arr_delcnd(void *ptr);

 public:
  supla_client_locations();
  virtual ~supla_client_locations();
  void load(int ClientID);
  int count();
  void add_location(int Id, const char *Caption);
  bool remote_update(void *srpc);
  bool location_exists(int Id);
};

#endif /* CLIENTLOCATION_H_ */
