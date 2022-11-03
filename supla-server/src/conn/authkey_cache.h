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

#ifndef SUPLA_AUTHKEYCACHE_H_
#define SUPLA_AUTHKEYCACHE_H_

#include <functional>

#include "proto.h"

class supla_authkey_cache {
 private:
  static supla_authkey_cache global_instance;
  void* items;

 protected:
  int cache_size_limit;

 public:
  supla_authkey_cache(void);
  ~supla_authkey_cache(void);

  static supla_authkey_cache& get_global_instance(void);

  bool authkey_auth(const char guid[SUPLA_GUID_SIZE],
                    const char email[SUPLA_EMAIL_MAXSIZE],
                    const char authkey[SUPLA_AUTHKEY_SIZE], int* user_id,
                    std::function<bool(void)> db_authkey_auth);
  int get_cache_size_limit(void);
  int get_size(void);
};

#endif /* SUPLA_AUTHKEYCACHE_H_ */
