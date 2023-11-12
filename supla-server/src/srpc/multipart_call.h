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

#ifndef MULTIPART_CALL_H_
#define MULTIPART_CALL_H_

#include <string.h>

#include <vector>

#include "proto.h"

class supla_multipart_call {
 private:
  unsigned int call_id;
  unsigned long long expires_at;

  typedef struct {
    char *data;
    size_t data_size;
  } part_t;

  std::vector<part_t> parts;

 public:
  supla_multipart_call(unsigned int call_id, int expire_in_msec);
  virtual ~supla_multipart_call();

  bool is_expired(void);
  unsigned int get_call_id(void);
  void part_push(char *data, size_t data_size);
  char *part_pop(size_t *data_size);
};

#endif /* MULTIPART_CALL_STORE_H_ */
