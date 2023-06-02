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

#include <stdlib.h>

supla_multipart_call::supla_multipart_call(unsigned int call_id,
                                           int expire_in_msec) {
  this->call_id = call_id;

  struct timeval now = {};
  gettimeofday(&now, nullptr);

  expires_at = now.tv_sec * 1000000 + now.tv_usec + expire_in_msec * 1000;
}

supla_multipart_call::~supla_multipart_call() {
  for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
    delete[] it->data;
  }
}

bool supla_multipart_call::is_expired(void) {
  struct timeval now = {};
  gettimeofday(&now, nullptr);

  unsigned long long now_usec = now.tv_sec * 1000000 + now.tv_usec;
  return now_usec >= expires_at;
}

unsigned int supla_multipart_call::get_call_id(void) { return call_id; }

void supla_multipart_call::part_push(char *data, size_t data_size) {
  if (data && data_size) {
    part_t p = {};
    p.data = new char[data_size];
    memcpy(p.data, data, data_size);
    p.data_size = data_size;
    parts.push_back(p);
  }
}

char *supla_multipart_call::part_pop(size_t *data_size) {
  char *result = nullptr;

  if (parts.size()) {
    part_t part = parts.front();
    if (data_size) {
      *data_size = part.data_size;
    }
    result = part.data;
    parts.erase(parts.begin());
  }

  return result;
}
