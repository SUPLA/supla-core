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

#include "multipart_call.h"

supla_multipart_call::supla_multipart_call(unsigned int call_id,
                                           int expire_in_msec) {
  this->call_id = call_id;
  std::chrono::milliseconds mills(expire_in_msec);
  expires_at = std::chrono::steady_clock::now() + mills;
}

supla_multipart_call::~supla_multipart_call() {
  for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
    delete[] it->data;
  }
}

bool supla_multipart_call::is_expired(void) {
  return std::chrono::steady_clock::now() <= expires_at;
}

unsigned int supla_multipart_call::get_call_id(void) { return get_call_id(); }

void supla_multipart_call::part_push(char *data, size_t data_size) {
  parts.push_back({.data = data, .data_size = data_size});
}

void *supla_multipart_call::part_pop(size_t *data_size) {
  void *result = nullptr;

  if (parts.size()) {
    part_t part = parts.back();
    if (data_size) {
      *data_size = part.data_size;
    }
    result = part.data;
    parts.pop_back();
  }

  return result;
}
