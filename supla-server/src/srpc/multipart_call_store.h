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

#ifndef MULTIPART_CALL_STORE_H_
#define MULTIPART_CALL_STORE_H_

#include <vector>

#include "srpc/multipart_call.h"

class supla_multipart_call_store {
 private:
  std::vector<supla_multipart_call *> calls;

 public:
  supla_multipart_call_store(void);
  virtual ~supla_multipart_call_store();

  void release(unsigned int call_id);
  supla_multipart_call *get(unsigned int call_id);
  void add(supla_multipart_call *call);
};

#endif /* MULTIPART_CALL_STORE_H_ */
